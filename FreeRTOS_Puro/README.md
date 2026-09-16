# FreeRTOS puro

Esta ruta sustituye el ring buffer manual por una Queue FreeRTOS de ocho
elementos. Conserva secuencia, instante de produccion, tres fases, metricas de
rechazo/lectura y representacion de ocupacion mediante PC13.

| Tarea | Funcion |
|---|---|
| `producer_task` | producir con el periodo de la fase activa |
| `consumer_task` | consumir sin bloqueo y medir orden/latencia |
| `phase_task` | cambiar de fase cada 6000 ms |
| `led_task` | representar la ocupacion de la Queue |

Requiere kernel, port RISC-V, heap, `FreeRTOSConfig.h` y tick funcional.
