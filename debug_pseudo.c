#include <lexbor/css/css.h>
#include <stdio.h>
#include <string.h>

int main() {
    const char *css = "div::before { content: ''; } span::after { color: blue; }";
    
    lxb_css_parser_t *parser = lxb_css_parser_create();
    lxb_css_parser_init(parser, NULL);
    lxb_css_stylesheet_t *stylesheet = lxb_css_stylesheet_parse(parser, (const lxb_char_t *)css, strlen(css));
    
    if (!stylesheet) {
        printf("Failed to parse stylesheet\n");
        lxb_css_parser_destroy(parser, true);
        return 1;
    }
    
    if (stylesheet && stylesheet->root) {
        lxb_css_rule_list_t *list = (lxb_css_rule_list_t *)stylesheet->root;
        lxb_css_rule_t *rule = list->first;
        
        printf("Processing rules...\n");
        while (rule) {
            printf("Rule type: %d\n", rule->type);
            if (rule->type == LXB_CSS_RULE_STYLE) {
                lxb_css_rule_style_t *style = lxb_css_rule_style(rule);
                lxb_css_selector_list_t *sel_list = style->selector;
                
                while (sel_list) {
                    lxb_css_selector_t *sel = sel_list->first;
                    printf("Selector chain:\n");
                    
                    while (sel) {
                        printf("  Type: %d, Name: %.*s\n", sel->type, (int)sel->name.length, sel->name.data);
                        sel = sel->next;
                    }
                    
                    sel_list = sel_list->next;
                }
            }
            rule = rule->next;
        }
    } else {
        printf("No root or stylesheet\n");
    }
    
    lxb_css_stylesheet_destroy(stylesheet, true);
    lxb_css_parser_destroy(parser, true);
    return 0;
}
