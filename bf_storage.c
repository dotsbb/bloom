#include <stdio.h>
#include "bf_types.h"

//Storage operations
int bf_dump_to_file(const bloom_filter_t *bf, const char *fname)
{
    fputs("Saving snapshot...\n", stderr);
    size_t shouldwrite = (bf->m + (CHAR_BIT - 1)) / CHAR_BIT;
    FILE *f = fopen(fname, "wb");
    if (f) {
        size_t bwritten = fwrite(bf->space, 1, shouldwrite, f);
        if (bwritten != shouldwrite) {
            fprintf(stderr, "Should write: %ld bytes. Written %ld bytes.\n", shouldwrite, bwritten);
            return -1;
        }
        fclose(f);
        return 0;
    } else {
        fputs("Error opening file for writting.\n", stderr);
        return -1;
    }
}

int bf_load_from_file(bloom_filter_t *bf, const char *fname)
{
    FILE *f = fopen(fname, "rb");
    if (f) {
        size_t shouldread = (bf->m + (CHAR_BIT - 1)) / CHAR_BIT;
        size_t bread = fread(bf->space, 1, shouldread, f);
        if (bread != shouldread) {
            fprintf(stderr, "Should read: %ld bytes. Read %ld bytes.\n", shouldread, bread);
            return -1;
        }
        fclose(f);
        return 0;
    } else {
        fputs("Error opening file for reading.\n", stderr);
        return -1;
    }
}