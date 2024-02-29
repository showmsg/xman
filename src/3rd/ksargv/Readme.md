# ksargv

V0.0.3.20210623

------

## 项目简介

> 解析 c语言中的 argv参数
>
> 示例：./template -version -sfa -ip 192.168.131 886 -g true -s 
> 示例：./template_vals --host 127.0.0.1 8899 -s

## 文件目录

>├── tools 				// git astyle tools

## 安装说明

* 直接将ksargv.c ksargv.h 复制到用户程序目录中即可
* 可以使用 make 编译测试用例

```shell
[kira@kira-virtual-machine ksargv]$ make
gcc -Wall -g -O0 -std=c99   -c -o template.o template.c
gcc -Wall -g -O0 -std=c99   -c -o ksargv.o ksargv.c
gcc -Wall -g -O0 -std=c99 -o template template.o ksargv.o
[kira@kira-virtual-machine ksargv]$ 
```

## 使用说明

* 解析命令行参数，详见测试用例 template.c，效果如下

```shell
[kira@kira-virtual-machine ksargv]$ ./template -version -sfa -ip 192.168.131 886 -g true -s 6.6 -k
version: -sfa
ip = 192.168.131
port = 886
greate, thank you
thanks your grade 6.600000
thanks you, i will keep that
[kira@kira-virtual-machine ksargv]$ 
```

```
[niuwanli@swcentos7 ksargv]$ ./template_vals --host 192.168.1.1 -s
	ksargv.c,	argc_get_options,134 >> new options: --host
	ksargv.c,	argc_get_options,153 >> new values: --host:192.168.1.1
	ksargv.c,	argc_get_options,134 >> new options: -s
erro
host = unknown
port = 65536
start = 1
```

## 维护说明

## 注意

## 关于作者

>Autho: KiraSkyler
>Email: kiraskyler@outlook.com / kiraskyler@qq.com

## 贡献者/贡献组织

## 鸣谢

## 版权信息

> 该项目签署了GPL 授权许可，详情请参阅官网

>This program is free software: you can redistribute it and/or modify
>it under the terms of the GNU General Public License as published by
>the Free Software Foundation, either version 3 of the License, or
>(at your option) any later version.
>This program is distributed in the hope that it will be useful,
>but WITHOUT ANY WARRANTY; without even the implied warranty of
>MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
>GNU General Public License for more details.
>You should have received a copy of the GNU General Public License
>along with this program.  If not, see <https://www.gnu.org/licenses/>.

## 更新日志

* V0.0.3.20210623
  * 初始化项目，初步测试，已测试内容包括所有的支持解析项目，内存溢出等