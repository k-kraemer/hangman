/*
MIT License

Copyright (c) 2021 Kevin Krämer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Standard library
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
// Local modules
#include "gallows.h"
#include "utils/dict.h"

#ifndef _ALPH_SIZE
    #define _ALPH_SIZE 26 // classic english alphabet
#endif

// Rows in ui
enum line {
    HEADER,
    WORD = 2,
    INPUT,
    USED_LETTERS,
    STATUS
};

// Globals from gallows.h
extern unsigned int tries;
extern const char* rope;
extern const char* head;
extern const char* upper_body;
extern const char* legs;
extern const char* vertical_bar;
extern const char* hill;

/* Helper functions */

void print_hidden_word(WINDOW* ui, int x, char* word);

unsigned int print_found_letters(WINDOW* ui, int x, char* word, char letter);

void print_solution(WINDOW* ui, int x, char* word);

void print_gallows(void);

void trim_newline(char* word);

int is_used(const char* arr, const char item, const size_t s);


int main(int argc, char** argv)
{
    char input[2]; // [0]: one char, [1]: 0
    char used_letters[_ALPH_SIZE];
    memset(used_letters, 0, sizeof(used_letters));

    char* word = NULL;
    unsigned int game_round = 0;
    unsigned int found_letters = 0;
    unsigned int right_guess = 0;

    int ui_x, ui_y = 0; // Dimensions of the terminal
    int word_start_x = 0; // column for first letter of the word

    if(argc < 2)
    {
        if( dict_open("dict.txt") )
        {
            printf("Error: Could not find a dictionary!\n");
            printf("Specify the path: hangman <path to dictionary>\n");
            return -1;
        }
    }
    else if( dict_open(argv[1]) < 0 )
    {
        printf("Error: Could not open the dictionary!\n");
        return -1;
    }

    word = dict_get_random_word();

    /* Init curses */

    WINDOW* ui = initscr();

    if(ui == NULL)
    {
        free(word);
        dict_close();
        return -1;
    }

    // exit at SIGINT
    cbreak();

    getmaxyx(ui, ui_y, ui_x);

    word_start_x = (int)(ui_x/2)-strlen(word);

    if(ui_y < 14 || ui_x < 32)
    {
        printf("Error: Need a bigger terminal!\n");
        free(word);
        dict_close();
        endwin();
        return -1;
    }

    /* Print ui */

    // Print header
    attron(A_BOLD);
    mvprintw(HEADER, (int)(ui_x/2)-7, "-- HANGMAN --");
    attroff(A_BOLD);


    trim_newline(word);

    print_hidden_word(ui, word_start_x, word);

    /* MAIN LOOP */
    while(true)
    {
        refresh();

        /* Get user input */
        mvprintw(INPUT, 0, "Enter a letter:  "); // delete last letter with the extra space
        wmove(ui, INPUT, 16);
        wgetnstr(ui, input, 1); // read one char
        *input = tolower(*input);

        mvprintw(STATUS, 0, "                     "); // clear status line

        /* Handle user input */
        if(!isalpha(*input))
        {
            mvprintw(STATUS, 0, "Only letters allowed!");
            continue;
        }
        else if(is_used(used_letters, *input, sizeof(used_letters)))
        {
            mvprintw(STATUS, 0, "Already used!");
            continue;
        }
        else
        {
            used_letters[game_round++] = *input;

            wmove(ui, USED_LETTERS, 0);
            for (size_t i = 0; i < _ALPH_SIZE; i++)
            {
                if(used_letters[i] == 0) break;
                printw("%c,", used_letters[i]);
            }

            right_guess = print_found_letters(ui, word_start_x, word, *input);

            if(right_guess)
            {
                found_letters += right_guess;
                if(found_letters == strlen(word))
                {
                    mvprintw(STATUS, 0, "You have won!");
                    break;
                }
            }
            else
            {
                tries--;
                print_gallows();
                if(tries == 0)
                {
                    mvprintw(STATUS, 0, "You have lost!");
                    print_solution(ui, word_start_x, word);
                    break;
                }
            }
        }
    } // MAIN LOOP

    getch();
    endwin();

    free(word);
    dict_close();

    return 0;
}

/**
 * Print underscores to indicate the number of letters in the word
 * Hello -> _ _ _ _ _
*/
void print_hidden_word(WINDOW* ui, int x, char* word)
{
    wmove(ui, WORD, x);

    for (size_t i = 0; i < strlen(word); i++)
    {
        printw("_ ");
    }
}

/**
 * Check if the guessed letter is in the word
 * If so print the letter at the correct position
 * Return the number hits
*/
unsigned int print_found_letters(WINDOW* ui, int x, char* word, char letter)
{
    unsigned int hits = 0;
    char letter_low = tolower(letter);

    for (size_t i = 0; i < strlen(word); i++)
    {
        if( tolower(word[i]) == letter_low )
        {
            wmove(ui, WORD, x + (i<<1) );
            printw("%c", word[i]);
            hits++;
        }
    }
    return hits;
}

/**
 * Print the gallows depending on how many faults the player made
*/
void print_gallows(void)
{
    switch (tries)
    {
    case 6:
        mvprintw(STATUS+7, 0, "%s", hill);
        break;
    case 5:
        mvprintw(STATUS+2, 1, "%s", vertical_bar);
        break;
    case 4:
        mvprintw(STATUS+1, 1, "%s", horizontal_bar);
        break;
    case 3:
        mvprintw(STATUS+2, 8, "%s", rope);
        break;
    case 2:
        mvprintw(STATUS+3, 8, "%s", head);
        break;
    case 1:
        mvprintw(STATUS+4, 7, "%s", upper_body);
        break;
    case 0:
        mvprintw(STATUS+5, 7, "%s", legs);
    default:
        break;
    }
}

/**
 * Print the word if the player fails
*/
void print_solution(WINDOW* ui, int x, char* word)
{
    wmove(ui, WORD, x);
    for (size_t i = 0; i < strlen(word); i++)
    {
        printw("%c ", word[i]);
    }
}

/**
 * Remove the trailing newline
*/
void trim_newline(char* word)
{
    size_t str_size = strlen(word);

    if( word[str_size-1] == '\n' )
    {
        word[str_size-1] = 0;
    }
}

/**
 * Check if an item is already in the array
*/
int is_used(const char* arr, const char item, const size_t s)
{
    for (size_t i = 0; i < s; i++)
    {
        if (arr[i] == item)
        {
            return 1;
        }
    }
    return 0;
}
