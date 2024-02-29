## ==> make -f Makefile.sample

BASE_INCLUDE = ../../include/base -I../../include/core  -I../../include/strategy

LIB_DIR   = ../../libs

BIN_DIR = ../../bin

#CC_CFLAGS = -g -Wall -O2 -Os
CC_CFLAGS = -g -Wall -Wformat -finline

CC_XMAN_LFLAGS = -lm -lrt -lpthread  -lxmans 

 
all: xload xbasket xpurch  xauction  xbasket3 xbond xauction2

xbond:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XBond.c XBond2.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -D__TEST__ -o $@
	mv $@ $(BIN_DIR)/.

xauction:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XCallAuction.c  -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -D__TEST__ -o $@
	mv $@ $(BIN_DIR)/.

xauction2:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XCallAuction2.c  -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -D__TEST__ -o $@
	mv $@ $(BIN_DIR)/.
	
xload:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XLoadStrategy.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xbasket:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE) XBasket.c XBasket2.c  -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xbasket3:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE) XBasket.c XBasket3.c  -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xbondlnk:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XBondLnk.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xbondmnt:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XBondMnt.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xfundt:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XFundT0.c -D__BTEST__ -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

xpurch:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XPurchase.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.
xmarket:
	@echo -e "Compile \e[32;1m$@\033[0m..."
	gcc $(CC_CFLAGS) -I$(BASE_INCLUDE)  XMarket.c -L$(LIB_DIR) $(CC_XMAN_LFLAGS) -o $@
	mv $@ $(BIN_DIR)/.

clean:
	@rm -fr *.o *.dSYM
