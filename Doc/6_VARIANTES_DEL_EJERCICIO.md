# 6. Implementaciones definitivas

| Implementacion | Contenedor | Temporizacion |
|---|---|---|
| Referencia C | `ring_buffer_t` de ocho elementos | SysTimer de 1 ms y bucle principal |
| Assembly puro | almacenamiento y metadatos propios | SysTimer de 1 ms y despacho Assembly |
| FreeRTOS puro | `QueueHandle_t` de longitud ocho | tareas, delays y tick del kernel |

## Referencia

`Src/main.c`, `Src/ring_buffer.c` y `Src/systimer.c` permanecen como control y
son las fuentes seleccionadas por el CMake de la raiz.

## Assembly puro

`Ensamblador_RISCV_Puro/main.S` implementa inicializacion del hardware,
SysTimer/ECLIC, push, pop, fases, metricas y señalizacion LED sin C de
aplicacion. La infraestructura de startup y enlazado sigue proviniendo del SDK.

## FreeRTOS puro

`FreeRTOS_Puro/main.c` usa una Queue de ocho elementos, tareas productora y
consumidora, una tarea de fase y otra de LED. No llama al ring buffer original.
