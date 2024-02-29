#include <stdio.h>
#include <stdlib.h>
#include "../ksargv/ksargv.h"

void argv_erro(e_argv_type* argv, s_ksargv_value* values, unsigned int values_count, e_argv_erro erro)
{
    printf("erro\n");
}

int main(int argc, char* argv[])
{
    char host[] = "unknown";
    int port = 65536;
    bool start = false;

    s_ksargv_elems argv_elems[] = 
    {
        {
            .option = (char* []){"--host", "-h", NULL},
            .args = (e_argv_type []){ARGV_STRING, ARGV_INT, ARGV_END},
            .parse_tpye = ATGV_PARSE_VALS,
            .vals = (void* []){host, &port},
            .function = argv_erro,
        },
        {
            .option = (char* []){"--start", "-s", NULL},
            .args = (e_argv_type []){ARGV_BOOL, ARGV_END},
            .parse_tpye = ATGV_PARSE_VALS,
            .vals = (void* []){&start},
            .function = argv_erro,
        }
    };

    ksargv_parse_argv(argv, argv_elems, 2);

    printf("host = %s\n", host);
    printf("port = %d\n", port);
    printf("start = %d\n", start);
}