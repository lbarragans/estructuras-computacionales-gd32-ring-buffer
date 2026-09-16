# Ejercicio 07 - Ring buffer productor-consumidor

**Curso:** Estructuras Computacionales  
**Autora:** Laura Daniela Barragan Silva  
**Plataforma:** GD32VW553HMQ6/HMQ7, RISC-V RV32

## Objetivo

Estudiar una FIFO circular de ocho elementos con productores y consumidores
de distinta velocidad. Cada elemento conserva su secuencia y el instante de
produccion para medir orden, ocupacion, desbordamientos y latencia.

## Tres caminos

| Camino | Archivos | Concepto |
|---|---|---|
| Referencia | `Src/main.c`, `Src/ring_buffer.c`, `Src/systimer.c` | FIFO circular C con planificacion temporal |
| RISC-V puro | `Ensamblador_RISCV_Puro/main.S` | `head`, `tail`, `count`, push y pop en Assembly |
| FreeRTOS puro | `FreeRTOS_Puro/main.c` | productor y consumidor comunicados por una cola FreeRTOS |

La referencia original permanece como compilacion principal de CMake. Las
alternativas son independientes y no se compilan simultaneamente con `Src/`.

## Experimento

| Fase de 6 s | Productor | Consumidor | Resultado esperado |
|---|---:|---:|---|
| Equilibrada | 400 ms | 400 ms | ocupacion baja |
| Sobrecarga | 100 ms | 500 ms | cola llena y rechazos |
| Drenaje | 800 ms | 100 ms | vaciado de la cola |

PC13 permanece apagado con la cola vacia, encendido con ocupacion parcial y
parpadea rapidamente cuando esta llena.

## Estructura

```text
07_Ring_Buffer_Producer_Consumer/
├── Src/                         # referencia original
├── Inc/
├── Ensamblador_RISCV_Puro/      # FIFO y aplicacion RV32 Assembly
├── FreeRTOS_Puro/               # aplicacion basada en Queue
├── Doc/
├── CMakeLists.txt
└── README.md
```

## Compilacion de la referencia

1. Copie `tools/local_config.example.ps1` como `tools/local_config.ps1`.
2. Configure las rutas locales del SDK, toolchain y OpenOCD.
3. Ejecute en VS Code `1. Verificar entorno GD32`.
4. Ejecute `5. Compilar y programar GD32`.
5. Para depurar, ejecute una vez `6. Preparar depuracion`.

## Estado

| Implementacion | Estado |
|---|---|
| Referencia C | funcional y seleccionada por CMake |
| Assembly puro | fuente lista; integracion y placa pendientes |
| FreeRTOS puro | fuente lista; kernel, port e integracion pendientes |

Consulte `Doc/6_VARIANTES_DEL_EJERCICIO.md` y
`Doc/7_PLAN_DE_VALIDACION.md`. Ninguna alternativa se declara validada en
hardware hasta compilarla y probarla sobre la placa.
