#pragma once

#include <array>
#include <random>
#include <algorithm>
#include <vector>
#include <string_view>
#include <limits>

#include "wordList.hpp"


template<uint8_t numberOfLines, uint8_t numberOfColumns>
class PuzzleWords {
private:
    static constexpr uint8_t invalidValue{0xFF};
    static constexpr uint8_t minWordlength{4};
    static constexpr uint8_t maxNumberOfTriesToFindBox{100};
    static constexpr uint8_t maxNumberOfTriesToPickWord{20};
    static constexpr size_t potentialWordsReserveSize{100};

    static std::mt19937 &getRandomGenerator() {
        thread_local std::random_device rd;
        thread_local std::mt19937 gen(rd());
        return gen;
    }

    enum class Direction : uint8_t {
        Line,
        Column,
        Diagonal,
        Invalid
    };

    struct Constraint {
        uint8_t position;
        uint8_t letter;
    };

    using Constraints = std::vector<Constraint>;

    struct WordParam {
        uint8_t line{invalidValue};
        uint8_t column{invalidValue};
        Direction direction{Direction::Invalid};
        uint8_t length{invalidValue};
        uint32_t id{invalidValue};
    };

public:
    struct WordInfo {
        std::string_view word{};
        uint8_t startLine{0};
        uint8_t startColumn{0};
        uint8_t endLine{0};
        uint8_t endColumn{0};
        uint8_t length{0};
        Direction direction{Direction::Invalid};
        bool isStrikethrough{false};
    };

private:
    uint32_t numberOfWordsInDictionary{};
    std::vector<uint32_t> pickedUpWords{};
    std::vector<WordInfo> wordsInfo{};
    uint32_t numberOfWordsToFind{std::numeric_limits<uint32_t>::max()};

    struct Grid {
        std::array<std::array<uint8_t, numberOfColumns>, numberOfLines> element{};

        [[nodiscard]] const std::array<std::array<uint8_t, numberOfColumns>, numberOfLines> &getElement() const {
            return element;
        }

        uint8_t &at(uint8_t line, uint8_t column) {
            return element.at(line).at(column);
        }

        [[nodiscard]] const uint8_t &at(uint8_t line, uint8_t column) const {
            return element.at(line).at(column);
        }
    };

    Grid gridWords{};

    void selectRandomBox(const Grid &arrayWords, WordParam &word) {
        word.line = invalidValue;
        word.column = invalidValue;

        uint8_t maxLineSize{};
        uint8_t maxColumnSize{};

        switch (word.direction) {
            case Direction::Line:
                maxLineSize = numberOfLines - 1;
                maxColumnSize = numberOfColumns - minWordlength;
                break;
            case Direction::Column:
                maxLineSize = numberOfLines - minWordlength;
                maxColumnSize = numberOfColumns - 1;
                break;
            case Direction::Diagonal:
                maxLineSize = numberOfLines - minWordlength;
                maxColumnSize = numberOfColumns - minWordlength;
                break;
            case Direction::Invalid:
                break;
        }

        std::uniform_int_distribution<uint8_t> randomLineNumber(0, maxLineSize);
        std::uniform_int_distribution<uint8_t> randomColumnNumber(0, maxColumnSize);

        auto &gen = getRandomGenerator();
        uint8_t numberOfTry{};
        do {
            numberOfTry++;
            word.line = randomLineNumber(gen);
            word.column = randomColumnNumber(gen);
        } while ((arrayWords.at(word.line, word.column) != 0) && (numberOfTry < numberOfLines * numberOfColumns));

        if (numberOfTry >= numberOfLines * numberOfColumns) {
            word.line = invalidValue;
            word.column = invalidValue;
        }
    }

    Constraints buildWordConstraints(const Grid &arrayWords, const WordParam &wordParam) {
        Constraints constraints{};

        for (uint8_t wordlengthIndex{}; wordlengthIndex < wordParam.length; wordlengthIndex++) {
            uint8_t letter{};
            switch (wordParam.direction) {
                case Direction::Line:
                    letter = arrayWords.at(wordParam.line, wordParam.column + wordlengthIndex);
                    break;
                case Direction::Column:
                    letter = arrayWords.at(wordParam.line + wordlengthIndex, wordParam.column);
                    break;
                case Direction::Diagonal:
                    letter = arrayWords.at(wordParam.line + wordlengthIndex, wordParam.column + wordlengthIndex);
                    break;
                case Direction::Invalid:
                    break;
            }
            if (letter > 0) {
                constraints.push_back({wordlengthIndex, letter});
            }
        }
        return constraints;
    }

    void selectRandomSize(WordParam &word) {
        uint8_t maxSize{};
        word.length = invalidValue;
        switch (word.direction) {
            case Direction::Line:
                maxSize = numberOfColumns - word.column;
                break;
            case Direction::Column:
                maxSize = numberOfLines - word.line;
                break;
            case Direction::Diagonal:
                maxSize = std::min(numberOfLines - word.line, numberOfColumns - word.column);
                break;
            case Direction::Invalid:
                break;
        }

        if (maxSize >= minWordlength) {
            std::uniform_int_distribution<uint8_t> randomSize(minWordlength, maxSize);
            auto &gen = getRandomGenerator();
            word.length = randomSize(gen);
        }
    }

    void choosePotentialWord(const std::vector<uint32_t> &listPotentialWords, WordParam &word) {
        if (listPotentialWords.empty()) {
            return;
        }

        auto &gen = getRandomGenerator();
        std::uniform_int_distribution<size_t> randomWordGen(0, listPotentialWords.size() - 1);

        uint8_t numberOfTry{0};
        while (numberOfTry < maxNumberOfTriesToPickWord) {
            const size_t chosenWord = randomWordGen(gen);
            const uint32_t candidateWordId = listPotentialWords.at(chosenWord);

            if (std::find(pickedUpWords.begin(), pickedUpWords.end(), candidateWordId) == pickedUpWords.end()) {
                word.id = candidateWordId;
                return;
            }
            numberOfTry++;
        }
    }

    void findWordAccordingToConstraints(const Constraints &constraints, WordParam &word) {
        std::vector<uint32_t> listPotentialWords;
        listPotentialWords.reserve(potentialWordsReserveSize);
        uint32_t wordIndex{};
        word.id = invalidValue;

        for (const auto &wordView: listWords) {
            if (wordView.length() == word.length) {
                bool isConstraintOk{true};
                for (const auto &constraint: constraints) {
                    if (wordView.at(constraint.position) != constraint.letter) {
                        isConstraintOk = false;
                        break;
                    }
                }
                if (isConstraintOk) {
                    listPotentialWords.push_back(wordIndex);
                }
            }
            wordIndex++;
        }
        choosePotentialWord(listPotentialWords, word);
    }

    void fillGridWithWord(const WordParam &word, Grid &arrayWords) {
        WordInfo info;
        const std::string_view wordStr = listWords.at(word.id);
        info.word = wordStr;
        info.startLine = word.line;
        info.startColumn = word.column;
        info.length = word.length;
        info.direction = word.direction;
        info.isStrikethrough = false;

        switch (word.direction) {
            case Direction::Line:
                info.endLine = word.line;
                info.endColumn = word.column + word.length - 1;
                break;
            case Direction::Column:
                info.endLine = word.line + word.length - 1;
                info.endColumn = word.column;
                break;
            case Direction::Diagonal:
                info.endLine = word.line + word.length - 1;
                info.endColumn = word.column + word.length - 1;
                break;
            case Direction::Invalid:
                break;
        }

        wordsInfo.push_back(info);

        for (uint8_t sizeIndex{}; sizeIndex < word.length; sizeIndex++) {
            switch (word.direction) {
                case Direction::Line:
                    arrayWords.at(word.line, word.column + sizeIndex) = wordStr.at(sizeIndex);
                    break;
                case Direction::Column:
                    arrayWords.at(word.line + sizeIndex, word.column) = wordStr.at(sizeIndex);
                    break;
                case Direction::Diagonal:
                    arrayWords.at(word.line + sizeIndex, word.column + sizeIndex) = wordStr.at(sizeIndex);
                    break;
                case Direction::Invalid:
                    break;
            }
        }
    }

    void completeGridWithRandomLetters(const Grid &arrayWords) {
        std::uniform_int_distribution<> randomWord('a', 'z');
        auto &gen = getRandomGenerator();

        for (uint8_t line{}; line < numberOfLines; line++) {
            for (uint8_t column{}; column < numberOfColumns; column++) {
                if (arrayWords.at(line, column) > 0) {
                    gridWords.at(line, column) = arrayWords.at(line, column);
                } else {
                    gridWords.at(line, column) = randomWord(gen);
                }
            }
        }
    }

public:
    explicit PuzzleWords(uint32_t numberOfWordsToGenerate = std::numeric_limits<uint32_t>::max()) {
        static_assert(numberOfColumns >= minWordlength,
                      "please increase number of column or decrease min size of words to be found");
        static_assert(numberOfLines >= minWordlength,
                      "please increase number of lines or decrease min size of words to be found");
        numberOfWordsToFind = numberOfWordsToGenerate;
        numberOfWordsInDictionary = listWords.size();
    }

    [[nodiscard]] std::vector<std::string> getWordsOfGrid() const {
        std::vector<std::string> listOfWordsInGrid{};
        listOfWordsInGrid.reserve(wordsInfo.size());
        for (const auto &info: wordsInfo) {
            listOfWordsInGrid.emplace_back(info.word);
        }
        return listOfWordsInGrid;
    }

    [[nodiscard]] const std::array<std::array<uint8_t, numberOfColumns>, numberOfLines> &getGrid() const {
        return gridWords.getElement();
    }

    [[nodiscard]] const std::vector<WordInfo> &getWordsInfo() const {
        return wordsInfo;
    }

    [[nodiscard]] size_t getWordCount() const {
        return wordsInfo.size();
    }

    [[nodiscard]] size_t getRemainingWordCount() const {
        size_t count = 0;
        for (const auto &info: wordsInfo) {
            if (!info.isStrikethrough) count++;
        }
        return count;
    }

    bool strikethroughWord(size_t wordIndex) {
        if (wordIndex < wordsInfo.size()) {
            wordsInfo.at(wordIndex).isStrikethrough = true;
            return true;
        }
        return false;
    }

    [[nodiscard]] int getNextWordToStrikethrough() const {
        for (size_t i = 0; i < wordsInfo.size(); i++) {
            if (!wordsInfo.at(i).isStrikethrough) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    [[nodiscard]] bool areAllWordsStrikethrough() const {
        return getRemainingWordCount() == 0;
    }

    void resetStrikethrough() {
        for (auto &info: wordsInfo) {
            info.isStrikethrough = false;
        }
    }

    [[nodiscard]] static constexpr uint8_t getNumberOfLines() { return numberOfLines; }
    [[nodiscard]] static constexpr uint8_t getNumberOfColumns() { return numberOfColumns; }
    [[nodiscard]] static constexpr uint8_t getMinWordLength() { return minWordlength; }

    void generateGridOfWords() {
        constexpr uint32_t maxNumberOfTry{numberOfLines * numberOfColumns * 2};
        gridWords = Grid{};
        pickedUpWords.clear();
        wordsInfo.clear();
        wordsInfo.reserve(numberOfWordsToFind);

        Grid arrayWords{};

        auto &gen = getRandomGenerator();
        std::uniform_int_distribution<uint8_t> randomDirectionGen(0, static_cast<uint8_t>(Direction::Invalid) - 1);

        uint32_t tryToFindWord{0};

        while (tryToFindWord < maxNumberOfTry && pickedUpWords.size() < numberOfWordsToFind) {
            tryToFindWord++;
            WordParam word{};

            word.direction = static_cast<Direction>(randomDirectionGen(gen));
            selectRandomBox(arrayWords, word);
            selectRandomSize(word);

            if (word.length != invalidValue && word.line != invalidValue && word.column != invalidValue) {
                const auto constraints = buildWordConstraints(arrayWords, word);

                findWordAccordingToConstraints(constraints, word);
                if (word.id != invalidValue) {
                    fillGridWithWord(word, arrayWords);
                    pickedUpWords.push_back(word.id);
                }
            }
        }
        completeGridWithRandomLetters(arrayWords);
    }
};
