#ifndef CHOOSE_H_
#define CHOOSE_H_

extern void choose_setIterationLong(unsigned long);
extern void choose_setIterationFile(FILE *);
extern void choose_setIterationString(char *);

extern unsigned long choose_next();

#endif
