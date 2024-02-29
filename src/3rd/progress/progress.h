#ifndef CLIB_PROGRESS_H
#define CLIB_PROGRESS_H 1

#define _GNU_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

/*
 * for macOS
 */
#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/**
 * `progress.c` version
 */
#define PROGRESS_VERSION 0.0.4
#define PROGRESS_MAX_LISTENER_POOL 32

struct progress_data;

typedef enum progress_event_types {
    PROGRESS_EVENT_START
  , PROGRESS_EVENT_PROGRESS
  , PROGRESS_EVENT_END
} progress_event_type_t;


typedef struct progress_event {
  progress_event_type_t type;
} progress_event_t;

typedef void (* progress_cb_t) (struct progress_data *data);

typedef struct progress_event_listener {
  progress_cb_t handle;
  progress_event_t *event;
  struct progress_data *data;
} progress_event_listener_t;

typedef struct progress {
  int value;
  int total;
  int listener_count;
  double elapsed;
  size_t width;
  struct timespec start;
  bool started;
  bool finished;
  char *bar_char;
  char *bg_bar_char;
  char *fmt;
  progress_event_listener_t listeners[PROGRESS_MAX_LISTENER_POOL];
} progress_t;


typedef struct progress_data {
  progress_t *holder;
  int value;
  int ts;
} progress_data_t;

progress_t *
progress_new (int total, size_t width);

progress_event_t *
progress_event_new(progress_event_type_t type);

void
progress_event_free (progress_event_t *event);

progress_event_listener_t *
progress_event_listener_new (progress_event_t *event, progress_cb_t cb);

void
progress_event_listener_free (progress_event_listener_t *listener);

progress_data_t *
progress_data_new (progress_t *holder, int value);

void
progress_data_free (progress_data_t *data);

bool
progress_on (progress_t *progress, progress_event_type_t event, progress_cb_t cb);

bool
progress_emit (progress_t *progress, progress_event_t *event, progress_data_t *data);

bool
progress_change_value (progress_t *progress, int value, bool increment);


bool
progress_value (progress_t *progress, int value);

bool
progress_tick (progress_t *progress, int value);

void
progress_write (progress_t *progress);

void
progress_free (progress_t *progress);

void
progress_inspect (progress_t *progress);


#endif
