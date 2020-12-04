# wordPuzzleGameGenerator

Example in the main.cpp

Will generate a grid of words accorind to number of lines and columns.

List of words can be completed/replaced in 'wordList.hpp'.

Words will be put in the grid in line or column. It could also be put in diagnoal if this line is modified:
```c++
std::uniform_int_distribution<> randomDirectionGen(0, Direction::diagonal - 1);
```

# known problem

A word can override another. For example: word 'frose' can be replaced by 'frosen'.
