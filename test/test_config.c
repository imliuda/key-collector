#include <stdio.h>

#include "../src/config.h"

int main() {
    config_load("/tmp/test.conf");
    config_dumps();
}
