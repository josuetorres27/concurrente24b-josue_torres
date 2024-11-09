## Ejercicio 66 [ping_pong_realistic]

Un ejercicio clásico de paso de mensajes es simular un juego de tenis de mesa (ping-pong) entre dos procesos. Uno lanza la bola al otro, quien la recibe y la regresa al primero, y así sucesivamente. Los dos jugadores son incansables y nunca pierden un servicio. Haga que su programa simule este comportamiento. Si su programa es invocado con una cantidad distinta a dos jugadores, debe reportar un mensaje de error y finalizar.

Haga que cada proceso imprima en la salida estándar un mensaje cuando hace un servicio. Puede permitir que el usuario especifique un segundo argumento de línea de comandos para establecer una espera en milisegundos que tarda un jugador en servir desde que recibe la bola. Esto puede ayudar a hacer más legible la salida disminuyendo el indeterminismo. El siguiente podría ser un ejemplo de ejecución hipotético.

    bin/ping_pong_perfect 2 500
    0 serves
    1 serves
    0 serves
    1 serves
    ^C

Adapte su solución al ejercicio anterior para que sea más realista. En el primer argumento de línea de comandos, el usuario indicará el marcador de victoria de la partida. Por ejemplo, si se indica 10, el primer jugador que anote 10 goles ganará la partida y el juego termina.

Los jugadores en esta versión no son perfectos. El segundo y tercer argumento de línea de comandos indica la probabilidad de acierto en cada servicio (tiro o pase) del primer y segundo jugador respectivamente. Por ejemplo, la siguiente ejecución indica que el jugador 1 (proceso 0) tiene una tasa de acierto del 85.5% de los tiros y el jugador 2 de un 88%.

    mpiexec -n 2 ping_pong 3 85.5 88
    1: 24 1
    2: 53 0
    3: 11 0
    0 wins 2 to 1

En la salida estándar indique una línea por cada ronda y una línea final con el resultado. En cada ronda se indica la cantidad de servicios (pases) jugados y el proceso que la ganó. La última línea indica el marcador final o si hubo un empate.