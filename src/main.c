#include "log.h"
#include "engine.h"

#include "version.h"

#include <stdio.h>

int main(int argc, char** argv) {
    if (cortex_log_init(stderr)) {
        return -1;
    }

    cortex_log("Starting cortex version " CORTEX_VERSION);
    
    cortex_options opts;

    if (cortex_options_load(&opts, argc, argv)) {
        cortex_error("Invalid options!\n");

        fprintf(stderr, "usage: %s [-d --debug]\n", argv[0]);

        return -1;
    }

    cortex_engine eng;
    
    if (cortex_engine_init(&eng, &opts)) {
        cortex_error("Engine init failed!\n");
        return -1;
    }

   int res = cortex_engine_run(&eng);

   cortex_engine_free(&eng);

   if (res) {
       cortex_error("Engine terminated with FAILURE code %d", res);
   } else {
       cortex_log("Engine terminated with code %d", res);
   }

   cortex_log("Goodbye!");
   return -1;
}
