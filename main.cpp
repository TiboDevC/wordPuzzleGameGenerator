#include <iostream>

#include "generateMotsMeles.hpp"

int main()
{
    constexpr uint8_t numberOfLinesS{8};
    constexpr uint8_t numberOfColumnsS{8};

    PuzzleWords<numberOfLinesS, numberOfColumnsS> puzzleWords{};

    puzzleWords.generateGridOfWords();

	std::cout << "Number of words: " << puzzleWords.getWordsOfGrid().size() << std::endl;
    for (const auto &word : puzzleWords.getWordsOfGrid())
    {
        std::cout << word << std::endl;
    }

    for (uint8_t line{}; line < numberOfLinesS; line++)
    {
        for (uint8_t column{}; column < numberOfColumnsS; column++)
        {
            std::cout << puzzleWords.getGrid().at(line).at(column) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
