#include <stdio.h>
#include <string.h>

typedef struct entry_t* Entry;
typedef struct country_t* Country;
typedef struct dataset_t* Dataset;

typedef enum {
    AspectConfirmed = 0,
    AspectRecovered = 1,
    AspectDead = 2,
} Aspect;

typedef void (*format_write_dataset_raw)(FILE* write_fp, Dataset ds);
typedef void (*format_write_dataset_aspect)(FILE* write_fp, Dataset ds, Aspect a);
typedef void (*format_read_dataset_raw)(FILE* read_fp, Dataset ds);
typedef void (*format_read_dataset_aspect)(FILE* read_fp, Dataset ds, Aspect a);

struct format_t {
    const char* name;
    format_write_dataset_raw write_dataset_raw;
    format_write_dataset_aspect write_dataset_aspect;
    format_read_dataset_raw read_dataset_raw;
    format_read_dataset_aspect read_dataset_aspect;
};

typedef struct format_t* Format;

// ulklc_covid19 (name="ulklc") (https://github.com/ulklc/covid19-timeseries)

Format ULKLCCovid19Format(void);

// CSSEGISandData (name="cssegi") (https://github.com/CSSEGISandData/COVID-19.git)

Format CSSEGISandDataFormat(void);

static inline Format GetFormatFromName(const char* name) {
    Format
        f = ULKLCCovid19Format();   if (f && !strcmp(f->name, name)) return f;
        f = CSSEGISandDataFormat(); if (f && !strcmp(f->name, name)) return f;
    return NULL;
}
