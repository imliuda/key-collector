#include "../src/config.h"

int main() {
    struct config *config = config_new();
    config_load(config, "../etc/osclt/osclt.conf");
}
