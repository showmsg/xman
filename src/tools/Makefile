## ==> make -f Makefile.sample

BASE_INCLUDE = ../../include/base -I../../include/core -I../3rd/libxml

LIB_DIR   = ../../libs

BIN_DIR = ../../sbin

#CC_CFLAGS = -g -Wall -O2 -Os
CC_CFLAGS = -g -Wall -O2

CC_XMAN_LFLAGS = -lm -lxshm -lxmans -lpthread

all: xhorder xbskbuy xptest xclean

xhorder:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE) XHandOrder.c  -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/


xptest:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XPtest.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xclean:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XCleanData.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.
	
xbskbuy:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE) XBskBatch.c  -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/

clean:
	@rm -fr *.o *.dSYM
