### Glosario

* Programación serial: secuencia, línea de ensamblaje, pasos hechos en serie, monohilo.

* Programación concurrente: se realizan varias acciones por unidad de tiempo. Similar a multitasking.

* Programación paralela: tipo de programación donde múltiples tareas o partes de un programa se ejecutan al mismo tiempo en varios procesadores o núcleos, para resolver un problema más rápido.

* Concurrencia de tareas: capacidad de un programa de manejar múltiples tareas a la vez, aunque no necesariamente se estén ejecutando simultáneamente (pueden alternarse en el tiempo).

* Paralelismo de datos: dividir un conjunto de datos en partes más pequeñas, de modo que varios procesadores puedan trabajar en ellas al mismo tiempo, acelerando el procesamiento.

* Recursos compartidos: gran volumen de recursos que es compartido entre varios actores. Hay un orden para acceder a ellos. Hay más competencia. Algunos recursos que se pueden compartir son: CPU, memoria, dispositivos periféricos, red, tiempo de atención del sistema operativo, etc.

* Recursos distribuidos: los recursos están distribuidos entre todos los actores, cada uno tiene un poco. Los recursos no están acumulados en un solo lugar. No hay competencia.

* Cores (núcleos): solo ejecutan una cosa a la vez, son la cantidad de procesos que se pueden ejecutar concurrentemente al mismo tiempo.

* Procesos: instancias de programas "vivos" (que suceden en este momento). Generalmente no les interesa lo que hacen los demás programas, son independientes y no comparten recursos. Son unidades administrativas de los programas. Tienen su propio code segment, data segment, etc.

* Hilo de ejecución: arreglo de valores que el SO carga en los registros del CPU y permiten ejecutar código en ella. Son parecidos a los procesos, pero interdependientes. Dependen de algún otro hilo o de un proceso padre, no pueden auto crearse o reproducirse sin el sistema operativo o sin un hilo padre. Si el hilo padre muere los hilos hijos también. Se relacionan con los recursos compartidos. Comparten los mismos espacios de memoria (son memoria compartida), por lo que pueden modificar los datos de otros hilos que no tienen nada que ver.

* Indeterminismo: falta de conocimiento acerca de cuál hilo se va a ejecutar primero, o cuál hilo va a terminar primero. Se ejecutan hilos de un mismo proceso, pero no se sabe cuál se va a ejecutar (va a comenzar) primero. El programador no lo controla. Es imposible de evitar.

* Memoria privada: espacio de memoria que es asignado exclusivamente a un proceso o hilo, y al que no se puede acceder directamente desde otros procesos o hilos sin pasar por el sistema operativo. Esta memoria es protegida para evitar interferencias no autorizadas, asegurando que los datos del proceso sean seguros y estén aislados.

* Memoria compartida: área de memoria que puede ser accedida por múltiples procesos o hilos al mismo tiempo. Se utiliza para facilitar la comunicación entre procesos, pero también introduce riesgos de condiciones de carrera, ya que varios hilos pueden intentar leer o modificar la memoria simultáneamente.

* Espera activa: técnica en la que un hilo de ejecución entra en un bucle repetitivo, revisando constantemente si una condición ha cambiado, en lugar de entrar en un estado de suspensión o bloqueo. Tiene la desventaja de ser ineficiente, ya que consume tiempo de CPU mientras el hilo espera, lo que puede reducir el rendimiento general del sistema.

* Condición de carrera: todos los hilos necesitan un mismo recurso pero no todos pueden acceder a él al mismo tiempo. Si el acceso no está correctamente controlado, puede resultar en un comportamiento impredecible o errores. Se producen típicamente cuando varios hilos compiten por modificar o leer el mismo recurso en memoria compartida, sin que haya mecanismos de exclusión mutua o sincronización para que el acceso sea seguro.

* Control de concurrencia: técnicas y mecanismos que se emplean para asegurar que las tareas concurrentes no interfieran entre sí de manera indebida, evitando problemas como las condiciones de carrera.

* Seguridad condicional: mecanismo que permite que los hilos esperen a que ocurra una condición específica antes de continuar. Es útil en situaciones donde un hilo debe esperar a que otro hilo complete una tarea antes de proceder.

* Exclusión mutua / Mutex (mutual exclusion): mecanismo para asegurar que solo un hilo o proceso pueda acceder a un recurso compartido en un momento dado. Estructura que ayuda a que, independientemente de la cantidad de hilos que se ejecutan, todos llegan a un embudo que los limita a pasar por un cuello de botella, el cual debe ser lo más corto posible. Va a dejar pasar los hilos uno por uno. Va en la parte de memoria compartida para que todos tengan acceso a él. Lock --> wait. Unlock --> signal.

* Semáforo: entero con signo, con tres características:
1. Cuando se crea, éste se inicializa con un entero cualquiera (negativo, cero, o positivo), pero después de inicializado las únicas dos operaciones que están permitidas es incrementar en uno (signal) y decrementar en uno (wait) al semáforo. No se puede leer el valor actual del semáforo.
2. Cuando un hilo decrementa un semáforo, si el resultado es negativo, el hilo es bloqueado y no puede continuar hasta que otro hilo incremente el semáforo.
3. Cuando un hilo incrementa un semáforo, si hay otros threads esperando, uno de ellos será desbloqueado. Tanto el hilo que incrementa el semáforo como el que fue desbloqueado siguen ejecutándose concurrentemente. Si hay varios hilos esperando, no hay forma de saber cuál de ellos será el desbloqueado por el scheduler del sistema operativo. El programador no tiene forma de saber si al incrementar un semáforo, se desbloqueará o no un hilo en espera, dado que no se puede leer el valor actual del semáforo por la regla 1.

* Barrera:

* Variable de condición:

* Candado de lectura y escritura:

* Descomposición: en un problema, la descomposición es identificar las tareas o unidades de trabajo que se pueden realizar de forma independiente, y por lo tanto, de forma paralela. Una vez definidas, las tareas se consideran unidades indivisibles de computación.

* Mapeo: consiste en asignar las unidades de trabajo hechas en la descomposición a los ejecutantes (procesos o hilos de ejecución).

* Incremento de velocidad / speedup / Aceleración: métrica de qué tanto rendimiento se gana al dividir las tareas en varias unidades de procesamiento. Se calcula como la relación entre el tiempo que tarda una computación previa a la optimización (Tbefore), contra el tiempo que tarda la misma computación posterior a la optimización (Tafter).

* Comunicación punto a punto entre procesos:

* Comunicación colectiva entre procesos:

* Reducción:

* Memoria de código: espacio de memoria donde el código (instrucciones) se copia ya codificado en lenguaje máquina.

* Memoria de datos: memoria estática, variables globales del programa, variables constantes.

* Stack frame: sección del stack dedicada a un llamado de función en particular.

* PC: program counter.

* RSP: stack pointer.

* Assert: biblioteca de pruebas unitarias, indica si la memoria es válida o no. Si no es válida detiene el programa.

* Buffer: es como un sistema de correos, en el que el productor deja mensajes dentro del buffer para que el consumidor los lea en cuanto tenga tiempo. Es como un buzón en el que se dejan mensajes que no requieren comunicación directa ni respuesta inmediata. Espacio de memoria donde algunos procesos pueden poner mensajes que otro proceso va a consumir eventualmente. El buffer acotado tiene un tamaño fijo, el no acotado no tiene restricciones de tamaño.