/// @copyright 2020 ECCI, Universidad de Costa Rica. All rights reserved
/// This code is released under the GNU Public License version 3
/// @author Jeisson Hidalgo-Céspedes <jeisson.hidalgo@ucr.ac.cr>

#ifndef ASSEMBLERTEST_HPP
#define ASSEMBLERTEST_HPP

#include "Assembler.hpp"
#include "NetworkMessage.hpp"

/// Example of the AssemblerTest class
class AssemblerTest : public Assembler<NetworkMessage, NetworkMessage> {
  DISABLE_COPY(AssemblerTest);

 protected:
  double packageLossPercent = 0.0;  /// Percentage of lost packages
  size_t consumerCount = 0;  /// Number of consumers in the network
  size_t packageLossCount = 0;  /// Number of messages that were lost

 public:
  /// Constructor
  explicit AssemblerTest(const double packageLossPercent,
    const size_t consumerCount);
  /// Consume the messages in its own execution thread
  int run() override;
  /// Override this method to process any data extracted from the queue
  void consume(NetworkMessage data) override;
};

#endif  // ASSEMBLERTEST_HPP
