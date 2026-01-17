#ifndef CSSOPTIM_LIST_H
#define CSSOPTIM_LIST_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Opaque handle for a string list.
 */
typedef struct string_list string_list_t;

/**
 * @brief Creates a new string list.
 * @return Pointer to a new string list, or NULL on failure.
 */
string_list_t *string_list_create(void);

/**
 * @brief Destroys a string list and frees all its memory.
 * @param list The list to destroy.
 */
void string_list_destroy(string_list_t *list);

/**
 * @brief Adds a string to the list if it doesn't already exist.
 * @param list The list to add to.
 * @param str The string to add (will be duplicated).
 * @return true if added, false if it already exists or on failure.
 */
bool string_list_add(string_list_t *list, const char *str);

/**
 * @brief Checks if a string is in the list.
 * @param list The list to check.
 * @param str The string to look for.
 * @return true if found, false otherwise.
 */
bool string_list_contains(const string_list_t *list, const char *str);

/**
 * @brief Gets the number of items in the list.
 * @param list The list.
 * @return Number of items.
 */
size_t string_list_count(const string_list_t *list);

/**
 * @brief Gets an item at a specific index.
 * @param list The list.
 * @param index The index.
 * @return Pointer to the string, or NULL if out of bounds.
 */
const char *string_list_get(const string_list_t *list, size_t index);

/**
 * @brief Gets the raw items array (use with caution).
 * @param list The list.
 * @return Pointer to the array of string pointers.
 */
const char **string_list_items(const string_list_t *list);

#endif // CSSOPTIM_LIST_H
