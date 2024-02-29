//
// throw.h
// copyright (c) 2013 joseph werle <joseph.werle@gmail.com>
//


#ifndef THROW_H
#define THROW_H 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define QUOTE_X(t)#t
#define QUOTE(t)QUOTE_X(t)

#define ERROR_NEW_CONSTANT(c) error_new(c, QUOTE_X(#c))


typedef struct {
  int code;
  char *name;
} error_t;


FILE *THROW_FD = NULL;

void
throw (void *type, char *msg);

error_t
error_new (int code, char *name);


error_t
error_new (int code, char *name) {
  error_t err;
  err.code = code;
  err.name = name;
  return err;
}


void
throw (void *type, char *msg) {
  error_t *err = (error_t *) type;
  // default to `stderr` stream
  if (NULL == THROW_FD) {
    THROW_FD = stderr;
  }

  fprintf(THROW_FD, "%s (%d): %s\n", err->name, err->code, msg);
}


#endif
