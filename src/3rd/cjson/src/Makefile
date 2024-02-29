## ==> make -f Makefile.sample

BASE_INCLUDE = ../include

LIB_DIR   = ../../../../libs

#CC_CFLAGS = -g -Wall -O2 -Os
CC_CFLAGS = -g -Wall

CC_XMAN_LFLAGS = -lm -lpthread

SRCS = cJSON.c cJSON_Utils.c s2j.c

OBJS = $(patsubst %.c,%.o,$(SRCS)) 

all: libcjson.a libcjson.so clean

$(OBJS):$(SRCS)
	gcc -shared -fPIC $(CC_CFLAGS) -I$(BASE_INCLUDE) $(SRCS) -c $^
libcjson.a:$(OBJS)
	@echo -e "Compile \e[32;1m$@\033[0m..."
	ar rc $@ $^
	mv $@ $(LIB_DIR)/.
libcjson.so:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc -shared -fPIC $(CC_CFLAGS) -I$(BASE_INCLUDE) $(SRCS) -L$(LIB_DIR) $(CC_XMAN_LFAGS) -o $@
	mv  $@ $(LIB_DIR)/dll/.
clean:
	@rm -fr *.o *.dSYM
