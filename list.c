#include "list.h"
#include <stdlib.h>

#define LIST_INIT_CAPACITY 20

LIST *list_new() {
     LIST *nl = (LIST *)malloc(sizeof(LIST));
     nl->data = calloc(LIST_INIT_CAPACITY,sizeof(void *));
     nl->capacity = LIST_INIT_CAPACITY;
     nl->length = 0;
     return nl;
}

void list_free(LIST *l) {
     free(l->data);
     free(l);
}

void list_freeData(LIST *l, Destructor d) {
     long i, len;
     len = list_length(l);
     for(i = 0; i < len; i++) {
	  d(list_get(l, i));
     }
     list_free(l);
}

long list_length(LIST *l) {
     return l->length;
}

void *list_get(LIST *l, long i) {
     return l->data[i];
}

void list_add(LIST *l, void *data) {
     if(l->length >= l->capacity) {
	  long i;
	  void **np;
	  size_t newcap = l->capacity * 2;
	  np = calloc(newcap,sizeof(void *));
	  for(i = 0; i < l->length; i++) {
	       np[i] = l->data[i];
	  }
	  free(l->data);
	  l->data = np;
	  l->capacity = newcap;
     }
     l->data[l->length++] = data;
}

void list_append(LIST *a, LIST *b) {
     int i;
     for(i = 0; i < b->length; i++)
	  list_add(a,list_get(b,i));
}
void list_appendAndFree(LIST *a, LIST *b) {
     int i;
     for(i = 0; i < b->length; i++)
	  list_add(a,list_get(b,i));
     list_free(b);
}

void *list_getRand(LIST *l) {
     long i;
     i = random() % list_length(l);
     return list_get(l,i);
}

LIST *list_addToNew(void *element) {
     LIST *theList = list_new();
     list_add(theList, element);
     return theList;
}

LIST *list_reverse(LIST *list) {
     long len = list_length(list), i;
     LIST *reversed = list_new();
     for(i = len-1; i >= 0; i--) {
	  list_add(reversed,list_get(list,i));
     }
     return reversed;
}
