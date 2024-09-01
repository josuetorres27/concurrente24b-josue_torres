## Ejemplo 5. Hola mundo ordenado con espera activa [hello_order_busywait]
Haga que los threads saluden siempre en orden. Es decir, si se crean w threads, la salida sea siempre en orden

Hello from thread 0 of w
Hello from thread 1 of w
Hello from thread 2 of w
...
Hello from thread w of w
Utilice espera activa como mecanismo de sincronización (control de concurrencia).

# Ejecute varias veces su solución de hello_order_busywait con la cantidad máxima de threads que el su sistema operativo le permite. ¿Es consistente la duración de su solución con espera activa?
No, la duración no fue totalmente consistente o constante tras varias ejecuciones.

# Describa el comportamiento del programa y explique la razón de este comportamiento en el archivo readme.md dentro de su carpeta hello_order_busywait. Indique en qué condiciones sería apremiante utilizar espera activa, o en caso de no haberlas, sugiera soluciones alternativas.
El programa crea un cierto número de hilos, los cuales imprimen un mensaje de saludo en orden secuencial (desde el hilo 0 hasta el último). Para mantener el orden, los hilos utilizan un mecanismo de espera activa, donde cada hilo espera en un bucle hasta que next_thread tenga un valor igual a su número de hilo.

La razón de este comportamiento es que la espera activa hace que cada hilo esté comprobando constantemente si es su turno para imprimir, lo cual resulta en un consumo innecesario de CPU, ya que los hilos no realizan ninguna otra tarea mientras esperan. Esto hace que el tiempo de ejecución varíe debido a la carga del sistema.

Una solución alternativa puede ser el uso de semáforos, los cuales pueden controlar el número de hilos que acceden a un recurso compartido o permiten que un hilo se despierte cuando sea su turno.