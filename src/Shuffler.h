//
//      File     : Shuffler.h
//      Abstract : Shuffler classes.
//

#ifndef SHUFFLER_H
#define SHUFFLER_H

#include <csignal>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <random>
#include <termios.h>
#include <vector>

using Deck = std::vector<unsigned int>;
class ShuffleThread;

//      Class    : Shuffler
//      Abstract : Class for shuffling a deck of n cards.
class Shuffler {
public:
  Shuffler(size_t n,
           size_t m,
           size_t maxTrials,
           size_t numThreads,
           unsigned int seed,
           bool stdShuff);
  Shuffler(size_t n, size_t m) : Shuffler(n, m, SIZE_MAX,
                                          1, 0xdeadbeef,
                                          false) {};
  Shuffler() : Shuffler(52, 12) {};
  ~Shuffler() {
    // std::cout << "Enabling ECHOCTL." << std::endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &_start);
  }; // DTOR

  Shuffler(const Shuffler &) = delete; // Copy CTOR
  Shuffler &operator=(const Shuffler &) = delete; // Copy assignment
  Shuffler(Shuffler &&) = delete; // Move CTOR
  Shuffler &operator=(Shuffler &&) = delete; // Move assignment

  void computeProbs();
  void simulate();
  void accumulate(ShuffleThread &thread);

 private:
  friend class ShuffleThread;

  bool checkIntr();
  void printStats();

  // Data
  size_t _n;
  size_t _m;
  size_t _maxTrials;
  size_t _numThreads;
  std::default_random_engine _prng;

  std::vector<double> _probability;

  Deck _deck;
  size_t _trials;
  std::mutex _countMutex;
  std::vector<size_t> _counts;
  bool _stdShuff;

  struct termios _start;
  struct termios _update;
}; // Shuffler


//      Class    : ShuffleThread
//      Abstract : One shuffle thread.
class ShuffleThread {
public:
  ShuffleThread(Shuffler &parent,
                size_t trials,
                unsigned int seed); // CTOR
  ~ShuffleThread() {}; // DTOR

  void operator()() {
    run();
  } // functor operator

  ShuffleThread(const ShuffleThread &) = default; // Copy CTOR
  ShuffleThread &operator=(const ShuffleThread &) = default; // Copy assignment
  ShuffleThread(ShuffleThread &&) = default; // Move CTOR
  ShuffleThread &operator=(ShuffleThread &&) = default; // Move assignment
private:
  friend class Shuffler;

  void run();
  void runOne();
  void shuffle();

  Shuffler &_parent;

  size_t _n;
  size_t _m;
  size_t _trials;
  std::default_random_engine _prng;

  Deck _deck;
  std::vector<size_t> _counts;
  bool _stdShuff;
}; // ShuffleThread

#endif // SHUFFLER_H
