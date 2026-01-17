#include "cssoptim/list.h"
#include <stdlib.h>
#include <string.h>

struct string_list {
  char **items;
  size_t count;
  size_t capacity;
};

string_list_t *string_list_create(void) {
  string_list_t *list = malloc(sizeof(string_list_t));
  if (list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
  }
  return list;
}

void string_list_destroy(string_list_t *list) {
  if (!list) return;
  for (size_t i = 0; i < list->count; i++) {
    free(list->items[i]);
  }
  free(list->items);
  free(list);
}

bool string_list_add(string_list_t *list, const char *str) {
  if (!list || !str) return false;
  if (string_list_contains(list, str)) return false;
  
  if (list->count == list->capacity) {
    size_t new_capacity = (list->capacity == 0) ? 16 : list->capacity * 2;
    char **new_items = realloc(list->items, new_capacity * sizeof(char *));
    if (!new_items) return false;
    list->items = new_items;
    list->capacity = new_capacity;
  }
  
  size_t slen = strlen(str);
  char *dup = malloc(slen + 1);
  if (!dup) return false;
  
  memcpy(dup, str, slen + 1);
  list->items[list->count++] = dup;
  return true;
}

bool string_list_contains(const string_list_t *list, const char *str) {
  if (!list || !str) return false;
  for (size_t i = 0; i < list->count; i++) {
    if (strcmp(list->items[i], str) == 0) return true;
  }
  return false;
}

size_t string_list_count(const string_list_t *list) {
  return list ? list->count : 0;
}

const char *string_list_get(const string_list_t *list, size_t index) {
  if (!list || index >= list->count) return NULL;
  return list->items[index];
}

const char **string_list_items(const string_list_t *list) {
  return list ? (const char **)list->items : NULL;
}
