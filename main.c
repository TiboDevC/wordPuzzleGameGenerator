#include "generateMotsMeles.h"

int main(void)
{
	fillGrid();

	printf("listWords: %d\n", (int) numberOfWordsInGrid);
	for (unsigned char wordInList = 0; wordInList < numberOfWordsInGrid; wordInList++) {
		printf("- %s\n", wordsInGrid[wordInList]);
	}

	for (unsigned lineIndex2 = 0; lineIndex2 < NUMBER_OF_LINES; lineIndex2++) {
		for (unsigned columnIndex2 = 0; columnIndex2 < NUMBER_OF_COLUMNS; columnIndex2++) {
			if (globalGrid[lineIndex2][columnIndex2] > 0) {
				printf("%c ", globalGrid[lineIndex2][columnIndex2]);
			} else {
				printf("  ");
			}
		}
		printf("\n");
	}

	return 1;
}
