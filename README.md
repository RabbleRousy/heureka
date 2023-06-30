# Heureka Engine
## Features
- Functional chess engine written from scratch
- Including GUI using [SFML](https://www.sfml-dev.org/)
- UCI protocol supported
- Bitboard move generation with magic numbers
- Negamax Search with various poorly implemented optimizations
- Transposition Table (currently disabled)
- *WIP: Position evaluation using [NNUE](https://www.chessprogramming.org/NNUE)*
## A from-the-scratch C++ chess engine
Heureka Engine was written in 2021 as preparation for my final Bachelor's semester, to refresh and train my C++ skills.
The initial development was mainly driven by [this amazing video by Sebastian Lague](https://youtu.be/U4ogK0MIzqk), but I quickly fell down the rabbithole which is chess programming, and started tackling more advanced concepts.

Topic of my Bachelor's thesis was to implement a positional evaluation function using an "Efficiently Updatable Neural Network", which I tried but failed to train using [mlpack](https://github.com/mlpack/mlpack).
My theses is available online [here](https://simonhetzer.de/downloads/bachelor-thesis) (english abstract only, the whole thing is in german).

## Results
Before starting work on my Bachelor's thesis, the engine performs pretty bad against most other engines, and would probably be rated ~1500 elo. [Here are some of the games.](https://www.chess.com/c/22g76CWzr)
This is due to the lack of a good evaluation function, as I did not have the time to implement anything better than [evaluating by piece position](https://www.chessprogramming.org/Simplified_Evaluation_Function).
Once I improve the static evaluation function or replace it by NNUE evaluation, I expect the engine to increase dramatically in strength. The search is already pretty fast because of the bitboard move generation, but there is also still a lot to improve.

### PERFT Results from starting position
| Depth | ms |
| ----------- | ----------- |
| 0 | 0 |
| 1 | 0 |
| 2 | 6 |
| 3 | 6 |
| 4 | 7 |
| 5 | 85 |
| 6 | 1996 |
| 7 | 49518 |
### Search speed: ~3.5 million positions/second
