//
//      File     : Main.cc
//      Abstract : Driver
//

#include <algorithm>
#include <csignal>
#include <cstdint>
#include <numeric>
#include <random>
#include <vector>
#include <iomanip>
#include <iostream>

using Deck = std::vector<unsigned int>;

//      Function : handleIntr
//      Abstract : Handle a SIGINT
static volatile bool gIntrSeen = false;
void
handleIntr(int sig)
{
  gIntrSeen = true;
} // handleIntr


//      Class    : Shuffler
//      Abstract : Class for shuffling a deck of n cards.
class Shuffler {
public:
  Shuffler(size_t n, size_t m, unsigned int seed) :
    _n(n),
    _m(m),
    _prng(seed),
    _trials(0)
  {
    _deck.resize(_n);
    std::iota(_deck.begin(), _deck.end(), 0);
    _counts.resize(_m+1, 0);
  }; // CTOR
  Shuffler(size_t n, size_t m) : Shuffler(n, m, 0xdeadbeef) {};
  Shuffler() : Shuffler(52, 12) {};
  ~Shuffler() {}; // DTOR

  Shuffler(const Shuffler &) = delete; // Copy CTOR
  Shuffler &operator=(const Shuffler &) = delete; // Copy assignment
  Shuffler(Shuffler &&) = delete; // Move CTOR
  Shuffler &operator=(Shuffler &&) = delete; // Move assignment

  void runOne() {
    shuffle();
    size_t cnt = 0;
    while(_deck[cnt] < _m) {
      ++cnt;
    } // while
    ++_counts[cnt];
    ++_trials;
    checkIntr();
  } // runOne


private:
  void shuffle() { std::shuffle(_deck.begin(), _deck.end(), _prng); };
  Deck &getDeck() { return _deck; };

  void checkIntr() {
    if (::gIntrSeen) {
      ::gIntrSeen = false;
      printStats();
    } // if
  } // checkIntr

  void printStats() {
    std::cout << "Trials: " << _trials << std::endl;
    for (size_t jdx = 0; jdx <= _m; ++jdx) {
      double p = (static_cast<double>(_counts[jdx]) /
                  static_cast<double>(_trials));
      std::cout << std::setw(2) << jdx << " : "
                << std::setw(16)
                << std::fixed
                << std::setprecision(14)
                << p
                << std::endl;

    } // for
    std::cout << std::endl;
  } // printStats

  // Data
  size_t _n;
  size_t _m;
  std::default_random_engine _prng;

  Deck _deck;
  size_t _trials;
  std::vector<size_t> _counts;
}; // Shuffler


//      Function : main
//      Abstract : Main driver.
int
main(int argc, char *argv[])
{
  signal(SIGINT, handleIntr);

  Shuffler shuffler{};

  while (true) {
    shuffler.runOne();
  } // while

  return 0;
} // main
