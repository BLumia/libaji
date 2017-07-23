#include <stdio.h>
#include "configman.h"

int main() {
    laji_conf_open("./aji.conf");
    while (laji_conf_next_variable() == 0) {
        printf("var reached under [%s]! %s=%s\n",
                    laji_conf_get_section(), laji_conf_get_varname(), laji_conf_get_raw_variable());
    }
    laji_conf_close();
}
