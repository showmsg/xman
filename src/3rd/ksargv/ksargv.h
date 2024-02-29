#ifndef __KSARGV_H__
#define __KSARGV_H__

#include <stdlib.h>
#include <stdbool.h>

#define KSARGV_LOG_DEBUG(fmt, args...)          (printf("\t%s,\t%s,%d >> "fmt"\n", __FILE__, __FUNCTION__, __LINE__, ##args))
#define KSARGV_LOG_ERRO(fmt, args...)           (printf("\t%s,\t%s,%d >> "fmt"\n", __FILE__, __FUNCTION__, __LINE__, ##args))   
// #define DBG_MALLOC                          (1)

#ifdef DBG_MALLOC
    #define KSARGV_MALLOC(argument)                dbg_malloc(argument, __FILE__, __FUNCTION__, __LINE__)
    #define KSARGV_FREE(argument)                  dbg_free(argument, __FILE__, __FUNCTION__, __LINE__)
    #define KSARGV_REALLOC(argument1,argument2)    dbg_realloc(argument1,argument2, __FILE__, __FUNCTION__, __LINE__)
#else
    #define KSARGV_MALLOC(size)             malloc(size)
    #define KSARGV_REALLOC(mem, size)       realloc(mem, size)
    #define KSARGV_FREE(mem)                free(mem)
#endif

typedef enum
{
    ARGV_ERRO_NONE,                 // not erro
    ARGV_ERRO_LESS_ARGS,            // less args in cmd
    ARGV_ERRO_PARSE,
    ARGV_ERRO,                      // some erro can not describe easy
} e_argv_erro;

typedef enum
{
    ARGV_STRING,                    // arg is string
    ARGV_INT,                       // int
    ARGV_BOOL,                      // arg can be "true" or "false"
    ARGV_DOUBLE,                    // double
    ARGV_END,                       // must pust it in end <e_argv_type> array end
} e_argv_type;

typedef enum
{
    ATGV_PARSE_FUNC,                // parse result in function
    ATGV_PARSE_VALS,                // parse result in vals
    ATGV_PARSE_NULL,
} e_argv_parse_type;

typedef struct s_ksargv_value
{
    e_argv_type type;               // arg from cmd type

    union u_ksargv_value            // look it base on type
    {
        char* str;
        int num_i;
        double num_d;
        bool num_b;
    } value;
} s_ksargv_value;

/**
 * argv from <iksargv_parse_argv> argv
 * values is array store values, if something erro, it will be NULL, and values_count will be zero
 * values_count values array count, if something erro, it will be NULL, and values_count will be zero
 * erro store why this function is erro, if there is no erro, it will be set ARGV_ERRO_NONE, when
 *  it be set ARGV_ERRO, mybe you should check ksargv source code or connect coder
*/
typedef void (*f_ksargv_function)(e_argv_type* argv, s_ksargv_value* values, unsigned int values_count, e_argv_erro erro);

/**
 * option your command name string array, it must set the last string NULL
 * args is array, it store what kind of cmd you want store
 * function will be doing if this option type in cmd
*/
typedef struct
{
    char**              option;
    e_argv_type*        args;

    e_argv_parse_type   parse_tpye;
    void**              vals;
    f_ksargv_function   function;
} s_ksargv_elems;

/**
 * just for dbg, it will help you check out of mem
 * if you want do it pleast use macro DBG_MALLOC
*/
extern void* dbg_malloc(size_t size, const char* file, const char* func, int line);
extern void* dbg_realloc(void* mem, size_t size, const char* file, const char* func, int line);
extern void dbg_free(void* mem, const char* file, const char* func, int line);
extern void dbg_print_mem(void);

/**
 * the slow way to parse argv, it base on traversal
 *
 * argv from main(***, ***)
 * elems is array, you have to package your cmd orignaztion
 * elems_count discribe the elems array elements count
*/
extern int ksargv_parse_argv(char** argv, s_ksargv_elems* elems, size_t elems_count);

#endif
