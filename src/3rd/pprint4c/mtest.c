#include "pprint.h"

struct person {
  int age;
  int height;
  struct like {
    char *type;
    char *name;
  } like;
};

typedef enum enum_t {
  A = 1 << 0,
  B = 1 << 1,
  C = 1 << 2,
  D = 1 << 3,
} enum_t;

typedef struct sub2_t {
  char *msg;
} sub2_t;

typedef struct sub_t {
  int sub_dat1;
  int sub_dat2;
  int sub_dat3;
  sub2_t *psub2a;
  sub2_t *psub2b;
} sub_t;

typedef struct data_t {
  int data_int;
  char c;
  char *str;
  void *addr;
  enum_t e;
  sub_t sub;
  char *text;
  sub_t *psub;
} data_t;

int main() {
  char out[10240];
  char sub_out[10240];

  struct person johndoe = {
    .age = 6,
    .like = {
      .type =  "Software-Developing",
      .name = "C"
    }
  };

  structInfo person_info[] = {
    {
      .data = &johndoe,
      .structName = "person",
      .level = 0 /* the first must be level 0 */
    },
  };
  pprint_struct("johndoe", person_info, ARRAY_SIZE(person_info));

  int hoge = 10;
  char *str = "abcd";
  struct sub2_t sub2 = {
    .msg = "abcdefghijklmnopqrstuvwxyz"
    "abcdefghijklmnopqrstuvwxyz"
    "abcdefghijklmnopqrstuvwxyz",
  };
  struct sub_t sub = {
    .sub_dat1 = 1,
    .sub_dat2 = 2,
    .sub_dat3 = 3,
    .psub2b   = &sub2,
  };
  struct data_t d = {
    .data_int = 1,
    .c        = 'x',
    .str      = str,
    .addr     = &hoge,
    .e        = A | C | D,
    .sub = {
      .sub_dat1 = 123,
      .sub_dat2 = -1,
      .sub_dat3 = 0xa,
    },
    .text = "hogehoge",
    .psub = &sub,
  };

  structInfo data_info[] = {
    {
      .data = &d,
      .structName = "data_t",
      .level = 0 /* the first must be level 0 */
    },
    {
      .data = &hoge,
      .func = pprint_int,
      .level = 1,
    },
    {
      .data = &sub,
      .structName = "sub_t",
      .level = 1
    },
    {
      .data = &sub2,
      .structName = "sub2_t",
      .level = 2
    },
  };
  pprint_struct("d", data_info, ARRAY_SIZE(data_info));

  return 0;
}

/* vim: set ts=2 sw=2 expandtab: */
