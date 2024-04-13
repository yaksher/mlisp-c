#include <stdlib.h>
#include <stdio.h>
#include "ast.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        fprintf(stderr, "Failed to open file: %s\n", argv[1]);
        return 1;
    }
    Ast ast = ast_deserialize(file);
    fclose(file);
    ast_print(ast);
    return 0;
}