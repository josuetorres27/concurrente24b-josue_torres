### Glosario

* Programación serial: secuencia, línea de ensamblaje, pasos hechos en serie, monohilo.

* Programación concurrente: se realizan varias acciones por unidad de tiempo. Similar a multitasking.

* Programación paralela:

* Concurrencia de tareas:

* Paralelismo de datos:

* Recursos compartidos: gran volumen de recursos que es compartido entre varios actores. Hay un orden para acceder a ellos. Hay más competencia. Algunos recursos que se pueden compartir son: CPU, memoria, dispositivos periféricos, red, tiempo de atención del sistema operativo, etc.

* Recursos distribuidos: los recursos están distribuidos entre todos los actores, cada uno tiene un poco. Los recursos no están acumulados en un solo lugar. No hay competencia.

* Cores (núcleos): solo ejecutan una cosa a la vez, son la cantidad de procesos que se pueden ejecutar concurrentemente al mismo tiempo.

* Procesos: instancias de programas "vivos" (que suceden en este momento). Generalmente no les interesa lo que hacen los demás programas, son independientes y no comparten recursos. Son unidades administrativas de los programas. Tienen su propio code segment, data segment, etc.

* Hilo de ejecución: arreglo de valores que el SO carga en los registros del CPU y permiten ejecutar código en ella. Son parecidos a los procesos, pero interdependientes. Dependen de algún otro hilo o de un proceso padre, no pueden auto crearse o reproducirse sin el sistema operativo o sin un hilo padre. Si el hilo padre muere los hilos hijos también. Se relacionan con los recursos compartidos. Comparten los mismos espacios de memoria (son memoria compartida), por lo que pueden modificar los datos de otros hilos que no tienen nada que ver.

* Indeterminismo: falta de conocimiento acerca de cuál hilo se va a ejecutar primero, o cuál hilo va a terminar primero. Se ejecutan hilos de un mismo proceso, pero no se sabe cuál se va a ejecutar (va a comenzar) primero. El programador no lo controla.

* Memoria privada: espacio de memoria al que nada accede sin permiso del SO.

* Memoria compartida:

* Espera activa: ciclo que hace a un hilo de ejecución esperar repetitivamente hasta que una condición se haga falsa.

* Condición de carrera:

* Control de concurrencia:

* Seguridad condicional:

* Exclusión mutua:

* Semáforo:

* Barrera:

* Variable de condición:

* Candado de lectura y escritura:

* Descomposición:

* Mapeo:

* Incremento de velocidad:

* Comunicación punto a punto entre procesos:

* Comunicación colectiva entre procesos:

* Reducción:

* Memoria de código: espacio de memoria donde el código (instrucciones) se copia ya codificado en lenguaje máquina.

* Memoria de datos: memoria estática, variables globales del programa, variables constantes.

* Stack frame: sección del stack dedicada a un llamado de función en particular.

* PC: program counter.

* RSP: stack pointer.
