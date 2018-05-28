#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum state {
    USER, NICE, SYSTEM, IDLE, IOWAIT, IRQ, SOFTIRQ, STEAL, GUEST, GUEST_NICE
};

int main() {
    FILE *fp = fopen("/proc/stat", "r");
    if (!fp) {
        perror("open \"/proc/stat\" failed");
        exit(EXIT_FAILURE);
    }

    char *line = NULL;
    size_t nchar = 0;

    while (getline(&line, &nchar, fp) != -1) {
        if (strstr(line, "cpu") != line)
            continue;

        char *s = line, *f, *c;
        size_t len = strlen(s);
        if (len > 0 && s[len - 1] == '\n')
            s[len - 1] = '\0';

        /* cpu */
        c = strtok(s, " ");
        if (strcmp(c, "cpu") == 0)
            strcpy(c, "all");

        /* user */
        f = strtok(NULL, " ");
        printf ("sys.cpu.user,cpu=%s %s\n", c, f ? f : "0");

        /* nice */
        f = strtok(NULL, " ");
        printf ("sys.cpu.nice,cpu=%s %s\n", c, f ? f : "0");

        /* system */
        f = strtok(NULL, " ");
        printf ("sys.cpu.system,cpu=%s %s\n", c, f ? f : "0");

        /* idle */
        f = strtok(NULL, " ");
        printf ("sys.cpu.idle,cpu=%s %s\n", c, f ? f : "0");

        /* iowait */
        f = strtok(NULL, " ");
        printf ("sys.cpu.iowait,cpu=%s %s\n", c, f ? f : "0");

        /* irq */
        f = strtok(NULL, " ");
        printf ("sys.cpu.irq,cpu=%s %s\n", c, f ? f : "0");

        /* softirq */
        f = strtok(NULL, " ");
        printf ("sys.cpu.softirq,cpu=%s %s\n", c, f ? f : "0");

        /* steal */
        f = strtok(NULL, " ");
        printf ("sys.cpu.steal,cpu=%s %s\n", c, f ? f : "0");

        /* guest */
        f = strtok(NULL, " ");
        printf ("sys.cpu.guest,cpu=%s %s\n", c, f ? f : "0");

        /* guest_nice */
        f = strtok(NULL, " ");
        printf ("sys.cpu.user,guest_nice=%s %s\n", c, f ? f : "0");

        free(line);
        line = NULL;
        nchar = 0;
    }
}
