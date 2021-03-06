#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "bf_types.h"
#include "bf_ops.h"

#define BYTES_FOR_PID sizeof('.') + sizeof('-') + 2.5 * sizeof(pid_t) + sizeof('\0') + 1
/* bytes for decimal record of int := 
:= ceil(log10(INT_MAX)) + sign + zero-byte (+ 1 to avoid real ceil call)
*/

//Storage operations
int _bf_commit_snapshot(const char *src, const char *dst) {
    int ret = -1;
    if (rename(src, dst) == 0) {
        ret = 0;
    } else {
        fputs("Unable to rename inprogress snapshot into actual location.\n",
            stderr);
    }
    return ret;
}

int bf_dump_to_file(const bloom_filter_t *bf, const char *fname)
{
    int ret = -1;
    size_t bwritten;
    size_t shouldwrite;

    bf_dump_header_t hdr;
    hdr.m = bf->m;
    hdr.k = bf->k;

    fputs("Saving snapshot...\n", stderr);

    char *inprogress_fn = malloc(strlen(fname) + BYTES_FOR_PID);
    sprintf(inprogress_fn, "%s.%d", fname, getpid());

    FILE *f = fopen(inprogress_fn, "wb");

    if (f) {
        if (
        1 == fwrite(&hdr.m, sizeof(hdr.m), 1, f) &&
        1 == fwrite(&hdr.k, sizeof(hdr.k), 1, f) &&
        1 == fwrite(&hdr.reserved, sizeof(hdr.reserved), 1, f) ) {
            shouldwrite = (bf->m + (CHAR_BIT - 1)) / CHAR_BIT;
            bwritten = fwrite(bf->space, 1, shouldwrite, f);
            if (bwritten == shouldwrite) {
                ret = _bf_commit_snapshot(inprogress_fn, fname);
            } else {
                fprintf(stderr, "Should write: %lu bytes. Written %lu bytes.\n",
                    (unsigned long) shouldwrite, (unsigned long) bwritten);
            }
        } else {
            fputs("Unable to write header.\n", stderr);
        }
        fclose(f);
    } else {
        fputs("Error opening file for writting.\n", stderr);
    }
    free(inprogress_fn);
    return ret;
}

bloom_filter_t *bf_load_from_file(const char *fname)
{
    bloom_filter_t *ret = NULL;
    bloom_filter_t *bf;
    size_t shouldread;
    size_t bread;

    bf_dump_header_t hdr;

    FILE *f = fopen(fname, "rb");
    if (f) {
        if (
        1 == fread(&hdr.m, sizeof(hdr.m), 1, f) &&
        1 == fread(&hdr.k, sizeof(hdr.k), 1, f) &&
        1 == fread(&hdr.reserved, sizeof(hdr.reserved), 1, f) ) {
            if ((hdr.m != 0) && !(hdr.m & (hdr.m - 1))) {
                if ((bf = bf_create(hdr.m, hdr.k))) {
                    shouldread = (hdr.m + (CHAR_BIT - 1)) / CHAR_BIT;
                    bread = fread(bf->space, 1, shouldread, f);
                    if (bread == shouldread) {
                        ret = bf;
                    } else {
                        fprintf(stderr, "Should read: %lu bytes. Read %lu bytes.\n",
                            (unsigned long) shouldread, (unsigned long) bread);
                        bf_destroy(bf);
                    }
                } else {
                    fputs("bf_create failed!\n", stderr);
                }
            } else {
                fputs("m must be power of 2.\n", stderr);
            }
        } else {
            fputs("Unable to read header.\n", stderr);
        }
        fclose(f);
    } else {
        fputs("Error opening file for reading.\n", stderr);
    }
    return ret;
}
