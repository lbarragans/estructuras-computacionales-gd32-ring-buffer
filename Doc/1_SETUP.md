# 1. Preparacion del entorno

## Requisitos

- placa GD32VW553HMQ6/HMQ7 con LED PC13;
- WCH-Link o CMSIS-DAP conectado por JTAG;
- Visual Studio Code normal, CMake y Ninja;
- Nuclei RISC-V GCC/GDB;
- OpenOCD y `GD32VW55x_Firmware_Library_V1.6.0`.

La instalacion general se explica en el repositorio
`gd32vw553-vscode-cmake-guide`.

## Abrir el proyecto

1. Inicie Visual Studio Code.
2. Seleccione `File > Open Folder`.
3. Abra `07_Ring_Buffer_Producer_Consumer`.
4. Acepte `Trust the authors` si aparece el modo restringido.

La raiz visible debe contener directamente `CMakeLists.txt`, `Src`, `Inc`,
`tools` y `.vscode`.

## Configuracion local

Desde el Explorador de VS Code, duplique
`tools/local_config.example.ps1`, renombre la copia como
`tools/local_config.ps1` y complete:

| Variable | Ubicacion |
| --- | --- |
| `GD32_SDK_ROOT` | Raiz de la biblioteca oficial GD32VW55x |
| `NUCLEI_TOOLCHAIN_DIR` | Carpeta `NucleiRISCVGCC/bin` |
| `OPENOCD_ROOT` | Carpeta que contiene `bin` y `scripts` |

El archivo local esta ignorado por Git y no debe publicarse.

## Verificar desde VS Code

1. Abra `Terminal > Run Task`.
2. Seleccione `1. Verificar entorno GD32`.
3. Compruebe que todos los componentes aparezcan como `[OK]`.

## Extensiones recomendadas

- `marus25.cortex-debug`;
- `ms-vscode.cpptools`;
- `ms-vscode.cmake-tools`.
