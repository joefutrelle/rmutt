#include <stdlib.h>
#include <stdio.h>
#include "choose.h"

extern long iteration;

long iteration = -1;

void choose_setIteration(long i) {
     iteration = i;
}

long choose_next(long nChoices) {
     if(nChoices == 0) {
	  fprintf(stderr,"zero choices!\n");
     }
     if(iteration == -1) {
	  return random() % nChoices;
     } else {
	  long choice = iteration % nChoices;
	  iteration /= nChoices;
	  return choice;
     }
}
