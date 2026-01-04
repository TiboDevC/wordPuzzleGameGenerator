#pragma once

#include <array>
#include <random>
#include <algorithm>
#include <vector>
#include <string_view>
#include <limits>
#include <unordered_set>

#include "wordList.hpp"


template<uint8_t numberOfLines, uint8_t numberOfColumns>
class PuzzleWords {
private:
    static constexpr uint8_t invalidValue{0xFF};
    static constexpr uint8_t minWordlength{4};
    static constexpr uint8_t maxNumberOfTriesToPickWord{100};
    static constexpr size_t potentialWordsReserveSize{100};
    static constexpr uint16_t relaxConstraintsAfterAttempts{100};
    static constexpr uint16_t maxTotalAttempts{1000};
    static constexpr uint8_t maxFirstWordPlacementAttempts{10};
    static constexpr uint8_t positionsToSamplePerWord{5};
    static constexpr uint8_t maxWordLengthSupported{32};

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
    std::unordered_set<uint32_t> pickedUpWordsSet{};
    std::vector<WordInfo> wordsInfo{};
    uint32_t numberOfWordsToFind{std::numeric_limits<uint32_t>::max()};

    std::array<std::vector<uint32_t>, maxWordLengthSupported> wordsByLength{};

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

        // If length is not set, we can't determine the box
        if (word.length == invalidValue) {
            return;
        }

        uint8_t maxLineSize{};
        uint8_t maxColumnSize{};

        switch (word.direction) {
            case Direction::Line:
                if (word.length > numberOfColumns) {
                    return;
                }
                maxLineSize = numberOfLines - 1;
                maxColumnSize = numberOfColumns - word.length;
                break;
            case Direction::Column:
                if (word.length > numberOfLines) {
                    return;
                }
                maxLineSize = numberOfLines - word.length;
                maxColumnSize = numberOfColumns - 1;
                break;
            case Direction::Diagonal:
                if (word.length > numberOfLines || word.length > numberOfColumns) {
                    return;
                }
                maxLineSize = numberOfLines - word.length;
                maxColumnSize = numberOfColumns - word.length;
                break;
            case Direction::Invalid:
                return;
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

            if (!pickedUpWordsSet.contains(candidateWordId)) {
                word.id = candidateWordId;
                return;
            }
            numberOfTry++;
        }
    }

    void findWordAccordingToConstraints(const Constraints &constraints, WordParam &word) {
        word.id = invalidValue;

        // Use index to only check words of the right length
        if (word.length >= wordsByLength.size()) {
            return;
        }

        const auto &wordsOfLength = wordsByLength.at(word.length);
        if (wordsOfLength.empty()) {
            return;
        }

        std::vector<uint32_t> listPotentialWords;
        listPotentialWords.reserve(std::min<size_t>(potentialWordsReserveSize, wordsOfLength.size()));

        // Only iterate through words of the target length
        for (const uint32_t wordIndex: wordsOfLength) {
            const auto &wordView = listWords.at(wordIndex);

            bool isConstraintOk = true;
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

    [[nodiscard]] bool doesWordFit(const WordParam &word) const {
        switch (word.direction) {
            case Direction::Line:
                return (word.column + word.length <= numberOfColumns);
            case Direction::Column:
                return (word.line + word.length <= numberOfLines);
            case Direction::Diagonal:
                return (word.line + word.length <= numberOfLines) &&
                       (word.column + word.length <= numberOfColumns);
            case Direction::Invalid:
                return false;
        }
        return false;
    }

    [[nodiscard]] uint8_t countIntersections(const Grid &arrayWords, const WordParam &word) const {
        uint8_t intersections = 0;

        for (uint8_t i = 0; i < word.length; ++i) {
            uint8_t checkLine = word.line;
            uint8_t checkCol = word.column;

            switch (word.direction) {
                case Direction::Line:
                    checkCol += i;
                    break;
                case Direction::Column:
                    checkLine += i;
                    break;
                case Direction::Diagonal:
                    checkLine += i;
                    checkCol += i;
                    break;
                case Direction::Invalid:
                    break;
            }

            if (arrayWords.at(checkLine, checkCol) != 0) {
                intersections++;
            }
        }

        return intersections;
    }

    bool tryPlaceWord(WordParam &word, Grid &arrayWords, const uint16_t totalAttempts) {
        const uint8_t intersections = countIntersections(arrayWords, word);

        const bool acceptPosition = pickedUpWords.empty() ||
                                    intersections > 0 ||
                                    totalAttempts > relaxConstraintsAfterAttempts;

        if (!acceptPosition) {
            return false;
        }

        const auto constraints = buildWordConstraints(arrayWords, word);
        findWordAccordingToConstraints(constraints, word);

        if (word.id != invalidValue) {
            fillGridWithWord(word, arrayWords);
            pickedUpWords.push_back(word.id);
            pickedUpWordsSet.insert(word.id);
            return true;
        }

        return false;
    }

    bool tryPlaceWordAtPosition(WordParam &word, Grid &arrayWords, uint8_t line, uint8_t column,
                                const uint16_t totalAttempts) {
        word.line = line;
        word.column = column;

        if (!doesWordFit(word)) {
            return false;
        }

        return tryPlaceWord(word, arrayWords, totalAttempts);
    }

    bool tryPlaceFirstWord(Grid &arrayWords, uint8_t attempt, auto &gen,
                           std::uniform_int_distribution<uint8_t> &randomDirectionGen,
                           uint8_t maxPossibleLength) {
        WordParam firstWord{};
        firstWord.direction = static_cast<Direction>(randomDirectionGen(gen));

        // Try different starting positions
        if (attempt == 0) {
            firstWord.line = numberOfLines / 2;
            firstWord.column = numberOfColumns / 2;
        } else {
            std::uniform_int_distribution<uint8_t> posDist(0, std::max<uint8_t>(0, maxPossibleLength - minWordlength));
            firstWord.line = posDist(gen);
            firstWord.column = posDist(gen);
        }

        // Try to place a long word
        for (uint8_t len = maxPossibleLength; len >= minWordlength; --len) {
            firstWord.length = len;

            if (doesWordFit(firstWord)) {
                findWordAccordingToConstraints({}, firstWord);
                if (firstWord.id != invalidValue) {
                    fillGridWithWord(firstWord, arrayWords);
                    pickedUpWords.push_back(firstWord.id);
                    pickedUpWordsSet.insert(firstWord.id);
                    return true;
                }
            }
        }

        return false;
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

        for (uint32_t i = 0; i < numberOfWordsInDictionary; ++i) {
            const size_t len = listWords.at(i).length();
            if (len < wordsByLength.size()) {
                wordsByLength.at(len).push_back(i);
            }
        }
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
        gridWords = Grid{};
        pickedUpWords.clear();
        pickedUpWordsSet.clear();
        wordsInfo.clear();

        const uint32_t targetWords = (numberOfWordsToFind == std::numeric_limits<uint32_t>::max())
                                         ? numberOfLines * numberOfColumns / 4
                                         : numberOfWordsToFind;
        wordsInfo.reserve(targetWords);

        Grid arrayWords{};

        auto &gen = getRandomGenerator();
        std::uniform_int_distribution<uint8_t> randomDirectionGen(0, static_cast<uint8_t>(Direction::Invalid) - 1);

        const uint8_t maxPossibleLength = std::max(numberOfLines, numberOfColumns);

        // Place first word - try multiple positions if center fails
        bool firstWordPlaced = false;
        for (uint8_t attempt = 0; attempt < maxFirstWordPlacementAttempts && !firstWordPlaced; ++attempt) {
            firstWordPlaced = tryPlaceFirstWord(arrayWords, attempt, gen, randomDirectionGen, maxPossibleLength);
        }

        // If we couldn't place first word, abort
        if (!firstWordPlaced) {
            completeGridWithRandomLetters(arrayWords);
            return;
        }

        // Place remaining words with aggressive limits for MCU
        placeRemainingWords(arrayWords, targetWords, maxPossibleLength, gen, randomDirectionGen);

        completeGridWithRandomLetters(arrayWords);
    }

private:
    void placeRemainingWords(Grid &arrayWords, uint32_t targetWords, uint8_t maxPossibleLength,
                             std::mt19937 &gen, std::uniform_int_distribution<uint8_t> &randomDirectionGen) {
        uint16_t totalAttempts = 0;

        std::uniform_int_distribution<uint8_t> lengthDist(minWordlength, maxPossibleLength);
        std::uniform_int_distribution<uint8_t> lineDist(0, numberOfLines - 1);
        std::uniform_int_distribution<uint8_t> colDist(0, numberOfColumns - 1);

        while (pickedUpWords.size() < targetWords && totalAttempts < maxTotalAttempts) {
            totalAttempts++;

            WordParam word{};
            word.direction = static_cast<Direction>(randomDirectionGen(gen));
            word.length = lengthDist(gen);

            for (uint8_t sample = 0; sample < positionsToSamplePerWord; ++sample) {
                const uint8_t line = lineDist(gen);
                const uint8_t column = colDist(gen);

                if (tryPlaceWordAtPosition(word, arrayWords, line, column, totalAttempts)) {
                    break; // Word placed successfully
                }
            }
        }
    }
};
