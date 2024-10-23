// Copyright 2023 Jeisson Hidalgo jeisson.hidalgo@ucr.ac.cr CC-BY-4
// Commented by Josué Torres Sibaja C37853

#include <omp.h>
#include <cassert>
#include <iostream>
#include <random>

#include "mergesort.hpp"

/**
 * @brief Overload of the << operator to print vectors.
 *
 * @tparam Type Type of the elements in the vector.
 * @param out Output stream.
 * @param values Vector to be printed.
 * @return std::ostream& Reference to the output stream for chaining.
 */
template <typename Type>
std::ostream& operator<<(std::ostream& out, const std::vector<Type>& values) {
  for (size_t index = 0; index < values.size(); ++index) {
    out << (index ? " " : "") << values[index];
  }
  return out;
}

/**
 * @brief Checks if a vector is sorted in ascending order.
 *
 * @tparam Type Type of the elements in the vector.
 * @param values Vector to be checked.
 * @return true If the vector is sorted, false if the vector is not sorted.
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
 * @brief Fills a vector with random integer values.
 *
 * @param count Number of random values to generate.
 * @param values Vector to be filled with random integers.
 */
void random_fill(const size_t count, std::vector<int>& values) {
  /** See https://en.cppreference.com/w/cpp/algorithm/merge. */
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<> dis(0, count);

  values.reserve(count);
  for (size_t index = 0; index < count; ++index) {
    values.push_back(dis(mt));
  }
}

/**
 * @brief Main function to execute the mergesort algorithm.
 *
 * @details Command line arguments:
 * - argv[1]: Number of random elements to generate (optional).
 * - argv[2]: Number of threads to use (optional).
 * - argv[3]: Verbosity flag, 1 for verbose output, 0 for silent (optional).
 *
 * @param argc Number of arguments passed from the command line.
 * @param argv Array of arguments passed from the command line.
 * @return int Exit status.
 */
int main(int argc, char* argv[]) {
  /** Analyze arguents. */
  const size_t count = argc >= 2 ? std::stoull(argv[1]) : 0;
  const int thread_count = argc >= 3 ? atoi(argv[2]) : omp_get_max_threads();
  const bool verbose = argc >= 4 ? std::stoi(argv[3]) : 1;

  /** Fill values array. */
  std::vector<int> values;
  if (count == 0) {
    /** No count of random elements, read from stdin. */
    int value = 0;
    while (std::cin >> value) {
      values.push_back(value);
    }
  } else {
    /** Fill vector with random values. */
    random_fill(count, values);
    /** If verbose mode, print the random vector. */
    if (argc >= 4 && std::stoi(argv[3]) == 1) {
      std::cout << values << std::endl;
    }
  }

  /** Perform mergesort on the values. */
  mergesort(values, thread_count);
  /** Print sorted values, unless verbose mode is disabled. */
  if (verbose) {
    std::cout << values << std::endl;
  }

  /** Assert that the vector is correctly sorted. */
  assert(is_sorted(values));
}
