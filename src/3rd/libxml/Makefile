## ==> make -f Makefile.sample

BASE_INCLUDE = ./

LIB_DIR   = ../../../libs

BIN_DIR = ./

#CC_CFLAGS = -g -Wall -O2 -Os
CC_CFLAGS = -Wall -Wextra

CC_XMAN_LFLAGS = 

SRCS = ezxml.c
OBJS = $(patsubst %.c,%.o,$(SRCS)) 

all:libezxml.a libezxml.so clean tests
$(OBJS):$(SRCS)
	gcc -shared -fPIC $(CC_CFLAGS) -I$(BASE_INCLUDE) $(SRCS) -c $^
libezxml.a:$(OBJS)
	@echo -e "Compile \e[32;1m$@\033[0m..."
	ar rc $@ $^
	mv $@ $(LIB_DIR)/.
libezxml.so:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc -shared -fPIC $(CC_CFLAGS) -I$(BASE_INCLUDE) $(SRCS) -L$(LIB_DIR) $(CC_XMAN_LFAGS) -o $@
	mv  $@ $(LIB_DIR)/dll/.
tests:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc  $(CC_CFLAGS) -I$(BASE_INCLUDE) ezxml.c test.c -L$(LIB_DIR) $(CC_XMAN_LFAGS) -o $@
	
clean:
	@rm -fr *.o *.dSYM
