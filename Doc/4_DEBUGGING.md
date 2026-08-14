# 4. Depuracion

## Iniciar

1. Ejecute `Terminal > Run Task > 6. Preparar depuracion` una sola vez.
2. Abra `Run and Debug`.
3. Seleccione `Debug GD32VW553 - Cortex Debug`.
4. Pulse el boton verde.

## Breakpoints recomendados

En `Src/ring_buffer.c`:

1. `buffer->storage[buffer->head] = item;` para observar un `push`;
2. `*item = buffer->storage[buffer->tail];` para observar un `pop`;
3. `buffer->overflows++;` para detenerse cuando se llena;
4. `buffer->underflows++;` para detenerse cuando queda vacio.

En `Src/main.c`:

5. primera linea de `enter_phase()` para observar cada cambio de fase;
6. primera linea de `consumer_run()` para medir latencia.

No coloque un breakpoint permanente en `eclic_mtip_handler()`: se ejecuta cada
milisegundo.

## Watch

```text
g_systimer_ticks
g_phase
g_ring_buffer
g_ring_buffer.storage
g_ring_buffer.head
g_ring_buffer.tail
g_ring_buffer.count
g_ring_buffer.high_watermark
g_ring_buffer.pushes
g_ring_buffer.pops
g_ring_buffer.overflows
g_ring_buffer.underflows
g_producer_attempts
g_consumer_attempts
g_last_produced_sequence
g_last_consumed_sequence
g_last_latency_ms
g_max_latency_ms
g_sequence_errors
```

## Secuencia de observacion

### Cerca de 6 segundos

Antes de entrar a sobrecarga se espera:

```text
g_phase = PHASE_BALANCED
g_ring_buffer.count = 0 o 1
g_ring_buffer.overflows = 0
g_sequence_errors = 0
```

Al continuar, `g_phase` cambia a `PHASE_OVERLOAD`.

### Cerca de 12 segundos

El productor de 100 ms supera al consumidor de 500 ms:

```text
g_phase = PHASE_OVERLOAD
g_ring_buffer.count = 8
g_ring_buffer.high_watermark = 8
g_ring_buffer.overflows > 0
```

### Cerca de 18 segundos

Durante drenaje el consumidor de 100 ms vacia la cola:

```text
g_phase = PHASE_DRAIN
g_ring_buffer.count = 0
g_ring_buffer.underflows > 0
g_sequence_errors = 0
```

Los valores exactos pueden cambiar si el procesador permanece detenido. Los
breakpoints alteran el tiempo real del experimento, pero no deben romper las
invariantes del buffer.

## Seguir un dato

1. Detengase antes de un `push` y anote `item.sequence` y `head`.
2. Continue hasta un `pop` del mismo numero.
3. Compruebe que sale por `tail` en el orden de llegada.
4. Compare `produced_at_ms` con `g_last_latency_ms`.
