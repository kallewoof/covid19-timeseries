#include <string.h>
#include <assert.h>

#include "dataset.h"

VList VListReadCSV(FILE* fp) {
    VList vl = malloc(sizeof(struct vlist_t));
    vl->vcount = 0;
    size_t cap = 4;
    vl->values = malloc(sizeof(char*) * cap);
    char buf[1024];
    char* p = buf;
    int running = 1;
    while (running) {
        char c = fgetc(fp);
        switch (c) {
        case 0:
        case EOF:
        case '\n':
        case ',':
            if (p > buf) {
                if (vl->vcount == cap) {
                    cap <<= 1;
                    vl->values = realloc(vl->values, sizeof(char*) * cap);
                }
                *p = 0;
                vl->values[vl->vcount++] = strdup(buf);
                p = buf;
            }
            running = c == ',';
            break;
        default:
            *(p++) = c;
        }
    }
    return vl;
}

uint32_t VListGetUI32(VList vl, size_t idx) {
    if (vl->vcount <= idx) {
        fprintf(stderr, "error: value %zu out of bounds in VListGetUI32() with vcount=%zu\n", idx, vl->vcount);
        exit(1);
    }
    return atoi(vl->values[idx]);
}

float VListGetFloat(VList vl, size_t idx) {
    if (vl->vcount <= idx) {
        fprintf(stderr, "error: value %zu out of bounds in VListGetFloat() with vcount=%zu\n", idx, vl->vcount);
        exit(1);
    }
    return atof(vl->values[idx]);
}

const char* VListString(VList vl) {
    static char* buf = NULL;
    static size_t cap = 0;
    size_t len = vl->vcount + 1; // ',' * count + \0
    for (size_t i = 0; i < vl->vcount; ++i) len += strlen(vl->values[i]);
    if (cap < len) {
        cap = len;
        buf = buf ? realloc(buf, cap) : malloc(cap);
    }
    char* p = buf;
    for (size_t i = 0; i < vl->vcount; ++i) {
        p += sprintf(p, "%s%s", p == buf ? "" : ",", vl->values[i]);
    }
    return buf;
}

void VListDone(VList vl) {
    for (size_t i = 0; i < vl->vcount; ++i) free(vl->values[i]);
    free(vl->values);
    free(vl);
}

Entry EntryNew(Entry prev, time_t date, uint32_t confirmed, uint32_t recovered, uint32_t dead) {
    Entry e = malloc(sizeof(struct entry_t));
    if (prev) prev->next = e;
    e->date = date;
    e->crd[0] = confirmed;
    e->crd[1] = recovered;
    e->crd[2] = dead;
    e->next = NULL;
    return e;
}

Entry EntryNewWithAspect(Entry prev, time_t date, uint8_t aspect, uint32_t value) {
    Entry e = malloc(sizeof(struct entry_t));
    if (prev) prev->next = e;
    e->date = date;
    // note: non-aspect values are garbage values
    e->crd[aspect] = value;
    e->next = NULL;
    return e;
}

void EntryDone(Entry e) {
    for (Entry n = e->next; e; e = n, n = n->next) {
        free(e);
    }
}

time_t GetDate(uint16_t year, uint8_t month, uint8_t day) {
    struct tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = 12;
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return mktime(&tm);
}

time_t GetDateFromGoofyString(const char* goofball /* YYYY/MM/DD */) {
    uint32_t y, m, d;
    if (3 > sscanf(goofball, "%u/%u/%u", &y, &m, &d)) return 0;
    return GetDate(y, m, d);
}

const char* GetGoofyDateStringFromTimestamp(time_t ts) {
    static char* s = NULL;
    if (!s) s = (char*)malloc(11);
    struct tm* t = localtime(&ts);
    sprintf(s, "%04d/%02d/%02d",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday);
    return s;
}

time_t IterateTimestamp(time_t t) { return t + 24 * 60 * 60; }

Country CountryNew(const char code[2], const char* name, const char* region, float lat, float lon) {
    Country c = malloc(sizeof(struct country_t));
    memcpy(c->code, code, 2);
    c->name = strdup(name);
    c->region = strdup(region);
    c->lat = lat;
    c->lon = lon;
    c->head = c->tail = NULL;
    c->start = GetDateFromGoofyString(DATA_START);
    return c;
}

uint16_t CountryGetIndex(Country c) {
    return CountryCodeGetIndex(c->code);
}

static inline uint16_t CCI(const char c) {
    return (c >= '0' && c <= '9') ? c - '0' : 10 + c - 'A';
}

uint16_t CountryCodeGetIndex(const char code[2]) {
    uint16_t rv = CCI(code[0]) * 36 + CCI(code[1]);
    assert(rv < COUNTRY_LIMIT);
    return rv;
}

void CountryInsertEntry(Country c, uint32_t confirmed, uint32_t recovered, uint32_t dead) {
    Entry e = EntryNew(c->tail, c->tail ? IterateTimestamp(c->tail->date) : c->start, confirmed, recovered, dead);
    if (c->head == NULL) c->head = e;
    c->tail = e;
}

void CountryInsertAspect(Country c, uint8_t a, uint32_t value) {
    Entry e = EntryNewWithAspect(c->tail, c->tail ? IterateTimestamp(c->tail->date) : c->start, a, value);
    if (c->head == NULL) c->head = e;
    c->tail = e;
}

Dataset DatasetNew(void) {
    Dataset ds = malloc(sizeof(struct dataset_t));
    ds->country = calloc(sizeof(Country), COUNTRY_LIMIT);
    return ds;
}

Country DatasetGetCountryByCode(Dataset ds, const char code[2]) {
    return ds->country[CountryCodeGetIndex(code)];
}

void DatasetInsertCountry(Dataset ds, Country c) {
    ds->country[CountryGetIndex(c)] = c;
}

static const char* countrycodes[] = {
    "Afghanistan","AF",
    "Åland Islands","AX",
    "Albania","AL",
    "Algeria","DZ",
    "American Samoa","AS",
    "Andorra","AD",
    "Angola","AO",
    "Anguilla","AI",
    "Antarctica","AQ",
    "Antigua and Barbuda","AG",
    "Argentina","AR",
    "Armenia","AM",
    "Aruba","AW",
    "Australia","AU",
    "Austria","AT",
    "Azerbaijan","AZ",
    "Bahamas","BS",
    "Bahrain","BH",
    "Bangladesh","BD",
    "Barbados","BB",
    "Belarus","BY",
    "Belgium","BE",
    "Belize","BZ",
    "Benin","BJ",
    "Bermuda","BM",
    "Bhutan","BT",
    "Bolivia"," Plurinational State of","BO",
    "Bonaire"," Sint Eustatius and Saba","BQ",
    "Bosnia and Herzegovina","BA",
    "Botswana","BW",
    "Bouvet Island","BV",
    "Brazil","BR",
    "British Indian Ocean Territory","IO",
    "Brunei Darussalam","BN",
    "Bulgaria","BG",
    "Burkina Faso","BF",
    "Burundi","BI",
    "Cambodia","KH",
    "Cameroon","CM",
    "Canada","CA",
    "Cape Verde","CV",
    "Cayman Islands","KY",
    "Central African Republic","CF",
    "Chad","TD",
    "Channel Islands", "X2",
    "Chile","CL",
    "China","CN",
    "Christmas Island","CX",
    "Cocos (Keeling) Islands","CC",
    "Colombia","CO",
    "Comoros","KM",
    "Congo","CG",
    "Congo"," the Democratic Republic of the","CD",
    "Cook Islands","CK",
    "Costa Rica","CR",
    "Côte d'Ivoire","CI",
    "Croatia","HR",
    "Cuba","CU",
    "Curaçao","CW",
    "Cyprus","CY",
    "Czech Republic","CZ",
    "Denmark","DK",
    "Djibouti","DJ",
    "Dominica","DM",
    "Dominican Republic","DO",
    "Ecuador","EC",
    "Egypt","EG",
    "El Salvador","SV",
    "Equatorial Guinea","GQ",
    "Eritrea","ER",
    "Estonia","EE",
    "Ethiopia","ET",
    "Falkland Islands (Malvinas)","FK",
    "Faroe Islands","FO",
    "Fiji","FJ",
    "Finland","FI",
    "France","FR",
    "French Guiana","GF",
    "French Polynesia","PF",
    "French Southern Territories","TF",
    "Gabon","GA",
    "Gambia","GM",
    "Georgia","GE",
    "Germany","DE",
    "Ghana","GH",
    "Gibraltar","GI",
    "Greece","GR",
    "Greenland","GL",
    "Grenada","GD",
    "Guadeloupe","GP",
    "Guam","GU",
    "Guatemala","GT",
    "Guernsey","GG",
    "Guinea","GN",
    "Guinea-Bissau","GW",
    "Guyana","GY",
    "Haiti","HT",
    "Heard Island and McDonald Islands","HM",
    "Holy See (Vatican City State)","VA",
    "Honduras","HN",
    "Hong Kong","HK",
    "Hungary","HU",
    "Iceland","IS",
    "India","IN",
    "Indonesia","ID",
    "Iran"," Islamic Republic of","IR",
    "Iraq","IQ",
    "Ireland","IE",
    "Isle of Man","IM",
    "Israel","IL",
    "Italy","IT",
    "Jamaica","JM",
    "Japan","JP",
    "Jersey","JE",
    "Jordan","JO",
    "Kazakhstan","KZ",
    "Kenya","KE",
    "Kiribati","KI",
    "Korea"," Democratic People's Republic of","KP",
    "Korea"," Republic of","KR",
    "Kuwait","KW",
    "Kyrgyzstan","KG",
    "Lao People's Democratic Republic","LA",
    "Latvia","LV",
    "Lebanon","LB",
    "Lesotho","LS",
    "Liberia","LR",
    "Libya","LY",
    "Liechtenstein","LI",
    "Lithuania","LT",
    "Luxembourg","LU",
    "Macao","MO",
    "Macedonia"," the Former Yugoslav Republic of","MK",
    "Madagascar","MG",
    "Malawi","MW",
    "Malaysia","MY",
    "Maldives","MV",
    "Mali","ML",
    "Malta","MT",
    "Marshall Islands","MH",
    "Martinique","MQ",
    "Mauritania","MR",
    "Mauritius","MU",
    "Mayotte","YT",
    "Mexico","MX",
    "Micronesia"," Federated States of","FM",
    "Moldova"," Republic of","MD",
    "Monaco","MC",
    "Mongolia","MN",
    "Montenegro","ME",
    "Montserrat","MS",
    "Morocco","MA",
    "Mozambique","MZ",
    "Myanmar","MM",
    "Namibia","NA",
    "Nauru","NR",
    "Nepal","NP",
    "Netherlands","NL",
    "New Caledonia","NC",
    "New Zealand","NZ",
    "Nicaragua","NI",
    "Niger","NE",
    "Nigeria","NG",
    "Niue","NU",
    "Norfolk Island","NF",
    "Northern Mariana Islands","MP",
    "Norway","NO",
    "Oman","OM",
    "Pakistan","PK",
    "Palau","PW",
    "Palestine"," State of","PS",
    "Panama","PA",
    "Papua New Guinea","PG",
    "Paraguay","PY",
    "Peru","PE",
    "Philippines","PH",
    "Pitcairn","PN",
    "Poland","PL",
    "Portugal","PT",
    "Puerto Rico","PR",
    "Qatar","QA",
    "Réunion","RE",
    "Romania","RO",
    "Russian Federation","RU",
    "Rwanda","RW",
    "Saint Barthélemy","BL",
    "Saint Helena"," Ascension and Tristan da Cunha","SH",
    "Saint Kitts and Nevis","KN",
    "Saint Lucia","LC",
    "Saint Martin (French part)","MF",
    "Saint Pierre and Miquelon","PM",
    "Saint Vincent and the Grenadines","VC",
    "Samoa","WS",
    "San Marino","SM",
    "Sao Tome and Principe","ST",
    "Saudi Arabia","SA",
    "Senegal","SN",
    "Serbia","RS",
    "Seychelles","SC",
    "Sierra Leone","SL",
    "Singapore","SG",
    "Sint Maarten (Dutch part)","SX",
    "Slovakia","SK",
    "Slovenia","SI",
    "Solomon Islands","SB",
    "Somalia","SO",
    "South Africa","ZA",
    "South Georgia and the South Sandwich Islands","GS",
    "South Sudan","SS",
    "Spain","ES",
    "Sri Lanka","LK",
    "Sudan","SD",
    "Suriname","SR",
    "Svalbard and Jan Mayen","SJ",
    "Swaziland","SZ",
    "Sweden","SE",
    "Switzerland","CH",
    "Syrian Arab Republic","SY",
    "Taiwan"," Province of China","TW",
    "Tajikistan","TJ",
    "Tanzania"," United Republic of","TZ",
    "Thailand","TH",
    "Timor-Leste","TL",
    "Togo","TG",
    "Tokelau","TK",
    "Tonga","TO",
    "Trinidad and Tobago","TT",
    "Tunisia","TN",
    "Turkey","TR",
    "Turkmenistan","TM",
    "Turks and Caicos Islands","TC",
    "Tuvalu","TV",
    "Uganda","UG",
    "Ukraine","UA",
    "United Arab Emirates","AE",
    "United Kingdom","GB",
    "United States","US",
    "United States Minor Outlying Islands","UM",
    "Uruguay","UY",
    "Uzbekistan","UZ",
    "Vanuatu","VU",
    "Venezuela"," Bolivarian Republic of","VE",
    "Viet Nam","VN",
    "Virgin Islands"," British","VG",
    "Virgin Islands"," U.S.","VI",
    "Wallis and Futuna","WF",
    "Western Sahara","EH",
    "Yemen","YE",
    "Zambia","ZM",
    "Zimbabwe","ZW"
};

uint16_t GetCountryCode(const char* countryName, const char* region, char* cc_out) {
    assert(!strcmp("Zimbabwe", countrycodes[249 >> 1]));
    const char* inputs[3] = {countryName, region, NULL};
    for (size_t x = 0; inputs[x]; ++x) {
        const char* input = inputs[x];
        int l = 0, r = 250, m;
        while (l < r) {
            m = l + ((r-l) >> 1);
            const char* candidate = countrycodes[m << 1];
            int c = strcmp(input, candidate);
            if (c < 0) {
                r = m;
            } else if (c > 0) {
                l = m;
            } else {
                const char* cc = countrycodes[(m << 1) + 1];
                if (cc_out) {
                    cc_out[0] = cc[0];
                    cc_out[1] = cc[1];
                }
                return CountryCodeGetIndex(cc);
            } 
        }
    }
    fprintf(stderr, "could not get country code for %s / %s\n", countryName, region);
    exit(1);
}
