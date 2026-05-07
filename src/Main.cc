//
//      File     : Main.cc
//      Abstract : Driver
//

#include "Args.h"
#include "Shuffler.h"

//      Struct    : ShuffleArgs
//      Abstract : Command line argument parser.
struct ShuffleArgs : public argparse::Args {
  size_t &n = kwarg("n", "size of deck.").set_default(52);
  size_t &m = kwarg("m", "number of target (face) cards.").set_default(12);
  size_t &maxTrials = kwarg("M,max_trials",
                            "maximum number of trials.").set_default(SIZE_MAX);
  size_t &numThreads = kwarg("t,threads",
                            "number of threads.").set_default(1);
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
If you have multiple cores available, you can speed up the simulation by
specifying more than one thread.
)" << std::endl;
  } // epilog
}; // ShuffleArgs


//      Function : checkArgs
//      Abstract : Check if arguments have reasonable values.
bool
checkArgs(ShuffleArgs &args)
{
  bool status = true;
  if (args.m > args.n) {
    std::cerr << "Argument m must be less than or equal to argument n."
              << std::endl;
    status = false;
  } // if
  if (args.numThreads == 0 &&
      args.maxTrials > 0) {
    std::cerr << "There must be at least one thread for simulation."
              << std::endl;
    status = false;
  } // if

  return status;
} // checkArgs


//      Abstract : Main driver.
int
main(int argc, char *argv[])
{
  ShuffleArgs args = argparse::parse<ShuffleArgs>(argc, argv);
  args.print();

  if (checkArgs(args)) {
    Shuffler shuffler(args.n,
                      args.m,
                      args.maxTrials,
                      args.numThreads,
                      args.seed ? args.seed.value()
                      : std::random_device()(),
                      ! args.homebrew);;

    shuffler.computeProbs();
    if (args.maxTrials > 0) {
      shuffler.simulate();
    } // if
    std::cout << std::endl;
  } else {
    exit(1);
  } // if args ok

  return 0;
} // main
