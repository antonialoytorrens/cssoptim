#include "html_scan.h"
#include <lexbor/html/parser.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/collection.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* Implementation of string_list_t for tracking used CSS classes and tags.
 * This provides a simple generic dynamic array for storing unique strings.
 */

// Initialize a new list with default values
void string_list_init(string_list_t *list) {
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

// Free all memory associated with the list and its items
void string_list_destroy(string_list_t *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

// Add a string to the list if it's not already present
void string_list_add(string_list_t *list, const char *str) {
    if (!list || !str) return;
    // Avoid duplicates
    if (string_list_contains(list, str)) return;
    
    // Resize buffer if needed
    if (list->count == list->capacity) {
        list->capacity = (list->capacity == 0) ? 16 : list->capacity * 2;
        list->items = realloc(list->items, list->capacity * sizeof(char *));
    }
    
    // Store a copy of the string
    char *dup = malloc(strlen(str) + 1);
    if (dup) {
        strcpy(dup, str);
        list->items[list->count++] = dup;
    }
}

// Check if the list already contains the specified string
bool string_list_contains(string_list_t *list, const char *str) {
    if (!list || !str) return false;
    for (size_t i = 0; i < list->count; i++) {
        if (strcmp(list->items[i], str) == 0) return true;
    }
    return false;
}

/* DOM Traversal Helper
 * Recursively scans an element and its children for the 'class' attribute and tag names.
 */
static lxb_status_t scan_element(lxb_dom_element_t *element, string_list_t *classes, string_list_t *tags) {
    size_t value_len = 0;
    
    // 1. Collect tag name if tags list is provided
    if (tags) {
        const lxb_char_t *tag_name = lxb_dom_element_local_name(element, &value_len);
        if (tag_name && value_len > 0) {
            char *name_copy = malloc(value_len + 1);
            if (name_copy) {
                memcpy(name_copy, tag_name, value_len);
                name_copy[value_len] = '\0';
                string_list_add(tags, name_copy);
                free(name_copy);
            }
        }
    }

    // 2. Collect classes if classes list is provided
    if (classes) {
        const lxb_char_t *class_attr = lxb_dom_element_get_attribute(element, (const lxb_char_t*)"class", 5, &value_len);
        
        if (class_attr && value_len > 0) {
            // Split the class string by whitespace into individual classes
            char *copy = malloc(value_len + 1);
            if (copy) {
                memcpy(copy, class_attr, value_len);
                copy[value_len] = '\0';
                
                char *token = strtok(copy, " \t\n\r");
                while (token) {
                    string_list_add(classes, token);
                    token = strtok(NULL, " \t\n\r");
                }
                free(copy);
            }
        }
    }
    
    // Traverse children nodes
    lxb_dom_node_t *node = lxb_dom_interface_node(element);
    lxb_dom_node_t *child = node->first_child;
    
    while (child) {
        if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
            scan_element(lxb_dom_interface_element(child), classes, tags);
        }
        child = child->next;
    }
    
    return LXB_STATUS_OK;
}

// Public API: Scans an HTML string for all unique CSS classes and tags
void scan_html(const char *content, size_t length, string_list_t *classes, string_list_t *tags) {
    if (!content || length == 0) return;

    lxb_html_document_t *document = lxb_html_document_create();
    if (!document) return;

    // Parse HTML content using Lexbor
    lxb_status_t status = lxb_html_document_parse(document, (const lxb_char_t *)content, length);
    if (status != LXB_STATUS_OK) {
        lxb_html_document_destroy(document);
        return;
    }
    
    // Start scanning from the body element
    lxb_dom_element_t *body = (lxb_dom_element_t *)lxb_html_document_body_element(document);
    if (body) {
        scan_element(body, classes, tags);
    } else {
        // Fallback: iterate from the root of the document
        lxb_dom_node_t *root = lxb_dom_interface_node(document);
        lxb_dom_node_t *child = root->first_child;
        while (child) {
            if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
                scan_element(lxb_dom_interface_element(child), classes, tags);
            }
            child = child->next;
        }
    }

    lxb_html_document_destroy(document);
}

// Public API: Scans JS/TS/JSX content for potential class names in strings
void scan_js(const char *content, size_t length, string_list_t *list) {
    if (!content || length == 0) return;

    /* Simple state machine to extract string literals from JS.
     * Captures strings inside ' ', " ", or ` `.
     */
    bool in_string = false;
    char quote_char = 0;
    const char *start = NULL;
    
    for (size_t i = 0; i < length; i++) {
        char c = content[i];
        
        if (in_string) {
            if (c == '\\') {
                i++; // Skip escaped character
                continue;
            }
            if (c == quote_char) {
                // Found end of string literal
                size_t token_len = &content[i] - start;
                if (token_len > 0) {
                    char *str = malloc(token_len + 1);
                    if (str) {
                        memcpy(str, start, token_len);
                        str[token_len] = '\0';
                        
                        // Split string content by whitespace and delimiters as potential classes
                        char *temp = malloc(token_len + 1);
                        if (temp) {
                            strcpy(temp, str);
                            char *t = strtok(temp, " \t\n\r,;:");
                            while (t) {
                                string_list_add(list, t);
                                t = strtok(NULL, " \t\n\r,;:");
                            }
                            free(temp);
                        }
                        free(str);
                    }
                }
                in_string = false;
            }
        } else {
            // Looking for start of a string
            if (c == '\'' || c == '"' || c == '`') {
                in_string = true;
                quote_char = c;
                start = &content[i+1];
            }
            // Basic comment skipping
            if (c == '/' && i+1 < length) {
                if (content[i+1] == '/') {
                    // Line comment: skip until newline
                    i += 2;
                    while (i < length && content[i] != '\n') i++;
                } else if (content[i+1] == '*') {
                    // Block comment: skip until */
                    i += 2;
                    while (i < length - 1 && !(content[i] == '*' && content[i+1] == '/')) i++;
                    i++; // skip /
                }
            }
        }
    }
}
