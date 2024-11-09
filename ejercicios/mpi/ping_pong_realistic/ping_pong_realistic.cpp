// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include <mpi.h>
#include <iostream>
#include <random>

/**
 * @brief Runs the ping pong game simulation.
 *
 * @details This function initializes MPI, sets up the game parameters, and
 * alternates between two players until one player reaches the winning score.
 * Results are communicated between processes.
 */
int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Check for correct number of arguments.
  if (argc != 4) {
    if (rank == 0) {
      std::cerr << "Usage: " << argv[0]
        << " <points_to_win> <prob_player_0> <prob_player_1>" << std::endl;
    }
    MPI_Finalize();
    return 1;
  }

  // Game parameters.
  int points_to_win = std::stoi(argv[1]);
  float player_0_accuracy = std::stof(argv[2]);
  float player_1_accuracy = std::stof(argv[3]);

  int points_count[2] = {0, 0};  ///< Array to keep score for both players.
  bool finalize_game = false;    ///< Flag to indicate if the game should end.
  int services_count = 0;        ///< Number of services.
  int rounds_count = 0;          ///< Number of rounds played.

  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 100.0);

  while (!finalize_game) {
    int winner = -1;  ///< To store the winner of each round.

    // Determine the current player based on the service count.
    if (rank == services_count % 2) {
      float win_rate = (rank == 0) ? player_0_accuracy : player_1_accuracy;
      bool player_scored = dis(gen) < win_rate;

      if (player_scored) {
        points_count[rank]++;
        winner = rank;
      }

      MPI_Send(&player_scored, 1, MPI_C_BOOL, 1 - rank, 0, MPI_COMM_WORLD);
    } else {
      bool player_scored;
      MPI_Recv(&player_scored, 1, MPI_C_BOOL, 1 - rank, 0, MPI_COMM_WORLD,
        MPI_STATUS_IGNORE);

      if (player_scored) {
        points_count[1 - rank]++;
        winner = 1 - rank;
      }
    }

    services_count++;  ///< Increment the number of services each round.

    // Only process 0 prints the round results.
    if (rank == 0 && winner != -1) {
      std::cout << rounds_count + 1 << ": " << services_count - 1 << " "
        << winner << std::endl;
      rounds_count++;
    }

    // Process 0 checks if a player has won.
    if (rank == 0 &&
      (points_count[0] >= points_to_win || points_count[1] >= points_to_win)) {
      finalize_game = true;
      int game_winner = (points_count[0] >= points_to_win) ? 0 : 1;
      int game_loser = 1 - game_winner;
      std::cout << game_winner << " wins " << points_count[game_winner]
        << " to " << points_count[game_loser] << std::endl;
    }

    // Broadcast game state and score to all processes.
    MPI_Bcast(&finalize_game, 1, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    MPI_Bcast(points_count, 2, MPI_INT, 0, MPI_COMM_WORLD);
  }

  MPI_Finalize();
  return 0;
}
