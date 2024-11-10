// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

/**
 * @file relay_race_dist.cpp
 * @brief Simulates a relay race using MPI, where each team consists of two
 * processes, each handling a stage of the relay. Results are collected and
 * displayed.
 *
 * @note The program can be compiled with (in case Makefile or other
 * compilation commands do not work):
 *
 * mpicxx -o relay_race_dist relay_race_dist.cpp
 * mpiexec --oversubscribe -np 10 ./relay_race_dist 1100 900
 *
 */

#include <mpi.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <iomanip>
#include <algorithm>

#define fail(msg) throw std::runtime_error(msg)

/**
 * @brief Simulates the relay race with two stages.
 *
 * @details Each process represents a part of a team, alternating between stage
 * 1 and stage 2. Team completion times are collected and ranked.
 *
 * @param process_number Current process number in the MPI communicator.
 * @param process_count Total number of processes in the MPI communicator.
 */
void simulate_relay_race(int argc, char* argv[], int process_number,
  int process_count);

/**
 * @brief Executes stage 1 of the relay race for the specified team.
 *
 * @details This function simulates the delay for stage 1 and signals the next
 * process to start stage 2.
 *
 * @param stage1_delay Delay in milliseconds for stage 1.
 * @param process_number Current process number in the MPI communicator.
 * @param team_number Number of the team executing this stage.
 */
void run_stage1(int stage1_delay, int process_number, int team_number);

/**
 * @brief Executes stage 2 of the relay race for the specified team.
 *
 * @details This function waits for a signal from stage 1 and then simulates
 * the delay for stage 2.
 *
 * @param stage2_delay Delay in milliseconds for stage 2.
 * @param process_number Current process number in the MPI communicator.
 * @param team_number Number of the team executing this stage.
 */
void run_stage2(int stage2_delay, int process_number, int team_number);

/**
 * @brief Main function initializing MPI and simulating the relay race.
 */
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  if (MPI_Init(&argc, &argv) == MPI_SUCCESS) {
    try {
      int process_number = -1;
      MPI_Comm_rank(MPI_COMM_WORLD, &process_number);

      int process_count = -1;
      MPI_Comm_size(MPI_COMM_WORLD, &process_count);

      simulate_relay_race(argc, argv, process_number, process_count);
    } catch (const std::runtime_error& exception) {
      if (exception.what()) {
        std::cerr << exception.what() << std::endl;
      }
      error = EXIT_FAILURE;
    }
    MPI_Finalize();
  } else {
    std::cerr << "Error: could not init MPI" << std::endl;
    error = EXIT_FAILURE;
  }
  return error;
}

void simulate_relay_race(int argc, char* argv[], int process_number,
  int process_count) {
  if (argc != 3) {
    fail("Usage: relay_race_dist <stage1_delay> <stage2_delay>");
  }
  if (process_count < 2 || process_count % 2 != 0) {
    fail("Error: must use an even number of processes, at least 2");
  }

  const int team_count = process_count / 2;
  const int stage1_delay = atoi(argv[1]);
  const int stage2_delay = atoi(argv[2]);

  const int team_number = process_number / 2;

  double start_time = MPI_Wtime();

  if (process_number % 2 == 0) {
    run_stage1(stage1_delay, process_number, team_number);
  } else {
    run_stage2(stage2_delay, process_number, team_number);
  }

  if (process_number % 2 != 0) {
    double end_time = MPI_Wtime();
    double total_time = end_time - start_time;
    MPI_Send(&total_time, 1, MPI_DOUBLE, 0, team_number, MPI_COMM_WORLD);
  }

  if (process_number == 0) {
    std::vector<std::pair<int, double>> results;

    for (int i = 0; i < team_count; ++i) {
      double team_time;
      MPI_Recv(&team_time, 1, MPI_DOUBLE, 2 * i + 1, i, MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);
      results.emplace_back(i + 1, team_time);
    }

    std::sort(results.begin(), results.end(),
      [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
      return a.second < b.second;
    });

    std::cout << std::fixed << std::setprecision(5);
    for (size_t i = 0; i < results.size(); ++i) {
      std::cout << "Place " << i + 1 << ": team " << results[i].first
        << " in " << results[i].second << "s" << std::endl;
    }
  }
}

void run_stage1(int stage1_delay, int process_number, int team_number) {
  usleep(1000 * stage1_delay);
  MPI_Send(nullptr, 0, MPI_BYTE, process_number + 1, team_number,
    MPI_COMM_WORLD);
}

void run_stage2(int stage2_delay, int process_number, int team_number) {
  MPI_Recv(nullptr, 0, MPI_BYTE, process_number - 1, team_number,
    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  usleep(1000 * stage2_delay);
}
