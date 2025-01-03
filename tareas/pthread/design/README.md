## Diagrama UML
![El diagrama muestra la relación entre las estructuras de datos y las funciones que las utilizan en el programa. A su vez, estas funciones crean los hilos y les permiten acceder a los datos de las estructuras. En el diagrama se muestra que los hilos apuntan hacia las estructuras SharedData y ThreadData, ya que estos modifican los datos compartidos y acceden a los datos privados para realizar su función correctamente. El flujo del programa inicia con el hilo principal, ya que este configura la simulación con los datos leídos del archivo, permitiendo que se inicialice la estructura SimData y a partir de ahí se inicialicen las demás estructuras.](UML_diagram.svg)


## Diagrama de memoria
![El diagrama representa el uso de las diferentes secciones de memoria del programa. El Code Segment almacena las instrucciones del código; el Stack Segment contiene al hilo principal y los hilos trabajadores, describiendo las funciones que estos realizan; y el Heap Segment contiene las asignaciones dinámicas de memoria que se realizan durante el programa.](Memory_diagram.svg)


## Pseudocódigo
```pseudo
// Pseudocódigo centrado en las funciones que involucran concurrencia y manejo
// de hilos


define struct SimData
  variable bin_name
  variable delta_t, alpha, h, epsilon
end

define struct SharedData
  variable data
  variable cols, rows
  variable delta_t, alpha, h, epsilon
end

define struct ThreadData
  variable start_row, end_row
  pointer shared_data
end


procedure thread_sim(data)
  initialize private_data as ThreadData
  initialize shared_data as private_data.shared_data

  allocate memory for temp_data as an array with dimensions shared_data.rows x
    shared_data.cols

  for i from start_row to end_row - 1:
    for j from 1 to shared_data.cols - 1:
      set temperature to shared_data.data[i][j]

      calculate next_temp using the heat equation formula

      store next_temp in temp_data[i][j]
    end for
  end for

  for i from private_data.start_row to private_data.end_row - 1:
    for j from 1 to shared_data.cols - 1:
      copy temp_data[i][j] to shared_data.data[i][j]
    end for
  end for

  free the memory allocated for temp_data
  return
end procedure


procedure simulate(shared_data, thread_count)
  allocate memory for threads as an array of <thread_count> elements
  allocate memory for thread_data as an array of ThreadData elements

  initialize num_states to 0
  set eq_point to false

  while eq_point is false:
    increment num_states
    set eq_point to true

    copy shared_data.data to data_local

    for i from 0 to thread_count - 1:
      calculate rows_per_thread

      assign start_row and end_row for each thread

      create thread using thread_sim
    end for

    join threads

    for i from 1 to shared_data.rows - 1:
      for j from 1 to shared_data.cols - 1:
        calculate delta as the absolute difference between data_local[i][j] and
          shared_data.data[i][j]

        if delta is greater than epsilon
          set eq_point to false
        end if
      end for
    end for
  end while

  return num_states
end procedure


procedure configure_simulation(dir, sim_params, lines, job_name, thread_count)
  allocate memory for shared_data
  allocate memory for sim_states as an array of <lines> elements

  for i from 0 to lines - 1:
    build file path for binary file
    open binary file for reading

    read rows and cols from file
    allocate memory for shared_data.data as an array with dimensions
      shared_data.rows x shared_data.cols

    for each row in shared_data.rows:
      read binary data into shared_data.data[row]
    end for

    assign shared_data.delta_t, alpha, h, and epsilon from sim_params[i]

    set num_states by calling simulate(shared_data, thread_count)
    store num_states in sim_states[i]

    call write_plate to save simulation result for shared_data.data

    free the memory allocated for shared_data.data
    close the file
  end for

  create_report

  free the memory allocated for sim_states and shared_data
end procedure
