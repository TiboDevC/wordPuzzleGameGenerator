#pragma once

#include <iostream>
#include <array>
#include <random>
#include <cstring>

#include "wordList.hpp"

std::random_device rd;
std::mt19937 gen(rd());

template<uint8_t numberOfLines, uint8_t numberOfColumns>
class PuzzleWords
{
private:
    static constexpr uint8_t invalidValue{0xFF};
    static constexpr uint8_t minWordlength{4};

    enum Direction
    {
        line,
        column,
        diagonal,
        invalid
    };
    typedef std::vector<std::array<uint8_t, 2>> Constraints; // first is letter start, second is the letter

    struct WordParam
    {
        uint8_t line{invalidValue};
        uint8_t column{invalidValue};
        Direction direction{Direction::invalid};
        uint8_t length{invalidValue};
        uint32_t id{invalidValue};
    };

    uint32_t numberOfWordsInDictionnary{};
    std::vector<uint32_t> pickedUpWords{};
    uint32_t numberOfWordsToFound{0xFFFFFFFF};

    struct Grid
    {
        std::array<std::array<uint8_t, numberOfColumns>, numberOfLines> element{};

        [[nodiscard]] const std::array<std::array<uint8_t, numberOfColumns>, numberOfLines> &getElement() const
        {
            return element;
        }

        uint8_t &at(const uint8_t &line, const uint8_t &column)
        {
            return element.at(line).at(column);
        }

        [[nodiscard]] const uint8_t &at(const uint8_t &line, const uint8_t &column) const
        {
            return element.at(line).at(column);
        }
    };

    Grid gridWords{};

    void selectRandomBox(const Grid &arrayWords, WordParam &word)
    {
        word.line = invalidValue;
        word.column = invalidValue;

        uint8_t randomColumn{invalidValue};
        uint8_t randomLine{invalidValue};

        uint8_t maxLineSize{};
        uint8_t maxColumnSize{};

        switch (word.direction)
        {
            case Direction::line:
                maxLineSize = numberOfLines - 1;
                maxColumnSize = numberOfColumns - minWordlength;
                break;
            case Direction::column:
                maxLineSize = numberOfLines - minWordlength;
                maxColumnSize = numberOfColumns - 1;
                break;
            case Direction::diagonal:
                maxLineSize = numberOfLines - minWordlength;
                maxColumnSize = numberOfColumns - minWordlength;
                break;
            case invalid:
            default:
                break;
        }

        std::uniform_int_distribution<> randomLineNumber(0, maxLineSize);
        std::uniform_int_distribution<> randomColumnNumber(0, maxColumnSize);

        uint8_t numberOfTry{};
        do
        {
            numberOfTry++;
            randomLine = randomLineNumber(gen);
            randomColumn = randomColumnNumber(gen);
        } while ((arrayWords.at(randomLine, randomColumn) != 0) and (numberOfTry < numberOfLines * numberOfColumns));
        if (numberOfTry < numberOfLines * numberOfColumns)
        {
            word.line = randomLine;
            word.column = randomColumn;
        }
    }

    Constraints buildWordConstraints(const Grid &arrayWords, const WordParam &wordParam)
    {
        Constraints constraints{};

        for (uint8_t wordlengthIndex{}; wordlengthIndex < wordParam.length; wordlengthIndex++)
        {
            uint8_t letter{};
            switch (wordParam.direction)
            {
                case Direction::line:
                    letter = arrayWords.at(wordParam.line, wordParam.column + wordlengthIndex);
                    break;
                case Direction::column:
                    letter = arrayWords.at(wordParam.line + wordlengthIndex, wordParam.column);
                    break;
                case Direction::diagonal:
                    letter = arrayWords.at(wordParam.line + wordlengthIndex, wordParam.column + wordlengthIndex);
                    break;
                case invalid:
                default:
                    break;
            }
            if (letter > 0)
            {
                constraints.push_back({wordlengthIndex, letter});
            }
        }
        return constraints;
    }

    void selectRandomSize(WordParam &word)
    {
        uint8_t maxSize{};
        word.length = invalidValue;
        switch (word.direction)
        {
            case Direction::line:
                maxSize = numberOfColumns - word.column;
                break;
            case Direction::column:
                maxSize = numberOfLines - word.line;
                break;
            case Direction::diagonal:
                maxSize = std::min(numberOfLines - word.line, numberOfColumns - word.column);
                break;
            case invalid:
            default:
                break;
        }

        if (maxSize >= minWordlength)
        {
            std::uniform_int_distribution<> randomSize(minWordlength, maxSize);
            word.length = randomSize(gen);
        }
    }

    void choosePotentialWord(const std::vector<uint32_t> &listPotentialWords, WordParam &word)
    {
        if (!listPotentialWords.empty())
        {
            constexpr uint8_t maxNumberOfTry{20};
            uint32_t choosenWord{invalidValue};
            uint8_t numberOfTry{0};
            do
            {
                std::uniform_int_distribution<> randomWordGen(0, listPotentialWords.size() - 1);
                choosenWord = randomWordGen(gen);
                numberOfTry++;
            } while (std::find(pickedUpWords.begin(), pickedUpWords.end(), listPotentialWords.at(choosenWord)) !=
                     pickedUpWords.end() and
                     numberOfTry < maxNumberOfTry);
            if (numberOfTry < maxNumberOfTry)
            {
                word.id = listPotentialWords.at(choosenWord);
            }
        }
    }

    void findWordAccordingToConstraints(const std::vector<std::array<uint8_t, 2>> &constraints, WordParam &word)
    {
        std::vector<uint32_t> listPotentialWords;
        uint32_t wordIndex{};
        word.id = invalidValue;

        for (auto &listWord : listWords)
        {
            if (std::strlen(listWord) == word.length)
            {
                bool isConstraintOk{true};
                for (const auto &constraint : constraints)
                {
                    if (listWord[constraint.at(0)] != constraint.at(1))
                    {
                        isConstraintOk = false;
                        break;
                    }
                }
                if (isConstraintOk)
                {
                    listPotentialWords.push_back(wordIndex);
                }
            }
            wordIndex++;
        }
        choosePotentialWord(listPotentialWords, word);
    }

    void fillGridWithWord(const WordParam &word, Grid &arrayWords)
    {
        for (uint8_t sizeIndex{}; sizeIndex < word.length; sizeIndex++)
        {
            switch (word.direction)
            {
                case Direction::line:
                    arrayWords.at(word.line, word.column + sizeIndex) = listWords[word.id][sizeIndex];
                    break;
                case Direction::column:
                    arrayWords.at(word.line + sizeIndex, word.column) = listWords[word.id][sizeIndex];
                    break;
                case Direction::diagonal:
                    arrayWords.at(word.line + sizeIndex, word.column + sizeIndex) = listWords[word.id][sizeIndex];
                    break;
                case invalid:
                default:
                    break;
            }
        }
    }

    void completeGridWithRandomLetters(const Grid &arrayWords)
    {
        std::uniform_int_distribution<> randomWord('a', 'z');

        for (uint8_t line{}; line < numberOfLines; line++)
        {
            for (uint8_t column{}; column < numberOfColumns; column++)
            {
                if (arrayWords.at(line, column) > 0)
                {
                    gridWords.at(line, column) = arrayWords.at(line, column);
                } else
                {
                    gridWords.at(line, column) = randomWord(gen);
                }
            }
        }
    }

    void displayGrid()
    {
        std::cout << "words(" << pickedUpWords.size() << "): " << std::endl;
        for (const auto &word : pickedUpWords)
        {
            std::cout << "- " << listWords[word] << std::endl;
        }

        std::cout << "full grid:" << std::endl;
        for (uint8_t line{}; line < numberOfLines; line++)
        {
            for (uint8_t column{}; column < numberOfColumns; column++)
            {
                std::cout << gridWords.at(line, column) << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

public:
    explicit PuzzleWords(const uint32_t &numberOfWordsToGenerate = 0xFFFFFFFF)
    {
        static_assert(numberOfColumns >= minWordlength,
                      "please increase number of column or decrease min size of words to be found");
        static_assert(numberOfLines >= minWordlength,
                      "please increase number of lines or decrease min size of words to be found");
        numberOfWordsToFound = numberOfWordsToGenerate;
        for (const auto &word : listWords)
        {
            (void) word;
            numberOfWordsInDictionnary++;
        }
    }

    [[nodiscard]] std::vector<std::string> getWordsOfGrid() const
    {
        std::vector<std::string> listOfWordsInGrid{};
        for (const auto &wordNumber : pickedUpWords)
        {
            listOfWordsInGrid.emplace_back(listWords[wordNumber]);
        }
        return listOfWordsInGrid;
    }

    [[nodiscard]] const std::array<std::array<uint8_t, numberOfColumns>, numberOfLines> &getGrid() const
    {
        return gridWords.getElement();
    }

    void generateGridOfWords()
    {
        constexpr uint32_t maxNumberOfTry{numberOfLines * numberOfColumns * 2};
        gridWords = Grid{};
        pickedUpWords.clear();

        Grid arrayWords{};

        std::uniform_int_distribution<> randomWordGen(0, numberOfWordsInDictionnary - 1);
        std::uniform_int_distribution<> randomDirectionGen(0, Direction::invalid - 1);

        uint32_t tryToFindWord{0};

        while (tryToFindWord < maxNumberOfTry && pickedUpWords.size() < numberOfWordsToFound)
        {
            tryToFindWord++;
            WordParam word{};

            word.direction = static_cast<Direction>(randomDirectionGen(gen));
            selectRandomBox(arrayWords, word);
            selectRandomSize(word);

            if (word.length != invalidValue and word.line != invalidValue and word.column != invalidValue)
            {
                const auto constraints = buildWordConstraints(arrayWords, word);

                findWordAccordingToConstraints(constraints, word);
                if (word.id != invalidValue)
                {
                    fillGridWithWord(word, arrayWords);
                    pickedUpWords.push_back(word.id);

                    word = WordParam{};
                }
            }
        }
        completeGridWithRandomLetters(arrayWords);

        // displayGrid();
    }
};
