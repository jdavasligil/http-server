#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "server/serve_h1.h"
#include "server/serve_h2.h"
#include "server/serve_h3.h"

extern int errno;

static inline int handle_args(char *argv[]);

int main(int argc, char *argv[]) {
    errno = 0;
    int err;

    switch (argc) {
    case 1:
        err = serve_h1();
        break;
    default:
       err = handle_args(argv); 
    }

    err = serve_h2();     

    return err;
}

int handle_args(char *argv[]) {
    char c = **argv;
    int err;
    switch (c) {
    case '1':
        err = serve_h1();
        break;
    case '2':
        err = serve_h2();
        break;
    case '3':
        err = serve_h3();
        break;
    default:
        errno = -1;
    }

    return err;
}
