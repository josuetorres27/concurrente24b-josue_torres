/// @copyright 2020 ECCI, Universidad de Costa Rica. All rights reserved
/// This code is released under the GNU Public License version 3
/// @author Jeisson Hidalgo-Céspedes <jeisson.hidalgo@ucr.ac.cr>

#include "ProducerTest.hpp"
#include "Log.hpp"
#include "Util.hpp"

/**
 * Constructor initializes the producer with the number of packages to produce,
 * the delay between productions, and the number of consumers in the system.
 */
ProducerTest::ProducerTest(size_t packageCount, int productorDelay,
  size_t consumerCount)
  : packageCount(packageCount)
  , productorDelay(productorDelay)
  , consumerCount(consumerCount) {
}

int ProducerTest::run() {
  // Produce each asked message
  for (size_t index = 0; index < this->packageCount; ++index) {
    /// Create a network message and send it to the queue
    this->produce(this->createMessage(index));
  }
  // Produce an empty message to communicate we finished
  this->produce(NetworkMessage());

  // Report production is done
  Log::append(Log::INFO, "Producer", std::to_string(this->packageCount)
    + " menssages sent");
  return EXIT_SUCCESS;
}

NetworkMessage ProducerTest::createMessage(size_t index) const {
  /// The message's source is always this producer (ID 1)
  const uint16_t source = 1;
  // Target is selected by random
  const uint16_t target = 1 + Util::random(0,
    static_cast<int>(this->consumerCount));
  // IMPORTANT: This simulation uses sleep() to mimics the process of
  // producing a message. However, you must NEVER use sleep() for real projects
  Util::sleepFor(this->productorDelay);
  // Create and return a copy of the network message
  return NetworkMessage(target, source, index);
}
