## Ejemplo 12. Arreglo falso compartido [false_sharing_array]
Estudie y ejecute el programa dado en sus cuatro modos, usando los argumentos de línea de comandos 0 a 3. Todos los modos realizan la misma tarea de actualizar dos elementos de un arreglo cien millones de veces. En los modos 0 y 1 los dos elementos se actualizan de forma serial. En los modos 2 y 3 dos hilos concurrentemente actualizan los elementos, como se muestra en la siguiente tabla.


Se espera que las versiones seriales tarden aproximadamente lo mismo y que las versiones concurrentes tarden aproximadamente la mitad de las versiones seriales. Sin embargo, esto no ocurre. Trate de explicar por qué.

R/ La razón por la que las versiones concurrentes no son dos veces más rápidas que las seriales se debe principalmente a la falsa compartición de datos. Esto ocurre cuando los hilos actualizan elementos que están cerca en memoria, como en el modo 2 (primer y segundo elemento), generando conflictos en la caché y ralentizando el proceso. Aunque en el modo 3 (primer y último elemento) los elementos están más separados y hay menos interferencia, otros factores como la sobrecarga de sincronización y la gestión de hilos también evitan que los modos concurrentes sean tan rápidos como se espera.
