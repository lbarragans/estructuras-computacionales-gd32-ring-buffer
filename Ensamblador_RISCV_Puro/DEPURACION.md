# Depuracion Assembly

Observe estas etiquetas globales:

- `g_ring_storage`, `g_head`, `g_tail` y `g_count`;
- `g_high_watermark`, `g_pushes`, `g_pops`;
- `g_overflows` y `g_underflows`;
- `g_phase`, `g_phase_changes` y los periodos activos;
- `g_last_produced_sequence`, `g_last_consumed_sequence`;
- `g_last_latency_ms`, `g_max_latency_ms`, `g_sequence_errors`.

Integre `main.S` como unica fuente de aplicacion y conserve startup/linker del
SDK. No compile simultaneamente `Src/main.c`, `Src/ring_buffer.c` ni
`Src/systimer.c`. Compruebe direcciones de perifericos con la version del SDK y
complete el plan de validacion sobre la placa.
