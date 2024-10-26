## Ejercicio en clase (25/10/2024) [hello_task_sep]

Cree un programa con MPI que lance 2 procesos.
El proceso 0 imprime un mensaje. El proceso 1 imprime los números del 0 al 10.
El programa debe validar que se lancen 2 procesos únicamente.

### Manual de usuario para instalación y compilación en MPI (Ubuntu/Linux)
### 1. Instalar Open MPI
Para instalar Open MPI en Ubuntu, se debe abrir la terminal y ejecutar:

    sudo apt update
    sudo apt install openmpi-bin libopenmpi-dev

### 2. Compilar el programa MPI
Una vez instalado Open MPI, se debe utilizar mpic++ para compilar el código:

    mpic++ -o hello_task_sep hello_task_sep.cpp

### 3. Ejecutar el programa con MPI
Ejecutar el programa con exactamente 2 procesos:

    mpirun -np 2 ./hello_task_sep

### 4. Notas importantes
Se puede verificar que Open MPI está correctamente instalado ejecutando mpirun --version.

La cantidad de procesos (-np 2) debe coincidir con lo que espera el programa para que funcione correctamente (2 procesos).