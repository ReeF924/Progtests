#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool isWhite;
    bool isQueen;
} Piece;

typedef struct {
    signed char row;
    signed char col;
    Piece *piece;
} PiecePosition;

typedef struct {
    Piece *data[26][26];
    signed char count;
} Array2D;

typedef struct {
    Piece *data;
    int size;
    int count;
} PiecePositionArray;

typedef struct {
    signed char row;
    signed char col;
} Position;

void createPositionArray(PiecePositionArray *array, const int size) {
    array->size = size;
    array->count = 0;
    array->data = (Piece *) malloc(size * sizeof(Piece));
}

void push(PiecePositionArray *array, const Piece *piece) {
    if (array->count == array->size) {
        array->size *= 2;
        array->data = (Piece *) realloc(array->data, array->size * sizeof(Piece));
        if (array->data == nullptr) {
            printf("Memory allocation failed.\n");
            exit(1);
        }
    }
    array->data[array->count++] = *piece;
}

void clearPositionArray(PiecePositionArray *array) {
    array->count = 0;
}

void freePositionArray(PiecePositionArray *array) {
    free(array->data);
}

void freeBoard(Array2D *array) {
    for (int i = 0; i < array->count; ++i) {
        for (int j = 0; j < array->count; ++j) {
            if (array->data[i][j] != nullptr) {
                free(array->data[i][j]);
            }
        }
    }
}

void addToPosition(Array2D *array, Piece *piece, const signed char rowIndex, const signed char colIndex) {
    if (array->data[rowIndex][colIndex] != nullptr) {
        printf("Nespravny vstup.\n");
        exit(1);
    }
    array->data[rowIndex][colIndex] = piece;
}

void checkArrayIsNull(const Array2D array) {
    for (int i = 0; i < array.count; ++i) {
        for (int j = 0; j < array.count; ++j) {
            if (array.data[i][j] == nullptr) {
                printf("Null\n");
            }
        }
    }
}

bool checkBoundaries(const signed char row, const signed char col, const signed char maxFieldIndex) {
    return row >= 0 && row < maxFieldIndex && col >= 0 && col < maxFieldIndex;
}

char getCharFromRow(const signed char number) {
    return (char) (number + 'a');
}

void printMove(const char *prefixString, const signed char row, const signed char col, const signed char newRow,
               const signed char newCol, const short eatenPieces) {
    const char colChar = getCharFromRow(col);
    const char newColChar = getCharFromRow(newCol);
    if (eatenPieces > 0) {
        printf("%s%c%d -> %c%d +%d\n", prefixString, colChar, row + 1, newColChar, newRow + 1, eatenPieces);
    } else {
        printf("%s%c%d -> %c%d\n", prefixString, colChar, row + 1, newColChar, newRow + 1);
    }
}

void concatMoveString(char *newPrefix, const char *prefixString, short strLen, const signed char row,
                      const signed char col) {

    const char colChar = getCharFromRow(col);

    strncpy(newPrefix, prefixString, strLen);
    newPrefix[strLen] = colChar;

    //adds the new row number (can be 2 digits)
    snprintf(&newPrefix[strLen + 1], 5, "%d", row + 1);

    strLen = (short) strlen(newPrefix);

    strcpy(&newPrefix[strLen], " -> ");
}

int moveAllPieces(Array2D *array, signed char maxFieldIndex);

void playRock(Array2D *array, signed char row, signed char col,
              int *movesCount, signed char maxFieldIndex);

void jumpRock(Array2D *array, const char *prefixString, char row, char col, char maxFieldIndex,
              short eatenPieces, int *movesCount);

void playQueen(Array2D *array, signed char row, signed char col,
               int *movesCount, signed char maxFieldIndex);

void jumpQueen(Array2D *array, const char *prefixString, char row, char col,
               char maxFieldIndex, short eatenPieces, int *movesCount, Position comingFrom);


int main(void) {
    printf("Velikost hraci plochy:\n");
    int boardSize;
    int res = scanf(" %d", &boardSize);

    if (res != 1 || boardSize < 3 || boardSize > 26) {
        printf("Nespravny vstup.\n");
        return 1;
    }

    // char newLine;
    // scanf("%c", &newLine);
    // if (newLine != '\n') {
    //     printf("Nespravny vstup.\n");
    //     return 1;
    // }

    Array2D array;
    array.count = (signed char) boardSize;
    memset(array.data, 0, sizeof(array.data));

    printf("Pozice kamenu:\n");


    char colorType;
    char col;
    int row;

    res = scanf(" %c %c%d", &colorType, &col, &row);

    while (res != EOF) {

        //for easier debug
        // if (colorType == '/') {
        //     break;
        // }

        if (res != 3 || col < 'a' || col > 'a' + boardSize - 1 || row < 1 || row > boardSize) {
            printf("Nespravny vstup.\n");
            return 1;
        }

        col -= 'a';
        row--;

        if (!((row % 2 == 0 && col % 2 == 0) || (row % 2 == 1 && col % 2 == 1))) {
            printf("Nespravny vstup.\n");
            return 1;
        }

        Piece *piece;
        switch (colorType) {
            case 'b':
                piece = (Piece *) malloc(sizeof(Piece));

                piece->isQueen = false;
                piece->isWhite = false;

                addToPosition(&array, piece, (signed char) row, (signed char) col);
                break;
            case 'w':
                piece = (Piece *) malloc(sizeof(Piece));
                piece->isQueen = false;
                piece->isWhite = true;
                addToPosition(&array, piece, (signed char) row, (signed char) col);
                break;
            case 'B':
                piece = (Piece *) malloc(sizeof(Piece));
                piece->isQueen = true;
                piece->isWhite = false;
                addToPosition(&array, piece, (signed char) row, (signed char) col);
                break;
            case 'W':
                piece = (Piece *) malloc(sizeof(Piece));
                piece->isQueen = true;
                piece->isWhite = true;
                addToPosition(&array, piece, (signed char) row, (signed char) col);
                break;
            default:
                printf("Nespravny vstup.\n");
                return 1;
        }
        res = scanf(" %c %c%d", &colorType, &col, &row);
    }

    int moves = moveAllPieces(&array, (signed char) boardSize);
    printf("Celkem ruznych tahu: %d\n", moves);

    freeBoard(&array);
    return 0;
}

int moveAllPieces(Array2D *const array, const signed char maxFieldIndex) {
    int movesCount = 0;

    for (signed char row = 0; row < maxFieldIndex; ++row) {
        for (signed char col = 0; col < maxFieldIndex; ++col) {
            const Piece *piece = array->data[row][col];
            if (piece != nullptr && piece->isWhite) {
                if (piece->isQueen) {
                    //queen moves

                    //remove the queen from the board (not needed for standard rock)
                    PiecePosition playingPiece = {row, col, (Piece *) piece};
                    array->data[row][col] = nullptr;
                    //playQueen
                    playQueen(array, row, col, &movesCount, maxFieldIndex);
                    array->data[row][col] = playingPiece.piece;
                    continue;
                }
                //rock moves

                playRock(array, row, col, &movesCount, maxFieldIndex);
            }
        }
    }
    return movesCount;
}


void playRock(Array2D *const array, const signed char row, const signed char col,
              int *movesCount, const signed char maxFieldIndex) {
    const char *prefixString = "";
    bool movedRight = false;

    //move right
    if (checkBoundaries((signed char) (row + 1), (signed char) (col + 1), maxFieldIndex)) {
        //can it move there?
        if (array->data[row + 1][col + 1] == nullptr) {
            //move
            printMove(prefixString, row, col, (signed char) (row + 1), (signed char) (col + 1), 0);
            (*movesCount)++;
            movedRight = true;
        }
    }
    //move left
    if (checkBoundaries((signed char) (row + 1), (signed char) (col - 1), maxFieldIndex)) {
        //can it move there?
        if (array->data[row + 1][col - 1] == nullptr) {
            //move
            printMove(prefixString, row, col, (signed char) (row + 1), (signed char) (col - 1), 0);
            (*movesCount)++;
            //in case it can move both ways, it can't jump
            if (movedRight) {
                return;
            }
        }
    }

    //jumpRock
    jumpRock(array, prefixString, row, col, maxFieldIndex, 0, movesCount);
}

void jumpRock(Array2D *const array, const char *prefixString, const char row, const char col,
              const char maxFieldIndex, const short eatenPieces, int *movesCount) {
    //twice, once for right and once for left move
    for (int colDirection = 1; colDirection >= -1; colDirection -= 2) {
        //for easier debug
        // Piece *adjacentPiece = array->data[row + 1][col + colDirection];
        // Position piecePos;
        // piecePos.row = (signed char) (row + 1);
        // piecePos.col = (signed char) (col + colDirection);
        // Piece *targetFieldPiece = array->data[row + 2][col + colDirection * 2];


        if (!checkBoundaries((signed char) (row + 2), (signed char) (col + colDirection * 2), maxFieldIndex)) {
            continue;
        }
        if (array->data[row + 1][col + colDirection] == nullptr) {
            continue;
        }
        if (array->data[row + 2][col + colDirection * 2] != nullptr) {
            continue;
        }
        if (array->data[row + 1][col + colDirection]->isWhite) {
            continue;
        }

        //save eaten piece, so we can return it back in the end
        const PiecePosition eatenPiece = {
            (signed char) (row + 1), (signed char) (col + colDirection), array->data[row + 1][col + colDirection]
        };
        array->data[row + 1][col + colDirection] = nullptr;

        printMove(prefixString, row, col, (signed char) (row + 2), (signed char) (col + colDirection * 2),
                  (short) (eatenPieces + 1));

        (*movesCount)++;

        //adjust prefixString for possible printing
        const short strLen = (short) strlen(prefixString);
        char *newPrefix = (char *) malloc(strLen + 8);

        if (newPrefix == nullptr) {
            printf("Memory allocation failed.\n");
            exit(1);
        }
        concatMoveString(newPrefix, prefixString, strLen, row, col);

        //recursive call
        jumpRock(array, newPrefix, (signed char) (row + 2), (signed char) (col + colDirection * 2), maxFieldIndex,
                 (short) (eatenPieces + 1), movesCount);

        array->data[eatenPiece.row][eatenPiece.col] = eatenPiece.piece;
        free(newPrefix);
    }
}


void playQueen(Array2D *const array, const signed char row, const signed char col,
               int *movesCount, const signed char maxFieldIndex) {
    const char *prefixString = "";


    for (signed char rowDirection = 1; rowDirection >= -1; rowDirection -= 2) {
        for (signed char colDirection = 1; colDirection >= -1; colDirection -= 2) {
            signed char newRow = (signed char)(row + rowDirection);
            signed char newCol = (signed char)(col + colDirection);

            while (checkBoundaries(newRow, newCol, maxFieldIndex) && array->data[newRow][newCol] == nullptr) {
                printMove(prefixString, row, col, newRow, newCol, 0);
                (*movesCount)++;

                newRow = (signed char) (newRow + rowDirection);
                newCol = (signed char) (newCol + colDirection);
            }
        }
    }

    //hasn't jumped yet, so can jump in all directions
    Position comingFrom = {0, 0};
    //jumpQueen
    jumpQueen(array, prefixString, row, col, maxFieldIndex, 0, movesCount, comingFrom);
}

void jumpQueen(Array2D *const array, const char *prefixString, const char row, const char col,
               const char maxFieldIndex, const short eatenPieces, int *movesCount, const Position comingFrom) {
    // Piece *adjacentPiece = array->data[row + rowDirection][col + colDirection];
    // Position piecePos;
    // piecePos.row = (signed char) (row + rowDirection);
    // piecePos.col = (signed char) (col + colDirection);
    // Piece *targetFieldPiece = array->data[row + rowDirection * 2][col + colDirection * 2];


    //for with 3 directions (all 4 directions except the one it came from (if it's not first jump) (gonna need an arg for that)
    for (signed char rowDirection = 1; rowDirection >= -1; rowDirection -= 2) {
        for (signed char colDirection = 1; colDirection >= -1; colDirection -= 2) {
            if (rowDirection == comingFrom.row && colDirection == comingFrom.col) {
                continue;
            }


            Piece *foundPiece = nullptr;
            signed char rowOffset = rowDirection;
            signed char colOffset = colDirection;

            //find the first piece in the direction
            while (checkBoundaries((signed char) (row + rowOffset), (signed char) (col + colOffset),
                                   maxFieldIndex)) {
                if (array->data[row + rowOffset][col + colOffset] != nullptr) {
                    foundPiece = array->data[row + rowOffset][col + colOffset];
                    break;
                }

                rowOffset = (signed char) (rowOffset + rowDirection);
                colOffset = (signed char) (colOffset + colDirection);
            }

            if (foundPiece == nullptr || foundPiece->isWhite) {
                continue;
            }

            //jump

            //save eaten piece, so we can return it back in the end
            const PiecePosition eatenPiece = {
                (signed char) (row + rowOffset), (signed char) (col + colOffset),
                array->data[row + rowOffset][col + colOffset]
            };

            array->data[row + rowOffset][col + colOffset] = nullptr;
            rowOffset = (signed char) (rowOffset + rowDirection);
            colOffset = (signed char) (colOffset + colDirection);

            //jump to all the possible squares
            while (checkBoundaries((signed char) (row + rowOffset),
                                   (signed char) (col + colOffset), maxFieldIndex)) {
                if (array->data[row + rowOffset][col + colOffset] != nullptr) {
                    break;
                }


                printMove(prefixString, row, col, (signed char) (row + rowOffset), (signed char) (col + colOffset),
                          (short) (eatenPieces + 1));

                (*movesCount)++;

                //adjust prefixString for possible printing
                const short strLen = (short) strlen(prefixString);
                char *newPrefix = (char *) malloc(strLen + 8);

                if (newPrefix == nullptr) {
                    printf("Memory allocation failed.\n");
                    exit(1);
                }
                concatMoveString(newPrefix, prefixString, strLen, row, col);

                //so it can't turn 180 degrees
                Position oppositeComingFrom = {(signed char) -rowDirection, (signed char) -colDirection};

                //recursive call
                jumpQueen(array, newPrefix, (signed char) (row + rowOffset), (signed char) (col + colOffset),
                          maxFieldIndex, (short) (eatenPieces + 1), movesCount, oppositeComingFrom);

                free(newPrefix);
                rowOffset = (signed char) (rowOffset + rowDirection);
                colOffset = (signed char) (colOffset + colDirection);
            }

            array->data[eatenPiece.row][eatenPiece.col] = eatenPiece.piece;
        }
    }
}