# Reporte de optimizaciones

Para medir las duraciones se utilizó el test-case job003 ya que se puede considerar una prueba mediana.
La comparación NO se realizó con un caso de prueba grande por cuestiones de tiempo (1 solo caso serial duraba hasta 30 minutos, como mínimo, en ejecutarse).

## Optimizaciones seriales

| Iter. | Etiqueta | Duración (s) | Speedup | Descripción corta |
|--------------|--------------|--------------|--------------|--------------|
| 0 | Serial0 | 89.283 | 1.00 | Versión serial inicial (Tarea01) |
| 1 | Serial1 | 72.071 | 1.24 | No hacer cálculos innecesarios |
| 2 | - | (Se encicla) | - | Intercambiar punteros de matriz copia a matriz original |

Versión serial original (Tarea01)

Las constantes lecturas de archivos pueden ser un problema para la eficiencia del programa.
También se realizan varios ciclos for en la parte de realizar la simulación, y dentro de los ciclos se hacen bastantes copias de matriz a matriz. Otro punto importante es que se utiliza mucha memoria dinámica, y el estar haciendo malloc/calloc y free constantemente puede que esté afectando al rendimiento.
Por otro lado, la lectura del archivo de trabajo me parec eficiente ya que se realiza una única vez en todo el programa. También creo que el utilizar variables con datos de tamaño fijo pueden tener un impacto beneficioso en la eficiencia del programa.

Iteración 1

La optimización realizada se enfoca en reducir cálculos innecesarios y mejorar la eficiencia del bucle de simulación. Originalmente, dentro de cada iteración, se calculaba repetidamente una expresión constante para cada celda de la matriz. Este cálculo se movió fuera del bucle, precalculando el valor de `alpha_delta` una vez antes de la iteración, evitando así operaciones repetitivas. Además, la creación de la matriz copia se movió para la función `handle_matrix()`. Esta modificación mejoró el desempeño en un 24%, reduciendo el tiempo de ejecución de 89.283s a 72.071s. La optimización puede representarse así:

```c
double alpha_delta = file_content.delta * file_content.alpha / (file_content.h * file_content.h);

for (each cell in matrix) {
    matrix_copy[i][j] = T_ij + alpha_delta * (T_neighbors - 4 * T_ij);
}
```

Iteración 2

Se intentó optimizar el código intercambiando punteros de matriz copia a matriz original en vez de utilizar un doble for y copiarlos manualmente en cada iteración del ciclo para calcular la temperatura de cada celda en `run_simulation()`, pero esto solo produjo ciclos infinitos.

## Optimizaciones concurrentes

| Iter. | Etiqueta | Duración (s) | Speedup | Descripción corta |
|--------------|--------------|--------------|--------------|--------------|
| - | SerialI |  | 1.00 | Versión serial final |
| 1 | Conc0 |  |  | Versión concurrente inicial (Tarea02) |
| 2 |  |  |  |  |

Versión concurrente inicial (Tarea02)

Describa aquí qué aspectos tenía la versión concurrente (Tarea02) no eficientes que podrían o fueron opitimizados luego. También aspectos que usted considera eficientes, los cuales explicarían por qué no amerita una optimización.

Iteración 1

Para cada iteración del ciclo de optimización, cree una subsección como esta. Resuma en máximo un párrafo en qué consiste la optimización. Su objetivo es que quien lea este documento entienda claramente qué se intentó mejorar. Provea trocitos de código fuente o pseudocódigo que ayude a entender la optimización potencial. Indique si el intento logró incrementar el desempeño. En caso de no lograrlo, conjeture la causa. Eso ayudaría a ahorrar tiempo a otras personas que tengan la misma inquietud.

## Comparación de optimizaciones

## Comparación del grado de concurrencia
