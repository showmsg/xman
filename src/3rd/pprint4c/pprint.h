#ifndef _PPRINT_H_
#define _PPRINT_H_

typedef char* (*pprint_value_func)(char* buf, int buf_size, void* ptr);

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*(array)))

typedef struct structInfo structInfo;
struct structInfo {
  void *data;             /* address of struct/variable */
  pprint_value_func func; /* callback function for normal variable */
  char *structName;       /* struct name. if null or empty, then it is a normal variable */
  int level;              /* 0:top level struct, 1: first level struct or normal variable, ... */
};

/* Entry point of the API */
extern void pprint_struct(char *varName, structInfo info[], int size);

/* value callback printing functions */
extern char *pprint_int   (char* buf, int buf_size, void *ptr);
extern char *pprint_long  (char* buf, int buf_size, void *ptr);
extern char *pprint_float (char* buf, int buf_size, void *ptr);
extern char *pprint_double(char* buf, int buf_size, void *ptr);

#endif

/* vim: set ts=2 sw=2 expandtab: */
