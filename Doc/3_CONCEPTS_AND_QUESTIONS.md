# 3. Conceptos y preguntas

## FIFO y buffer circular

FIFO significa *First In, First Out*: el primer dato almacenado debe ser el
primero en salir. Un buffer circular reutiliza un arreglo fijo; cuando un
indice supera la ultima posicion vuelve a cero.

```c
next_index = (current_index + 1U) % RING_BUFFER_CAPACITY;
```

No se desplazan elementos y no se reserva memoria durante la ejecucion.

## Campos de la estructura

| Campo | Funcion |
| --- | --- |
| `storage[8]` | Memoria fija para los elementos |
| `head` | Posicion de la proxima escritura |
| `tail` | Posicion de la proxima lectura |
| `count` | Numero actual de elementos |
| `high_watermark` | Mayor ocupacion observada |
| `pushes` | Inserciones exitosas |
| `pops` | Extracciones exitosas |
| `overflows` | Inserciones rechazadas por estar lleno |
| `underflows` | Lecturas intentadas cuando estaba vacio |

## Invariantes

Durante una ejecucion correcta siempre se cumple:

```text
0 <= head < 8
0 <= tail < 8
0 <= count <= 8
pushes - pops = count
high_watermark <= 8
```

`head == tail` puede significar vacio o lleno. En esta implementacion `count`
elimina esa ambiguedad.

## Productor

El productor crea un `ring_item_t` con:

```text
sequence, produced_at_ms
```

Solo incrementa `next_sequence` cuando `push` tiene exito. Si el buffer esta
lleno, conserva el numero para reintentarlo; asi los elementos aceptados
mantienen una secuencia continua.

## Consumidor

El consumidor retira el elemento apuntado por `tail`, comprueba la secuencia y
calcula:

```text
latencia = tiempo_de_consumo - tiempo_de_produccion
```

La latencia aumenta durante la sobrecarga porque los datos esperan dentro de
la cola.

## Overflow y underflow

- **Overflow:** el productor intenta escribir cuando `count == 8`. El dato se
  rechaza sin sobrescribir informacion pendiente.
- **Underflow:** el consumidor intenta leer cuando `count == 0`. No se entrega
  un dato inventado.

Ambos eventos son parte deliberada del experimento y quedan registrados.

## Concurrencia

En este ejercicio productor y consumidor se ejecutan en `main()`, por lo que
no modifican simultaneamente la estructura. Si uno se trasladara a una ISR o a
otro hart, seria necesario analizar atomicidad, secciones criticas, barreras de
memoria y el modelo productor-unico/consumidor-unico.

## Preguntas para clase

1. ¿Por que no se desplazan los elementos despues de cada `pop`?
2. ¿Que diferencia existe entre capacidad y ocupacion?
3. ¿Por que `head == tail` es ambiguo sin `count`?
4. ¿Que informacion perderiamos si sobrescribieramos el dato mas antiguo?
5. ¿Por que la latencia aumenta cuando el productor es mas rapido?
6. ¿Que representa `high_watermark`?
7. ¿Por que los datos aceptados conservan orden aun cuando hay overflows?
8. ¿Que cambiaria si el buffer almacenara bytes recibidos por UART?
9. ¿Que variables deberian ser atomicas si el productor estuviera en una ISR?
10. ¿Cuando conviene descartar el dato nuevo y cuando el dato mas antiguo?

## Actividades sugeridas

1. Cambie la capacidad de 8 a 4 y compare los overflows.
2. Iguale los periodos de la fase de sobrecarga.
3. Modifique `push` para sobrescribir el elemento mas antiguo y documente el
   cambio de politica.
4. Agregue a `ring_item_t` un campo `value` y procese su promedio.
