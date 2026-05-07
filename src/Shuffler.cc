//
//      File     : Shuffler.cc
//      Abstract : Shuffler classes.
//

#include "Shuffler.h"

#include <algorithm>
#include <cassert>
#include <thread>

//      Function : handler
//      Abstract : Handle a SIGINT
static volatile sig_atomic_t gSignal = 0;
void
handler(int sig)
{
  gSignal = sig;
} // handler


//      Function : Shuffler::Shuffler
//      Abstract : CTOR.
Shuffler::Shuffler(size_t n,
                   size_t m,
                   size_t maxTrials,
                   size_t numThreads,
                   unsigned int seed,
                   bool stdShuff) :
  _n(n),
  _m(m),
  _maxTrials(maxTrials),
  _numThreads(numThreads),
  _prng(seed),
  _trials(0),
  _stdShuff(stdShuff)
{
  signal(SIGINT, handler);
  signal(SIGQUIT, handler);

  tcgetattr(STDIN_FILENO, &_start);
  _update = _start;
  // Disable CTL echo.
  _update.c_lflag &= ~ECHOCTL;
  tcsetattr(STDIN_FILENO, TCSANOW, &_update);

  std::cout << "seed=" << seed << std::endl;
  _deck.resize(_n);
  std::iota(_deck.begin(), _deck.end(), 0);
  _counts.resize(_m+1, 0);
}; // Shuffler::Shuffler


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
} // Shuffler::printStats


//      Function : main
//      Function : Shuffler::simulate
//      Abstract : Simulate shuffles.
void
Shuffler::simulate()
{
    std::cout << R"(
Type Ctrl+C (SIGINT) to print out current statistics.
Type Ctrl+\ (SIGQUIT) to print statistics and exit.
)" << std::endl;

    const size_t chunkSz = 1 << 20;
  std::uniform_int_distribution<size_t> seeds(0, SIZE_MAX);

  while (checkIntr()) {
    std::vector<ShuffleThread> insts;
    std::vector<std::thread> threads;

    for (size_t tdx = 0; tdx < _numThreads; ++ tdx) {
      size_t trials = std::min(chunkSz, (_maxTrials - _trials));
      insts.emplace_back(*this, trials, seeds(_prng));
      threads.emplace_back(insts[tdx]);
      _trials += trials;
    } // for

    for (size_t tdx = 0; tdx < _numThreads; ++ tdx) {
      threads[tdx].join();
    } // for
  } // while
} // Shuffler::simulate


//      Function : Shuffler::accumulate
//      Abstract : Accumulate results from a thread.
void
Shuffler::accumulate(ShuffleThread &thread)
{
  assert(_m == thread._m);
  for (size_t idx = 0; idx <= _m; ++idx) {
    _counts[idx] += thread._counts[idx];
  } // for
} // Shuffler::accumulate


//      Function : ShuffleThread::ShuffleThread
//      Abstract : CTOR

ShuffleThread::ShuffleThread(Shuffler &parent,
                             size_t trials,
                             unsigned int seed)
  : _parent(parent),
    _n(parent._n),
    _m(parent._m),
    _trials(trials),
    _prng(seed),
    _stdShuff(_parent._stdShuff)
{
  _deck.resize(_n);
  std::iota(_deck.begin(), _deck.end(), 0);
  _counts.resize(_m+1, 0);
} // ShuffleThread::ShuffleThread


//      Function : ShuffleThread::run
//      Abstract : Run a thread.
void
ShuffleThread::run()
{
  while (_trials--) {
    runOne();
  } // while
  _parent.accumulate(*this);
} // ShuffleThread::run

//      Function : ShuffleThread::runOne
//      Abstract : Run one shuffle.
void
ShuffleThread::runOne()
{
  shuffle();
  size_t cnt = 0;
  while(_deck[cnt] < _m) {
    ++cnt;
  } // while
  ++_counts[cnt];
} // ShuffleThread::runOne


//      Function : ShuffleThread::shuffle
//      Abstract : Shuffle the deck.
void
ShuffleThread::shuffle()
{
  if (_stdShuff) {
    std::shuffle(_deck.begin(), _deck.end(), _prng);
  } else {
    std::uniform_int_distribution<> rng(0, _n-1);
    for (auto &card : _deck) {
      const size_t jdx = rng(_prng);
      std::swap(card, _deck[jdx]);
    } // for
  } // if
} // ShuffleThread::shuffle


