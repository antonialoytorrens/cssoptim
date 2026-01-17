#include <lexbor/css/css.h>
#include <lexbor/css/parser.h>
#include <lexbor/css/stylesheet.h>
#include <lexbor/css/rule.h>
#include <lexbor/css/selectors/selector.h>
#include "css_proc.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <lexbor/css/at_rule.h>
#include <lexbor/core/serialize.h>
#include <strings.h>

// --- Garbage Collection for Block Strings ---
typedef struct garbage_node {
    char *ptr;
    struct garbage_node *next;
} garbage_node_t;

static garbage_node_t *garbage_head = NULL;

static void register_garbage(char *ptr) {
    if (!ptr) return;
    garbage_node_t *node = malloc(sizeof(garbage_node_t));
    if (node) {
        node->ptr = ptr;
        node->next = garbage_head;
        garbage_head = node;
    }
}

static void clear_garbage() {
    garbage_node_t *node = garbage_head;
    while (node) {
        garbage_node_t *next = node->next;
        if (node->ptr) free(node->ptr);
        free(node);
        node = next;
    }
    garbage_head = NULL;
}

// --- Dependency List ---
#define MAX_DEPS 1024
typedef struct {
    char *items[MAX_DEPS];
    size_t count;
} dep_list_t;

static void add_dep(dep_list_t *list, const char *name, size_t len) {
    if (list->count >= MAX_DEPS) return;
    for (size_t i = 0; i < list->count; i++) {
        if (strncmp(list->items[i], name, len) == 0 && list->items[i][len] == '\0') {
            return;
        }
    }
    char *copy = malloc(len + 1);
    if (!copy) return;
    memcpy(copy, name, len);
    copy[len] = '\0';
    list->items[list->count++] = copy;
}

static void free_deps(dep_list_t *list) {
    for (size_t i = 0; i < list->count; i++) {
        free(list->items[i]);
    }
    list->count = 0;
}

static bool is_dep_used(const dep_list_t *list, const char *name, size_t len) {
    for (size_t i = 0; i < list->count; i++) {
        if (strncmp(list->items[i], name, len) == 0 && list->items[i][len] == '\0') {
            return true;
        }
    }
    return false;
}

// --- Serializer Callback ---
static lxb_status_t string_serializer_cb(const lxb_char_t *data, size_t len, void *ctx) {
    char **buffer = (char **)ctx;
    size_t current_len = *buffer ? strlen(*buffer) : 0;
    char *new_buf = realloc(*buffer, current_len + len + 1);
    if (!new_buf) return LXB_STATUS_ERROR;
    *buffer = new_buf;
    memcpy(*buffer + current_len, data, len);
    (*buffer)[current_len + len] = '\0';
    return LXB_STATUS_OK;
}

// --- Nested Processing Helper ---
typedef void (*nested_cb_t)(lxb_css_rule_t *root, void *ctx);

static void process_nested_block(lxb_css_at_rule__undef_t *undef, nested_cb_t cb, void *ctx) {
    if (!undef || !undef->block.data) return;

    lxb_css_parser_t *parser = lxb_css_parser_create();
    lxb_css_parser_init(parser, NULL);
    lxb_css_stylesheet_t *ss = lxb_css_stylesheet_parse(parser, undef->block.data, undef->block.length);

    if (ss && ss->root) {
        cb(ss->root, ctx);
        
        char *new_block = NULL;
        lxb_css_rule_serialize(ss->root, string_serializer_cb, &new_block);
        
        if (new_block) {
            undef->block.data = (lxb_char_t*)new_block;
            undef->block.length = strlen(new_block);
            register_garbage(new_block);
        } else {
             undef->block.data = (lxb_char_t*)"";
             undef->block.length = 0;
        }
        lxb_css_stylesheet_destroy(ss, true);
    }
    lxb_css_parser_destroy(parser, true);
}

// --- Forward Declarations ---
// --- Forward Declarations ---
static void pass1_filter_rules(lxb_css_rule_t *rule, char **used_classes, int class_count, char **used_tags, int tag_count);
static void pass2_collect_deps(lxb_css_rule_t *rule, dep_list_t *vars, dep_list_t *anims);
static bool pass3_refine_rules(lxb_css_rule_t *rule, dep_list_t *used_vars, dep_list_t *used_anims);

// --- Wrappers and Helpers ---

// Pass 1 Wrapper
struct pass1_ctx {
    char **used_classes;
    int count;
    char **used_tags;
    int tag_count;
};
static void pass1_cb(lxb_css_rule_t *root, void *ctx) {
    struct pass1_ctx *p1 = (struct pass1_ctx *)ctx;
    pass1_filter_rules(root, p1->used_classes, p1->count, p1->used_tags, p1->tag_count);
}

// Pass 2 Wrapper
struct pass2_ctx {
    dep_list_t *vars;
    dep_list_t *anims;
};
static void pass2_cb(lxb_css_rule_t *root, void *ctx) {
     struct pass2_ctx *p2 = (struct pass2_ctx *)ctx;
     pass2_collect_deps(root, p2->vars, p2->anims);
}

// Pass 3 Wrapper
struct pass3_ctx {
    dep_list_t *vars;
    dep_list_t *anims;
};
static void pass3_cb(lxb_css_rule_t *root, void *ctx) {
    struct pass3_ctx *p3 = (struct pass3_ctx *)ctx;
    pass3_refine_rules(root, p3->vars, p3->anims);
}

// Helper: Check if class is used
static bool is_class_used(const char *class_name, size_t len, const char **used_classes, size_t class_count) {
    if (!used_classes || class_count == 0) return false;
    for (size_t i = 0; i < class_count; i++) {
        if (strncmp(class_name, used_classes[i], len) == 0 && used_classes[i][len] == '\0') {
            return true;
        }
    }
    return false;
}

// Helper: Check if tag is used
static bool is_tag_used(const char *tag_name, size_t len, const char **used_tags, size_t tag_count) {
    if (!used_tags || tag_count == 0 || !tag_name) return false;
    
    // Case-insensitive check for tags is safer, though inputs are likely normalized.
    // Assuming simple case for now.
    for (size_t i = 0; i < tag_count; i++) {
        if (strncasecmp(tag_name, used_tags[i], len) == 0 && used_tags[i][len] == '\0') {
            return true;
        }
    }
    return false;
}

// Helper: Check selector chain
static bool should_keep_selector_node(lxb_css_selector_list_t *list, const char **used_classes, size_t class_count, const char **used_tags, size_t tag_count) {
    lxb_css_selector_t *sel = list->first;
    while (sel) {
        if (sel->type == LXB_CSS_SELECTOR_TYPE_CLASS) {
            if (!is_class_used((const char *)sel->name.data, sel->name.length, used_classes, class_count)) {
                return false; 
            }
        } else if (sel->type == LXB_CSS_SELECTOR_TYPE_ELEMENT) {
            // Check if it's a known tag that should be filtered
            if (tag_count > 0 && sel->name.length > 0) {
                 // Standard HTML tags only. Ignore * or complex namespaces if any for now.
                 if (strcmp((const char*)sel->name.data, "*") != 0 && 
                    !is_tag_used((const char*)sel->name.data, sel->name.length, used_tags, tag_count)) {
                     // printf("Removing unused tag: %.*s\n", (int)sel->name.length, sel->name.data);
                     return false;
                 }
            }
        } else if (sel->type == LXB_CSS_SELECTOR_TYPE_PSEUDO_CLASS_FUNCTION) {
             // Example: :not(.class)
             // We need to inspect arguments? 
             // Lexbor might store nested selectors in sel->u.func.selector?
             // Actually, verify struct structure first.
        }
        sel = sel->next;
    }
    return true;
}

// --- Implementations ---

// Helper: Check if a bad style rule (raw string) should be kept
static bool should_keep_bad_style(const lxb_char_t *data, size_t len, char **used_classes, int class_count, char **used_tags, int tag_count) {
    (void)used_tags; (void)tag_count;
    if (!data || len == 0) return false;
    
    bool has_classes = false;
    bool has_used_class = false;
    // Track tags implicitly? Bad style parsing is weak.
    // If it looks like a tag (starts with alpha, no dot/hash prefix), we should check it?
    // Doing full tokenization on bad style is hard. 
    // Current requirement: "h1 is not used, remove it".
    // "h1" likely comes as a STYLE rule, not bad style. 
    // For bad style, let's keep existing class logic for now, or maybe expand if needed later.
    // Actually, let's play safe and ONLY filter based on classes in bad style for now to avoid false negatives on property values taking "tag-like" names.
    
    size_t i = 0;
    while (i < len) {
        // Look for class start '.'
        if (data[i] == '.') {
            // Check if next is identifier start
            if (i + 1 < len && (isalpha(data[i+1]) || data[i+1] == '_' || data[i+1] == '-')) {
                has_classes = true;
                i++; // Skip dot
                
                size_t start = i;
                // Scan identifier
                while (i < len && (isalnum(data[i]) || data[i] == '_' || data[i] == '-')) {
                    i++;
                }
                size_t name_len = i - start;
                
                if (is_class_used((const char*)(data + start), name_len, (const char**)used_classes, class_count)) {
                    has_used_class = true;
                }
                continue;
            }
        }
        i++;
    }
    
    // If it has classes, it MUST have at least one used class to be kept.
    // If it has NO classes (e.g. tag selectors only), keep it conservatively.
    if (has_classes && !has_used_class) {
        return false;
    }
    return true;
}

// PASS 1
static void pass1_filter_rules(lxb_css_rule_t *rule, char **used_classes, int class_count, char **used_tags, int tag_count) {
    if (rule == NULL) return;

    if (rule->type == LXB_CSS_RULE_LIST || rule->type == LXB_CSS_RULE_STYLESHEET) {
        lxb_css_rule_list_t *list = (lxb_css_rule_list_t *)rule;
        lxb_css_rule_t *current = list->first;
        
        while (current) {
            bool remove = false;
            lxb_css_rule_t *next = current->next;

            if (current->type == LXB_CSS_RULE_STYLE) {
                lxb_css_rule_style_t *style = lxb_css_rule_style(current);
                lxb_css_selector_list_t *sel_list = style->selector;
                lxb_css_selector_list_t *prev = NULL;
                bool has_any_used = false;
                
                while (sel_list) {
                    lxb_css_selector_list_t *next_node = sel_list->next;
                    
                    if (should_keep_selector_node(sel_list, (const char**)used_classes, class_count, (const char**)used_tags, tag_count)) {
                        has_any_used = true;
                        prev = sel_list;
                    } else {
                        if (prev) prev->next = next_node;
                        else style->selector = next_node;
                        
                        if (next_node) next_node->prev = prev;
                        
                        lxb_css_selector_list_destroy(sel_list);
                    }
                    sel_list = next_node;
                }
                
                if (!has_any_used) {
                    remove = true;
                }
            } else if (current->type == LXB_CSS_RULE_AT_RULE) {
                lxb_css_rule_at_t *at = (lxb_css_rule_at_t *)current;
                
                if (at->type == LXB_CSS_AT_RULE__UNDEF) {
                    struct pass1_ctx ctx = { .used_classes = used_classes, .count = class_count, .used_tags = used_tags, .tag_count = tag_count };
                    process_nested_block(at->u.undef, pass1_cb, &ctx);
                    
                    if (at->u.undef->block.length == 0) {
                        remove = true;
                    } 
                }
            } else if (current->type == LXB_CSS_RULE_LIST) {
                 pass1_filter_rules(current, used_classes, class_count, used_tags, tag_count);
            } else if (current->type == LXB_CSS_RULE_BAD_STYLE) {
                // Handle BAD_STYLE (rules that failed full parsing, e.g. due to complex pseudo-classes)
                // Filter them by checking if they contain unused classes in their raw selector string.
                lxb_css_rule_bad_style_t *bad = (lxb_css_rule_bad_style_t *)current;
                if (!should_keep_bad_style(bad->selectors.data, bad->selectors.length, used_classes, class_count, used_tags, tag_count)) {
                    remove = true;
                }
            }

            if (remove) {
                if (current->prev) current->prev->next = current->next;
                else list->first = current->next;
                
                if (current->next) current->next->prev = current->prev;
                else list->last = current->prev;
                
                lxb_css_rule_destroy(current, true);
            }
            current = next;
        }
    }
}

// PASS 2
static void pass2_collect_deps(lxb_css_rule_t *rule, dep_list_t *vars, dep_list_t *anims) {
    if (rule->type == LXB_CSS_RULE_STYLE) {
        lxb_css_rule_style_t *style = lxb_css_rule_style(rule);
        if (style->declarations) {
            lxb_css_rule_t *decl_rule = style->declarations->first;
            while (decl_rule) {
                if (decl_rule->type == LXB_CSS_RULE_DECLARATION) {
                    lxb_css_rule_declaration_t *decl = (lxb_css_rule_declaration_t *)decl_rule;
                    char *name = NULL;
                    char *value = NULL;
                    lxb_css_rule_declaration_serialize_name(decl, string_serializer_cb, &name);
                    lxb_css_rule_declaration_serialize(decl, string_serializer_cb, &value);
                    
                    if (value) {
                        char *p = value;
                        while ((p = strstr(p, "var(--"))) {
                            p += 6;
                            char *end = strchr(p, ')');
                            if (end) {
                                add_dep(vars, p, end - p);
                            }
                        }
                        
                        if (name && (strcmp(name, "animation") == 0 || strcmp(name, "animation-name") == 0)) {
                             char *vcopy = malloc(strlen(value) + 1);
                             if (vcopy) {
                                 strcpy(vcopy, value);
                                 char *val_start = strchr(vcopy, ':');
                                 if (val_start) val_start++; else val_start = vcopy;
                                 
                                 char *tok = strtok(val_start, " ,;");
                                 while (tok) {
                                     add_dep(anims, tok, strlen(tok));
                                     tok = strtok(NULL, " ,;");
                                 }
                                 free(vcopy);
                             }
                        }
                    }
                    if (name) free(name);
                    if (value) free(value);
                }
                decl_rule = decl_rule->next;
            }
        }
    }
    else if (rule->type == LXB_CSS_RULE_LIST) {
        lxb_css_rule_list_t *l = lxb_css_rule_list(rule);
        lxb_css_rule_t *child = l->first;
        while (child) {
            pass2_collect_deps(child, vars, anims);
            child = child->next;
        }
    }
    else if (rule->type == LXB_CSS_RULE_AT_RULE) {
        lxb_css_rule_at_t *at = (lxb_css_rule_at_t *)rule;
        if (at->type == LXB_CSS_AT_RULE__UNDEF) {
            struct pass2_ctx ctx = { .vars = vars, .anims = anims };
            process_nested_block(at->u.undef, pass2_cb, &ctx);
        }
    }
}

// PASS 3
static bool pass3_refine_rules(lxb_css_rule_t *rule, dep_list_t *used_vars, dep_list_t *used_anims) {
    if (rule->type == LXB_CSS_RULE_STYLE) {
        lxb_css_rule_style_t *style = lxb_css_rule_style(rule);
        if (style->declarations) {
            lxb_css_rule_t *decl_rule = style->declarations->first;
            lxb_css_rule_t *next_decl = NULL;
            while (decl_rule) {
                next_decl = decl_rule->next;
                if (decl_rule->type == LXB_CSS_RULE_DECLARATION) {
                    lxb_css_rule_declaration_t *decl = (lxb_css_rule_declaration_t *)decl_rule;
                    char *name = NULL;
                    lxb_css_rule_declaration_serialize_name(decl, string_serializer_cb, &name);
                    
                    if (name && strncmp(name, "--", 2) == 0) {
                        if (!is_dep_used(used_vars, name + 2, strlen(name) - 2)) {
                            if (decl_rule->prev) decl_rule->prev->next = decl_rule->next;
                            else style->declarations->first = decl_rule->next;
                            
                            if (decl_rule->next) decl_rule->next->prev = decl_rule->prev;
                            else style->declarations->last = decl_rule->prev;
                            
                            style->declarations->count--;
                            lxb_css_rule_destroy(decl_rule, true);
                        }
                    }
                    if (name) free(name);
                }
                decl_rule = next_decl;
            }
        }
        return (style->selector != NULL && (style->declarations == NULL || style->declarations->count > 0));
    }
    else if (rule->type == LXB_CSS_RULE_LIST) {
        lxb_css_rule_list_t *l = lxb_css_rule_list(rule);
        lxb_css_rule_t *child = l->first;
        lxb_css_rule_t *next_child = NULL;
        while (child) {
            next_child = child->next;
            if (!pass3_refine_rules(child, used_vars, used_anims)) {
                 if (child->prev) child->prev->next = child->next;
                 else l->first = child->next;
                 if (child->next) child->next->prev = child->prev;
                 else l->last = child->prev;
                 lxb_css_rule_destroy(child, true);
            }
            child = next_child;
        }
        return l->first != NULL;
    }
    else if (rule->type == LXB_CSS_RULE_AT_RULE) {
        lxb_css_rule_at_t *at = (lxb_css_rule_at_t *)rule;
        
        if (at->type == LXB_CSS_AT_RULE__UNDEF) {
            struct pass3_ctx ctx = { .vars = used_vars, .anims = used_anims };
            process_nested_block(at->u.undef, pass3_cb, &ctx);
             if (at->u.undef->block.length == 0) {
                 return false;
             }
        }
        
        char *name = NULL;
        lxb_css_rule_at_serialize_name(at, string_serializer_cb, &name);
        
        bool keep = true;
        if (name && strcmp(name, "keyframes") == 0) {
             keep = false;
             if (at->type == LXB_CSS_AT_RULE__UNDEF || at->type == LXB_CSS_AT_RULE__CUSTOM) {
                lexbor_str_t *prelude = (at->type == LXB_CSS_AT_RULE__UNDEF) ? &at->u.undef->prelude : &at->u.custom->prelude;
                if (prelude && prelude->data) {
                    char *temp = malloc(prelude->length + 1);
                    if (temp) {
                        memcpy(temp, prelude->data, prelude->length);
                        temp[prelude->length] = '\0';
                        char *clean_name = strtok(temp, " \t\n\r");
                        if (clean_name && is_dep_used(used_anims, clean_name, strlen(clean_name))) {
                            keep = true;
                        }
                        free(temp);
                    }
                }
            }
        }
        if (name) free(name);
        return keep;
    }
    return true;
}

// --- Main API ---

bool css_validate(const char *css_content, size_t length) {
    if (!css_content || length == 0) return false;
    lxb_css_parser_t *parser = lxb_css_parser_create();
    lxb_css_parser_init(parser, NULL);
    lxb_css_stylesheet_t *stylesheet = lxb_css_stylesheet_parse(parser, (const lxb_char_t *)css_content, length);
    bool success = (stylesheet != NULL);
    lxb_css_stylesheet_destroy(stylesheet, true);
    lxb_css_parser_destroy(parser, true);
    return success;
}

char *css_optimize(const char *css_content, size_t length, 
                   const char **used_classes, size_t class_count,
                   const char **used_tags, size_t tag_count) {
    if (!css_content || length == 0) return NULL;

    lxb_css_parser_t *parser = lxb_css_parser_create();
    lxb_css_parser_init(parser, NULL);
    lxb_css_stylesheet_t *stylesheet = lxb_css_stylesheet_parse(parser, (const lxb_char_t *)css_content, length);

    if (!stylesheet) {
        lxb_css_parser_destroy(parser, true);
        return NULL;
    }

    if (stylesheet->root) {
        // PASS 1: Filter rules by selector (classes and tags)
        // Now pass used_tags and tag_count
        pass1_filter_rules(stylesheet->root, (char **)used_classes, class_count, (char **)used_tags, tag_count);
    }
    
    dep_list_t *used_vars = malloc(sizeof(dep_list_t));
    dep_list_t *used_anims = malloc(sizeof(dep_list_t));
    if (used_vars) memset(used_vars, 0, sizeof(dep_list_t));
    if (used_anims) memset(used_anims, 0, sizeof(dep_list_t));

    if (stylesheet->root && used_vars && used_anims) {
        pass2_collect_deps(stylesheet->root, used_vars, used_anims);
    }
    
    if (stylesheet->root && used_vars && used_anims) {
         pass3_refine_rules(stylesheet->root, used_vars, used_anims);
    }
    
    char *output = NULL;
    if (stylesheet->root) {
        lxb_css_rule_serialize(stylesheet->root, string_serializer_cb, &output);
    }
    
    if (!output) {
        output = calloc(1, 1);
    }

    if (used_vars) { free_deps(used_vars); free(used_vars); }
    if (used_anims) { free_deps(used_anims); free(used_anims); }
    
    lxb_css_stylesheet_destroy(stylesheet, true);
    lxb_css_parser_destroy(parser, true);
    
    clear_garbage();

    return output;
}
