#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DATA_START "2020/01/22"

#define COUNTRY_LIMIT 1296

time_t GetDate(uint16_t year, uint8_t month, uint8_t day);
time_t GetDateFromGoofyString(const char* goofball /* YYYY/MM/DD */);
const char* /* YYYY/MM/DD */ GetGoofyDateStringFromTimestamp(time_t ts);
time_t IterateTimestamp(time_t t);

struct vlist_t {
    char** values;
    size_t vcount;
};

typedef struct vlist_t* VList;

VList VListReadCSV(FILE* fp);
uint32_t VListGetUI32(VList vl, size_t idx);
float VListGetFloat(VList vl, size_t idx);
const char* VListString(VList vl);
void VListDone(VList vl);

struct entry_t {
    time_t date;
    uint32_t crd[3];
    struct entry_t* next;
};

typedef struct entry_t* Entry;

Entry EntryNew(Entry prev, time_t date, uint32_t confirmed, uint32_t recovered, uint32_t dead);
Entry EntryNewWithAspect(Entry prev, time_t date, uint8_t aspect, uint32_t value); // note: non-aspect values are initially garbage values; each must be set before being read
void EntryDone(Entry e);

struct country_t {
    char code[2];
    char* name;
    char* region;
    float lat, lon;
    time_t start;
    Entry head, tail;
};

typedef struct country_t* Country;

Country CountryNew(const char code[2], const char* name, const char* region, float lat, float lon);
void CountryInsertEntry(Country c, uint32_t confirmed, uint32_t recovered, uint32_t dead);
void CountryInsertAspect(Country c, uint8_t a, uint32_t value);
uint16_t CountryGetIndex(Country c);
uint16_t CountryCodeGetIndex(const char code[2]);
uint16_t GetCountryCode(const char* countryName, const char* region, char* cc_out);

struct dataset_t {
    Country* country; // [0..COUNTRY_LIMIT]
};

typedef struct dataset_t* Dataset;

Dataset DatasetNew(void);
Country DatasetGetCountryByCode(Dataset ds, const char code[2]);
void DatasetInsertCountry(Dataset ds, Country c);
