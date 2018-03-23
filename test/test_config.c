#include <locale.h>
#include <stdint.h>

#include "../src/config.h"
#include "../src/map.h"
#include "../src/list.h"

int main() {
    if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
        fprintf(stderr, "can't set local C.UTF-8.\n");
        exit(EXIT_FAILURE);
    }

    struct config *config = config_load("../etc/osclt/osclt.conf");
    config_dumps(config);
}
