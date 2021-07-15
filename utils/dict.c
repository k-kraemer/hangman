#include "dict.h"

/* State */

typedef struct dict_t
{
    FILE* file;
    unsigned int line_cnt;
} dict_t;

dict_t dict = {
    .file = NULL,
    .line_cnt = 0
};

/* Methods */

// Private method
unsigned int dict_get_line_cnt()
{
    char* buffer = NULL;
    size_t buff_size = 0;
    unsigned int lines = 0;

    while (getline(&buffer, &buff_size, dict.file) != -1)
    {
        lines++;
    }

    free(buffer); // allocated by getline

    rewind(dict.file); // reset file cursor

    return lines;
}

char* dict_get_random_word()
{
    size_t buff_size = 20;
    char* line = malloc(buff_size);

    int random_line = 0;
    int line_cnt = 0;

    srand(time(0));

    // Choose a random line which not exceeds the lines of the file
    random_line = rand() % dict.line_cnt;

    // Get that line
    while (getline(&line, &buff_size, dict.file) != -1)
    {
        if(line_cnt == random_line) break;
        line_cnt++;
    }

    rewind(dict.file); // reset file cursor

    return line;
}

int dict_open(const char* pathname)
{
    FILE* fp = fopen(pathname, "r");
    if(fp == NULL)
    {
        return -1;
    }

    dict.file = fp;
    dict.line_cnt = dict_get_line_cnt();

    if(dict.line_cnt == 0)
    {
        return -1;
    }

    return 0;
}

void dict_close()
{
    fclose(dict.file);
}
