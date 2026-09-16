# Integracion FreeRTOS

1. Incorporar las fuentes del kernel, port RISC-V y un `heap_x.c`.
2. Añadir `FreeRTOSConfig.h` compatible con el GD32VW553.
3. Sustituir las fuentes de `Src/` por `main.c` de esta carpeta.
4. Añadir al include path los headers del kernel y del port.
5. Verificar la fuente del tick y su IRQ antes de programar.
6. Ejecutar `Doc/7_PLAN_DE_VALIDACION.md` durante al menos 18 segundos.

No compilar simultaneamente esta aplicacion y los tres archivos de `Src/`.
