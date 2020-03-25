#include "format.h"
#include "dataset.h"

// Province/State,Country/Region,Lat,Long,1/22/20,1/23/20,1/24/20,...

void cssegi_write_dataset_aspect_header(FILE* write_fp, Entry e) {
    fprintf(write_fp, "Province/State,Country/Region,Lat,Long");
    for (; e; e = e->next) {
        fprintf(write_fp, ",%s", GetGoofyDateStringFromTimestamp(e->date));
    }
    fputc('\n', write_fp);
}

void cssegi_write_dataset_aspect(FILE* write_fp, Dataset ds, Aspect a) {
    int need_header = 1;
    for (size_t i = 0; i < COUNTRY_LIMIT; ++i) {
        Country c = ds->country[i];
        if (!c) continue;
        if (need_header) {
            need_header = 0;
            cssegi_write_dataset_aspect_header(write_fp, c->head);
        }
        fprintf(write_fp, "%s,%s,%f,%f", c->name, c->region, c->lat, c->lon);
        for (Entry e = c->head; e; e = e->next) {
            fprintf(write_fp, ",%u", e->crd[a]);
        }
        fputc('\n', write_fp);
    }
}

void cssegi_read_dataset_aspect(FILE* read_fp, Dataset ds, Aspect a) {
    static char countryName[256];
    static char region[256];
    float lat, lon;
    
    uint32_t y, m, d;
    static char countryCode[2];
    uint32_t v;
    Country c = NULL;
    char buf[1024];
    for (;;) {
        if (!fscanf(read_fp, "%s,%s,%f,%f", countryName, region, &lat, &lon)) break;
        uint16_t idx = GetCountryCode(countryName, region, countryCode);
        c = ds->country[idx];
        if (!c) c = ds->country[idx] = CountryNew(countryCode, countryName, region, lat, lon);
        if (c->head) {
            // iterating on existing data
            for (Entry e = c->head; e; e = e->next) {
                if (!fscanf(read_fp, ",%u", &e->crd[a])) {
                    fprintf(stderr, "failed to read entry for %s (aspect %d)\n", GetGoofyDateStringFromTimestamp(e->date), a);
                    exit(1);
                }
            }
        } else {
            // initial data
            while (fscanf(read_fp, ",%u", &v)) {
                CountryInsertAspect(c, a, v);
            }
        }
    }
}

Format CSSEGISandDataFormat(void) {
    static struct format_t f = {"cssegi", NULL, cssegi_write_dataset_aspect, NULL, cssegi_read_dataset_aspect};
    return &f;
}
