## Ejemplo 46. Distribución del rango con argumentos [hybrid_distr_arg]

Modifique el programa del Ejemplo 45 (hello_hybrid) para que reciba un rango indicado como dos números enteros [a,b[ en los argumentos de línea de comandos. Su programa debe distribuir (particionar) el rango entre los procesos de forma lo más equitativa posible, y dentro de los procesos debe distribuir los subrangos entre los hilos de ejecución.

El hilo principal de cada proceso debe reportar en la salida estándar el rango asignado al proceso. Use la notación hostname:process.thread: message como prefijo para simplificar la salida. Cada hilo secundario debe reportar su rango asignado antecedido por un tabulador. La siguiente podría ser una interacción con su programa en dos nodos de un clúster, cada uno con un proceso por nodo, y cada nodo con tres CPU:

    $ mpiexec -n 2 -f hosts.txt bin/hybrid_distr_arg 3 20
    hostname1:0: range [3, 12[ size 9
      hostname1:0.0: range [3,6[ size 3
      hostname1:0.1: range [6,9[ size 3
      hostname1:0.2: range [9,12[ size 3
    hostname2:1: range [12, 20[ size 8
      hostname2:1.0: range [12,15[ size 3
      hostname2:1.1: range [15,18[ size 3
      hostname2:1.2: range [18,20[ size 2

Reporte además el tamaño del rango asignado a cada proceso e hilo de ejecución.