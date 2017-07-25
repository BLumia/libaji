#ifndef __LAJI_CONF
#define __LAJI_CONF

enum laji_conf_datetype {
    LAJI_INT,
    LAJI_STR,
    LAJI_ARR,
    LAJI_FLT,
    LAJI_CHR,
    LAJI_LAJI
};

int laji_conf_open(const char* filename);
int laji_conf_next_variable();
char* laji_conf_get_section();
char* laji_conf_get_varname();
int laji_conf_get_variable(void* variable);
enum laji_conf_datetype laji_conf_get_type();
char* laji_conf_get_raw_variable();
int laji_conf_close();
char* laji_trim(char *buffer); // util for removing whitespace

#endif /* __LAJI_CONF */