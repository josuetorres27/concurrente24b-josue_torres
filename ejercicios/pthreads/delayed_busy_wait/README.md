## Ejecute al menos tres veces su solución (sin sanitizers) con la cantidad máxima de hilos de ejecución que su sistema operativo le permite crear y una espera de 50µs.


## Anote la mayor de las duraciones que obtuvo de sus corridas:
Utilizando 16 hilos, el mayor tiempo obtenido tras varias ejecuciones fue de: 0.003225389s.

## ¿Hubo una mejora de la espera constante respecto a la espera activa pura?
Sí.


## Ejecute al menos tres veces su solución pseudoaleatoria con la cantidad máxima de hilos y un máximo de espera de 50µs. Tome la mayor de las duraciones. ¿Hubo una mejora de la espera pseudoaleatoria respecto a la espera constante?
No hubo mejora, ya que la cantidad de hilos es baja, pero podría haberla si se usara una cantidad de hilos más grande.


## Comparación de las esperas. ¿Mejora el tiempo de ejecución de los dos tipos de esperas (constante y pseudoaleatoria) si disminuye o incrementa la espera máxima de los hilos?
En las ejecuciones que hice la cantidad de microsegundos no parece afectar en la mejora del tiempo de ejecución, ya que varios números más altos de microsegundos tuvieron tiempos de ejecución mejores que números más bajos.


## Indique si la espera activa con retraso es una solución óptima, y en caso negativo, provea una idea que pueda lograr este ideal:
La espera con retraso constante no es una solución óptima porque sigue utilizando recursos innecesarios, ya que los hilos siguen revisando repetidamente si es su turno, aunque en intervalos más largos.