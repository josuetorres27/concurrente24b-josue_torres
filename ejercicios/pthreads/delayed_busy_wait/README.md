## Ejercicio 14 [delayed_busy_wait]
Espera activa con retraso. ¿Se corrige el problema de la espera activa si en lugar de tener un ciclo vacío, se espera un cierto tiempo? Copie su carpeta ejemplos/pthreads/hello_order_busywait a ejercicios/pthreads/delayed_busy_wait. Permita que el usuario pueda invocar su programa dos argumentos de línea de comandos: la cantidad de hilos a crear, y la cantidad de microsegundos a esperar cuando no es el turno del hilo de ejecución.

### Ejecute al menos tres veces su solución (sin sanitizers) con la cantidad máxima de hilos de ejecución que su sistema operativo le permite crear y una espera de 50µs.
### Anote la mayor de las duraciones que obtuvo de sus corridas:
Utilizando 2740 hilos (la cantidad máxima que permite el sistema son 27407, pero se utilizaron menos para evitar sobrecargas), el mayor tiempo obtenido tras varias ejecuciones fue de: 1.074669638s.


### ¿Hubo una mejora de la espera constante respecto a la espera activa pura?
Sí.


### Ejecute al menos tres veces su solución pseudoaleatoria con la cantidad máxima de hilos y un máximo de espera de 50µs. Tome la mayor de las duraciones. ¿Hubo una mejora de la espera pseudoaleatoria respecto a la espera constante?
La mejora fue mínima, pero podría mejorar más si se usara una cantidad de hilos más grande y la comparación fuera con un máximo de espera mayor.


### Comparación de las esperas. ¿Mejora el tiempo de ejecución de los dos tipos de esperas (constante y pseudoaleatoria) si disminuye o incrementa la espera máxima de los hilos?
Los tiempos de ejecución varían mínimamente, pero se aprecia una mejora en la espera pseudoaleatoria con cantidades de microsegundos mayores.


### Cree una gráfica donde el eje-x son las duraciones dadas por argumento al programa. El eje-y son los tiempos de ejecución de los programas. El gráfico tendrá dos series, una para la espera constante y otra para la espera pseudoaleatoria.
![Se presenta un cuadro comparativo con los tiempos de ejecución dados por la espera constante (microsegundos ingresados por el usuario) y la espera pseudoaleatoria (máximo de microsegundos de espera ingresado por el usuario). En la comparación se aprecia cómo los tiempos varían entre ambas esperas, con diferencias pequeñas, que terminan produciendo que cada una genere tiempos de ejecución menores en la mitad de los casos. Como se mencionó antes, la cantidad baja de hilos puede ser un factor causante de la poca diferencia entre esperas, pero se logra apreciar cómo la espera pseudoaleatoria puede generar tiempos de ejecución menores con topes de espera más altos, pero sin seguridad de que esto suceda también con topes de espera más bajos (por ejemplo, el caso de 1µs).](Constant_vs_Pseudorandom.png)


### Indique cuál fue el mejor tiempo máximo de espera obtenido y los efectos de disminuir o incrementarlo. Conjeture y trate de explicar por qué ocurre este comportamiento. Finalmente indique si la espera activa con retraso es una solución óptima, y en caso negativo, provea una idea que pueda lograr este ideal.
Al probar con la espera "tope" de 100µs, imprimiendo la cantidad pseudoaleatoria de microsegundos generada por varias ejecuciones, la mayor fue de 97µs. Si este número se disminuyera, posiblemente se darían tiempos de ejecución menores, y si se incrementa sería el caso contrario.

La espera con retraso constante no es una solución óptima porque sigue utilizando recursos innecesarios, ya que los hilos siguen revisando repetidamente si es su turno, aunque en intervalos más largos.
