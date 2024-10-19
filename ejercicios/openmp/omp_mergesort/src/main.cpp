// Copyright 2023 Jeisson Hidalgo jeisson.hidalgo@ucr.ac.cr CC-BY-4
// Adaptado por Josué Torres Sibaja C37853

#include <omp.h>
#include <cassert>
#include <iostream>
#include <random>

#include "mergesort.hpp"

/**
 * @brief Función sobrecargada de mergesort para ordenar un vector completo.
 *
 * @tparam Type Tipo de los elementos del vector.
 * @param values Referencia al vector que se debe ordenar.
 */
template <typename Type>
void mergesort(std::vector<Type>& values) {
  #pragma omp parallel default(none) shared(values)
  #pragma omp single
  mergesort(values, 0, values.size() - 1);
}

/**
 * @brief Sobrecarga del operador de salida para imprimir los elementos de un
 * vector.
 *
 * @tparam Type Tipo de los elementos del vector.
 * @param out Flujo de salida.
 * @param values Vector cuyos elementos se imprimirán.
 * @return Referencia al flujo de salida.
 */
template <typename Type>
std::ostream& operator << (std::ostream& out,
  const std::vector<Type>& values) {
  for (size_t index = 0; index < values.size(); ++index) {
    out << (index ? " " : "") << values[index];
  }
  return out;
}

/**
 * @brief Verifica si un vector está ordenado en orden ascendente.
 *
 * @tparam Type Tipo de los elementos del vector.
 * @param values Vector que se verificará.
 * @return true Si el vector está ordenado.
 * @return false Si el vector no está ordenado.
 */
template <typename Type>
bool is_sorted(const std::vector<Type>& values) {
  for (size_t index = 1; index < values.size(); ++index) {
    if (values[index] < values[index - 1]) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Llena un vector con números enteros aleatorios.
 *
 * @param count Cantidad de números aleatorios a generar.
 * @param values Vector donde se almacenarán los números generados.
 */
void random_fill(const size_t count, std::vector<int>& values) {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dis(0, count);

  values.reserve(count);
  for (size_t index = 0; index < count; ++index) {
    values.push_back(dis(mt));
  }
}

/**
 * @brief Función principal del programa.
 *
 * @param argc Número de argumentos pasados al programa.
 * @param argv Argumentos pasados al programa.
 * @return int Código de salida del programa.
 */
int main(int argc, char* argv[]) {
  /** Analizar los argumentos del programa. */
  const size_t count = argc >= 2 ? std::stoull(argv[1]) : 0;
  const bool verbose = argc >= 4 ? std::stoi(argv[3]) : 1;

  /** Llenar el vector de valores. */
  std::vector<int> values;
  if (count == 0) {
    int value = 0;
    while (std::cin >> value) {
      values.push_back(value);
    }
  } else {
    /** Llenar el vector con valores aleatorios. */
    random_fill(count, values);
    /** Si el modo verbose está activado, imprimir el vector aleatorio. */
    if (argc >= 4 && std::stoi(argv[3]) == 1) {
      std::cout << values << std::endl;
    }
  }

  /** Ordenar el vector utilizando mergesort. */
  mergesort(values);

  /**
   * Imprimir los valores ordenados, a menos que el modo verbose esté
   * desactivado.
   */
  if (verbose) {
    std::cout << values << std::endl;
  }

  /** Verificar que el vector esté ordenado. */
  assert(is_sorted(values));
}
