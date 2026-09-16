# Ensamblador RISC-V puro

`main.S` contiene toda la aplicacion sin C:

- inicializacion GPIO, SysTimer y ECLIC;
- FIFO de ocho registros `{sequence, produced_at_ms}`;
- operaciones push/pop con retorno circular mediante mascara `& 7`;
- fases equilibrada, sobrecarga y drenaje;
- watermark, overflows, underflows, orden y latencia;
- estado de ocupacion representado mediante PC13.

El startup, linker script y wrapper de interrupciones continúan siendo
infraestructura del SDK. La integracion se explica en `DEPURACION.md`.
