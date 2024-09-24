/// @copyright 2020 ECCI, Universidad de Costa Rica. All rights reserved
/// This code is released under the GNU Public License version 3
/// @author Jeisson Hidalgo-Céspedes <jeisson.hidalgo@ucr.ac.cr>

#include <cstdlib>
#include <iostream>

#include "ProducerConsumerTest.hpp"
#include "AssemblerTest.hpp"
#include "ConsumerTest.hpp"
#include "DispatcherTest.hpp"
#include "ProducerTest.hpp"

const char* const usage =
  "Usage: <prodcons> <packages> <consums> <prod_del> <disp_del> <cons_del>\n"
  "\n"
  "  packages     number of packages to be produced\n"
  "  consums      number of consumer threads\n"
  "  prod_del     delay of producer to create a package\n"
  "  disp_del     delay of dispatcher to dispatch a package\n"
  "  cons_del     delay of consumer to consume a package\n"
  "  loss_perc    percent of loss package loss (0-100)\n"
  "\n"
  "Delays are in millisenconds, negatives are maximums for random delays\n";

ProducerConsumerTest::~ProducerConsumerTest() {
  /**
   * Clean up dynamically allocated memory for producer, dispatcher, consumers,
   * and assembler.
   */
  delete this->producer;
  delete this->dispatcher;
  for (ConsumerTest* consumer : this->consumers)
    delete consumer;
  delete this->assembler;
}

int ProducerConsumerTest::start(int argc, char* argv[]) {
  // Parse arguments and store values in object members
  if ( int error = this->analyzeArguments(argc, argv) ) {
    return error;
  }
  // Create objects for the simulation
  this->producer = new ProducerTest(this->packageCount, this->productorDelay,
    this->consumerCount + 1);
  /// Create the dispatcher with a delay and initialize its queue
  this->dispatcher = new DispatcherTest(this->dispatcherDelay);
  this->dispatcher->createOwnQueue();
  // Create each producer
  this->consumers.resize(this->consumerCount);
  for ( size_t index = 0; index < this->consumerCount; ++index ) {
    this->consumers[index] = new ConsumerTest(this->consumerDelay);
    assert(this->consumers[index]);
    /// Initialize each consumer's queue
    this->consumers[index]->createOwnQueue();
  }
  this->assembler = new AssemblerTest(this->packageLossPercent,
    this->consumerCount);
  this->assembler->createOwnQueue();
  // Communicate simulation objects
  // Producer push network messages to the dispatcher queue
  this->producer->setProducingQueue(this->dispatcher->getConsumingQueue());
  // Dispatcher delivers to each consumer, and they should be registered
  for ( size_t index = 0; index < this->consumerCount; ++index ) {
    this->dispatcher->registerRedirect(index + 1,
      this->consumers[index]->getConsumingQueue());
  }
  /// Register the assembler in the dispatcher
  this->dispatcher->registerRedirect(this->consumerCount + 1,
    this->assembler->getConsumingQueue());
  this->assembler->setProducingQueue(this->dispatcher->getConsumingQueue());

  // Start the simulation
  this->producer->startThread();
  this->dispatcher->startThread();
  for ( size_t index = 0; index < this->consumerCount; ++index ) {
    this->consumers[index]->startThread();
  }
  this->assembler->startThread();

  // Wait for objets to finish the simulation
  this->producer->waitToFinish();
  this->dispatcher->waitToFinish();
  for ( size_t index = 0; index < this->consumerCount; ++index ) {
    this->consumers[index]->waitToFinish();
  }
  this->assembler->waitToFinish();

  // Simulation finished
  return EXIT_SUCCESS;
}

int ProducerConsumerTest::analyzeArguments(int argc, char* argv[]) {
  /// Verify that the correct number of arguments are provided
  if (argc != 7) {
    std::cout << usage;
    return EXIT_FAILURE;
  }
  /// Parse and store each argument in corresponding variables
  int index = 1;
  this->packageCount = std::strtoull(argv[index++], nullptr, 10);
  this->consumerCount = std::strtoull(argv[index++], nullptr, 10);
  this->productorDelay = std::atoi(argv[index++]);
  this->dispatcherDelay = std::atoi(argv[index++]);
  this->consumerDelay = std::atoi(argv[index++]);
  this->packageLossPercent = std::atof(argv[index++]);

  // todo: Validate that given arguments are fine
  return EXIT_SUCCESS;
}
