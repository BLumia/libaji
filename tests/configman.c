#include <stdio.h>
#include "configman.h"

int main() {
    char charval, strval[616];
    int intval;
    double fltval;

    laji_conf_open("./aji.conf");
    while (laji_conf_next_variable() == 0) {
        switch (laji_conf_get_type()) {
            case LAJI_INT:
                laji_conf_get_variable(&intval);
                printf("INT var reached under [%s]! %s=%d\n",
                        laji_conf_get_section(), laji_conf_get_varname(), intval);
                break;
            case LAJI_FLT:
                laji_conf_get_variable(&fltval);
                printf("FLT var reached under [%s]! %s=%lf\n",
                        laji_conf_get_section(), laji_conf_get_varname(), fltval);
                break;
            case LAJI_CHR:
                laji_conf_get_variable(&charval);
                printf("CHR var reached under [%s]! %s=%c\n",
                        laji_conf_get_section(), laji_conf_get_varname(), charval);
                break;
            case LAJI_STR:
                laji_conf_get_variable(strval);
                printf("STR var reached under [%s]! %s=%s\n",
                        laji_conf_get_section(), laji_conf_get_varname(), strval);
                break;
            default:
                printf("var reached under [%s]! %s=%s\n",
                        laji_conf_get_section(), laji_conf_get_varname(), laji_conf_get_raw_variable());
        }
    }
    laji_conf_close();
}
