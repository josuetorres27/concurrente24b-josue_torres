## Ejemplo 5. Hola mundo ordenado con espera activa [hello_order_busywait]
Haga que los threads saluden siempre en orden. Es decir, si se crean w threads, la salida sea siempre en orden

Hello from thread 0 of w
Hello from thread 1 of w
Hello from thread 2 of w
...
Hello from thread w of w
Utilice espera activa como mecanismo de sincronización (control de concurrencia).

Ejecute varias veces su solución de hello_order_busywait con la cantidad máxima de threads que el su sistema operativo le permite. ¿Es consistente la duración de su solución con espera activa?

Describa el comportamiento del programa y explique la razón de este comportamiento en el archivo readme.md dentro de su carpeta hello_order_busywait. Indique en qué condiciones sería apremiante utilizar espera activa, o en caso de no haberlas, sugiera soluciones alternativas.