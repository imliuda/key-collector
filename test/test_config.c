#include <locale.h>
#include "../src/config.h"

int main() {
    setlocale(LC_CTYPE, "C.UTF-8");

    struct config *config = config_new();
    config_load(config, "../etc/osclt/osclt.conf");
}
