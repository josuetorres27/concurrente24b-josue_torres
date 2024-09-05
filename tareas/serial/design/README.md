## Diagrama UML
![En el diagrama se presentan 5 módulos diferentes, de los cuales el módulo Plate, ubicado en el centro, representa la estructura de datos del mismo nombre. Los otros cuatro módulos representan los archivos/bloques de código fuente en los que se divide el programa. Las flechas entre los módulos representan las relaciones que tienen las diferentes partes del código. Por ejemplo, los módulos que incluyen al archivo "plate.h" envían una flecha hacia su módulo, y el módulo main.c envía una flecha hacia todos los otros módulos, ya que este utiliza funciones y partes de cada uno.](UML_diagram.svg)


## Diagrama de memoria
![El diagrama representa el uso de las diferentes secciones de memoria del programa. El Code Segment almacena las instrucciones del código; el Stack Segment contiene las variables locales y las direcciones de retorno; el Heap Segment contiene las asignaciones dinámicas de memoria relacionadas con la estructura Plate; y el Data Segment no se utiliza, ya que no existen variables globales.](Memory_diagram.svg)


## Pseudocódigo
```pseudo
define MAX_PATH_LENGTH = 260


define struct Plate
  variable rows
  variable cols
  variable data
end


procedure format_time(seconds, text, capacity)
  convert seconds to GMT structure
  format date and time in the format "YYYY/MM/DD HH:MM:SS"
  return text
end procedure


procedure create_report(job_file, plate_filename, delta_t, alpha, h, epsilon,
  k, time_seconds)
  declare report_filename

  copy job_file into report_filename
  if job_file ends with ".txt":
    replace ".txt" with ".tsv" in report_filename
  else:
    append ".tsv" to report_filename
  end if

  open report_filename for appending

  declare formatted_time
  call format_time with time_seconds and store result in formatted_time

  close file
end procedure


procedure read_dimensions(filepath, plate)
  file = open filepath for reading in binary mode

  read 8 bytes into plate.rows
  read 8 bytes into plate.cols

  close file
end procedure


procedure read_plate(filepath, plate)
  file = open filepath for reading in binary mode

  move file pointer forward by 16 bytes
  allocate memory for plate.data with plate.rows pointers

  for i = 0 to plate.rows - 1:
    allocate memory for plate.data[i] with plate.cols elements
    read plate.cols elements into plate.data[i]
  end for

  close file
end procedure


procedure simulate(plate, delta_t, alpha, h, epsilon, k, time_seconds)
  initialize max_delta
  allocate memory for next_plate as a 2D array with dimensions plate.rows x
    plate.cols

  set k to 0
  set time_seconds to 0

  do
    for i from 1 to plate.rows - 1
      for j from 1 to plate.cols - 1
        calculate next_plate[i][j] using the heat equation formula

        calculate delta as the absolute difference between next_plate[i][j] and
          plate.data[i][j]

        if delta is greater than max_delta:
          set max_delta to delta
        end if
      end for
    end for

    copy next_plate values to plate.data

    increment k by 1
    add delta_t to time_seconds

  while max_delta is greater than epsilon

  free the memory allocated for next_plate
end procedure


procedure write_plate(filepath, plate)
  file = open filepath for writing in binary mode

  write 8 bytes from plate.rows into file
  write 8 bytes from plate.cols into file

  for i = 0 to plate.rows - 1:
    write plate.cols elements from plate.data[i] into file
  end for

  close file
end procedure


procedure main(argc, argv)
  set arguments

  open job_file for reading

  while reading from file into data is successful:
    construct filepath

    declare plate as a Plate structure

    call read_dimensions(filepath, plate)
    call read_plate(filepath, plate)

    call simulate with plate and data

    call create_report with plate data

    construct output_filename using plate_filename and k

    call write_plate with output_filename and plate

    free memory allocated for plate data
  end while

  close file
end procedure
