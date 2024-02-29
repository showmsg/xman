#include "../ksargv/ksargv.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

typedef struct
{
    bool select;            // is elem select
} s_ksargv_elems_status;

typedef struct s_values
{
    struct s_values* next;/* data */
    char* val;
}s_values;

typedef struct s_options
{
    struct s_options* next;
    struct s_values* values;

    char*   argv;
}s_options;

/*********************************** dbg malloc *******************************************/
static int dbg_malloc_time = 0;

void* dbg_malloc(size_t size, const char* file, const char* func, int line)
{
    KSARGV_LOG_DEBUG("%s,\t%s,%d", file, func, line);
    dbg_malloc_time++;
    return malloc(size);
}

void* dbg_realloc(void* mem, size_t size, const char* file, const char* func, int line)
{
    KSARGV_LOG_DEBUG("%s,\t%s,%d", file, func, line);
    return realloc(mem, size);
}

void dbg_free(void* mem, const char* file, const char* func, int line)
{
    KSARGV_LOG_DEBUG("%s,\t%s,%d", file, func, line);
    dbg_malloc_time--;
    free(mem);
}

void dbg_print_mem(void)
{
    KSARGV_LOG_DEBUG("mem all alloc time = %d\n", dbg_malloc_time);
}

/*************************************** parse slow **************************************************/
int get_elem_argv_count(e_argv_type* args)
{
    int res = 0;

    while(args[res] < ARGV_END)
        res++;

    return res;
}

int get_opt_vals_count(s_options* option)
{
    s_values* val = option->values;
    int res = 0;

    while(val != NULL)
    {
        val = val->next;
        res++;
    }
        
    return res;
}

int argv_get_int(char* mess, bool* res)
{
    int a = atoi(mess);
    if(a == 0 && mess[0] != '0')
        *res = false;
    return a;
}

double argv_get_double(char* mess, bool* res)
{
    char* endpoint;
    double num = strtod(mess, &endpoint);
    if(endpoint == mess)
        *res = false;
    return num;
}

bool argv_get_bool(char* mess, bool* res)
{
    *res = true;
    
    if (mess == NULL)
        return true;
    if(strcmp(mess, "false") == 0 || strcmp(mess, "False") == 0)
        return false;
    else if(strcmp(mess, "true") == 0 || strcmp(mess, "True") == 0)
        return true;
    else
        return false;
}

/************************************ version v2.0 **************************************************************/
s_options* argc_get_options(char** argv)
{
    s_options* opts = NULL;
    s_options* opts_head = NULL;
    s_values* val = NULL;

    for (int i = 0; argv[i] != NULL; i++)
    {
        if (argv[i][0] == '-')
        {
            /** new opts */
            if (opts == NULL)
            {
                opts = KSARGV_MALLOC(sizeof(s_options));
                opts_head = opts;
            }
            else
            {
                s_options* opt_buff = KSARGV_MALLOC(sizeof(s_options));
                opts->next = opt_buff;
                opts = opt_buff;
            }

            KSARGV_LOG_DEBUG("new options: %s", argv[i]);
            opts->values = NULL;
            opts->argv = argv[i];
            val = NULL;
        }
        else
        {
            if (val == NULL)
            {
                opts->values = KSARGV_MALLOC(sizeof(s_options));
                val = opts->values;
            }
            else
            {
               s_values* val_buff = KSARGV_MALLOC(sizeof(s_options));
               val->next = val_buff;
               val = val_buff;
            }
            
            KSARGV_LOG_DEBUG("new values: %s:%s", opts->argv, argv[i]);
            val->next = NULL;
            val->val = argv[i];
        }
    }

    return opts_head;
}

static s_ksargv_elems* get_opt_elem(char* argv, s_ksargv_elems* elems, size_t elems_count, s_ksargv_elems_status* f_argvs)
{
    static int argv_index = 0;      // find result index from argv, static val good for performance

    for (int index = argv_index; index < elems_count; index++)
    {
        if(f_argvs[index].select == true)
            continue;

        for (int args_index = 0; elems[index].option[args_index] != NULL; args_index++)
            if (strcmp(argv, elems[index].option[args_index]) == 0)
            {
                argv_index = index;
                f_argvs[index].select = true;
                return &elems[index];
            }
    }
    
    for (int index = 0; index < argv_index; index++)
    {
        if(f_argvs[index].select == true)
            continue;

        for (int args_index = 0; elems[index].option[args_index] != NULL; args_index++)
            if (strcmp(argv, elems[index].option[args_index]) == 0)
            {
                argv_index = index;
                f_argvs[index].select = true;
                return &elems[index];
            }
    }

    return NULL;
}

void free_vals(s_options* opt)
{
    if (opt->values == NULL)
        return;

    opt->values = NULL;
    
    s_values* val = opt->values;
    while (val != NULL)
    {
        s_values* val_buff = val;
        val = val->next;
        KSARGV_FREE(val_buff);
    }
}

void parse_vals(s_ksargv_elems* elem, s_options* option)
{
    int args_count = get_elem_argv_count(elem->args);

    s_ksargv_value* values = KSARGV_MALLOC(sizeof(s_ksargv_value) * args_count);
    s_values* val = option->values;

    /* loop, convert argv */
    for(int arg_index = 0; arg_index < args_count; arg_index++)
    {
        if(elem->args[arg_index] != ARGV_BOOL && val == NULL)
        {
            KSARGV_FREE(values);
            elem->function(elem->args, NULL, 0, ARGV_ERRO_LESS_ARGS);
            KSARGV_LOG_ERRO("parse args less");
            errorno = ERESTART;
            return;
        }
        
        bool res = true;
        values[arg_index].type = elem->args[arg_index];
        switch(elem->args[arg_index])
        {
            case ARGV_STRING:
                values[arg_index].value.str = val->val;
                val = val->next;
                break;

            case ARGV_INT:
                values[arg_index].value.num_i = argv_get_int(val->val, &res);
                val = val->next;
                break;

            case ARGV_BOOL:
                values[arg_index].value.num_b = argv_get_bool(val == NULL ? NULL : val->val, &res);
                if (val != NULL)
                    val = val->next;
                break;

            case ARGV_DOUBLE:
                values[arg_index].value.num_d = argv_get_double(val->val, &res);
                val = val->next;
                break;

            default:
                break;
        }
    }

    /* done, send */
    switch (elem->parse_tpye)
    {
    case ATGV_PARSE_FUNC:
        elem->function(elem->args, values, args_count, ARGV_ERRO_NONE);
        break;

    case ATGV_PARSE_VALS:
        for (int val_index = 0; val_index < args_count; val_index++)
        {
            switch(elem->args[val_index])
            {
                case ARGV_STRING:
                    sprintf(elem->vals[val_index], values[val_index].value.str);
                    break;

                case ARGV_INT:
                    *(int* )elem->vals[val_index] = values[val_index].value.num_i;
                    break;

                case ARGV_BOOL:
                    *(bool* )elem->vals[val_index] = values[val_index].value.num_b;
                    break;

                case ARGV_DOUBLE:
                    *(double* )elem->vals[val_index] = values[val_index].value.num_d;
                    break;

                default:
                    break;
            }
        }  
        break;
    
    default:
        break;
    }

    KSARGV_FREE(values);
}

int ksargv_parse_argv(char** argv, s_ksargv_elems* elems, size_t elems_count)
{
    if(argv[0] == NULL || argv[1] == NULL || elems_count == 0)
        return 0;
    
    s_ksargv_elems_status* f_argvs = KSARGV_MALLOC(sizeof(s_ksargv_elems_status) * elems_count);
    
    if(f_argvs == NULL)
    {
        errorno = ENOMEM;
        return -1;
    }

    s_options* opts = argc_get_options(&argv[1]);

    if (opts == NULL)
        return -1;

    while (opts != NULL)
    {
        s_ksargv_elems* elem = get_opt_elem(opts->argv, elems, elems_count, f_argvs);

        if (elem != NULL)
            parse_vals(elem, opts);
        
        s_options* opts_buff = opts->next;
        free_vals(opts);
        KSARGV_FREE(opts);
        opts = opts_buff;
    }

    return 0;
}
