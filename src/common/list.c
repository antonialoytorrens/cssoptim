#include "cssoptim/list.h"
#include <stdlib.h>
#include <string.h>

/* djb2 hash function */
static size_t str_hash(const char *str) {
  size_t h = 5381;
  int c;
  while ((c = (unsigned char)*str++))
    h = ((h << 5) + h) + (size_t)c;
  return h;
}

struct string_list {
  char **items;    /* ordered array of heap-copied strings (for indexed access) */
  size_t count;
  size_t capacity;
  char **ht;       /* open-addressing hash table; NULL slot = empty */
  size_t ht_size;  /* capacity of ht[], always a power of 2               */
};

/* Returns a pointer to the slot in ht where key is stored or should go.
 * Caller must ensure at least one empty slot exists (load < 1.0). */
static char **ht_find_slot(char **ht, size_t ht_size, const char *key) {
  size_t idx = str_hash(key) & (ht_size - 1);
  while (ht[idx] && strcmp(ht[idx], key) != 0)
    idx = (idx + 1) & (ht_size - 1);
  return &ht[idx];
}

/* Rebuild the hash table from the items array (called after ht grows). */
static void ht_rebuild(char **ht, size_t ht_size, char **items, size_t count) {
  for (size_t i = 0; i < count; i++) {
    char **slot = ht_find_slot(ht, ht_size, items[i]);
    *slot = items[i];
  }
}

/* Double the hash table; returns false on OOM. */
static bool ht_grow(string_list_t *list) {
  size_t new_size = (list->ht_size == 0) ? 32 : list->ht_size * 2;
  char **new_ht = calloc(new_size, sizeof(char *));
  if (!new_ht)
    return false;
  ht_rebuild(new_ht, new_size, list->items, list->count);
  free(list->ht);
  list->ht = new_ht;
  list->ht_size = new_size;
  return true;
}

string_list_t *string_list_create(void) {
  string_list_t *list = malloc(sizeof(string_list_t));
  if (list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    list->ht = NULL;
    list->ht_size = 0;
  }
  return list;
}

void string_list_destroy(string_list_t *list) {
  if (!list)
    return;
  for (size_t i = 0; i < list->count; i++)
    free(list->items[i]);
  free(list->items);
  free(list->ht);
  free(list);
}

bool string_list_contains(const string_list_t *list, const char *str) {
  if (!list || !str)
    return false;
  if (!list->ht_size) {
    /* Hash table not yet allocated — fallback linear scan. */
    for (size_t i = 0; i < list->count; i++)
      if (strcmp(list->items[i], str) == 0)
        return true;
    return false;
  }
  return *ht_find_slot(list->ht, list->ht_size, str) != NULL;
}

bool string_list_add(string_list_t *list, const char *str) {
  if (!list || !str)
    return false;
  if (string_list_contains(list, str))
    return false;

  /* Grow items array if full. */
  if (list->count == list->capacity) {
    size_t new_cap = (list->capacity == 0) ? 16 : list->capacity * 2;
    char **new_items = realloc(list->items, new_cap * sizeof(char *));
    if (!new_items)
      return false;
    list->items = new_items;
    list->capacity = new_cap;
  }

  /* Keep hash table load factor below 0.5. */
  if (list->ht_size == 0 || list->count + 1 > list->ht_size / 2) {
    if (!ht_grow(list))
      return false;
  }

  size_t slen = strlen(str);
  char *dup = malloc(slen + 1);
  if (!dup)
    return false;
  memcpy(dup, str, slen + 1);

  list->items[list->count++] = dup;
  *ht_find_slot(list->ht, list->ht_size, dup) = dup;
  return true;
}

size_t string_list_count(const string_list_t *list) {
  return list ? list->count : 0;
}

const char *string_list_get(const string_list_t *list, size_t index) {
  if (!list || index >= list->count)
    return NULL;
  return list->items[index];
}

const char **string_list_items(const string_list_t *list) {
  return list ? (const char **)list->items : NULL;
}
