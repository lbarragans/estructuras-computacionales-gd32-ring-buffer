# 7. Plan de validacion

## Comportamiento comun

- tres fases consecutivas de 6000 ms;
- capacidad de ocho elementos;
- rechazo al producir sobre una cola llena;
- intento fallido al consumir una cola vacia;
- secuencias FIFO y medicion de latencia;
- LED apagado, fijo o intermitente segun ocupacion.

## Referencia

Compilar con `build-debug` y observar `g_ring_buffer`, `g_phase`, secuencias,
latencias, errores y cambios de fase durante un ciclo completo de 18 segundos.

## Assembly

Integrar `main.S` como unica aplicacion. Observar `g_ring_storage`, `g_head`,
`g_tail`, `g_count`, `g_high_watermark`, `g_overflows`, `g_underflows`,
`g_phase`, secuencias y latencias. Revisar el `.lst` para confirmar la ausencia
de C de aplicacion.

## FreeRTOS

Integrar kernel, port, heap y configuracion. Confirmar una Queue de longitud
ocho, periodos por fase, rechazos sin bloqueo y orden de las secuencias.

## Criterio de cierre

Conservar evidencia de compilacion y placa durante al menos un ciclo completo.
No marcar una alternativa como validada solamente por inspeccion del codigo.
