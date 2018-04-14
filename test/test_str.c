#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../src/str.h"

int main() {
    struct strbuf *sb = strbufnew(2);
    strbufexts(sb, "asdf");
    printf("%s\n", strbufstr(sb));
    strbuffree(sb);

    // \x6d4b\x8bd5 : \xe6\xb5\x8b\xe8\xaf\x95 : 测试 
    wchar_t *ws = strutf8dec("\xe6\xb5\x8b\xe8\xaf\x95");
    printf("%x, %x, %x\n", ws[0], ws[1], ws[2]);

    char *u8bs = strutf8enc(ws);
    printf("%ls", u8bs);
    for (int i = 0; i < strlen(u8bs); i++) {
        printf("%hhx", u8bs[i]);
    }
    printf("\n");
    free(ws);
    free(u8bs);
}
