#include <iostream>

#include "generateMotsMeles.hpp"

int main()
{
    constexpr uint8_t numberOfLinesS{8};
    constexpr uint8_t numberOfColumnsS{8};

    MotsMeles<numberOfLinesS, numberOfColumnsS> motsMeles{};
    motsMeles.generateGridOfWords();
    for(const auto& word : motsMeles.getWordsOfGrid())
    {
        std::cout << listWords[word] << std::endl;
    }
    for(uint8_t line{}; line < numberOfLinesS; line++)
    {
        for(uint8_t column{}; column < numberOfColumnsS; column++)
        {
            std::cout << motsMeles.getGrid().at(line).at(column) << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}
