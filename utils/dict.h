// This module handels a dictionary as private state
// The provided methods allow to perform on this file

// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#pragma once

// Wrapper methods

int dict_open(const char* pathname);

void dict_close();

// Methods needed for hangman

char* dict_get_random_word();
