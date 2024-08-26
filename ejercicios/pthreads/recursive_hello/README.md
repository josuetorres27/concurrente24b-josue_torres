## Ejercicio 4 [recursive_hello]

* Copie su ejemplos/pthreads/hello a ejercicios/pthreads/recursive_hello. Puede renombrar ejercicios/pthreads/recursive_hello/src/hello.c a recursive_hello.c o si lo prefiere, main.c.

* Modifique a greet() para recibir un número de tipo size_t. Si este número es 0, greet() imprime una despedida y el número recibido. Luego termina su ejecución.

* Si el número que greet() recibe es mayor que cero, imprime un saludo y el número recibido. Luego crea un hilo para que invoque a greet() pero con un número menor que el recibido por parámetro.

* Modifique el procedimiento main() para crear una variable local de tipo size_t inicializada en 2. Envíe este número como parámetro de la subrutina greet() que correrá en el hilo secundario.

### Predicción de la salida del programa:

Hello from main thread
Hello from secondary thread. Received number: 2
Hello from secondary thread. Received number: 1
Goodbye! Received number: 0

### Salida real del programa:

Hello from main thread
Hello from secondary thread. Received number: 2
Hello from secondary thread. Received number: 1
Goodbye! Received number: 0

### La predicción fue acertada