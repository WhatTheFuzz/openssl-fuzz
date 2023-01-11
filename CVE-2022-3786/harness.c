#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* From punycode test. See link below.
** https://github.com/openssl/openssl/commit/a0af4a3c8b18c435a5a4afb28b3ad1a2730e6ea8#diff-83399d92c96bb1f4616b5c6f090053b95834cdbc7bb37bb0d835d1555f69e8ad
*/
#include <openssl/crypto.h>
#include <string.h>

#include "crypto/punycode.h"
#include "internal/nelem.h"

/* This is from crypto/punycode.c
** Why not the header? I dunno. */
#define LABEL_BUF_SIZE 512

int main(int argc, char ** argv){

    int fd;
    struct stat stat;
    int result;

    /* Ensure we have the correct number of arguments. */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Open the file passed in to argv[1]. */
    fd = open(argv[1], O_RDONLY);
    /* Check that the file exists. */
    if (fd < 0) {
        perror("Open");
        return EXIT_FAILURE;
    }

    /* Get the size of the file. */
    if (fstat(fd, &stat) < 0) {
        perror("fstat");
        return EXIT_FAILURE;
    }

    /* Create a buffer of that size. */
    char * in = malloc(stat.st_size);

    /* Read the contents of the file into the buffer at once. */
    if (read(fd, in, stat.st_size) < 0) {
        perror("read");
        return EXIT_FAILURE;
    }

    /* If DEBUG is defined, print the contents of the buffer.
    ** Users can set this at compilation time with -DDEBUG
    */
    #ifdef DEBUG
        printf("The contents of the file: %s", in);
    #endif

    /* Call the function we want to test. */
    char ulabel[256];
    size_t size = sizeof(ulabel) - 1;
    memset(ulabel, 0, sizeof(ulabel));
    result = ossl_a2ulabel(in, ulabel, &size);
    printf("ossl_a2ulabel returned: %d\n", result);

    /* Free the buffer and set it to NULL. */
    free(in);
    in = NULL;

    /* Exit the program. */
    return result;
}
