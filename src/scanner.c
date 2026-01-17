#include "cssoptim/scanner.h"
#include <ctype.h>
#include <lexbor/dom/collection.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/html/parser.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Implementation of scanner logic using Lexbor. */

/* DOM Traversal Helper
 * Recursively scans an element and its children for the 'class' attribute and
 * tag names.
 */
static lxb_status_t scan_element(lxb_dom_element_t *element,
                                 string_list_t *classes, string_list_t *tags,
                                 string_list_t *attrs) {
  // 1. Collect tag name
  if (tags) {
    const lxb_char_t *local_name = lxb_dom_element_local_name(element, NULL);
    if (local_name) {
      string_list_add(tags, (const char *)local_name);
    }
  }

  // 2. Collect attributes
  if (attrs) {
    lxb_dom_attr_t *attr = lxb_dom_element_first_attribute(element);
    while (attr) {
      const lxb_char_t *name = lxb_dom_attr_local_name(attr, NULL);
      size_t val_len = 0;
      const lxb_char_t *value = lxb_dom_attr_value(attr, &val_len);

      if (name) {
        // Add plain attribute name
        string_list_add(attrs, (const char *)name);

        if (value && val_len > 0) {
          // Add name=value pair
          size_t name_len = strlen((const char *)name);
          char *pair = malloc(name_len + val_len + 2);
          if (pair) {
            memcpy(pair, name, name_len);
            pair[name_len] = '=';
            memcpy(pair + name_len + 1, value, val_len);
            pair[name_len + 1 + val_len] = '\0';
            string_list_add(attrs, pair);
            free(pair);
          }
        }
      }
      // Move to next attribute
      lxb_dom_node_t *next_node =
          lxb_dom_node_next(lxb_dom_interface_node(attr));
      attr = (next_node && next_node->type == LXB_DOM_NODE_TYPE_ATTRIBUTE)
                 ? (lxb_dom_attr_t *)next_node
                 : NULL;
    }
  }

  // 3. Collect classes
  if (classes) {
    size_t value_len = 0;
    const lxb_char_t *class_attr = lxb_dom_element_get_attribute(
        element, (const lxb_char_t *)"class", 5, &value_len);
    if (class_attr && value_len > 0) {
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

  // Recursive traversal
  lxb_dom_node_t *child =
      lxb_dom_node_first_child(lxb_dom_interface_node(element));
  while (child) {
    if (child->type == LXB_DOM_NODE_TYPE_ELEMENT) {
      scan_element(lxb_dom_interface_element(child), classes, tags, attrs);
    }
    child = child->next;
  }

  return LXB_STATUS_OK;
}

void scan_html(const char *content, size_t length, string_list_t *classes,
               string_list_t *tags, string_list_t *attrs) {
  if (!content || length == 0)
    return;

  lxb_html_parser_t *parser = lxb_html_parser_create();
  if (!parser)
    return;
  lxb_html_parser_init(parser);

  lxb_html_document_t *document =
      lxb_html_parse(parser, (const lxb_char_t *)content, length);
  if (!document) {
    lxb_html_parser_destroy(parser);
    return;
  }

  lxb_dom_element_t *body =
      (lxb_dom_element_t *)lxb_html_document_body_element(document);
  if (body) {
    scan_element(body, classes, tags, attrs);
  } else {
    lxb_dom_node_t *node =
        lxb_dom_node_first_child(lxb_dom_interface_node(document));
    while (node) {
      if (node->type == LXB_DOM_NODE_TYPE_ELEMENT) {
        scan_element(lxb_dom_interface_element(node), classes, tags, attrs);
      }
      node = node->next;
    }
  }

  lxb_html_document_destroy(document);
  lxb_html_parser_destroy(parser);
}

// Public API: Scans JS/TS/JSX content for potential class names in strings
void scan_js(const char *content, size_t length, string_list_t *list) {
  if (!content || length == 0)
    return;

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

            // Split string content by whitespace and delimiters as potential
            // classes
            char *temp = malloc(token_len + 1);
            if (temp) {
              memcpy(temp, str, token_len + 1);
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
        start = &content[i + 1];
      }
      // Basic comment skipping
      if (c == '/' && i + 1 < length) {
        if (content[i + 1] == '/') {
          // Line comment: skip until newline
          i += 2;
          while (i < length && content[i] != '\n')
            i++;
        } else if (content[i + 1] == '*') {
          // Block comment: skip until */
          i += 2;
          while (i < length - 1 &&
                 !(content[i] == '*' && content[i + 1] == '/'))
            i++;
          i++; // skip /
        }
      }
    }
  }
}
