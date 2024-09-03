## Descripción del problema
El problema consiste en desarrollar una simulación por computadora para determinar el punto de equilibrio térmico de una lámina rectangular que recibe calor constante por su borde. La lámina se divide en una matriz de celdas cuadradas, cada una representando una sección de la lámina con una temperatura específica.


Para esta simulación se emplea una fórmula matemática, la cual calcula la próxima temperatura que cada celda tendrá cuando el estado de temperatura de la lámina cambie. Esta fórmula toma en cuenta la energía que la celda actual recibe de sus inmediaciones, es decir, la suma de las temperaturas de las cuatro vecinas, la energía que la celda pierde al distribuirse hacia las celdas vecinas, el área de cada celda, el tiempo permitido entre un estado de temperatura y otro, y la difusividad térmica del material de la lámina. De esta forma, la simulación avanza por los diferentes estados, actualizando la temperatura de cada celda.


El objetivo de la simulación es encontrar el momento en el que la lámina alcanza el equilibrio térmico, es decir, cuando el cambio de temperatura en todas las celdas internas es menor que un valor mínimo establecido, el cual se denota como epsilon. Si en un estado cualquiera alguna celda tiene un cambio de temperatura mayor que epsilon, la simulación continúa; si no, se detiene y crea un reporte con los resultados.


## Manual de uso
Pasos para compilar el programa:
1. Se puede compilar el programa utilizando el compilador gcc, por lo que se debe comprobar si este ya está instalado.
Para esto se debe ejecutar el siguiente comando en una terminal de Linux:

   gcc --version

Si no está instalado, se puede instalar con los siguientes comandos:

   sudo apt update
   sudo apt install gcc


2. Se debe acceder al directorio donde se encuentran los archivos necesarios para ejecutar el programa. Estos son: los archivos de código fuente (.c y .h) los archivos de trabajo que se utilizarán como parámetros, y los archivos de lámina que se mencionan en los archivos de trabajo.
Para esto se debe usar el comando 'cd', de esta forma:

   cd /ruta/al/directorio

La parte de '/ruta/al/directorio' se debe reemplazar con la dirección del directorio.


3. Se debe compilar el programa con el siguiente comando:

   gcc -o main main.c -lm

Con esto se generará el archivo ejecutable del programa.


4. Se debe ejecutar el programa con el siguiente comando:

   ./main job_file thread_count file_directory

Este comando ejecuta el programa, el cual recibe tres parámetros, los cuales son: 'job_file', el archivo que contiene las instrucciones para la simulación; 'thread_count', que indica el número de hilos, y 'file_directory', que especifica el directorio donde están los archivos de la lámina. Los dos últimos parámetros son opcionales, ya que en este momento el programa tiene una implementación serial y no necesita que se especifique la cantidad de hilos de ejecución, y si no se especifica el directorio, los archivos se buscan en el directorio actual.


## Créditos
Nombre del estudiante: Josué Torres Sibaja
Carnet: C37853
Información de contacto: josue.torressibaja@ucr.ac.cr