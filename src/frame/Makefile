## ==> make -f Makefile.sample

BASE_INCLUDE = ../../include/base -I../../include/core -I../../include/counter/oes -I../../include/counter/oes/transfer -I../../include/frame -I../3rd/libxml -I../../include/counter/ctp/transfer

LIB_DIR   = ../../libs

BIN_DIR = ../../bin

SBIN_DIR = ../../sbin

#CC_CFLAGS = -g -Wall -Wformat -finline -O2 -D__LATENCY__
CC_CFLAGS = -g -Wall -Wformat -finline
#CC_CFLAGS = -g -Wall -Wformat -finline -ftree-loop-linear -funroll-loops -fprefetch-loop-arrays -Wa,-R
CC_XMAN_LFLAGS = -lm -lrt -lpthread  -lxmans -lxoes  -loes_api  -lezxml 


all: xhisday xman xbtest xbtplus xmplus xmanc xclean

xman:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE) XMan.c  XExport.c XFastRebuild.c XShFastRebuild.c XSzFastRebuild.c  XStore.c XOrdeng.c XSignals.c -D__XMAN_FAST_REBUILD__ -D__HAS_FRONT__ -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xmanc:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE) XMan.c  XExport.c XFastRebuild.c XShFastRebuild.c XSzFastRebuild.c  XStore.c XOrdeng.c XSignals.c  -D__XMAN_FAST_REBUILD__  -D__USED_NCURSES__ -L$(LIB_DIR) -lncursesw  $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.
xmplus:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE) XMan.c  XExport.c XShRebuild.c XSzRebuild.c XRebuild.c  XStore.c XOrdeng.c XSignals.c   -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xhisday:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XHisDay.c  XExport.c  -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(SBIN_DIR)/.

xbtest:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XBackTest.c  XExport.c  XFastRebuild.c XShFastRebuild.c XSzFastRebuild.c XStore.c  XOrdeng.c  XSignals.c -D__XMAN_FAST_REBUILD__ -D__BTEST__  -D__HAS_FRONT__  -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(SBIN_DIR)/.
	
xbtplus:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XBackTest.c  XExport.c XSzRebuild.c XShRebuild.c XStore.c  XOrdeng.c XRebuild.c XSignals.c  -D__BTEST__   -D__USED_NCURSES__ -D__HAS_FRONT__ -D__STORE_RESNAP__  -L$(LIB_DIR) -lncursesw $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(SBIN_DIR)/.

test:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE) Test.c  XExport.c  -D__XMAN_FAST_REBUILD__ -D__HAS_FRONT__ -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.
	
xclean:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XCleanData.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(SBIN_DIR)/.
	
clean:
	@rm -fr *.o *.dSYM
