#include "format.h"
#include "dataset.h"

#define abort(msg...) do { fprintf(stderr, msg); fputc('\n', stderr); exit(1); } while (0)

int main(int argc, const char** argv) {
    if (argc < 4) abort("syntax: %s <input format> <output format> <input file> [<input file2> [...]]", argv[0]);
    Format f_in = GetFormatFromName(argv[1]);
    Format f_out = GetFormatFromName(argv[2]);

    if (!f_in) abort("input format unknown: %s", argv[1]);
    if (!f_out) abort("output format unknown: %s", argv[2]);

    Dataset ds = DatasetNew();

    if (f_in->read_dataset_aspect) {
        // input is aspect-based; we need to read from 3 inputs ordered "confirmed, recovered, dead"; multiples (6, 9, 12, ...) are not supported at this time
        if (argc != 6) abort("unsupported number of arguments (aspect based inputs require exactly 3 input files)");
        for (size_t i = 3; i < argc; ++i) {
            FILE* fp = fopen(argv[i], "r");
            if (!fp) abort("cannot open input file for reading: %s", argv[i]);
            f_in->read_dataset_aspect(fp, ds, i - 3);
            fclose(fp);
        }
    } else {
        // input is raw; no restrictions on input count
        for (size_t i = 3; i < argc; ++i) {
            FILE* fp = fopen(argv[i], "r");
            if (!fp) abort("cannot open input file for reading: %s", argv[i]);
            f_in->read_dataset_raw(fp, ds);
            fclose(fp);
        }
    }

    if (f_out->write_dataset_raw) {
        // output is raw; we can write a single file
        FILE* fp = fopen("output.csv", "w");
        f_out->write_dataset_raw(fp, ds);
        fclose(fp);
        printf("Output written to output.csv\n");
    } else {
        // output is aspect-based; we need to write 3 separate files
        const char* aspect_strings[] = {"output_confirmed.csv", "output_recovered.csv", "output_dead.csv", NULL};
        for (size_t x = 0; aspect_strings[x]; ++x) {
            FILE* fp = fopen(aspect_strings[x], "w");
            f_out->write_dataset_aspect(fp, ds, x);
            fclose(fp);
        }
        printf("Output written to output_*.csv\n");
    }
}
