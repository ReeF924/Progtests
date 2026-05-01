#include <stdio.h>
#include <stdlib.h>


typedef struct {
    size_t row;
    size_t col;
} Position;

typedef struct {
    Position *data;
    size_t size;
    size_t count;
} Array;

typedef struct {
    char *data;
    size_t size;
    size_t count;
} String;

constexpr char ARRAY_2D_SIZE = 26;

typedef struct {
    Array array[ARRAY_2D_SIZE];
} Static2DArray;

typedef struct {
    char value;
    bool tagged;
} TagChar;

typedef struct {
    TagChar **array;
    size_t rowCount;
    size_t colSize;
} Dynamic2DArray;

void createArray(Array *array, const size_t size) {
    array->data = (Position *) malloc(size * sizeof(Position));
    array->size = size;
    array->count = 0;
}

void createString(String *array, const size_t size) {
    array->data = (char *) malloc(size * sizeof(Position));
    array->size = size;
    array->count = 0;
}

void create2DStaticArray(Static2DArray *array) {
    for (int i = 0; i < ARRAY_2D_SIZE; i++) {
        Array subArr = {nullptr, 0, 0};
        createArray(&subArr, 10);
        array->array[i] = subArr;
    }
}

void create2DDynamicArray(Dynamic2DArray *array, const size_t colSize) {
    array->array = (TagChar **) malloc(sizeof(array->array));
    array->rowCount = 0;
    array->colSize = colSize;
}

void push(Dynamic2DArray *array, const size_t row, const size_t col, const char value) {
    if (array->rowCount <= row) {
        array->rowCount += 1;
        array->array = (TagChar **) realloc(array->array, array->rowCount * sizeof(array->array[0]));
    }

    if (col == 0) {
        array->array[row] = (TagChar *) malloc(array->colSize * sizeof(array->array[0]));
    }

    const TagChar tagChar = {value, false};
    array->array[row][col] = tagChar;
}

void pushPosition(Static2DArray *array, const int index, const Position pos) {
    //todo maybe rewrite, use a pointer var so it's not as messy


    if (array->array[index].count == array->array[index].size) {
        array->array[index].size *= 2;
        array->array[index].data = (Position *) realloc(array->array[index].data,
                                                        array->array[index].size * sizeof(array->array[index].data[0]));
    }

    array->array[index].data[array->array[index].count++] = pos;
}

void pushDic(Array *array, Position pos) {
    if (array->count == array->size) {
        array->size *= 2;
        array->data = (Position *) realloc(array->data, array->size * sizeof(array->data[0]));
    }
    array->data[array->count++] = pos;
}

void pushString(String *array, const char value) {
    if (array->count == array->size) {
        array->size *= 2;
        array->data = (char *) realloc(array->data, array->size * sizeof(array->data[0]));
    }
    array->data[array->count++] = value;
}

void freeDic(const Static2DArray *array) {
    for (int i = 0; i < ARRAY_2D_SIZE; i++) {
        free(array->array[i].data);
    }
}

void freeArray(const Dynamic2DArray *array) {
    for (size_t i = 0; i < array->rowCount; ++i) {
        free(array->array[i]);
    }
    free(array->array);
}

void freeArrays(const Dynamic2DArray *array, const Static2DArray *dic) {
    freeArray(array);
    freeDic(dic);
}

void loadCommandString(String *string);

void printTopSecret(const Dynamic2DArray *array);

void TagFoundChars(const Dynamic2DArray *array, Position pos, int rowDir, int colDir, size_t length);

int MatchWord(const Dynamic2DArray *array, const Static2DArray *dic, const String *word, char command);

int main(void) {
    printf("Osmismerka:\n");

    Static2DArray dic;
    Dynamic2DArray array;
    create2DStaticArray(&dic);

    char input;
    String string;
    createString(&string, 20);

    int res = scanf("%c", &input);

    do {
        if (res == EOF || res != 1) {
            printf("Nespravny vstup.1\n");
            free(string.data);
            freeDic(&dic);
            return 1;
        }
        const int charIndex = input - 'a';
        if (charIndex < 0 || charIndex > 26) {
            if (input == '.') {
                pushString(&string, input);
                res = scanf("%c", &input);
                continue;
            }
            printf("Nespravny vstup.\n");
            free(string.data);
            freeDic(&dic);
            return 1;
        }

        const Position pos = {0, string.count};
        pushDic(&dic.array[charIndex], pos);
        pushString(&string, input);
        res = scanf("%c", &input);

    } while (input != '\n');

    create2DDynamicArray(&array, string.count);
    for (size_t i = 0; i < string.count; i++) {
        push(&array, 0, i, string.data[i]);
    }
    size_t row = 1;
    const size_t colLength = string.count;
    free(string.data);


    while (true) {
        res = scanf("%c", &input);
        //empty line, end of input
        if (input == '\n') {
            break;
        }
        if (res == EOF) {
            printf("Nespravny vstup.\n");
            freeArrays(&array, &dic);
            return 1;
        }

        for (size_t col = 0; col < colLength; ++col) {
            if (res != 1) {
                printf("Nespravny vstup.\n");
                freeArrays(&array, &dic);
                return 1;
            }

            const int charIndex = input - 'a';
            if (charIndex < 0 || charIndex > 26) {
                if (input == '.') {
                    push(&array, row, col, input);
                    res = scanf("%c", &input);
                    continue;
                }
                printf("Nespravny vstup.\n");
                freeArrays(&array, &dic);
                return 1;
            }
            push(&array, row, col, input);
            const Position pos = {row, col};
            pushDic(&dic.array[charIndex], pos);
            res = scanf("%c", &input);
        }
        if (input != '\n' && res != EOF) {
            printf("Nespravny vstup.\n");
            freeArrays(&array, &dic);
            return 1;
        }
        row++;
    }

    char command;
    res = scanf("%c", &command);
    if (res == EOF) {
        // printf("Nespravny vstup.\n");
        freeArrays(&array, &dic);
        return 1;
    }

    String commandString;
    while (res != EOF) {
        if (res != 1) {
            printf("Nespravny vstup.\n");
            freeArrays(&array, &dic);
            return 1;
        }

        if (command == '?') {
            res = scanf("%c", &command);
            if (res == EOF || command != '\n') {
                printf("Nespravny vstup.\n");
                freeArrays(&array, &dic);
                return 1;
            }

            printTopSecret(&array);
        } else if (command == '#' || command == '-') {
            createString(&commandString, 20);
            loadCommandString(&commandString);

            if (commandString.data == nullptr) {
                printf("Nespravny vstup.\n");
                freeArrays(&array, &dic);
                return 1;
            }
            if (commandString.count < 2) {
                printf("Nespravny vstup.\n");
                freeArrays(&array, &dic);
                free(commandString.data);
                return 1;
            }

            const int wordsFound = MatchWord(&array, &dic, &commandString, command);
            pushString(&commandString, '\0');
            printf("%s: %dx\n", commandString.data, wordsFound);
            free(commandString.data);
        } else {
            printf("Nespravny vstup.\n");
            freeArrays(&array, &dic);
            return 1;
        }

        res = scanf(" %c", &command);
    }
    freeArrays(&array, &dic);
    return 0;
}

void loadCommandString(String *string) {
    char input;
    int res = scanf(" %c", &input);


    while (input != '\n' && res != EOF) {
        if (res != 1) {
            free(string->data);
            string->data = nullptr;
            return;
        }

        const unsigned char charIndex = input - 'a';
        if (charIndex < 0 || charIndex > 26) {
            free(string->data);
            string->data = nullptr;
            return;
        }
        pushString(string, input);
        res = scanf("%c", &input);
    }
}

void printTopSecret(const Dynamic2DArray *array) {
    printf("Tajenka:\n");

    int counter = 0;
    for (size_t i = 0; i < array->rowCount; ++i) {
        for (size_t j = 0; j < array->colSize; ++j) {
            const TagChar c = array->array[i][j];

            if (!c.tagged && c.value != '.') {
                printf("%c", c.value);
                counter++;

                if (counter % 60 == 0) {
                    printf("\n");
                }
            }
        }
    }
    if (counter % 60 != 0) {
        printf("\n");
    }
}

int MatchWord(const Dynamic2DArray *array, const Static2DArray *dic, const String *word, const char command) {
    int wordsFound = 0;

    if (word->count == 0) {
        return 0;
    }

    const int charIndex = word->data[0] - 'a';
    const Array *dicSection = &dic->array[charIndex];

    //starting at bottom-right moving upwards
    constexpr int rowIncrement[8] = {-1, 0, 1, 1, 1, 0, -1, -1};
    constexpr int colIncrement[8] = {1, 1, 1, 0, -1, -1, -1, 0};

    for (size_t i = 0; i < dicSection->count; ++i) {
        for (int direction = 0; direction < 8; ++direction) {
            Position pos = dicSection->data[i];

            size_t wordIndex = 1;
            for (; wordIndex < word->count; ++wordIndex) {
                pos.row += 1 + rowIncrement[direction];
                pos.col += 1 + colIncrement[direction];

                // check if index is still in the grid (pos.row is index, rowCount isn't)
                if (pos.row <= 0 || pos.col <= 0 || array->rowCount + 1 <= pos.row || array->colSize + 1 <= pos.col) {
                    break;
                }
                pos.row -= 1;
                pos.col -= 1;

                const char compareChar = word->data[wordIndex];
                if (array->array[pos.row][pos.col].value != compareChar) {
                    break;
                }
            }
            //found the searched word
            if (wordIndex == word->count) {
                wordsFound++;
                if (command == '-') {
                    //tag chars
                    TagFoundChars(array, dicSection->data[i], rowIncrement[direction], colIncrement[direction],
                                  word->count);
                }
            }
        }
    }
    return wordsFound;
}

void TagFoundChars(const Dynamic2DArray *array, Position pos, const int rowDir, const int colDir, const size_t length) {
    for (size_t i = 0; i < length; ++i) {
        array->array[pos.row][pos.col].tagged = true;
        pos.row += rowDir;
        pos.col += colDir;
    }
}
