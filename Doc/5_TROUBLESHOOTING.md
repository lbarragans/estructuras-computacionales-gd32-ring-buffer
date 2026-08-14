# 5. Solucion de problemas

## El proyecto no configura

Revise `tools/local_config.ps1` y ejecute `1. Verificar entorno GD32`. Si
cambio una ruta, elimine `build` desde el Explorador y vuelva a ejecutar
`2. Configurar CMake (Debug)`.

## `ninja: no work to do`

No es un error. Significa que no se modificaron fuentes desde la ultima
compilacion.

## El LED permanece apagado al inicio

Es normal durante los primeros instantes o cuando el buffer esta vacio. En la
fase equilibrada debe generar pulsos breves; luego se enciende durante la
sobrecarga y parpadea rapido al llenarse.

## No aparecen overflows

1. compruebe que `g_phase` llegue a `PHASE_OVERLOAD`;
2. confirme productor de 100 ms y consumidor de 500 ms;
3. observe durante mas de seis segundos;
4. revise `g_ring_buffer.count` y `g_ring_buffer.high_watermark`.

## Aparecen underflows

Son esperados en `PHASE_DRAIN`: el consumidor continua intentando leer despues
de vaciar la cola. No indican corrupcion de memoria.

## `g_sequence_errors` aumenta

No debe aumentar. Revise que `next_sequence` solo se incremente cuando `push`
tiene exito y que `tail` avance unicamente despues de un `pop` valido.

## El depurador altera los contadores

Los breakpoints detienen la CPU y modifican la relacion temporal entre fases.
Reinicie la sesion sin breakpoints para evaluar el comportamiento fisico.

## F5 no inicia

Abra `Run and Debug` y use el boton verde. Si no aparece la configuracion,
ejecute `6. Preparar depuracion` y recargue VS Code.

## Verificacion antes de publicar

Quien mantenga el repositorio puede comprobar que Git ignora:

```text
git check-ignore build
git check-ignore tools/local_config.ps1
git check-ignore .vscode/launch.json
```

Estos comandos no forman parte de la practica del estudiante.
