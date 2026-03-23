[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
# shuffler
## Description
Calculate and simulate solution of Problem M/A1 from the March/April
2026 MIT Alumni News Puzzle Corner:

**M/A1. Keep a straight face!** Given a shuffled deck of standard  
playing cards, start dealing them one after the other and keep  
going as long as you see a face card (jack, queen, or king). What  
is the probaility that you have seen 0, 1, ..., 12 face cards?

To get started, from the `shuffler` directory enter
```
./configure
cd src
make
./shuffler
```
Enter `./shuffler -?` to see all options.

The program first calculates the probabilities and then starts to
simulate random shuffles indefinitely. To print out current statistics,
type Ctrl+C (SIGINT). Type Ctrl+\ (SIGQUIT) to print statistics and exit.
