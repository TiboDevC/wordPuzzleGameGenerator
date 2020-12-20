#ifndef PUZZLEWORD_GENERATEMOTSMELES_H
#define PUZZLEWORD_GENERATEMOTSMELES_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tgmath.h>

#include "wordList.h"

#define NUMBER_OF_LINES      8
#define NUMBER_OF_COLUMNS    8
#define WORD_MIN_SIZE        4

#define MAX_NUM_OF_CANDIDATE_WORD 4000
#define INVALIE_VALUE 0xFFFF

char *wordsInGrid[50];
unsigned char numberOfWordsInGrid = 0;

unsigned char globalGrid[NUMBER_OF_LINES][NUMBER_OF_COLUMNS];
unsigned int candidateWords[MAX_NUM_OF_CANDIDATE_WORD];
unsigned char constraint[NUMBER_OF_LINES * 2];

enum Direction {
	eLine,
	eColumn,
	eDiagonal
};

unsigned int random_number(const unsigned int min_num, const unsigned int max_num) {
	unsigned int result = 0;

	if (min_num < max_num) {
		result = (rand() % (max_num - min_num)) + min_num;
	} else {
		result = min_num;
	}

	return result;
}

void initGrid(unsigned char grid[NUMBER_OF_LINES][NUMBER_OF_COLUMNS]) {
	for (unsigned char indexLine = 0; indexLine < NUMBER_OF_LINES; indexLine++) {
		for (unsigned char indexColumn = 0; indexColumn < NUMBER_OF_COLUMNS; indexColumn++) {
			grid[indexLine][indexColumn] = 0;
		}
	}
	for (unsigned char wordIndex = 0; wordIndex < numberOfWordsInGrid; wordIndex++) {
		if (wordsInGrid[wordIndex] != NULL) {
			free(wordsInGrid[wordIndex]);
		}
	}
	numberOfWordsInGrid = 0;
}

unsigned char pickUpRandomBox(unsigned char *line, unsigned char *column,
                              const unsigned char grid[NUMBER_OF_LINES][NUMBER_OF_COLUMNS]) {
	unsigned int numOfTry = 0;
	do {
		numOfTry++;
		*line = random_number(0, NUMBER_OF_LINES - WORD_MIN_SIZE + 1);
		*column = random_number(0, NUMBER_OF_COLUMNS - WORD_MIN_SIZE + 1);
	} while (grid[*line][*column] != 0 && numOfTry < NUMBER_OF_COLUMNS * NUMBER_OF_LINES * 20);

	if (grid[*line][*column] == 0) {
		return 1;
	}
	return 0;
}

void pickUpRandomDirection(enum Direction *direction) {
	*direction = random_number(0, eDiagonal);
}

void pickUpRandomWordSize(unsigned char *sizeWord, const enum Direction direction, const unsigned char line,
                          const unsigned char column) {
	unsigned char maxLenght = WORD_MIN_SIZE;
	switch (direction) {
		case eLine:
			maxLenght = NUMBER_OF_LINES - line;
			break;
		case eColumn:
			maxLenght = NUMBER_OF_COLUMNS - column;
			break;
		case eDiagonal:
			if (NUMBER_OF_LINES > NUMBER_OF_COLUMNS) {
				maxLenght = NUMBER_OF_COLUMNS - column;
			} else {
				maxLenght = NUMBER_OF_LINES - line;
			}
			break;
		default:
			break;
	}
	*sizeWord = random_number(WORD_MIN_SIZE, maxLenght);
}

void buildConstraint(const unsigned char line, const unsigned char column, const unsigned char direction,
                     const unsigned char wordSize, unsigned char grid[NUMBER_OF_LINES][NUMBER_OF_COLUMNS]) {
	unsigned char lineIndex = line;
	unsigned char columnIndex = column;

	for (unsigned char wordIndex = 0; wordIndex < wordSize; wordIndex++) {
		constraint[wordIndex] = grid[lineIndex][columnIndex];

		if (direction == eLine || direction == eDiagonal) {
			columnIndex++;
		}
		if (direction == eColumn || direction == eDiagonal) {
			lineIndex++;
		}
	}
}

unsigned int pickUpWord(const unsigned char wordSize) {

	for (unsigned int indexWordCandidate = 0; indexWordCandidate < MAX_NUM_OF_CANDIDATE_WORD; indexWordCandidate++) {
		candidateWords[indexWordCandidate] = INVALIE_VALUE;
	}
	unsigned int numberOfCandidateWords = 0;

	for (unsigned int indexWordList = 0; indexWordList < listWordsSize[wordSize] &&
	                                     numberOfCandidateWords < MAX_NUM_OF_CANDIDATE_WORD; indexWordList++) {
		unsigned char isCandidateWordOk = 1;
		for (unsigned char wordletterIndex = 0; wordletterIndex < wordSize; wordletterIndex++) {
			if (constraint[wordletterIndex] > 0 &&
			    listWords[wordSize][indexWordList][wordletterIndex] != constraint[wordletterIndex]) {
				isCandidateWordOk = 0;
				break;
			}
		}
		if (isCandidateWordOk) {
			candidateWords[numberOfCandidateWords] = indexWordList;
			numberOfCandidateWords++;
		}
	}
	if (numberOfCandidateWords > 0) {
		return candidateWords[random_number(0, numberOfCandidateWords)];
	}
	return INVALIE_VALUE;
}

void addWordIngrid(const unsigned int pickedUpWord, const unsigned char wordSize, const unsigned char line,
                   const unsigned char column, const enum Direction direction,
                   unsigned char grid[NUMBER_OF_LINES][NUMBER_OF_COLUMNS]) {
	if (pickedUpWord != INVALIE_VALUE) {
		unsigned char lineIndex = line;
		unsigned char columnIndex = column;

		wordsInGrid[numberOfWordsInGrid] = (char *) malloc(sizeof(char) * wordSize + 1);

		for (unsigned char indexLetterWord = 0; indexLetterWord < wordSize; indexLetterWord++) {
			grid[lineIndex][columnIndex] = listWords[wordSize][pickedUpWord][indexLetterWord];
			wordsInGrid[numberOfWordsInGrid][indexLetterWord] = listWords[wordSize][pickedUpWord][indexLetterWord];

			if (direction == eLine || direction == eDiagonal) {
				columnIndex++;
			}
			if (direction == eColumn || direction == eDiagonal) {
				lineIndex++;
			}
		}
		wordsInGrid[numberOfWordsInGrid][wordSize] = '\0';
		numberOfWordsInGrid++;
	}
}

void completeGrid(unsigned char grid[NUMBER_OF_LINES][NUMBER_OF_COLUMNS])
{
	for (unsigned lineIndex = 0; lineIndex < NUMBER_OF_LINES; lineIndex++) {
		for (unsigned columnIndex = 0; columnIndex < NUMBER_OF_COLUMNS; columnIndex++) {
			if (grid[lineIndex][columnIndex] == 0) {
				grid[lineIndex][columnIndex] = random_number('a', 'z');
			}
		}
	}
}

void fillGrid() {
	unsigned char line = 0;
	unsigned char column = 0;
	enum Direction direction = eLine;
	unsigned char wordSize = WORD_MIN_SIZE;
	srand(time(NULL));

	initGrid(globalGrid);

	for (unsigned int it = 0; it < NUMBER_OF_COLUMNS * NUMBER_OF_LINES * 10; it++) {

		if (pickUpRandomBox(&line, &column, globalGrid)) {
			pickUpRandomDirection(&direction);
			pickUpRandomWordSize(&wordSize, direction, line, column);
			buildConstraint(line, column, direction, wordSize, globalGrid);
			addWordIngrid(pickUpWord(wordSize), wordSize, line, column, direction, globalGrid);
		}
	}
	completeGrid(globalGrid);
}

#endif //PUZZLEWORD_GENERATEMOTSMELES_H
