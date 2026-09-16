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

Se ejecuta como aplicacion del MSDK V1.0.3g para usar kernel, port
Nuclei/ECLIC, heap y tick oficiales.

Desde VS Code ejecute **Terminal > Run Task > Build + Flash FreeRTOS**. La
salida visible recorre equilibrio, sobrecarga y drenaje cada 6 segundos.
