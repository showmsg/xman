## ==> make -f Makefile.sample

BASE_INCLUDE = ../../include/base -I../../include/core -I../../include/counter/oes -I../../include/counter/oes/transfer -I../3rd/libxml

LIB_DIR   = ../../libs

BIN_DIR = ..

#CC_CFLAGS = -g -Wall -O2
CC_CFLAGS = -g  -Wall -Wformat -finline -O2 -D__LATENCY__

CC_XMAN_LFLAGS = -lm -lpthread  -lxmans  -lezxml

SRCS = Mds2X.c Oes2X.c OesCom.c OesInit.c OesMkt.c OesTrd.c
OBJS = $(patsubst %.c,%.o,$(SRCS)) 

all: libxoes.a libxoes.so clean

$(OBJS):$(SRCS)
	gcc -shared -fPIC $(CC_CFLAGS) -I$(BASE_INCLUDE) $(SRCS) -c $^
libxoes.a:$(OBJS)
	@echo -e "Compile \e[32;1m$@\033[0m..."
	ar rc $@ $^
	mv $@ $(LIB_DIR)/.
libxoes.so:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc -shared -fPIC $(CC_CFLAGS) -I$(BASE_INCLUDE) $(SRCS) -L$(LIB_DIR) $(CC_XMAN_LFAGS) -o $@
	mv  $@ $(LIB_DIR)/dll/.
		
clean:
	@rm -fr *.o *.dSYM
