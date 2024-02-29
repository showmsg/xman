## ==> make -f Makefile.sample

BASE_INCLUDE = ../../include/base -I../../include/core -I../../include/counter/ctp -I../../include/xman/oes -I../3rd/libxml -I../../include/counter/ctp/transfer

LIB_DIR   = ../../libs/dll

BIN_DIR = ../../bin

#CC_CFLAGS = -g -Wall -O2
CC_CFLAGS = -g  -Wall -Wformat -finline -O2 -D__LATENCY__

CC_XMAN_LFLAGS = -lm -lrt -lpthread  -llev2mdapi -lxman -lxshm -lezxml -lcjson -lfasttraderapi

SRCS = CtpInit.cpp CtpMkt.cpp CtpTrd.cpp
OBJS = $(patsubst %.c,%.o,$(SRCS)) 

all: libxctp.so clean

$(OBJS):$(SRCS)
	g++ -shared -fPIC $(CC_CFLAGS) -I$(BASE_INCLUDE) $(SRCS) -c $^
libxctp.a:$(OBJS)
	@echo -e "Compile \e[32;1m$@\033[0m..."
	ar rc $@ $^
	ranlib $@
	mv $@ $(LIB_DIR)/.
libxctp.so:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	g++ -shared -fPIC $(CC_CFLAGS) -I$(BASE_INCLUDE) $(SRCS) -L$(LIB_DIR) $(CC_XMAN_LFAGS) -o $@
	mv  $@ $(LIB_DIR)/.
		
clean:
	@rm -fr *.o *.dSYM
