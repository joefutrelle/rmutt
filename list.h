#ifndef LIST_H_
#define LIST_H_

#include "mem.h"

typedef struct _list {
     long length;     /* how many elements are in the list */
     long capacity;   /* out of how many possible */
     void **data;     /* the items */
} LIST;

extern LIST *list_new();
extern void list_free(LIST *);
extern void list_freeData(LIST *, Destructor);

extern long list_length(LIST *);
extern void *list_get(LIST *, long);
extern void list_add(LIST *, void *); /* add to end of list */
extern void list_append(LIST *, LIST *);
extern void list_appendAndFree(LIST *, LIST *);

void *list_getRand(LIST *);

#endif
