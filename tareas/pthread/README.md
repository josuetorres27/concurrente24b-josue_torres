## Descripción del problema
El problema consiste en desarrollar una simulación por computadora para determinar el punto de equilibrio térmico de una lámina rectangular que recibe calor constante por su borde. La lámina se divide en una matriz de celdas cuadradas, cada una representando una sección de la lámina con una temperatura específica.


Para esta simulación se emplea una fórmula matemática, la cual calcula la próxima temperatura que cada celda tendrá cuando el estado de temperatura de la lámina cambie. Esta fórmula toma en cuenta la energía que la celda actual recibe de sus inmediaciones, es decir, la suma de las temperaturas de las cuatro vecinas, la energía que la celda pierde al distribuirse hacia las celdas vecinas, el área de cada celda, el tiempo permitido entre un estado de temperatura y otro, y la difusividad térmica del material de la lámina. De esta forma, la simulación avanza por los diferentes estados, actualizando la temperatura de cada celda.


El objetivo de la simulación es encontrar el momento en el que la lámina alcanza el equilibrio térmico, es decir, cuando el cambio de temperatura en todas las celdas internas es menor que un valor mínimo establecido, el cual se denota como epsilon. Si en un estado cualquiera alguna celda tiene un cambio de temperatura mayor que epsilon, la simulación continúa; si no, se detiene y crea un reporte con los resultados.


## Manual de uso
Pasos para compilar el programa:
1. Se puede compilar el programa utilizando el Makefile general. Se debe acceder al directorio raíz de la tarea (tareas/pthread) y ejecutar los siguientes comandos:

   make clean
   make
   bin/pthread [thread_count] <job_file.txt> <input_dir/>

De esta forma, el programa se debería ejecutar de forma correcta utilizando los parámetros personalizados. Es posible que al correr simulaciones muy grandes el programa tarde algunos minutos en terminar, por lo que se recomienda utilizar bastantes hilos. Los archivos de salida se generarán en el directorio especificado en <input_dir>.


## Créditos
Nombre del estudiante: Josué Torres Sibaja

Carnet: C37853

Información de contacto: josue.torressibaja@ucr.ac.cr


### Enlace al diseño del programa
[Diseño del programa](design/README.md)
