#ifndef LIST_H_
#define LIST_H_

#include "mem.h"

/* generic list support */

typedef struct _list {
     long length;     /* how many elements are in the list */
     long capacity;   /* out of how many possible */
     void **data;     /* the items */
} LIST;

extern LIST *list_new(); /* create a list */
extern void list_free(LIST *); /* free a list, but not its data */
extern void list_freeData(LIST *, Destructor); /* free a list and its data */

extern long list_length(LIST *); /* return the number of elements in a list */
extern void *list_get(LIST *, long); /* get an element from a list */
extern void list_add(LIST *, void *); /* add to end of list */
extern void list_append(LIST *, LIST *); /* append the second list to the first list */
extern void list_appendAndFree(LIST *, LIST *); /* append the 2nd list to 1st and free the 2nd */

void *list_getRand(LIST *); /* get a random element from a list */

#endif
