#include <stdint.h>

#include "gd32vw55x.h"
#include "ring_buffer.h"
#include "systimer.h"

#define LED_GPIO_PORT              GPIOC
#define LED_GPIO_PIN               GPIO_PIN_13
#define LED_GPIO_CLOCK             RCU_GPIOC

#define BALANCED_DURATION_MS       6000U
#define OVERLOAD_DURATION_MS       6000U
#define DRAIN_DURATION_MS          6000U
#define BALANCED_PRODUCER_MS       400U
#define BALANCED_CONSUMER_MS       400U
#define BALANCED_CONSUMER_START_MS 600U
#define OVERLOAD_PRODUCER_MS       100U
#define OVERLOAD_CONSUMER_MS       500U
#define DRAIN_PRODUCER_MS          800U
#define DRAIN_CONSUMER_MS          100U
#define LED_UPDATE_PERIOD_MS       50U
#define FULL_BLINK_HALF_PERIOD_MS  100U

typedef enum {
    PHASE_BALANCED = 0,
    PHASE_OVERLOAD,
    PHASE_DRAIN
} experiment_phase_t;

ring_buffer_t g_ring_buffer;
volatile experiment_phase_t g_phase = PHASE_BALANCED;
volatile uint32_t g_phase_changes = 0U;
volatile uint32_t g_producer_attempts = 0U;
volatile uint32_t g_consumer_attempts = 0U;
volatile uint32_t g_last_produced_sequence = 0U;
volatile uint32_t g_last_consumed_sequence = 0U;
volatile uint32_t g_last_latency_ms = 0U;
volatile uint32_t g_max_latency_ms = 0U;
volatile uint32_t g_sequence_errors = 0U;
volatile uint32_t g_background_iterations = 0U;
volatile uint32_t g_led_toggle_count = 0U;
volatile uint8_t g_led_is_on = 0U;

static uint32_t next_sequence = 1U;
static uint32_t next_producer_ms = 0U;
static uint32_t next_consumer_ms = 0U;
static uint32_t next_led_update_ms = 0U;
static uint32_t next_full_blink_ms = 0U;
static uint32_t phase_started_ms = 0U;
static uint32_t producer_period_ms = BALANCED_PRODUCER_MS;
static uint32_t consumer_period_ms = BALANCED_CONSUMER_MS;

static void led_init(void)
{
    rcu_periph_clock_enable(LED_GPIO_CLOCK);
    gpio_mode_set(LED_GPIO_PORT, GPIO_MODE_OUTPUT,
                  GPIO_PUPD_NONE, LED_GPIO_PIN);
    gpio_output_options_set(LED_GPIO_PORT, GPIO_OTYPE_PP,
                            GPIO_OSPEED_10MHZ, LED_GPIO_PIN);
    gpio_bit_reset(LED_GPIO_PORT, LED_GPIO_PIN);
}

static void led_set(uint8_t turn_on)
{
    uint8_t requested_state = (turn_on != 0U) ? 1U : 0U;

    if (requested_state == g_led_is_on) {
        return;
    }

    /* En la placa probada PC13 en alto enciende el LED. */
    if (requested_state != 0U) {
        gpio_bit_set(LED_GPIO_PORT, LED_GPIO_PIN);
    } else {
        gpio_bit_reset(LED_GPIO_PORT, LED_GPIO_PIN);
    }

    g_led_is_on = requested_state;
    g_led_toggle_count++;
}

static void led_toggle(void)
{
    led_set((uint8_t)(g_led_is_on == 0U));
}

static uint8_t time_reached(uint32_t now_ms, uint32_t deadline_ms)
{
    return ((int32_t)(now_ms - deadline_ms) >= 0) ? 1U : 0U;
}

static void producer_run(uint32_t now_ms)
{
    ring_item_t item;

    g_producer_attempts++;
    item.sequence = next_sequence;
    item.produced_at_ms = now_ms;

    if (ring_buffer_push(&g_ring_buffer, item) != 0U) {
        g_last_produced_sequence = item.sequence;
        next_sequence++;
    }
}

static void consumer_run(uint32_t now_ms)
{
    ring_item_t item;

    g_consumer_attempts++;

    if (ring_buffer_pop(&g_ring_buffer, &item) == 0U) {
        return;
    }

    if ((g_last_consumed_sequence != 0U) &&
        (item.sequence != (g_last_consumed_sequence + 1U))) {
        g_sequence_errors++;
    }

    g_last_consumed_sequence = item.sequence;
    g_last_latency_ms = now_ms - item.produced_at_ms;

    if (g_last_latency_ms > g_max_latency_ms) {
        g_max_latency_ms = g_last_latency_ms;
    }
}

static void enter_phase(experiment_phase_t next_phase, uint32_t now_ms)
{
    g_phase = next_phase;
    phase_started_ms = now_ms;
    g_phase_changes++;

    switch (next_phase) {
    case PHASE_BALANCED:
        producer_period_ms = BALANCED_PRODUCER_MS;
        consumer_period_ms = BALANCED_CONSUMER_MS;
        break;
    case PHASE_OVERLOAD:
        producer_period_ms = OVERLOAD_PRODUCER_MS;
        consumer_period_ms = OVERLOAD_CONSUMER_MS;
        break;
    case PHASE_DRAIN:
        producer_period_ms = DRAIN_PRODUCER_MS;
        consumer_period_ms = DRAIN_CONSUMER_MS;
        break;
    default:
        producer_period_ms = BALANCED_PRODUCER_MS;
        consumer_period_ms = BALANCED_CONSUMER_MS;
        g_phase = PHASE_BALANCED;
        break;
    }

    next_producer_ms = now_ms + producer_period_ms;
    next_consumer_ms = now_ms + consumer_period_ms;

    if (next_phase == PHASE_BALANCED) {
        next_consumer_ms = now_ms + BALANCED_CONSUMER_START_MS;
    }
}

static void phase_update(uint32_t now_ms)
{
    uint32_t elapsed_ms = now_ms - phase_started_ms;

    if ((g_phase == PHASE_BALANCED) &&
        (elapsed_ms >= BALANCED_DURATION_MS)) {
        enter_phase(PHASE_OVERLOAD, now_ms);
    } else if ((g_phase == PHASE_OVERLOAD) &&
               (elapsed_ms >= OVERLOAD_DURATION_MS)) {
        enter_phase(PHASE_DRAIN, now_ms);
    } else if ((g_phase == PHASE_DRAIN) &&
               (elapsed_ms >= DRAIN_DURATION_MS)) {
        enter_phase(PHASE_BALANCED, now_ms);
    }
}

static void led_status_update(uint32_t now_ms)
{
    if (g_ring_buffer.count == 0U) {
        led_set(0U);
        return;
    }

    if (g_ring_buffer.count < RING_BUFFER_CAPACITY) {
        led_set(1U);
        return;
    }

    /* El parpadeo rapido indica que el buffer esta lleno. */
    if (time_reached(now_ms, next_full_blink_ms) != 0U) {
        next_full_blink_ms = now_ms + FULL_BLINK_HALF_PERIOD_MS;
        led_toggle();
    }
}

int main(void)
{
    uint32_t now_ms;

    led_init();
    ring_buffer_init(&g_ring_buffer);
    systimer_init_1ms();

    now_ms = systimer_millis();
    phase_started_ms = now_ms;
    next_producer_ms = now_ms + producer_period_ms;
    next_consumer_ms = now_ms + BALANCED_CONSUMER_START_MS;
    next_led_update_ms = now_ms + LED_UPDATE_PERIOD_MS;
    next_full_blink_ms = now_ms + FULL_BLINK_HALF_PERIOD_MS;

    while (1) {
        now_ms = systimer_millis();
        phase_update(now_ms);

        if (time_reached(now_ms, next_producer_ms) != 0U) {
            next_producer_ms += producer_period_ms;
            producer_run(now_ms);
        }

        if (time_reached(now_ms, next_consumer_ms) != 0U) {
            next_consumer_ms += consumer_period_ms;
            consumer_run(now_ms);
        }

        if (time_reached(now_ms, next_led_update_ms) != 0U) {
            next_led_update_ms += LED_UPDATE_PERIOD_MS;
            led_status_update(now_ms);
        }

        g_background_iterations++;
    }
}
