// Copyright 2023 Jeisson Hidalgo jeisson.hidalgo@ucr.ac.cr CC-BY-4
// Adaptado por Josué Torres Sibaja C37853

#include <omp.h>
#include <algorithm>
#include <vector>

/**
 * @brief Ordena el contenido de un vector utilizando el algoritmo de
 * ordenamiento por mezcla (mergesort).
 *
 * @tparam Type Tipo de los elementos del vector.
 * @param values Referencia al vector que se debe ordenar.
 * @param left Índice izquierdo (inicio) del subvector a ordenar.
 * @param right Índice derecho (fin) del subvector a ordenar.
 */
template <typename Type>
void mergesort(std::vector<Type>& values, const ptrdiff_t left,
  const ptrdiff_t right) {
  /** Cantidad de elementos a ordenar. */
  const ptrdiff_t count = right - left;

  /** Verificar si la cantidad de datos es adecuada para utilizar mergesort. */
  if (count >= 1024) {
    const size_t mid = (left + right) / 2;

    #pragma omp taskgroup
    {
      /** Ordenar la mitad izquierda del arreglo de forma paralela. */
      #pragma omp task default(none) untied if (count >= 7500) shared(values), \
        firstprivate(left, mid)
      mergesort(values, left, mid);

      /** Ordenar la mitad derecha del arreglo de forma paralela. */
      #pragma omp task default(none) untied if (count >= 7500) shared(values), \
        firstprivate(mid, right)
      mergesort(values, mid + 1, right);

      #pragma omp taskyield
    }

    /** Fusionar ambas mitades en un vector temporal. */
    std::vector<Type> temp;
    temp.reserve(count + 1);
    std::merge(values.begin() + left, values.begin() + mid + 1,
      values.begin() + mid + 1, values.begin() + right + 1,
        std::back_inserter(temp));

    /** Copiar el vector temporal ordenado de vuelta al vector original. */
    std::copy(temp.begin(), temp.end(), values.begin() + left);
  } else {
    /** Si hay pocos elementos, ordenar con el algoritmo estándar. */
    std::sort(values.begin() + left, values.begin() + right + 1);
  }
}
