#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "configman.h"

char laji_conf_cur_section[61];
char laji_conf_cur_varname[61];
char laji_conf_cur_variable_raw[616]; // may be an array
int laji_arr_flag = 0;
char* laji_arr_buffer_ptr;

FILE* laji_conf_fileptr;

int laji_conf_open(const char* filename) {
    laji_conf_fileptr = fopen(filename, "r");
    if (laji_conf_fileptr == NULL) {
        perror("laji_conf_open()");
        return -1;
    }
    return 0;
}

int laji_conf_next_variable() {
    char buffer[100], buffer2[100], *buffer_ptr, *varbuf_ptr; // 61 but for robust 
laji_conf_nextline:
    if (fgets(buffer, 100, laji_conf_fileptr) != NULL) {
        
        // remove comments
        if (buffer[0] == '#') goto laji_conf_nextline; // strtok will not touch the first character..
        strtok(buffer, "#"); 

        buffer_ptr = laji_trim(buffer);
        int slen = strlen(buffer_ptr);

        if (laji_arr_flag == 1) {
            if (*buffer_ptr == ']') {
                *(laji_arr_buffer_ptr - 1) = ']';
                *laji_arr_buffer_ptr = '\0';
                laji_arr_flag = 0;
                return 0;
            } else {
                strncpy(laji_arr_buffer_ptr, buffer_ptr, slen);
                laji_arr_buffer_ptr += slen;
                *laji_arr_buffer_ptr = ',';
                laji_arr_buffer_ptr ++;
                goto laji_conf_nextline;
            }
        }

        if (*buffer_ptr == '[') { // section name line.
            strncpy(buffer2, buffer_ptr + 1, (slen - 2));
            buffer2[slen - 2] = '\0';
            strcpy(laji_conf_cur_section, laji_trim(buffer2));
            goto laji_conf_nextline;
        }
        
        varbuf_ptr = strtok(buffer_ptr, "=");
        varbuf_ptr = strtok(NULL, "=");
        strcpy(laji_conf_cur_varname, laji_trim(buffer_ptr));
        strcpy(laji_conf_cur_variable_raw, laji_trim(varbuf_ptr));
        
        if (laji_conf_cur_variable_raw[0] == '[') {
            laji_arr_flag = 1;
            laji_arr_buffer_ptr = laji_conf_cur_variable_raw + 1;
            goto laji_conf_nextline;
        }

        return 0;
    }
    return 1;
}

char* laji_conf_get_section() {
    return laji_conf_cur_section;
}

char* laji_conf_get_varname() {
    return laji_conf_cur_varname;
}

enum laji_conf_datetype laji_conf_get_type() {
    return LAJI_LAJI; // todo: impl
}

char* laji_conf_get_raw_variable() {
    return laji_conf_cur_variable_raw;
}

int laji_conf_close() {
    fclose(laji_conf_fileptr);
}

char* laji_trim(char *buffer) {
    size_t len = strlen(buffer);
    char *pstart = buffer;
    char *pend = buffer + len;

    if (buffer == NULL) return NULL;
    if (buffer[0] == '\0') return buffer;

    while(isspace(*(pend - 1))) { 
        pend--;
        *pend = '\0';
    }

    while(isspace(*pstart)) { 
        pstart++;
    }

    return pstart;
}
