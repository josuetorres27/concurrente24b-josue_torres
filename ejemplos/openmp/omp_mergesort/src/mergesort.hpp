// Copyright 2023 Jeisson Hidalgo jeisson.hidalgo@ucr.ac.cr CC-BY-4
// Commented by Josué Torres Sibaja C37853

#include <omp.h>
#include <algorithm>
#include <vector>

/**
 * @brief Recursively sorts a subrange of a vector using the mergesort
 * algorithm.
 *
 * @details This function splits the vector into two halves, sorts them in
 * parallel using OpenMP tasks, and then merges the sorted halves. If the
 * subrange is small enough, it uses std::sort as a fallback for efficiency.
 *
 * @tparam Type Type of the elements in the vector.
 * @param values Vector to be sorted.
 * @param left Left index of the subrange to be sorted.
 * @param right Right index of the subrange to be sorted.
 */
template <typename Type>
void mergesort(std::vector<Type>& values, const ptrdiff_t left,
  const ptrdiff_t right) {
  /** Count of elements we need to sort. */
  const ptrdiff_t count = right - left;

  /** If the data set is large enough, apply mergesort. */
  if (count >= 1024) {
    const size_t mid = (left + right) / 2;

    #pragma omp taskgroup
    {
      /** Sort the left half of the array. */
      #pragma omp task default(none) untied if(count >= 5000) \
        shared(values) firstprivate(left, mid)
      mergesort(values, left, mid);

      /** Sort the right half of the array. */
      #pragma omp task default(none) untied if(count >= 5000) \
        shared(values) firstprivate(mid, right)
      mergesort(values, mid + 1, right);

      /** Yield control to other tasks, if necessary. */
      #pragma omp taskyield
    }

    /** Merge the sorted halves into a temporary vector. */
    std::vector<Type> temp;
    temp.reserve(count + 1);
    std::merge(values.begin() + left, values.begin() + mid + 1,
      values.begin() + mid + 1, values.begin() + right + 1,
        std::back_inserter(temp));

    /** Copy the sorted temp vector back into the original vector. */
    std::copy(temp.begin(), temp.end(), values.begin() + left);
  } else {
    /** If the data set is small, use std::sort for efficiency. */
    std::sort(values.begin() + left, values.begin() + right + 1);
  }
}

/**
 * @brief Sorts an entire vector using the mergesort algorithm with OpenMP
 * parallelism.
 *
 * @details Function initiates the parallel region and calls the recursive
 * mergesort function.
 *
 * @tparam Type Type of the elements in the vector.
 * @param values Vector to be sorted.
 * @param thread_count Number of threads to use for parallelization (optional,
 * default is the maximum number of available threads).
 */
template <typename Type>
void mergesort(std::vector<Type>& values, const int thread_count =
  omp_get_max_threads()) {
  #pragma omp parallel num_threads(thread_count) default(none) shared(values)
  {
    /** Only one thread initializes the sorting process. */
    #pragma omp single
    mergesort(values, 0, values.size() - 1);
  }
}
