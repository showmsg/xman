CLR_NORMAL="\x1B[0m"
CLR_RED="\x1B[31m"
CLR_GREEN="\x1B[32m"
CLR_YELLOW="\x1B[33m"
CLR_BLUE="\x1B[34m"
CLR_NAGENTA="\x1B[35m"
CLR_CYAN="\x1B[36m"
CLR_WHITE="\x1B[37m"
CLR_RESET="\033[0m"
#################################################################

echo "\n"
echo -e "$CLR_BLUE 3.编译OES......$CLR_RESET"
cd  ../src/oes
make
if [ $? -ne 0 ];then
	echo "3.编译oes柜台错误"
	exit 0
fi

echo "\n"
echo -e "$CLR_BLUE 3.编译CTP......$CLR_RESET"
cd  ../ctp
make
if [ $? -ne 0 ];then
	echo "3.编译ctp柜台错误"
	exit 0
fi

echo "\n"
echo -e "$CLR_BLUE 3.编译交易平台......$CLR_RESET"
cd  ../frame
make
if [ $? -ne 0 ];then
	echo "3.编译oes柜台错误"
	exit 0
fi

echo -e "$CLR_BLUE 4.编译策略......$CLR_RESET"
cd ../strategy
make
if [ $? -ne 0 ];then
	echo "4.编译策略错误"
	exit 0
fi

echo -e "$CLR_BLUE 5.编译辅助工具......$CLR_RESET"
cd ../tools
make
if [ $? -ne 0 ];then
	echo "5.编译工具错误"
	exit 0
fi

cd ../../build


