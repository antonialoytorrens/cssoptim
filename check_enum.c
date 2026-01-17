#include <lexbor/css/css.h>
#include <stdio.h>

int main() {
    printf("LXB_CSS_RULE_UNDEF = %d\n", LXB_CSS_RULE_UNDEF);
    printf("LXB_CSS_RULE_STYLESHEET = %d\n", LXB_CSS_RULE_STYLESHEET);
    printf("LXB_CSS_RULE_LIST = %d\n", LXB_CSS_RULE_LIST);
    printf("LXB_CSS_RULE_AT_RULE = %d\n", LXB_CSS_RULE_AT_RULE);
    printf("LXB_CSS_RULE_STYLE = %d\n", LXB_CSS_RULE_STYLE);
    printf("LXB_CSS_RULE_BAD_STYLE = %d\n", LXB_CSS_RULE_BAD_STYLE);
    return 0;
}
