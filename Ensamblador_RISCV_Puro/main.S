.section .text
    .align 2

    /* Ejercicio 07: ring buffer productor-consumidor en RISC-V puro. */
    .equ RCU_AHB1EN,           0x40023830
    .equ GPIOC_CTL,            0x40020800
    .equ GPIOC_OMODE,          0x40020804
    .equ GPIOC_OSPD,           0x40020808
    .equ GPIOC_PUD,            0x4002080C
    .equ GPIOC_BOP,            0x40020818
    .equ GPIOC_BC,             0x40020828
    .equ GPIOC_CLOCK_BIT,      0x00000004
    .equ PC13_BIT,             0x00002000
    .equ PC13_FIELD_MASK,      0x0C000000
    .equ PC13_OUTPUT_MODE,     0x04000000
    .equ PC13_SPEED_10MHZ,     0x04000000
    .equ SYSTIMER_CMP_LO,      0xD1000008
    .equ SYSTIMER_CMP_HI,      0xD100000C
    .equ SYSTIMER_MTIMECTL,    0xD1000FF8
    .equ MTIMECTL_CMPCLREN,    0x00000002
    .equ ECLIC_INTIP7,         0xD200101C
    .equ ECLIC_INTIE7,         0xD200101D
    .equ ECLIC_INTATTR7,       0xD200101E
    .equ ECLIC_INTCTRL7,       0xD200101F
    .equ ECLIC_POS_EDGE,       0x02
    .equ SYSTIMER_COMPARE_1MS, 40000
    .equ RING_CAPACITY,        8

    .global main
    .type main, @function
main:
    /* GPIOC/PC13, LED activo en bajo e inicialmente apagado. */
    li      t0, RCU_AHB1EN
    lw      t1, 0(t0)
    ori     t1, t1, GPIOC_CLOCK_BIT
    sw      t1, 0(t0)
    fence   iorw, iorw
    li      t0, GPIOC_CTL
    lw      t1, 0(t0)
    li      t2, PC13_FIELD_MASK
    not     t2, t2
    and     t1, t1, t2
    li      t2, PC13_OUTPUT_MODE
    or      t1, t1, t2
    sw      t1, 0(t0)
    li      t0, GPIOC_OMODE
    lw      t1, 0(t0)
    li      t2, PC13_BIT
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0(t0)
    li      t0, GPIOC_OSPD
    lw      t1, 0(t0)
    li      t2, PC13_FIELD_MASK
    not     t2, t2
    and     t1, t1, t2
    li      t2, PC13_SPEED_10MHZ
    or      t1, t1, t2
    sw      t1, 0(t0)
    li      t0, GPIOC_PUD
    lw      t1, 0(t0)
    li      t2, PC13_FIELD_MASK
    not     t2, t2
    and     t1, t1, t2
    sw      t1, 0(t0)
    li      t0, GPIOC_BOP
    li      t1, PC13_BIT
    sw      t1, 0(t0)

    /* SysTimer de 1 ms y ECLIC IRQ 7. */
    li      t0, SYSTIMER_MTIMECTL
    li      t1, MTIMECTL_CMPCLREN
    sw      t1, 0(t0)
    li      t0, SYSTIMER_CMP_LO
    li      t1, -1
    sw      t1, 0(t0)
    li      t0, SYSTIMER_CMP_HI
    sw      zero, 0(t0)
    li      t0, SYSTIMER_CMP_LO
    li      t1, SYSTIMER_COMPARE_1MS
    sw      t1, 0(t0)
    li      t0, ECLIC_INTIP7
    sb      zero, 0(t0)
    li      t0, ECLIC_INTATTR7
    li      t1, ECLIC_POS_EDGE
    sb      t1, 0(t0)
    li      t0, ECLIC_INTCTRL7
    sb      zero, 0(t0)
    li      t0, ECLIC_INTIE7
    li      t1, 1
    sb      t1, 0(t0)
    li      t0, 8
    csrs    mstatus, t0

    /* Fase equilibrada inicial. */
    li      t1, 400
    la      t0, g_producer_period_ms
    sw      t1, 0(t0)
    la      t0, g_consumer_period_ms
    sw      t1, 0(t0)
    la      t0, g_next_producer_ms
    sw      t1, 0(t0)
    li      t1, 600
    la      t0, g_next_consumer_ms
    sw      t1, 0(t0)
    li      t1, 50
    la      t0, g_next_led_ms
    sw      t1, 0(t0)
    li      t1, 100
    la      t0, g_next_blink_ms
    sw      t1, 0(t0)
    li      t1, 6000
    la      t0, g_phase_deadline_ms
    sw      t1, 0(t0)

.Lmain_loop:
    la      t0, g_systimer_ticks
    lw      s0, 0(t0)              /* s0 = now_ms */

    /* Cambio de fase cada 6000 ms. */
    la      t0, g_phase_deadline_ms
    lw      t1, 0(t0)
    sub     t2, s0, t1
    blt     t2, zero, .Lproducer_check
    li      t2, 6000
    add     t1, t1, t2
    sw      t1, 0(t0)
    la      t0, g_phase
    lw      t1, 0(t0)
    addi    t1, t1, 1
    li      t2, 3
    bltu    t1, t2, .Lphase_store
    li      t1, 0
.Lphase_store:
    sw      t1, 0(t0)
    la      t0, g_phase_changes
    lw      t2, 0(t0)
    addi    t2, t2, 1
    sw      t2, 0(t0)
    beqz    t1, .Lphase_balanced
    li      t2, 1
    beq     t1, t2, .Lphase_overload
    li      t2, 800
    li      t3, 100
    j       .Lphase_periods
.Lphase_overload:
    li      t2, 100
    li      t3, 500
    j       .Lphase_periods
.Lphase_balanced:
    li      t2, 400
    li      t3, 400
.Lphase_periods:
    la      t0, g_producer_period_ms
    sw      t2, 0(t0)
    la      t0, g_consumer_period_ms
    sw      t3, 0(t0)
    add     t4, s0, t2
    la      t0, g_next_producer_ms
    sw      t4, 0(t0)
    add     t4, s0, t3
    bnez    t1, .Lstore_consumer_deadline
    addi    t4, s0, 600
.Lstore_consumer_deadline:
    la      t0, g_next_consumer_ms
    sw      t4, 0(t0)

.Lproducer_check:
    la      t0, g_next_producer_ms
    lw      t1, 0(t0)
    sub     t2, s0, t1
    blt     t2, zero, .Lconsumer_check
    la      t3, g_producer_period_ms
    lw      t3, 0(t3)
    add     t1, t1, t3
    sw      t1, 0(t0)
    la      t0, g_producer_attempts
    lw      t1, 0(t0)
    addi    t1, t1, 1
    sw      t1, 0(t0)
    mv      a0, s0
    jal     ra, ring_push

.Lconsumer_check:
    la      t0, g_next_consumer_ms
    lw      t1, 0(t0)
    sub     t2, s0, t1
    blt     t2, zero, .Lled_check
    la      t3, g_consumer_period_ms
    lw      t3, 0(t3)
    add     t1, t1, t3
    sw      t1, 0(t0)
    la      t0, g_consumer_attempts
    lw      t1, 0(t0)
    addi    t1, t1, 1
    sw      t1, 0(t0)
    mv      a0, s0
    jal     ra, ring_pop

.Lled_check:
    la      t0, g_next_led_ms
    lw      t1, 0(t0)
    sub     t2, s0, t1
    blt     t2, zero, .Lbackground
    addi    t1, t1, 50
    sw      t1, 0(t0)
    mv      a0, s0
    jal     ra, led_status_update

.Lbackground:
    la      t0, g_background_iterations
    lw      t1, 0(t0)
    addi    t1, t1, 1
    sw      t1, 0(t0)
    j       .Lmain_loop
    .size main, .-main

    /* a0 = produced_at_ms. Devuelve a0=1 si inserta, 0 si esta lleno. */
    .align 2
    .type ring_push, @function
ring_push:
    la      t0, g_count
    lw      t1, 0(t0)
    li      t2, RING_CAPACITY
    bne     t1, t2, .Lpush_space
    la      t0, g_overflows
    lw      t1, 0(t0)
    addi    t1, t1, 1
    sw      t1, 0(t0)
    li      a0, 0
    ret
.Lpush_space:
    la      t2, g_head
    lw      t3, 0(t2)
    slli    t4, t3, 3
    la      t5, g_ring_storage
    add     t5, t5, t4
    la      t4, g_next_sequence
    lw      t6, 0(t4)
    sw      t6, 0(t5)
    sw      a0, 4(t5)
    la      t5, g_last_produced_sequence
    sw      t6, 0(t5)
    addi    t6, t6, 1
    sw      t6, 0(t4)
    addi    t3, t3, 1
    andi    t3, t3, 7
    sw      t3, 0(t2)
    addi    t1, t1, 1
    sw      t1, 0(t0)
    la      t2, g_pushes
    lw      t3, 0(t2)
    addi    t3, t3, 1
    sw      t3, 0(t2)
    la      t2, g_high_watermark
    lw      t3, 0(t2)
    bgeu    t3, t1, .Lpush_done
    sw      t1, 0(t2)
.Lpush_done:
    li      a0, 1
    ret

    /* a0 = now_ms. Devuelve a0=1 si extrae, 0 si esta vacio. */
    .align 2
    .type ring_pop, @function
ring_pop:
    la      t0, g_count
    lw      t1, 0(t0)
    bnez    t1, .Lpop_data
    la      t0, g_underflows
    lw      t1, 0(t0)
    addi    t1, t1, 1
    sw      t1, 0(t0)
    li      a0, 0
    ret
.Lpop_data:
    la      t2, g_tail
    lw      t3, 0(t2)
    slli    t4, t3, 3
    la      t5, g_ring_storage
    add     t5, t5, t4
    lw      t4, 0(t5)              /* secuencia */
    lw      t6, 4(t5)              /* instante producido */
    la      t5, g_last_consumed_sequence
    lw      a1, 0(t5)
    beqz    a1, .Lsequence_ok
    addi    a1, a1, 1
    beq     a1, t4, .Lsequence_ok
    la      a1, g_sequence_errors
    lw      a2, 0(a1)
    addi    a2, a2, 1
    sw      a2, 0(a1)
.Lsequence_ok:
    sw      t4, 0(t5)
    sub     t6, a0, t6
    la      t5, g_last_latency_ms
    sw      t6, 0(t5)
    la      t5, g_max_latency_ms
    lw      a1, 0(t5)
    bgeu    a1, t6, .Llatency_done
    sw      t6, 0(t5)
.Llatency_done:
    addi    t3, t3, 1
    andi    t3, t3, 7
    sw      t3, 0(t2)
    addi    t1, t1, -1
    sw      t1, 0(t0)
    la      t2, g_pops
    lw      t3, 0(t2)
    addi    t3, t3, 1
    sw      t3, 0(t2)
    li      a0, 1
    ret

    /* a0 = now_ms. LED apagado/vacio, fijo/parcial, blink/lleno. */
    .align 2
    .type led_status_update, @function
led_status_update:
    la      t0, g_count
    lw      t1, 0(t0)
    beqz    t1, .Lled_off
    li      t2, RING_CAPACITY
    bltu    t1, t2, .Lled_on
    la      t0, g_next_blink_ms
    lw      t1, 0(t0)
    sub     t2, a0, t1
    blt     t2, zero, .Lled_return
    addi    t1, a0, 100
    sw      t1, 0(t0)
    la      t0, g_led_is_on
    lbu     t1, 0(t0)
    xori    t1, t1, 1
    sb      t1, 0(t0)
    beqz    t1, .Lled_hw_off
    j       .Lled_hw_on
.Lled_off:
    la      t0, g_led_is_on
    sb      zero, 0(t0)
.Lled_hw_off:
    li      t0, GPIOC_BOP
    li      t1, PC13_BIT
    sw      t1, 0(t0)
    ret
.Lled_on:
    la      t0, g_led_is_on
    li      t1, 1
    sb      t1, 0(t0)
.Lled_hw_on:
    li      t0, GPIOC_BC
    li      t1, PC13_BIT
    sw      t1, 0(t0)
.Lled_return:
    ret

    .align 2
    .global eclic_mtip_handler
    .type eclic_mtip_handler, @function
eclic_mtip_handler:
    li      t0, ECLIC_INTIP7
    sb      zero, 0(t0)
    la      t0, g_systimer_ticks
    lw      t1, 0(t0)
    addi    t1, t1, 1
    sw      t1, 0(t0)
    ret
    .size eclic_mtip_handler, .-eclic_mtip_handler

.section .data
    .align 2
    .global g_next_sequence
g_next_sequence:             .word 1

.section .bss
    .align 3
    .global g_ring_storage
g_ring_storage:              .space 64
    .align 2
    .global g_head
g_head:                      .word 0
    .global g_tail
g_tail:                      .word 0
    .global g_count
g_count:                     .word 0
    .global g_high_watermark
g_high_watermark:            .word 0
    .global g_pushes
g_pushes:                    .word 0
    .global g_pops
g_pops:                      .word 0
    .global g_overflows
g_overflows:                 .word 0
    .global g_underflows
g_underflows:                .word 0
    .global g_systimer_ticks
g_systimer_ticks:            .word 0
    .global g_phase
g_phase:                     .word 0
    .global g_phase_changes
g_phase_changes:             .word 0
    .global g_phase_deadline_ms
g_phase_deadline_ms:         .word 0
    .global g_producer_period_ms
g_producer_period_ms:        .word 0
    .global g_consumer_period_ms
g_consumer_period_ms:        .word 0
    .global g_next_producer_ms
g_next_producer_ms:          .word 0
    .global g_next_consumer_ms
g_next_consumer_ms:          .word 0
    .global g_next_led_ms
g_next_led_ms:               .word 0
    .global g_next_blink_ms
g_next_blink_ms:             .word 0
    .global g_producer_attempts
g_producer_attempts:         .word 0
    .global g_consumer_attempts
g_consumer_attempts:         .word 0
    .global g_last_produced_sequence
g_last_produced_sequence:    .word 0
    .global g_last_consumed_sequence
g_last_consumed_sequence:    .word 0
    .global g_last_latency_ms
g_last_latency_ms:           .word 0
    .global g_max_latency_ms
g_max_latency_ms:            .word 0
    .global g_sequence_errors
g_sequence_errors:           .word 0
    .global g_background_iterations
g_background_iterations:    .word 0
    .global g_led_is_on
g_led_is_on:                 .byte 0
