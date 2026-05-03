//
//      File     : Main.cc
//      Abstract : Driver
//

#include "Args.h"

#include <algorithm>
#include <cassert>
#include <csignal>
#include <cstdint>
#include <numeric>
#include <random>
#include <vector>
#include <iomanip>
#include <iostream>

#include <termios.h>

using Deck = std::vector<unsigned int>;

//      Function : handler
//      Abstract : Handle a SIGINT
static volatile sig_atomic_t gSignal = 0;
void
handler(int sig)
{
  gSignal = sig;
} // handler


//      Struct    : ShuffleArgs
//      Abstract : Command line argument parser.
struct ShuffleArgs : public argparse::Args {
  size_t &n = kwarg("n", "size of deck.").set_default(52);
  size_t &m = kwarg("m", "number of target (face) cards.").set_default(12);
  size_t &maxTrials = kwarg("M,max_trials",
                            "maximum number of trials.").set_default(SIZE_MAX);
  std::optional<unsigned int> &seed =
    kwarg("s,seed",
          "seed for random number generator.");
  bool &homebrew = flag("N,nonstandard",
                        "use homebrew version of shuffle().");

  void prolog() override {
    std::cout << R"(
Calculate and simulate solution of Problem M/A1 from the March/April
2026 MIT Alumni News Puzzle Corner.
)"<< std::endl;
  } // prolog

  void epilog() override {
    std::cout << R"(
For a randomly shuffled deck of n cards with m special cards, what is
the probability that the first k cards are special. The defaults are
n = 52 and m = 12. This corresponds to the original problem of a regular
deck with the face cards being special.

The program first calculates the probabilities and then starts to
simulate random shuffles indefinitely. To print out current statistics,
type Ctrl+C (SIGINT). Type Ctrl+\ (SIGQUIT) to print statistics and exit.
)" << std::endl;
  } // epilog
}; // ShuffleArgs


//      Class    : Shuffler
//      Abstract : Class for shuffling a deck of n cards.
class Shuffler {
public:
  Shuffler(size_t n, size_t m, size_t maxTrials,
           unsigned int seed, bool standard) :
    _n(n),
    _m(m),
    _maxTrials(maxTrials),
    _prng(seed),
    _trials(0),
    _standard(standard)
  {
    tcgetattr(STDIN_FILENO, &_start);
    _update = _start;
    _update.c_lflag &= ~ECHOCTL;
    // std::cout << "Disabling ECHOCTL." << std::endl;
    tcsetattr(STDIN_FILENO, TCSANOW, &_update);

    std::cout << "seed=" << seed << std::endl;
    _deck.resize(_n);
    std::iota(_deck.begin(), _deck.end(), 0);
    _counts.resize(_m+1, 0);
    computeProbs();
  }; // CTOR
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

  bool runOne() {
    shuffle();
    size_t cnt = 0;
    while(_deck[cnt] < _m) {
      ++cnt;
    } // while
    ++_counts[cnt];
    ++_trials;
    return checkIntr();
  } // runOne

private:
  void computeProbs();
  void shuffle();
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
  bool _standard;

  struct termios _start;
  struct termios _update;
}; // Shuffler


//      Function : Shuffler::computeProbs
//      Abstract : Compute the probabilities of each result. The
//      probability of seeing exactly k special cards is
//
//      (m!)*(n-m)*(n-k-1)!
//      ------------------- =
//           (m-k)!*n!
//
//      (m!)*(n-m)   (n-k-1)!
//      ---------- * ---------
//        (m-k)!        n!
void
Shuffler::computeProbs()
{
  std::vector<double> numerator;
  std::vector<double> denominator;
  std::cout << std::endl;
  double x = 0;
    _probability.resize(_m+1);
  for (size_t k = 0; k <= _m; ++k) {
    // reduced version of (m!*(n-m)) / (m-k)!
    x = _m;
    while (x > _m-k) {
      numerator.push_back(x--);
    } // while
    numerator.push_back(_n-_m);

    // reduced version of (n-k-1)!/n!
    x = _n;
    while (x > _n-k-1) {
      denominator.push_back(x--);
    } // while

    double p = 1.0;
    assert(denominator.size() == numerator.size());
    while (numerator.size()) {
      p /= denominator.back();
      denominator.pop_back();
      p *= numerator.back();
      numerator.pop_back();
    } // while
    _probability[k] = p;
    std::cout << std::setw(2) << k << " : " << p << std::endl;
  } // for each outcome;
} // Shuffler::computeProbs


//      Function : Shuffler::shuffle
//      Abstract : Shuffle the deck.
void
Shuffler::shuffle()
{
  if (_standard) {
    std::shuffle(_deck.begin(), _deck.end(), _prng);
  } else {
    std::uniform_int_distribution<> rng(0, _n-1);
    for (auto &card : _deck) {
      const size_t jdx = rng(_prng);
      std::swap(card, _deck[jdx]);
    } // for
    // for (size_t idx = 0; idx < _n; ++idx) {
    //   const size_t jdx = rng(_prng);
    //   std::swap(_deck[idx], _deck[jdx]);
    // } // for
  } // if
} // Shuffler::shuffle


//      Function : Shuffler::checkIntr
//      Abstract : Check for max trials, SIGINT and SIGQUIT. Return
//      false for SIGQUIT or max trials.
bool
Shuffler::checkIntr()
{
  if (::gSignal) {
    printStats();
    if (::gSignal == SIGQUIT) {
      return false;
    } // if quit
    ::gSignal = 0;
  } else if (_trials >= _maxTrials) {
    printStats();
    return false;
  } // if intr

  return true;
} // Shuffler::checkIntr


//      Function : Shuffler::printStats
//      Abstract : Print current statistics
void
Shuffler::printStats()
{
  std::cout << "\nTrials: " << _trials << std::endl;
  for (size_t idx = 0; idx <= _m; ++idx) {
    double e = (static_cast<double>(_trials) * _probability[idx]);
    std::cout << std::setw(2) << idx << " : "
              << std::setw(16)
              << _counts[idx] << " : "
              << std::fixed
              << std::setprecision(2)
              << e
              << std::endl;

  } // for

  // size_t cnt = 0;
  // for (const auto &val : _deck) {
  //   std::cout << std::setw(2) << val
  //             << ((++cnt % 13) ? " " : "\n")
  //             << std::flush;
  // } // for
} // Shuffler::printStats


//      Function : main
//      Abstract : Main driver.
int
main(int argc, char *argv[])
{
  signal(SIGINT, handler);
  signal(SIGQUIT, handler);

  auto args = argparse::parse<ShuffleArgs>(argc, argv);
  args.print();

  if (args.m <= args.n) {
    std::cout << R"(
Type Ctrl+C (SIGINT) to print out current statistics.
Type Ctrl+\ (SIGQUIT) to print statistics and exit.
)" << std::endl;
    Shuffler shuffler(args.n,
                      args.m,
                      args.maxTrials,
                      args.seed ? args.seed.value()
                      : std::random_device()(),
                      ! args.homebrew);;

    while (shuffler.runOne());
  } else {
    std::cerr << "Argument m must be less than or equal to argument n."
              << std::endl;
    exit(1);
  } // if args ok

  return 0;
} // main
