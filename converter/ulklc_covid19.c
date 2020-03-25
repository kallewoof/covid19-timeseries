#include "format.h"
#include "dataset.h"

// 2020/01/22,QA,Qatar,Asia,25.5,51.25,0,0,0

void ulklc_write_dataset_raw(FILE* write_fp, Dataset ds) {
    for (size_t i = 0; i < 625; ++i) {
        Country c = ds->country[i];
        if (!c) continue;
        char fmt[1024];
        sprintf(fmt, "%%s,%c%c,%s,%s,%f,%f,%%u,%%u,%%u\n", c->code[0], c->code[1], c->name, c->region, c->lat, c->lon);
        for (Entry e = c->head; e; e = e->next) {
            fprintf(write_fp, fmt, GetGoofyDateStringFromTimestamp(e->date), e->crd[0], e->crd[1], e->crd[2]);
        }
    }
}

void ulklc_read_dataset_raw(FILE* read_fp, Dataset ds) {
    time_t date;
    const char* countryCode;
    const char* countryName;
    const char* region;
    float lat, lon;
    uint32_t cc, cr, cd;
    Country c = NULL;
    uint8_t first = 1;
    for (;;) {
        VList vl = VListReadCSV(read_fp);
        if (vl->vcount == 0) {
            // we're done
            return;
        }
        if (9 != vl->vcount) {
            fprintf(stderr, "failed to parse input: %s\n", VListString(vl));
            exit(1);
        }
        if (vl->values[0][0] == 'd') {
            if (first) {
                first = 0;
                continue;
            }
            fprintf(stderr, "failed to parse input: %s\n", VListString(vl));
            exit(1);
        }
        date = GetDateFromGoofyString(vl->values[0]);
        if (strlen(vl->values[1]) != 2) {
            fprintf(stderr, "country code %s is not 2 letters in input: %s\n", vl->values[1], VListString(vl));
            exit(1);
        }
        countryCode = vl->values[1];
        countryName = vl->values[2];
        region = vl->values[3];
        lat = VListGetFloat(vl, 4);
        lon = VListGetFloat(vl, 5);
        cc = VListGetUI32(vl, 6);
        cr = VListGetUI32(vl, 7);
        cd = VListGetUI32(vl, 8);
        if (! (c && countryCode[0] == c->code[0] && countryCode[1] == c->code[1])) {
            uint16_t idx = CountryCodeGetIndex(countryCode);
            if (! (c = ds->country[idx])) {
                c = ds->country[idx] = CountryNew(countryCode, countryName, region, lat, lon);
            }
        }
        CountryInsertEntry(c, cc, cr, cd);
        VListDone(vl);
    }
}

Format ULKLCCovid19Format(void) {
    static struct format_t f = {"ulklc", ulklc_write_dataset_raw, NULL, ulklc_read_dataset_raw, NULL};
    return &f;
}
