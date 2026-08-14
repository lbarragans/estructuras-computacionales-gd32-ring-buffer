# 2. Compilacion y programacion

## Tareas disponibles

Abra `Terminal > Run Task`:

| Tarea | Funcion |
| --- | --- |
| `1. Verificar entorno GD32` | Comprueba herramientas y rutas |
| `2. Configurar CMake (Debug)` | Genera Ninja en `build/debug` |
| `3. Compilar GD32 (Debug)` | Construye el firmware |
| `4. Programar GD32 (Debug)` | Graba y verifica mediante OpenOCD |
| `5. Compilar y programar GD32` | Ejecuta 2, 3 y 4 en secuencia |
| `6. Preparar depuracion` | Genera el `launch.json` local |

Para el uso normal ejecute `5. Compilar y programar GD32`.

## Archivos generados

CMake y Ninja producen en `build/debug`:

- `GD32VW55x.elf`: ejecutable para flash y depuracion;
- `GD32VW55x.hex` y `GD32VW55x.bin`: imagenes de firmware;
- `GD32VW55x.map`: distribucion de memoria;
- `GD32VW55x.lst`: listado ensamblador.

## Resultado correcto

La programacion debe mostrar:

```text
** Programming Finished **
** Verified OK **
** Resetting Target **
```

Observe pulsos breves en la fase equilibrada, encendido durante la sobrecarga,
parpadeo rapido al llenarse y apagado despues del drenaje.
