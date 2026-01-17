#include <lexbor/css/css.h>
#include <stdio.h>
#include <string.h>

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

int main() {
    const char *css = "@namespace \"foo\"; @charset \"UTF-8\"; h1 { color: red; }";
    lxb_css_parser_t *parser = lxb_css_parser_create();
    lxb_css_parser_init(parser, NULL);
    lxb_css_stylesheet_t *ss = lxb_css_stylesheet_parse(parser, (const lxb_char_t *)css, strlen(css));

    if (ss && ss->root) {
        lxb_css_rule_list_t *list = (lxb_css_rule_list_t *)ss->root;
        lxb_css_rule_t *curr = list->first;
        while (curr) {
            char *ser = NULL;
            lxb_css_rule_serialize(curr, string_serializer_cb, &ser);
            if (curr->type == LXB_CSS_RULE_AT_RULE) {
                lxb_css_rule_at_t *at = (lxb_css_rule_at_t *)curr;
                if (at->type == LXB_CSS_AT_RULE__CUSTOM) {
                     char *ser2 = NULL;
                     lxb_css_at_rule__custom_serialize(at->u.custom, string_serializer_cb, &ser2);
                     printf("Rule Type: %d, At-Rule Type: %d, CUSTOM_SERIALIZE: [%s], lxb_css_rule_serialize: [%s]\n", 
                            curr->type, (int)at->type, ser2, ser);
                     free(ser2);
                } else {
                     printf("Rule Type: %d, At-Rule Type: %d, Serialized: [%s]\n", curr->type, (int)at->type, ser);
                }
            } else {
                printf("Rule Type: %d, Serialized: [%s]\n", curr->type, ser);
            }
            free(ser);
            curr = curr->next;
        }

        char *serialized = NULL;
        lxb_css_rule_serialize(ss->root, string_serializer_cb, &serialized);
        printf("Full Serialized: [%s]\n", serialized);
        free(serialized);
    }

    lxb_css_stylesheet_destroy(ss, true);
    lxb_css_parser_destroy(parser, true);
    return 0;
}
