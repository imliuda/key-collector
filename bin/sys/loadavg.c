#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) {
        perror("open \"/proc/loadavg\" failed");
        exit(EXIT_FAILURE);
    }

    double avg1, avg5, avg15;
    int nscan = fscanf(fp, "%lf %lf %lf", &avg1, &avg5, &avg15);
    if (nscan != 3) {
        perror("fscanf error");
        exit(EXIT_FAILURE);
    }

    printf("sys.load.avg1 %.2lf\n", avg1);
    printf("sys.load.avg5 %.2lf\n", avg5);
    printf("sys.load.avg15 %.2lf\n", avg15);
}
