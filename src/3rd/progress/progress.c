#include "../progress/progress.h"

/**
 * Implementation
 */


#if __STDC_VERSION__ >= 199901L
#define C99
char *
strdup (const char *str) {
  char *cpy = malloc(strlen(str)+1);

  if (cpy) {
    strcpy(cpy, str);
  }

  return cpy;
}
#endif

/*
 * Get current Unix timestamp with timespec format
 */
static void 
timestamp(struct timespec *ts) {
#ifdef __MACH__
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, ts);
#endif
}

/*
 * Return the difference of two timespec (millisecond)
 */
static double 
timespec_diff(struct timespec start, struct timespec end) {
  double d_start = 1000 * ((double)start.tv_sec + 1.0e-9 * start.tv_nsec);
  double d_end = 1000 * ((double)end.tv_sec + 1.0e-9 * end.tv_nsec);
  return d_start - d_end;
}

static char *
replace_str (char *strbuf, char *strold, char *strnew) {
  char *strret, *p = NULL;
  char *posnews, *posold;

  if (!strbuf) return NULL;
  if (!strold || !strnew || !(p = strstr(strbuf, strold)))
     return strdup(strbuf);

  size_t szold = strlen(strold);
  size_t sznew = strlen(strnew);
  size_t n = 1;

  

  while (n > 0) {
    if (!(p = strstr(p+1, strold))) break;
    else n++;
  }

  strret = (char *) malloc(strlen(strbuf) - (n * szold )+ (n * sznew) + 1);
  p = strstr(strbuf, strold);
  strncpy(strret, strbuf, (p-strbuf));
  strret[p-strbuf] = 0;
  posold = p+szold;
  posnews = strret+(p-strbuf);
  strcpy(posnews, strnew);
  posnews += sznew;

  while (n > 0) {
    if (!(p = strstr(posold, strold)))
      break;
    strncpy(posnews, posold, p-posold);
    posnews[p-posold] = 0;
    posnews += (p-posold);
    strcpy(posnews, strnew);
    posnews += sznew;
    posold = p+szold;
  }

  strcpy(posnews, posold);
  free(strbuf);
  return strret;
}

progress_t *
progress_new (int total, size_t width) {
  progress_t *progress = malloc(sizeof(progress_t));
  if (!progress) return NULL;
  progress->started = false;
  progress->finished = false;
  progress->value = 0;
  progress->elapsed = 0;
  progress->listener_count = 0;
  progress->bar_char = "=";
  progress->bg_bar_char = "-";
  progress->fmt = "";
  progress->total = total;
  progress->width = (width)? (int) width : total;
  return progress;
}

progress_event_t *
progress_event_new (progress_event_type_t type) {
  progress_event_t *event = malloc(sizeof(progress_event_t));
  if (!event) return NULL;
  event->type = type;
  return event;
}

void
progress_event_free (progress_event_t *event) {
  if (event) free(event);
}

progress_event_listener_t *
progress_event_listener_new (progress_event_t *event, progress_cb_t cb) {
  progress_event_listener_t *listener = malloc(sizeof(progress_event_listener_t));
  if (!listener) return NULL;
  listener->event = event;
  listener->handle = cb;
  return listener;
}

void
progress_event_listener_free (progress_event_listener_t *listener) {
  if (!listener) return;
  if (listener->data) progress_data_free(listener->data);
  //if (listener->event) progress_event_free(listener->event);
  //free(listener);
}

progress_data_t *
progress_data_new (progress_t *holder, int value) {
  progress_data_t *data = malloc(sizeof(progress_data_t));
  data->holder = holder;
  data->value = value;
  data->ts = (int) time(NULL);
  return data;
}

void
progress_data_free (progress_data_t *data) {
 // if (data) free(data);
}

bool
progress_on (progress_t *progress, progress_event_type_t event, progress_cb_t cb) {
  progress_event_t * ev = progress_event_new(event);
  progress_event_listener_t *listener = progress_event_listener_new(ev, cb);
  if (!listener) return false;
  memcpy(&progress->listeners[progress->listener_count++], listener, sizeof(progress_event_listener_t));
  free(listener);
  return true;
}

bool
progress_emit (progress_t *progress, progress_event_t *event, progress_data_t *data) {
  int count = progress->listener_count;
  int i;
  for (i = 0; i < count; ++i) {
    progress_event_listener_t *listener = &progress->listeners[i];
    if (event->type == listener->event->type) {
      listener->data = data;
      listener->handle(data);
    }
  }

  return false;
}

bool
progress_tick (progress_t *progress, int value) {
  return progress_change_value(progress, value, true);
}

bool
progress_value (progress_t *progress, int value) {
  return progress_change_value(progress, value, false);
}

bool
progress_change_value (progress_t *progress, int value, bool increment) {
  if (progress->finished) return false;
  if(increment == true) { 
    progress->value += value;
  } else {
    progress->value=value;
  }

  struct timespec ts_now;
  timestamp(&ts_now);
  
  if (progress->value > progress->total)
    progress->value = progress->total;

  if (!progress->started) {
    progress->started = true;
    struct timespec ts_start;
    timestamp(&ts_start);
    progress->start = ts_start;
  

    progress_event_t *event = progress_event_new(PROGRESS_EVENT_START);
    progress_data_t *data = progress_data_new(progress, value);
    progress_emit(progress, event, data);
    
    free(event);
    free(data);
  }

  progress->elapsed = timespec_diff(ts_now, progress->start);  
  progress_event_t *event = progress_event_new(PROGRESS_EVENT_PROGRESS);
  progress_data_t *data = progress_data_new(progress, value);
  progress_emit(progress, event, data);

  free(event);
  free(data);


  if (progress->value >= progress->total) {
    progress->finished = true;
    progress->value = progress->total;
    progress_event_t *event = progress_event_new(PROGRESS_EVENT_END);
    progress_data_t *data = progress_data_new(progress, value);
    progress_emit(progress, event, data);
   
    free(event);
    free(data);
  }

  return true;
}

void
progress_write (progress_t *progress) {
  int i = 0;
  int width = (int) progress->width;
  int percent = 100 * ((double) progress->value / (double) progress->total);
  int complete = (width * ((double) progress->value / (double) progress->total));
  int incomplete = width - (complete);
  double elapsed = progress->elapsed;
  char *fmt = malloc(512 * sizeof(char));
  char *bar = malloc((complete + incomplete) * sizeof(char));
  char *percent_str = malloc(sizeof(char)*20);
  char *elapsed_str = malloc(sizeof(char)*20);

  sprintf(percent_str, "%d%%", percent);
  if (elapsed > 1000) {
    sprintf(elapsed_str, "%.2fs", elapsed/1000);
  } else {
    sprintf(elapsed_str, "%.0fms", elapsed);
  }
  

  strcpy(fmt, "");
  strcat(fmt, progress->fmt);
  strcpy(bar, "");

  if (complete) {
    for (i = 0; i < complete; ++i) {
      bar[i] = *progress->bar_char;
    }
  }

  if (incomplete) {
    for (; i < complete + incomplete; ++i) {
       bar[i] = *progress->bg_bar_char;
    }
  }

  bar[i] = '\0';

  fmt = replace_str(fmt, ":bar", bar);
  fmt = replace_str(fmt, ":percent", percent_str);
  fmt = replace_str(fmt, ":elapsed", elapsed_str);

  printf("%c[2K", 27);
  printf("\r%s", fmt);

  fflush(stdout);
  free(bar);
  free(percent_str);
  free(elapsed_str);
  free(fmt);
}

void
progress_free (progress_t *progress) {
  int i;
  for (i = 0; i < progress->listener_count; ++i) {
    progress_event_free(progress->listeners[i].event);

  }

  free(progress);
}

void
progress_inspect (progress_t *progress) {
  printf("\n#progress =>\n");
  printf("    .value: %d\n", progress->value);
  printf("    .total: %d\n", progress->total);
  printf("    .listener_count: %d\n", progress->listener_count);
  printf("    .elapsed: %f\n", progress->elapsed);
  printf("    .start: %ld.%ld\n", (long)progress->start.tv_sec, progress->start.tv_nsec);  
  printf("    .width: %d\n", (int)progress->width);
  printf("    .started: %s\n", progress->started? "true" : "false");
  printf("    .finished: %s\n", progress->finished? "true" : "false");
  printf("    .bar_char: \"%s\"\n", progress->bar_char);
  printf("    .bg_bar_char: \"%s\"\n", progress->bg_bar_char);
  printf("    .fmt: \"%s\"\n", progress->fmt);
  printf("    .listeners[%d]\n", progress->listener_count);
}
