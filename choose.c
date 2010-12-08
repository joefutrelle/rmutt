#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <gmp.h>
#include "choose.h"

#define MODE_RANDOM 0
#define MODE_LONG   1
#define MODE_MP     2

int chooseMode = MODE_RANDOM;

unsigned long iteration;
mpz_t oracle;
mpz_t nextOracle;

void choose_setIterationLong(unsigned long i) {
    iteration = i;
    chooseMode = MODE_LONG;
}

void choose_setIterationFile(FILE *file) {
    mpz_inp_str(oracle, file, 10);
    fclose(file);
    chooseMode = MODE_MP;
}

void choose_setIterationString(char *string) {
    if (mpz_set_str(oracle, string, 10)) {
        fprintf(stderr, "error: iteration string is not a number\n");
        exit(-1);
    }
    if (!strcmp(string, "0")) { /* gmp doesn't like 0? */
        iteration = 0L;
        chooseMode = MODE_LONG;
    } else if (mpz_fits_ulong_p(oracle)) {
        iteration = mpz_get_ui(oracle);
        mpz_clear(oracle);
        chooseMode = MODE_LONG;
    } else {
        mpz_init(nextOracle);
        chooseMode = MODE_MP;
    }
}

unsigned long choose_next(unsigned long nChoices) {
    unsigned long choice;
    if (nChoices == 0) {
        fprintf(stderr, "zero choices!\n");
    }
    switch (chooseMode) {
    case MODE_LONG:
        choice = iteration % nChoices;
        iteration /= nChoices;
        break;
    case MODE_MP:
        choice = mpz_fdiv_q_ui(nextOracle, oracle, nChoices);
        mpz_swap(oracle, nextOracle);
        break;
    case MODE_RANDOM:
    default:
        choice = random() % nChoices;
        break;
    }
    return choice;
}
