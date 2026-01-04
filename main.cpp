#include <iostream>
#include "generateMotsMeles.hpp"

constexpr uint8_t gridSize = 5;
using PuzzleType = PuzzleWords<gridSize, gridSize>;

void displayGrid(const PuzzleType &puzzle) {
    const auto &grid = puzzle.getGrid();
    std::cout << "\n=== Grid ===" << std::endl;
    for (uint8_t line = 0; line < gridSize; line++) {
        for (uint8_t column = 0; column < gridSize; column++) {
            std::cout << static_cast<char>(grid.at(line).at(column)) << " ";
        }
        std::cout << std::endl;
    }
}

void displayWordsInfo(const PuzzleType &puzzle) {
    const auto &wordsInfo = puzzle.getWordsInfo();
    std::cout << "\n=== Words to find (" << puzzle.getWordCount() << " words) ===" << std::endl;

    for (size_t i = 0; i < wordsInfo.size(); i++) {
        const auto &info = wordsInfo.at(i);
        std::cout << (i + 1) << ". " << info.word
                << " - Position: (" << static_cast<int>(info.startLine)
                << "," << static_cast<int>(info.startColumn) << ") -> ("
                << static_cast<int>(info.endLine) << ","
                << static_cast<int>(info.endColumn) << ")"
                << (info.isStrikethrough ? " [STRIKETHROUGH]" : "")
                << std::endl;
    }
}

int main() {
    constexpr uint8_t maxWords = 100;

    PuzzleType puzzle{maxWords};

    std::cout << "=== Word Search Generator ===" << std::endl;
    std::cout << "Generating a " << static_cast<int>(gridSize)
              << "x" << static_cast<int>(gridSize) << " grid..." << std::endl;

    puzzle.generateGridOfWords();

    displayGrid(puzzle);
    displayWordsInfo(puzzle);

    // Simulation: strikethrough one word every 30 minutes
    std::cout << "\n=== Simulation: strikethrough words progressively ===" << std::endl;

    int count = 0;
    while (!puzzle.areAllWordsStrikethrough() && count < 3) // Strikethrough only 3 words for the demo
    {
        const int nextWord = puzzle.getNextWordToStrikethrough();
        if (nextWord >= 0) {
            const auto &info = puzzle.getWordsInfo().at(static_cast<size_t>(nextWord));
            std::cout << "\n[30 min] Striking through word: " << info.word << std::endl;
            puzzle.strikethroughWord(static_cast<size_t>(nextWord));
            std::cout << "Remaining words: " << puzzle.getRemainingWordCount() << std::endl;
            count++;
        }
    }

    std::cout << "\n=== Final state ===" << std::endl;
    displayWordsInfo(puzzle);

    if (puzzle.areAllWordsStrikethrough()) {
        std::cout << "\nAll words are strikethrough! Generating a new grid..." << std::endl;
        puzzle.generateGridOfWords();
        displayGrid(puzzle);
        displayWordsInfo(puzzle);
    }

    return 0;
}
