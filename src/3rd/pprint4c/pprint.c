#include <stdio.h> /* sprintf */
#include <stdlib.h> /* calloc, system */
#include <string.h> /* strcmp */
#include <sys/types.h>
#include <unistd.h> /* getpid, unlink */

#include "pprint.h"

extern const char *__progname;

static char *indent(char *buf, int buf_size, const char *msg, int level) {
  memset(buf, 0, buf_size);
  for (int i = 0; i < level; i++) {
    strcat(buf, "  ");
  }
  strcat(buf, msg);
  return buf;
}

/* in-place replace */
static void str_replace(char *target, const char *needle,
                        const char *replacement) {
  char buffer[1024] = { 0 };
  char *insert_point = &buffer[0];
  const char *tmp = target;
  size_t needle_len = strlen(needle);
  size_t repl_len = strlen(replacement);

  while (1) {
    const char *p = strstr(tmp, needle);

    /* walked past last occurrence of needle; copy remaining part */
    if (p == NULL) {
      strcpy(insert_point, tmp);
      break;
    }

    /* copy part before needle */
    memcpy(insert_point, tmp, p - tmp);
    insert_point += p - tmp;

    /* copy replacement string */
    memcpy(insert_point, replacement, repl_len);
    insert_point += repl_len;

    /* adjust pointers, move on */
    tmp = p + needle_len;
  }

  /* write altered string back to target */
  strcpy(target, buffer);
}

void pprint_struct(char *varName, structInfo info[], int info_size) {
  int i = 0;
  char command[1024];
  char gdbcmds[64];
  char structdump[64];

  char linebuf[4096];
  char tmpbuf[4096];
  char tmp[128];

  char result[20480];
  char tmp_result[20480];

  if (info_size <= 0) {
    return;
  }

  if (info[0].level != 0) {
    printf("First record's level must be zero.\n");
    return;
  }

  pid_t pid = getpid();

  sprintf(gdbcmds, "/tmp/gdbcmds.%d", pid);
  sprintf(structdump, "/tmp/struct.dump.%d", pid);

  memset(command, 0x00, sizeof(command));
  strcpy(command,
         "echo 'set print array on\nset print array-indexes on\nset print elements 0\nset print pretty on\n"); 
  for (i = 0; i < info_size; i++) {
    if (info[i].structName == NULL || info[i].structName[0] == '\0') {
      sprintf(command + strlen(command), "p *%p\n", info[i].data);
    } else {
      sprintf(command + strlen(command), "p (struct %s)*%p\n", info[i].structName,
              info[i].data);
    }
  }
  sprintf(command + strlen(command), "'> %s", gdbcmds);
  system(command);

  sprintf(command, "echo 'where\ndetach' | gdb -batch --command=%s %s %d > %s",
          gdbcmds, __progname, pid, structdump);
  system(command);

  /* remove the first line and replace '$1' with instanceName, then cat the file */
  sprintf(command, "sed -i -e 's/\\$1/%s/' -e '1d' %s; cat %s", varName,
          structdump, structdump);

  memset(result, 0x00, sizeof(result));  /* for top level struct result */
  memset(tmp_result, 0x00, sizeof(tmp_result)); /* for all other struct results */

  int gotFirstResult = 0;
  FILE *fp = popen(command, "r");
  if (fp != NULL) {
    while (fgets(linebuf, sizeof(linebuf) - 1, fp) != NULL) {
      if (!gotFirstResult) {
        sprintf(result + strlen(result), "%s", linebuf);
      } else {
        sprintf(tmp_result + strlen(tmp_result), "%s", linebuf);
      }

      if (strcmp(linebuf, "}\n") == 0) {
        gotFirstResult = 1;
      }
    }
  }
  pclose(fp);

  unlink(gdbcmds);
  unlink(structdump);

#ifdef DEBUG
  printf("result=[%s]\n", result);
#endif

  if (info_size == 1) {
    printf("%s\n", result);
    return;
  }

  /* replace '$2', '$3', ... to nothing. */
  for (i = 1; i < info_size; i++) {
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp, "$%d = ", i + 1);
    str_replace(tmp_result, tmp, " = ");
  }

#ifdef DEBUG
  printf("tmp_result=[%s]\n", tmp_result);
#endif

  i = 1;
  int k = 0 ;
  char str[info_size][20480]; /* note: str[0] is not used */
  memset(str, 0x00, sizeof(str));
  char *temp = str[i];
  char *p = strtok(tmp_result, "\n");
  do {
    if (info[i].structName == NULL || info[i].structName[0] == '\0') {
      sprintf(temp + strlen(temp), "%s\n", p);
      i++;
      k = 0;
      temp = str[i];
    } else {
      if (k == 0) { /* do not indent the first line */
        sprintf(temp + strlen(temp), "%s\n", p);
      } else {
        sprintf(temp + strlen(temp), "%s\n", indent(tmpbuf, sizeof(tmpbuf), p,
              info[i].level));
      }
      k++;
      if (strcmp(p, "}") == 0) {
        i++;
        k = 0;
        temp = str[i];
      }
    }
  } while ((p = strtok(NULL, "\n")) != NULL);

#ifdef DEBUG
  for (i = 1; i < info_size; i++) {
    printf("%d: %s\n", i, str[i]);
  }
#endif

  for (i = 1; i < info_size; i++) {
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(tmp, " = %p", info[i].data);
    /* remove the last '\n' if has */
    if (str[i][strlen(str[i]) - 1] = '\n') {
      str[i][strlen(str[i]) - 1] = '\0';
    }

    if (info[i].structName == NULL || info[i].structName[0] == '\0') { /* normal variable */
      memset(linebuf, 0x00, sizeof(linebuf));
      memset(tmpbuf, 0x00, sizeof(tmpbuf));

      sprintf(linebuf, " = %p %s", info[i].data, info[i].func(tmpbuf, sizeof(tmpbuf), info[i].data));
      str_replace(result, tmp, linebuf);
    } else { /* structur */
      str_replace(result, tmp, str[i]);
    }
  }

  printf("\n%s\n", result);

  return;
}

/* utilities functions */
char *pprint_int(char* buf, int buf_size, void *ptr) {
  int i = *(int *)ptr;
  snprintf(buf, buf_size, "%d", i);
  return buf;
}

char *pprint_long(char* buf, int buf_size, void *ptr) {
  long l = *(long *)ptr;
  snprintf(buf, buf_size, "%ld", l);
  return buf;
}

char *pprint_float(char* buf, int buf_size, void *ptr) {
  float f = *(float *)ptr;
  snprintf(buf, buf_size, "%f", f);
  return buf;
}

char *pprint_double(char* buf, int buf_size, void *ptr) {
  double d = *(double *)ptr;
  snprintf(buf, buf_size, "%lf", d);
  return buf;
}


/* vim: set ts=2 sw=2 expandtab: */
