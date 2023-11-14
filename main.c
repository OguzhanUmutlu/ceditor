#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
//#include <wchar.h>
//#include <locale.h>
#include <sys/stat.h>
#include "main.h"

#pragma clang diagnostic push

chr **lines;
uint *lineSizes;
uint lineCount = 0;
uint cursorLine = 0;
uint cursorKey = 0;
char *filename;

uint findDigitAmount(uint num) {
    if (num < 10) return 1;
    if (num < 100) return 2; // maybe makes it a *little* faster
    uint f = 1;
    while (num > 9) {
        num /= 10;
        f++;
    }
    return f;
}

void printCRN(chr ch) { // print single char + replacing the null
    switch (ch) {
        case 0x00:
        case '\xe0':
        case 0x1b:
            putchar('?');
            break;
        case '\r':
        case '\t':
            putchar(' ');
            break;
        default:
            putchar(ch);
    }
}

void printRN(chr *str, uint size) { // print replacing nulls
    for (uint i = 0; i < size; i++) {
        printCRN(str[i]);
    }
}

void clearScreen() {
    system(CLEAR_SCREEN);
}

void moveCursor(int row, int col) {
    printf("\x1b[%d;%dH", row, col);
}

void moveCursorLeft(uint n) {
    if (n == 0) return;
    printf("\x1b[%zuD", n);
}

void moveCursorRight(uint n) {
    if (n == 0) return;
    printf("\x1b[%zuC", n);
}

void moveCursorUp(uint n) {
    if (n == 0) return;
    printf("\x1b[%zuA", n);
}

void moveCursorDown(uint n) {
    if (n == 0) return;
    printf("\x1b[%zuB", n);
}

void rerenderAll() {
    clearScreen();
    printf(C_YELLOW"Press CMD+Q to quit.\n"C_RESET);

    printf(C_CYAN"%s\n\n\n\n"C_RESET, filename);

    uint i = 0;
    if (lineCount > 0) {
        uint max_digits = findDigitAmount(lineCount);
        while (1) {
            printf(CB_WHITE C_BLACK "  %zu  ", i + 1);
            uint di = max_digits - findDigitAmount(i + 1);
            for (uint j = 0; j < di; j++) {
                putchar(' ');
            }
            printf(C_RESET " ");
            printRN(lines[i], lineSizes[i]);
            i++;
            if (i == lineCount) break;
            printf("\n");
        }
        moveCursorLeft(lineSizes[lineCount - 1]);
        moveCursorUp(lineCount - 1);
    } else {
        printf(CB_WHITE C_BLACK "  1  " C_RESET " ");
    }

    moveCursorRight(cursorKey);
    moveCursorDown(cursorLine);
}

void insertCharAtCursor(chr ch) {
    chr *currentLine = lines[cursorLine];
    uint size = lineSizes[cursorLine];

    chr *newLine = realloc(currentLine, size + 1);
    if (newLine == NULL) {
        fprintf(stderr, C_RED"1Memory allocation failed.\n"C_RESET);
        exit(EXIT_FAILURE);
    }

    for (uint i = size; i > cursorKey; i--) {
        newLine[i] = newLine[i - 1];
    }

    newLine[cursorKey] = ch;
    lineSizes[cursorLine]++;
    lines[cursorLine] = newLine;
}

void removeCharBeforeCursor() {
    if (cursorKey == 0) return;
    chr *currentLine = lines[cursorLine];
    uint size = lineSizes[cursorLine];

    for (uint i = cursorKey; i < size; i++) {
        currentLine[i - 1] = currentLine[i];
    }

    chr *newLine = realloc(currentLine, size <= 1 ? 1 : size - 1);
    if (newLine == NULL) {
        fprintf(stderr, C_RED"2Memory allocation failed.\n"C_RESET);
        exit(EXIT_FAILURE);
    }

    lineSizes[cursorLine]--;
    lines[cursorLine] = newLine;
}

void insertLineAfterCursor() {
    chr **newLines = realloc(lines, (lineCount + 1) * sizeof(chr *));
    uint *newLineSizes = realloc(lineSizes, (lineCount + 1) * sizeof(uint));
    for (uint i = lineCount; i > cursorLine + 1; i--) {
        newLines[i] = newLines[i - 1];
        lineSizes[i] = lineSizes[i - 1];
    }

    // normally this should be 0, but malloc(0) is undefined behavior
    newLines[cursorLine + 1] = malloc(1 * sizeof(chr));
    lineSizes[cursorLine + 1] = 0;
    lineCount++;
    lines = newLines;
    lineSizes = newLineSizes;
}

void onPressQuit() {
    printf(C_YELLOW"\n\nExiting..."C_RESET);
    exit(EXIT_SUCCESS);
}

void onPressSave() {
    uint size = 0;
    for (uint i = 0; i < lineCount; i++) {
        size += lineSizes[i] + (i == lineCount - 1 ? 0 : 1);
    }
    chr *content = malloc((size + 1) * sizeof(chr));

    uint ind = 0;
    for (uint i = 0; i < lineCount; i++) {
        uint s = lineSizes[i];
        chr *line = lines[i];
        for (uint j = 0; j < s; j++) {
            content[ind] = line[j];
            ind++;
        }
        if (i != lineCount - 1) content[ind] = '\n';
        ind++;
    }

    content[size] = '\0';

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, C_RED"0Error opening file for writing: %s\n"C_RESET, filename);
        free(content);
        exit(EXIT_FAILURE);
    }
    //fputws(content, file);
    fputs(content, file);
    fclose(file);
    free(content);
    moveCursorUp(cursorLine + 2);
    moveCursorLeft(cursorKey + 3);
    printf(C_GREEN"Saved."C_RESET);
    moveCursorLeft(6);

    moveCursorDown(cursorLine + 2);
    moveCursorRight(cursorKey + 3);
}

void onPressUpArrow() {
    if (cursorLine == 0) return;
    moveCursorUp(1);
    cursorLine--;
    uint max_key = lineSizes[cursorLine];
    if (cursorKey > max_key) {
        moveCursorLeft(cursorKey - max_key);
        cursorKey = max_key;
    }
}

void onPressDownArrow() {
    if (cursorLine == lineCount - 1) return;
    moveCursorDown(1);
    cursorLine++;
    uint max_key = lineSizes[cursorLine];
    if (cursorKey > max_key) {
        moveCursorLeft(cursorKey - max_key);
        cursorKey = max_key;
    }
}

void onPressLeftArrow() {
    if (cursorKey == 0) {
        if (cursorLine == 0) return;
        cursorLine--;
        uint max_key = lineSizes[cursorLine];
        moveCursorUp(1);
        moveCursorLeft(cursorKey);
        moveCursorRight(max_key);
        cursorKey = max_key;
        return;
    }
    moveCursorLeft(1);
    cursorKey--;
}

void onPressRightArrow() {
    if (cursorKey >= lineSizes[cursorLine]) {
        if (cursorLine == lineCount - 1) return;
        cursorLine++;
        moveCursorDown(1);
        moveCursorLeft(cursorKey);
        cursorKey = 0;
        return;
    }
    moveCursorRight(1);
    cursorKey++;
}

void onPressAnyArrow() {
    GET_NEXT_CHARACTER(ch);
    switch (ch) {
        case 'H': // up
            onPressUpArrow();
            break;
        case 'P': // down
            onPressDownArrow();
            break;
        case 'K': // left
            onPressLeftArrow();
            break;
        case 'M': // right
            onPressRightArrow();
            break;
        default:
            return;
    }
}

void onPressBackspace() {
    if (cursorKey == 0) {
        if (cursorLine == 0) return;
        cursorLine--;
        cursorKey = lineSizes[cursorLine];
        moveCursorUp(1);
        moveCursorRight(cursorKey);
        // todo: also append the current line to the new(x-1) one
        return;
    }
    moveCursorLeft(1);
    chr *line = lines[cursorLine];
    uint line_size = lineSizes[cursorLine];
    for (uint i = cursorKey; i < line_size; i++) {
        printCRN(line[i]);
    }
    putchar(' ');
    moveCursorLeft(line_size - cursorKey + 1);
    removeCharBeforeCursor();
    cursorKey--;
}

void onPressEnter() {
    moveCursorDown(1);
    moveCursorLeft(cursorKey + 3 + 10);
    insertLineAfterCursor();
    cursorKey = 0;
    cursorLine++;
    uint max_digits = findDigitAmount(lineCount);
    printf(CB_WHITE C_BLACK "  %zu  ", cursorLine + 1);
    uint di = max_digits - findDigitAmount(cursorLine + 1);
    for (uint j = 0; j < di; j++) {
        putchar(' ');
    }
    printf(C_RESET " ");
}

void handleKey(int key) {
    if (key == CMD_Q) return onPressQuit();
    if (key == CMD_S) return onPressSave();
    if (key == ARROW_BEGIN || key == 0) return onPressAnyArrow();
    if (key == BACKSPACE) return onPressBackspace();
    if (key == ENTER1 || key == ENTER2) return onPressEnter();
    if (key == CMD_D) {
        puts("\nCode:\n");
        for (uint i = 0; i < lineCount; i++) {
            chr *line = lines[i];
            uint size = lineSizes[i];
            printf("%zu) ", i + 1);
            for (uint j = 0; j < size; j++) {
                printf("%d ", line[j]);
            }
            printf("\n");
        }
        printf("\nLine size: %zu\n", lineCount);
        return;
    }
    chr *line = lines[cursorLine];
    uint line_size = lineSizes[cursorLine];
    printCRN(key); // NOLINT(*-narrowing-conversions)
    for (uint i = cursorKey; i < line_size; i++) {
        printCRN(line[i]);
    }
    moveCursorLeft(line_size - cursorKey);
    insertCharAtCursor(key); // NOLINT(*-narrowing-conversions)
    cursorKey++;
    //printRN(lines[cursorLine], lineSizes[cursorLine]);
}

void handleSIGINT(__attribute__((unused)) int signum) {
    handleKey(CMD_C);
    signal(SIGINT, handleSIGINT);
}

void splitByCharacter(const chr *str, uint str_size, char spl, chr ***lineList, uint *count, uint **sizes) {
    if (str_size == 0) {
        *count = 1;
        *sizes = malloc(1 * sizeof(uint));
        *lineList = (chr **) malloc(1 * sizeof(chr *));

        // normally this should be 0, but malloc(0) is undefined behavior
        (*lineList)[0] = malloc(1 * sizeof(chr));
        (*sizes)[0] = 0;
        return;
    }
    *count = 1;
    for (uint i = 0; i < str_size; i++) {
        if (str[i] == spl) (*count)++;
    }
    *sizes = malloc(*count * sizeof(uint));
    if (*sizes == NULL) {
        fprintf(stderr, C_RED"3Memory allocation failed. \n"C_RESET);
        exit(EXIT_FAILURE);
    }
    uint tmpIndex = 0;
    uint tmpSize = 0;
    for (uint i = 0; i < str_size; i++) {
        chr ch = str[i];
        if (ch == spl) {
            (*sizes)[tmpIndex] = tmpSize;
            tmpSize = 0;
            tmpIndex++;
            if (i == str_size - 1) {
                (*sizes)[tmpIndex] = tmpSize;
            }
            continue;
        }
        tmpSize++;
        if (i == str_size - 1) {
            (*sizes)[tmpIndex] = tmpSize;
        }
    }

    *lineList = (chr **) malloc(*count * sizeof(chr *));
    if (lineList == NULL) {
        fprintf(stderr, C_RED"4Memory allocation failed.\n"C_RESET);
        exit(EXIT_FAILURE);
    }

    tmpIndex = 0;

    for (uint i = 0; i < *count; i++) {
        uint size = (*sizes)[i];
        chr *line = malloc(size * sizeof(chr));
        if (line == NULL) {
            fprintf(stderr, C_RED"5Memory allocation failed.\n"C_RESET);
            exit(EXIT_FAILURE);
        }
        uint k = 0;
        for (uint j = 0; j < size; j++) {
            chr ch = str[j + tmpIndex];
            line[k] = ch;
            k++;
        }
        (*lineList)[i] = line;
        tmpIndex += size + 1;
    }
}

int main(int argc, char **args) {
    if (argc != 2) {
        fprintf(stderr, C_RED"6Expected exactly 1 argument. Usage: editor <file>\n"C_RESET);
        return EXIT_FAILURE;
    }

    filename = args[1];
    if (access(filename, F_OK) == -1) {
        fprintf(stderr, C_RED"7File not found: %s\n"C_RESET, filename);
        return EXIT_FAILURE;
    }

    struct stat fileStat;

    if (stat(filename, &fileStat) == -1) {
        fprintf(stderr, C_RED"8Error getting file status: %s\n"C_RESET, filename);
        return EXIT_FAILURE;
    }

    if (!S_ISREG(fileStat.st_mode)) {
        fprintf(stderr, C_RED"9Expected a file: %s\n"C_RESET, filename);
        return EXIT_FAILURE;
    }

    FILE *file = fopen(filename, "rb");

    if (file == NULL) {
        fprintf(stderr, C_RED"10Error opening file: %s\n"C_RESET, filename);
        return EXIT_FAILURE;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char *contentF = malloc((file_size == 0 ? 1 : file_size) * sizeof(char)); // malloc(0) = undefined behavior
    if (contentF == NULL) {
        fprintf(stderr, C_RED"11Memory allocation failed: %s\n"C_RESET, filename);
        fclose(file);
        return EXIT_FAILURE;
    }

    size_t read_size = fread(contentF, 1, file_size + 1, file);

    if (read_size != file_size) {
        fprintf(stderr, C_RED"12Error reading file: %s\n"C_RESET, filename);
        fclose(file);
        free(contentF);
        return EXIT_FAILURE;
    }

    /*uint w_size = mbstowcs(NULL, contentF, 0);
    if (w_size == (size_t) -1) {
        fprintf(stderr, C_RED"13Error converting file to UTF-8: %s\n"C_RESET, filename);
        fclose(file);
        free(contentF);
        return EXIT_FAILURE;
    }

    chr *contentFW = malloc((w_size + 1) * sizeof(chr));
    if (contentFW == NULL) {
        fprintf(stderr, C_RED"14Memory allocation failed: %s\n"C_RESET, filename);
        fclose(file);
        free(contentF);
        return EXIT_FAILURE;
    }

    if (mbstowcs(contentFW, contentF, w_size + 1) == (size_t) -1) {
        fprintf(stderr, C_RED"15Error converting file to UTF-8: %s\n"C_RESET, filename);
        fclose(file);
        free(contentF);
        free(contentFW);
        return EXIT_FAILURE;
    }*/

    uint new_file_size = 0;

    for (uint i = 0; i < file_size; i++) {
        chr ch = contentF[i];
        if (ch != '\r') {
            new_file_size++;
        }
    }

    // malloc(0) = undefined behavior
    chr *content = malloc((new_file_size == 0 ? 1 : new_file_size) * sizeof(chr));

    uint j = 0;
    for (uint i = 0; i < file_size; i++) {
        chr ch = contentF[i];
        if (ch != '\r') {
            content[j] = ch;
            j++;
        }
    }

    //free(contentF);

    splitByCharacter(content, new_file_size, '\n', &lines, &lineCount, &lineSizes);

    rerenderAll();

    fclose(file);

    signal(SIGINT, handleSIGINT);
#pragma ide diagnostic ignored "EndlessLoop"
    HANDLE_KEYS(handleKey);
}

#pragma clang diagnostic pop