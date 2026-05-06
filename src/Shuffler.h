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
           unsigned int seed,
           bool stdShuff);
  Shuffler(size_t n, size_t m) : Shuffler(n, m, SIZE_MAX, 0xdeadbeef, false) {};
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
  bool checkIntr();
  void printStats();

  // Data
  size_t _n;
  size_t _m;
  size_t _maxTrials;
  std::default_random_engine _prng;

  std::vector<double> _probability;

  Deck _deck;
  size_t _trials;
  std::vector<size_t> _counts;
  bool _stdShuff;

  struct termios _start;
  struct termios _update;
}; // Shuffler


//      Class    : ShuffleThread
//      Abstract : One shuffle thread.
class ShuffleThread {
public:
  ShuffleThread(size_t n,
                size_t m,
                size_t trials,
                unsigned int seed,
                bool stdShuff); // CTOR
  ~ShuffleThread() {}; // DTOR

  void operator()() {
    run();
  } // functor operator

  ShuffleThread(const ShuffleThread &) = delete; // Copy CTOR
  ShuffleThread &operator=(const ShuffleThread &) = delete; // Copy assignment
  ShuffleThread(ShuffleThread &&) = delete; // Move CTOR
  ShuffleThread &operator=(ShuffleThread &&) = delete; // Move assignment
private:
  friend class Shuffler;

  void run();
  void runOne();
  void shuffle();

  size_t _n;
  size_t _m;
  size_t _trials;
  std::default_random_engine _prng;

  Deck _deck;
  std::vector<size_t> _counts;
  bool _stdShuff;
}; // ShuffleThread

#endif // SHUFFLER_H
