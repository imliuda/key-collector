#include <stdio.h>
#include <argp.h>




int main(int argc, char **argv) {
    argp_parse(&argp, argc, argv, 0, 0, 0);
       
    return 0;
}
