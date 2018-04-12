#include <stdio.h>

#include "../src/str.h"

int main() {
    struct strbuf *sb = strbufnew(2);
    strbufexts(sb, "asdf");
    printf("%s\n", strbufstr(sb));
    strbuffree(sb);
}
