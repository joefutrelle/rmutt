#ifndef DICT_H_
#define DICT_H_

/*
 * this trie-based dict is optimized for alphabetic lookup.
 * it has exactly the same API as the dict module and can be
 * used in its place
 */

#include "list.h"
#include "mem.h"

typedef void (*TraversalAction)(char *key, void *value, void *arg);

typedef struct _dict_entry {
     char *key;
     void *value;
} DICT_ENTRY;

typedef struct _dict_node {
     LIST *entries; /* entries (of type DICT_ENTRY) */
     struct _dict_node *children[26];
} DICT;

extern DICT *dict_new();
extern void dict_free(DICT *);
extern void dict_freeValues(DICT *, Destructor);

extern void dict_put(DICT *, char *key, void *data);
extern void *dict_get(DICT *, char *key);

/*
 * additional trie-only functions
 */
extern LIST *dict_getAll(DICT *, char *);
extern void dict_traverse(DICT *, TraversalAction, void *);

#endif
