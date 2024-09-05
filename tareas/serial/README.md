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


2. Se debe acceder al directorio donde se encuentran los archivos necesarios para ejecutar el programa (concurrente24b-josue_torres/tareas/serial, si el repositorio está clonado localmente). Estos son: los archivos de código fuente (.c y .h), que se encuentran dentro de la carpeta 'src', y los archivos de trabajo que se utilizarán como parámetros, que se encuentran en la carpeta 'test'.
Para esto se debe usar el comando 'cd', de esta forma:

   cd /ruta/al/directorio

La parte de '/ruta/al/directorio' se debe reemplazar con la dirección del directorio.


3. Se debe compilar el programa con el siguiente comando:

   gcc src/main.c src/utils.c src/plate.c src/plate.h -o heat_sim

Con esto se generará el archivo ejecutable del programa. En este caso se sugiere el nombre 'heat_sim', pero este puede ser cambiado al nombre que se desee.


4. Se debe ejecutar el programa con el siguiente comando:

   ./heat_sim test/job_file.txt 1 test/

Este comando ejecuta el programa, el cual recibe tres parámetros: 'job_file', el archivo que contiene las instrucciones para la simulación (se debe cambiar el nombre del archivo por el que se desee utilizar); 'thread_count', que indica el número de hilos (en esta implementación no se toma en cuenta ya que el programa es serial), y 'test', el cual es el directorio donde están los casos de prueba. Adicionalmente se debe cambiar el nombre 'heat_sim' por el que se haya escrito en el paso 3.


## Créditos
Nombre del estudiante: Josué Torres Sibaja

Carnet: C37853

Información de contacto: josue.torressibaja@ucr.ac.cr

[Diseño del programa](design/README.md)