#include <stdint.h>

#include "app_cfg.h"
#include "gd32vw55x_platform.h"
#include "wrapper_os.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "gd32vw55x.h"

#define LED_GPIO_PORT          GPIOC
#define LED_GPIO_PIN           GPIO_PIN_13
#define LED_GPIO_CLOCK         RCU_GPIOC
#define QUEUE_CAPACITY         8U
#define PHASE_DURATION_MS      6000U
#define LED_PERIOD_MS          50U
#define FULL_BLINK_MS          100U

typedef struct {
    uint32_t sequence;
    TickType_t produced_at;
} queue_item_t;

typedef enum {
    PHASE_BALANCED = 0,
    PHASE_OVERLOAD,
    PHASE_DRAIN
} experiment_phase_t;

static QueueHandle_t item_queue;
static volatile experiment_phase_t g_phase = PHASE_BALANCED;
static volatile uint32_t g_producer_attempts = 0U;
static volatile uint32_t g_consumer_attempts = 0U;
static volatile uint32_t g_overflows = 0U;
static volatile uint32_t g_underflows = 0U;
static volatile uint32_t g_last_produced_sequence = 0U;
static volatile uint32_t g_last_consumed_sequence = 0U;
static volatile uint32_t g_last_latency_ms = 0U;
static volatile uint32_t g_max_latency_ms = 0U;
static volatile uint32_t g_sequence_errors = 0U;
static volatile uint32_t g_phase_changes = 0U;
static volatile UBaseType_t g_high_watermark = 0U;
static volatile uint8_t g_led_is_on = 0U;

static void led_init(void)
{
    rcu_periph_clock_enable(LED_GPIO_CLOCK);
    gpio_mode_set(LED_GPIO_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
                  LED_GPIO_PIN);
    gpio_output_options_set(LED_GPIO_PORT, GPIO_OTYPE_PP,
                            GPIO_OSPEED_10MHZ, LED_GPIO_PIN);
    gpio_bit_reset(LED_GPIO_PORT, LED_GPIO_PIN);
}

static void led_set(uint8_t turn_on)
{
    const uint8_t state = (turn_on != 0U) ? 1U : 0U;

    if (state == g_led_is_on) {
        return;
    }
    if (state != 0U) {
        gpio_bit_set(LED_GPIO_PORT, LED_GPIO_PIN);
    } else {
        gpio_bit_reset(LED_GPIO_PORT, LED_GPIO_PIN);
    }
    g_led_is_on = state;
}

static TickType_t producer_period(void)
{
    if (g_phase == PHASE_OVERLOAD) {
        return pdMS_TO_TICKS(100U);
    }
    if (g_phase == PHASE_DRAIN) {
        return pdMS_TO_TICKS(800U);
    }
    return pdMS_TO_TICKS(400U);
}

static TickType_t consumer_period(void)
{
    if (g_phase == PHASE_OVERLOAD) {
        return pdMS_TO_TICKS(500U);
    }
    if (g_phase == PHASE_DRAIN) {
        return pdMS_TO_TICKS(100U);
    }
    return pdMS_TO_TICKS(400U);
}

static void producer_task(void *argument)
{
    uint32_t next_sequence = 1U;
    (void)argument;

    for (;;) {
        queue_item_t item;
        UBaseType_t occupied;

        vTaskDelay(producer_period());
        item.sequence = next_sequence;
        item.produced_at = xTaskGetTickCount();
        g_producer_attempts++;

        if (xQueueSend(item_queue, &item, 0U) == pdPASS) {
            g_last_produced_sequence = next_sequence++;
            occupied = uxQueueMessagesWaiting(item_queue);
            if (occupied > g_high_watermark) {
                g_high_watermark = occupied;
            }
        } else {
            g_overflows++;
        }
    }
}

static void consumer_task(void *argument)
{
    (void)argument;

    /* La referencia retrasa el primer consumo equilibrado hasta 600 ms. */
    vTaskDelay(pdMS_TO_TICKS(600U));
    for (;;) {
        queue_item_t item;
        uint32_t latency_ms;

        g_consumer_attempts++;
        if (xQueueReceive(item_queue, &item, 0U) == pdPASS) {
            if ((g_last_consumed_sequence != 0U) &&
                (item.sequence != (g_last_consumed_sequence + 1U))) {
                g_sequence_errors++;
            }
            g_last_consumed_sequence = item.sequence;
            latency_ms = (uint32_t)((xTaskGetTickCount() - item.produced_at) *
                                    portTICK_PERIOD_MS);
            g_last_latency_ms = latency_ms;
            if (latency_ms > g_max_latency_ms) {
                g_max_latency_ms = latency_ms;
            }
        } else {
            g_underflows++;
        }
        vTaskDelay(consumer_period());
    }
}

static void phase_task(void *argument)
{
    (void)argument;

    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(PHASE_DURATION_MS));
        taskENTER_CRITICAL();
        if (g_phase == PHASE_DRAIN) {
            g_phase = PHASE_BALANCED;
        } else {
            g_phase = (experiment_phase_t)((uint32_t)g_phase + 1U);
        }
        g_phase_changes++;
        taskEXIT_CRITICAL();
    }
}

static void led_task(void *argument)
{
    TickType_t last_blink = 0U;
    (void)argument;

    for (;;) {
        const UBaseType_t occupied = uxQueueMessagesWaiting(item_queue);
        const TickType_t now = xTaskGetTickCount();

        if (occupied == 0U) {
            led_set(0U);
        } else if (occupied < QUEUE_CAPACITY) {
            led_set(1U);
        } else if ((now - last_blink) >= pdMS_TO_TICKS(FULL_BLINK_MS)) {
            last_blink = now;
            led_set((uint8_t)(g_led_is_on == 0U));
        }
        vTaskDelay(pdMS_TO_TICKS(LED_PERIOD_MS));
    }
}

int main(void)
{
    sys_os_init();
    platform_init();
    led_init();
    item_queue = xQueueCreate(QUEUE_CAPACITY, sizeof(queue_item_t));

    if (item_queue == NULL) {
        for (;;) {
        }
    }

    BaseType_t producer_ok = xTaskCreate(
        producer_task, "Producer", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY + 2U, NULL
    );
    BaseType_t consumer_ok = xTaskCreate(
        consumer_task, "Consumer", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY + 2U, NULL
    );
    BaseType_t phase_ok = xTaskCreate(
        phase_task, "Phase", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY + 3U, NULL
    );
    BaseType_t led_ok = xTaskCreate(
        led_task, "LED", configMINIMAL_STACK_SIZE,
        NULL, tskIDLE_PRIORITY + 1U, NULL
    );

    if ((producer_ok != pdPASS) || (consumer_ok != pdPASS) ||
        (phase_ok != pdPASS) || (led_ok != pdPASS)) {
        for (;;) {
        }
    }

    sys_os_start();
    for (;;) {
    }
}
