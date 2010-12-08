/*
 * this trie-based dict is optimized for alphabetic lookup.
 * it has exactly the same API as the dict module and can be
 * used in its place
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dict.h"
#include "mem.h"

#undef DICT_DEBUG

/* create a new dictionary entry (key + value) */
DICT_ENTRY *dictEntry_new(char *key, void *value) {
    DICT_ENTRY *nde = (DICT_ENTRY *) malloc(sizeof(DICT_ENTRY));
    nde->key = strdup(key);
    nde->value = value;
    return nde;
}

void dictEntry_free(DICT_ENTRY *entry) {
    free(entry->key);
    free(entry);
}

void dictEntry_freeData(DICT_ENTRY *entry, Destructor d) {
    d(entry->value);
    free(entry->key);
    free(entry);
}

DICT *dict_new() {
    DICT *nd = calloc(1, sizeof(DICT));
    nd->entries = list_new();
    return nd;
}

void dict_free(DICT *d) {
    /* recursively free a DICT */
    int i;
    long il, len;
    for (i = 0; i < 26; i++) {
        if (d->children[i]) {
            dict_free(d->children[i]);
        }
    }
    len = list_length(d->entries);
    for (il = 0; il < len; il++) {
        DICT_ENTRY *e = (DICT_ENTRY *) list_get(d->entries, il);
        dictEntry_free(e);
    }
    list_free(d->entries);
    free(d);
}

void dict_freeValues(DICT *d, Destructor destructo) {
    /* recursively free a DICT */
    int i;
    long il, len;
    for (i = 0; i < 26; i++) {
        if (d->children[i]) {
            dict_freeValues(d->children[i], destructo);
        }
    }
    len = list_length(d->entries);
    for (il = 0; il < len; il++) {
        DICT_ENTRY *e = (DICT_ENTRY *) list_get(d->entries, il);
        dictEntry_freeData(e, destructo);
    }
    list_free(d->entries);
    free(d);
}

void _dict_put(DICT *d, char *key, char *orig_key, void *data) {
    char *kp = key;
    int ix;
#ifdef DICT_DEBUG
    printf("putting %s\n", key); /* debug */
#endif
    /* if the key is empty, we've found or created the appropriate
     leaf node and now just need to append an entry to it */
    if (!*kp) {
        DICT_ENTRY *newEntry;
        newEntry = dictEntry_new(orig_key, data);
#ifdef DICT_DEBUG
        printf("adding new entry %s (%p)\n", orig_key, newEntry);
        fflush(stdout);
#endif
        list_add(d->entries, newEntry);
        return;
    }
    /* otherwise we need to skip non-alpha chars */
    while (!isalpha(*kp)) {
        kp++;
        if (!*kp) { /* last char is non-alpha */
#ifdef DICT_DEBUG
            printf("last char of %s is non-alpha\n",orig_key);
#endif
            /* add to this node and quit */
            _dict_put(d, kp, orig_key, data);
            return;
        }
    }
    ix = tolower(*kp) - 'a';
    /* if the appropriate subtrie does not exist, create it */
    if (!d->children[ix]) {
#ifdef DICT_DEBUG
        printf("creating subtree for %c\n",*kp); /* debug */
#endif
        d->children[ix] = dict_new();
    }
    /* recurse */
    _dict_put(d->children[ix], kp + 1, orig_key, data);
}

void dict_put(DICT *d, char *key, void *data) {
    _dict_put(d, key, key, data);
}

void *_dict_get(DICT *d, char *key, char *orig_key) {
    char *kp = key;
    int ix;
    /* skip nonalphas */
    while (*kp && !isalpha(*kp)) {
        kp++;
    }
    if (!*kp) {
        /* do a linear search of this node's entries */
        int i, len;
        len = list_length(d->entries);
#ifdef DICT_DEBUG
        printf("searching %d entries for %s\n",len,orig_key); /* debug */
#endif
        /* search from the end, so that new values shadow old */
        for (i = len - 1; i >= 0; i--) {
            DICT_ENTRY *de = (DICT_ENTRY *) list_get(d->entries, i);
            if (!strcmp(orig_key, de->key)) {
                return de->value;
            }
        }
        return NULL;
    }
#ifdef DICT_DEBUG
    printf("getting %s\n",kp); /* debug */
#endif
    ix = tolower(*kp) - 'a';
    if (!d->children[ix])
        return NULL;
    return _dict_get(d->children[ix], kp + 1, orig_key);
}

LIST *_dict_getAll(DICT *d, char *key, char *orig_key) {
    char *kp = key;
    int ix;
    /* skip nonalphas */
    while (*kp && !isalpha(*kp)) {
        kp++;
    }
#ifdef DICT_DEBUG
    printf("getting %s\n",kp); /* debug */
#endif
    if (!*kp) {
        LIST *all;
        int i, len;
        all = list_new();
        len = list_length(d->entries);
        for (i = 0; i < len; i++) {
            DICT_ENTRY *de = (DICT_ENTRY *) list_get(d->entries, i);
            list_add(all, de->value);
        }
        return all;
    }
    ix = tolower(*kp) - 'a';
    if (!d->children[ix])
        return NULL;
    return _dict_get(d->children[ix], kp + 1, orig_key);
}

void *dict_get(DICT *d, char *key) {
    return _dict_get(d, key, key);
}

LIST *dict_getAll(DICT *d, char *key) {
    return _dict_getAll(d, key, key);
}

void dict_traverse(DICT *d, TraversalAction action, void *action_arg) {
    int i;
    if (!d)
        return;
    if (d->entries) {
        int len;
        len = list_length(d->entries);
#ifdef DICT_DEBUG
        printf("%d entries\n", len);
#endif
        for (i = 0; i < len; i++) {
            DICT_ENTRY *de = (DICT_ENTRY *) list_get(d->entries, i);
#ifdef DICT_DEBUG
            printf("entry %d = %p\n", i, de);
#endif
            action(de->key, de->value, action_arg);
        }
    }
    for (i = 0; i < 26; i++)
        if (d->children[i])
            dict_traverse(d->children[i], action, action_arg);
}

