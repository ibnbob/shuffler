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
You can perform the calculations and simulation for arbitrary deck sizes
and number of face cards using the --n and --m options. The defaults are
--n=52 and --m=12 which correspond to the original problem. Enter
`./shuffler -?` to see all options.

The program first calculates the probabilities and then starts to
simulate random shuffles indefinitely. To print out current statistics,
type Ctrl+C (SIGINT). Type Ctrl+\ (SIGQUIT) to print statistics and exit.

The probabilities for those that do not want to run the code are:
 0 : 0.769231  
 1 : 0.180995  
 2 : 0.039819  
 3 : 0.00812633  
 4 : 0.00152369  
 5 : 0.000259351  
 6 : 3.94664e-05  
 7 : 5.26219e-06  
 8 : 5.97976e-07  
 9 : 5.56257e-08  
10 : 3.97326e-09  
11 : 1.93818e-10  
12 : 4.84544e-12  
