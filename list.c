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

/*
main() {
     LIST *l = list_new();
     LIST_NODE *n;
     list_add(l, (void *)strdup("foo"));
     list_add(l, (void *)strdup("bar"));
     n = list_first(l);
     while(n) {
	  printf("%s\n", (char *)listNode_getData(n));
	  n = listNode_next(n);
     }
     list_freeData(l,free);
     list_free(l);
}
*/
