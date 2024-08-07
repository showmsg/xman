# XMan 量化交易系统使用说明  {#mainpage}

---
## 系统介绍
xman高频量化交易系统，由上海量赢科技有限公司(www.quantin.cn)自主开发，采用基于实时交易领域的全内存交易方案，支持宽睿OES和华鑫CTP,策略编写灵活。
目前提供开源半路板、盘前抢板和盘中抢板等功能。

## 功能说明
1. 自动打新股、可转债、配股配债等;
2. 收盘自动打逆回购;
3. 集合竞价抢板,在9:15分之前尝试下单,9:20之前根据当前封板情况进行撤单;
4. 盘中抢板提供涨停下单、封板下单和排版功能,以及大单撤单、时间撤单和封板后成交量突破撤单等功能;
5. 半路板提供堆量下单、下跌反弹买入、涨速拉升回落下单等功能;


## 环境安装
### 操作系统
需要centos 7.2及以上版本;

### gcc 9.2.0
wget https://down.24kplus.com/linux/gcc/gcc-install.sh
chmod +x gcc-install.sh && ./gcc-install.sh

### 编译
cd build && makefile.sh

### 文档生成
安装doxygen、graphviz生成对应的帮助文档

## 系统架构

### 目录结构

|目录|文件|说明|
|:--:|:--:|:--:|
|bin |    								|    		 |
|    |xman								|框架服务    |
|    |xbasket           				|盘中抢板程序    |
|    |xload								|策略加载服务|
|    |xpurch            				|新股新债申购|
|    |xwebserver                        |对外服务程序,提供基于websocket的协议接口|
|    |xauction                          |集合竞价抢单程序|
|    |xgem                              |半路板|
|    |xsell                             |卖出|
|conf|    								|    		 |
|    |system.conf       				|进程绑核及行情订阅配置|
|    |user.conf         				|用户配置|
|    |mds_client.conf   				|行情配置|
|    |oes_client.conf   				|交易配置|
|    |sdb.conf          				|内存配置|
|data|    								|    		|
|    |plot.conf         				|策略管理|
|    |submkt.csv        				|订阅行情文件,只在system.conf配置项oesmkt.resub为1时生效|
|sbin|    								|    |
|    |xhisday           				|历史行情处理|
|    |xhorder           				|手工报单|
|    |xbtest         					|策略回测|
|logs|    								|    |


## 进程说明
### XMan(框架服务)

	###############################################################
	#    量赢策略交易平台 (2.0.0)  
	# -a [all] 初始化并启动程序
	#                                                             
	# -i[init] 初始化数据及建立内存
	# -m[market] 启动行情接受,需要先init
	# -t[trade] 启动交易处理,需要先init
	# -o[xorden] 启动交易处理,需要先init
	# -r[xrebld] 重建丁单薄,需要先init
	# -n[xrsnap] 生成重建后快照,需要先init
	# -y[xstore] 存储重构好的快照,需要先init
	# -x[xetick] 存储逐笔数据,需要先init
	#                                                             
	# -e[export] 导出行情
	# -p[print] 打印当前系统状态
	#                                                             
	# -s[stop] 停止程序并清理内存
	###############################################################

### xload(策略加载服务)
该工具用于策略加载及修改使用，可以批量暂停篮子策略、批量修改证券代码状态等。

	###############################################################
	#    XMan策略交易平台(2.0.0) 
	# -h [help] 帮助
	# 默认不带参数,加载data\plot.conf策略
	# -t [type]操作类型,-1:默认,0:帮助,1:按证券代码+本地单号修改测量,2:按策略编号修改,3:按证券代码修改所有策略状态,4:按本地单号修改策略状态
	# -c [customer] 客户号
	# -m [market] 市场,1:上海,2:深圳
	# -s [security] 证券代码
	# -b [bstype] 买卖方向,1:买入,2:卖出
	# -l [localid] 策略请求编号,data\plot.conf中的块内容
	# -r [run] 修改的策略状态,1:启动,其它停止
	# -p [plotid] 策略编号

	# -u [localids]篮子标号,data\plot.conf块内容
	# -d [cdl]撤单率，万分比
	# 修改策略【按证券代码(需要输入-c -m -s -b -l -r)|按策略编号(-c -p -r)】
	# 如：
	# 1. 按证券代码启动策略
	# ./xload -t1 -ccustomer -m1 -s600837 -b1 -l3 -r1
	# 2. 按策略编号停止策略
	# ./xload -t2 -ccustomer -p30000487 -r0
	# 3. 按证券代码启动策略
	# ./xload -t3 -m1 -s600837 -r1
	# 4. 按本地单号停止策略状态
	# ./xload -t4 -l3 -r0
	# 5. 按篮子修改正确代码撤单率
	# ./xload -t5 -ccustomer -m1 -s600837 -b1 -u1,2 -d12
	###############################################################

### xhisday(历史行情处理)

	###############################################################
	#    量赢策略交易平台(2.0.0) 
	# -h [help] 帮助
	# -t [2:tsnap2csv(转重构后的快照),3:trade2csv(转交易),4:tick2csv(转逐笔和快照)]
	# -m [market] 市场
	# -s [securityid] 证券代码
	# -c [channel] 频道
	###############################################################

### xhorder(手工报单)

	###############################################################
	#     手动交易 1.0.0 
	# -t [业务模式 0:交易,1:查询行情,2:查资金,3:查持仓
	# -c [客户号]
	# -m [市场 1:上海,2:深圳(默认)] 
	# -s [证券代码 6位] 
	# -b [买卖 1: 买入,2:卖出(默认),3:逆回购卖出, 4:撤单]
	# -p [价格为实际价格*10000的整数]
	# -q [买卖数量]
	# -l [撤单编号]
	###############################################################

### xbtest(策略回测)

	###############################################################
	#    XMan策略交易平台(2.0.0) 
	# -h [help] 帮助
	# -t [1:backtest(回测平台启动),2:运行(设置策略后启动运行)
	# -i [staticfile] 静态文件二进制,回测时指定静态文件
	# -q [mktstorefile] 行情二进制文件，回测时指定
	# -m [market] 市场
	# -s [securityid] 证券代码
	# -c [channel] 频道号
	# -v [volume] 回放多少条暂停对应的时间
	# -k [time] 暂停的时间ms
	###############################################################

## 策略文件说明
### 篮子全局参数
|字段|中文|说明|
|-|-|-|
plotType|策略类型|ConRob:盘中抢板程序,AucRob:盘前抢板,HalfRob:半路板,Sell:卖出方案|
isForbidTrade|是否禁止|0：允许，1禁止|
customer|客户号| |
allowHoldQty|允许最大持有数量| |
slipBuyTimes|买滑点次数|默认3 |
slipSellTimes|卖滑点次数|默认5 |
ordGapTime|委托间隔| |
ctrGapTime|撤单间隔| |
isCtrlStop|撤单后是否停止|0:不停止,其他停止|
isAutoCtrl|是否允许自动撤单|1:自动撤单,0:不自动撤单;只有不自动撤单的情况下，客户端发起的撤单不会再报单 |
isOpenStop|涨停打开后是否继续下单|0:涨停停止,1:涨停不停止 |
isUpperStop|涨停后是否停止下单|0:涨停继续下单,1:涨停停止下单|
beginTime|开始委托时间|HHMMSSsss|
endTime|结束委托时间|HHMMSSsss|

#### 抢板买(xbasket)

| 序号 | 字段 | 中文 | 取值 | 说明 |
|---|---|---|---|---|
|1|market|市场|1：上海，2：深圳| |
|2|securityId|证券代码| |证券代码|
|3|bsType|买卖方向|1：买| |
|4|conPx|信号价格|-1：涨停价,其它实际价格 * 10000| |
|5|ordPx|买卖价格|-1：涨停价,其它实际价格 * 10000,0:跌停价| |
|6|qtyType|委托数量类型|0:数量，1：资金| |
|7|money,qty|委托金额or数量|数量或金额| |
|8|cdl|买一撤单率|0:不控制，其他万分比| |
|9|askQty|卖一未成交量|-1:不控制，其他未成交数量| |
|10|buyMoney|买一封单金额| |买一封单金额，万元|
|11|buyCtrlMoney|买一封单不足撤单|-1:不控制，其他万元| |
|12|isNoneTrade|是否允许交易|0：允许，1：不允许| |
|13|sign|方案标志|0：抢板| |
|14|kpzdf|开盘涨幅|小于-2000不控制| |
|15|upperQtyMulty|无效| | |
|16|upperQtyMultyMin|无效| | |
|17|nxtCtrlMoney|封板后1分钟成交量撤单|>0 有效|万元|
|18|followCtrlMoney|封板后连续2分钟成交量撤单|>0有效|万元|

说明:
1. 在离涨停2%区间如果有多个大单以涨停价买,则跟买;
2. 如果卖一未成交量不控制,则较小机会存在排撤;如果卖一量>0,则未达到封单情况下单,较小机会存在排撤;如果卖一设置为0,则根据封单金额下单,在封单金额不足撤单(首先要封单金额达到封单不足撤单金额以上,才会根据封单不足撤单);
3. 封单金额不足撤单,会根据两个交易所的响应时间达到时看当前封单金额是否足够,如果不足自动撤单;
4. 如果封单后,在下一分钟成交量超过设定值,撤单;
5. 如果封单后,连续2分钟成交量累计值超过设定值,撤单;
6. 每次撤单后,会根据设置的封单金额增长比例增加封单金额;
7. 可以根据plot.conf的设置,确定开盘涨停是否下单、涨停是否暂停、是否自动撤单、撤单后是否停止;
8. 如果设置自动撤单为否、撤单后停止则可以通过第三方进行撤单;

#### 新半路买(xgem)
| 序号 | 字段 | 中文 | 取值 | 说明 |
|---|---|---|---|---|
|1|market|市场|1：上海，2：深圳| |
|2|securityId|证券代码| |证券代码|
|3|bsType|买卖方向|1：买| |
|4|conPx|最低涨幅| |万分比|
|5|ordPx|最高涨幅| |万分比|
|6|qtyType|委托数量类型|0:数量，1：资金,卖出固定为0| |
|7|money,qty|委托金额or数量|数量或金额| |
|8|cdl|涨速| |万分比,实际涨速大于该值时通过,>0时最低涨速,<0时最高涨速|
|9|askQty|涨速|万分比,>0时最高涨速,<0时最低涨速 ||
|10|buyMoney|1分钟最小成交金额| |万元,默认1500|
|11|buyCtrlMoney|1分钟最大成交金额| |万元，默认10000|
|12|isNoneTrade|是否允许交易|0：允许，1：不允许| |
|13|sign|方案标志|0:正涨速,1:负涨速,2:正涨速回落| |
|14|kpzdf||开盘涨幅|设置小于-2200,不控制|
|15|upperQtyMulty|涨停价卖出挂单量是昨天的倍数|万分比,设置小于0不生效||
|16|upperQtyMultyMin|3分钟最大成交金额(正涨速有效)|万元,<=0 不限制||
|17|nxtCtrlMoney|信号触发开始买次数| 0从一次开始 | |
|18|followCtrlMoney|信号触发结束买次数|大于0有效,其它不限制| |


#### 正常卖(xsell)
| 序号 | 字段 | 中文 | 取值 | 说明 |
|---|---|---|---|---|
|1|market|市场|1：上海，2：深圳| |
|2|securityId|证券代码| |证券代码|
|3|bsType|买卖方向|2：卖| |
|4|conPx|时间批次1| |HHMMSSsss|
|5|ordPx|时间批次2| |HHMMSSsss|
|6|qtyType|委托数量类型|0:数量| |
|7|qty|数量| | |
|8|cdl|最低涨速| |万分比,涨速>最低涨速 <最高涨速,且1分钟成交金额小于多少时触发|
|9|askQty|3分成成交金额| |3分钟成交金额,万元,设置为0时不控制 |
|10|buyMoney|1分钟成交金额| |最近1分钟成交金额,万元,设置为0时涨速同时无意义;sign=0正常卖时小于成交金额卖出,sign=3时为大于成交金额卖出|
|11|buyCtrlMoney|最高涨速| |万分比，最高涨速要>=最低涨速|
|12|isNoneTrade|是否允许交易|0：允许，1：不允许| |
|13|sign|方案标志|0:正常卖出| |
|14|kpzdf|开盘涨幅|小于-2000不控制，该值设置要小于200|万分比,默认-2500,开盘涨跌幅小于设置且在设置的时间批次1和时间批次2行情往下卖出| 
|15|upperQtyMulty|拉高涨幅|为0,拉高回落不触发|万分比|
|16|upperQtyMultyMin|回落涨幅| |万分比|
|17|nxtCtrlMoney|托单/压单比例| |万分比|
|18|followCtrlMoney|托单笔数 | | 托单笔数>设置值,且托单/压单比例 > 设置值允许卖出|
|19|preHighPx|昨日最高价| | |
|20|preLowPx|昨日最低价| | |

说明:
1. 正常卖出方案,适用于抢板策略,根据涨跌幅、回落、均线等进行分批卖出;
2. 在收盘时根据与昨日最高最低进行比较,确定是否清仓;
3. 卖出根据sign分为多种方案正常卖、半路卖、盈利卖、分时间点批量卖和特殊卖;

#### 半路卖(xsell)
| 序号 | 字段 | 中文 | 取值 | 说明 |
|---|---|---|---|---|
|1|market|市场|1：上海，2：深圳| |
|2|securityId|证券代码| |证券代码|
|3|bsType|买卖方向|2：卖| |
|4|conPx|时间批次1| |HHMMSSsss|
|5|ordPx|时间批次2| |HHMMSSsss|
|6|qtyType|委托数量类型|0:数量| |
|7|qty|数量| | |
|8|cdl|最低涨速| |万分比,涨速>最低涨速 <最高涨速,且1分钟成交金额小于多少时触发|
|9|askQty|无效| | |
|10|buyMoney|1分钟成交金额| |最近1分钟成交金额,万元,设置为0时涨速同时无意义|
|11|buyCtrlMoney|最高涨速| |万分比，最高涨速要>=最低涨速|
|12|isNoneTrade|是否允许交易|0：允许，1：不允许| |
|13|sign|方案标志|2:正常卖出| |
|14|kpzdf|开盘涨幅|小于-2000不控制，该值设置要小于200|万分比,默认-2500,开盘涨跌幅小于设置且在设置的时间批次1和时间批次2行情往下卖出| 
|15|upperQtyMulty|拉高涨幅|为0,拉高回落不触发|万分比|
|16|upperQtyMultyMin|回落涨幅| |万分比|
|17|nxtCtrlMoney|托单/压单比例| |万分比|
|18|followCtrlMoney|托单笔数 | | 托单笔数>设置值,且托单/压单比例 > 设置值允许卖出|
|19|highPx|买入当天最高价| | |
|20|lowPx|买入当天最低价|  | |

#### 盈利卖出(xsell)
| 序号 | 字段 | 中文 | 取值 | 说明 |
|---|---|---|---|---|
|1|market|市场|1：上海，2：深圳| |
|2|securityId|证券代码| |证券代码|
|3|bsType|买卖方向|2：卖| |
|4|conPx|时间批次1| |HHMMSSsss|
|5|ordPx|时间批次2| |HHMMSSsss|
|6|qtyType|委托数量类型|0:数量| |
|7|qty|数量| | |
|8|cdl|最低涨速| |万分比|
|9|askQty|无效| | |
|10|buyMoney|1分钟成交金额| |当前成交金额或最近2分钟成交金额,万元,设置为0时涨速同时无意义|
|11|buyCtrlMoney|无效| | |
|12|isNoneTrade|是否允许交易|0：允许，1：不允许| |
|13|sign|标志|6:盈利卖出| |
|14|kpzdf|无效| | |
|15|upperQtyMulty|无效| | |
|16|upperQtyMultyMin|无效| | |
|17|nxtCtrlMoney|无效| | |
|18|followCtrlMoney|无效| | |
|19|highPx|买入当天最高价| | |
|20|lowPx|买入当天最低价|  | |


### 集合竞价抢板(xauction)

| 序号 | 字段 | 中文 | 取值 | 说明 |
|---|---|---|---|---|
|1|market|市场|1：上海，2：深圳| |
|2|securityId|证券代码| |证券代码|
|3|bsType|买卖方向|1:买,2：卖| |
|4|conPx|信号价格|-1自动转为涨停价|转债开盘为1300000,转债收盘1573000|
|5|ordPx|实际价格|-1自动转为涨停价,0转为跌停价|转债开盘为1300000,转债收盘1573000|
|6|qtyType|委托数量类型|0:数量,1:金额| |
|7|qty or money|数量| | |
|8|cdl|封单比例| |万分比,集合竞价买二量/集合竞价买一量,连续竞价撤单率,建议7000|
|9|askQty|无效| ||
|10|buyMoney|封单金额|大于0有效 | 连续竞价时封单金额不足撤单,万元|
|11|buyCtrlMoney|撤单率达到时封单金额不足撤单 |万元 |连续竞价时撤单率达到时，封单金额不足撤单 |
|12|isNoneTrade|是否允许交易|0：允许，1：不允许| |
|13|sign|方案标志|0:开盘集合竞价抢单| |
|14|kpzdf|无效| | |
|15|upperQtyMulty| 上海开始尝试报单时间| 91455090|91454550 |
|16|upperQtyMultyMin|深圳开始尝试报单时间 |91456000 | 91456000|
|17|nxtCtrlMoney|频繁报单时间间隔(us)|默认200| |
|18|followCtrlMoney|无效| | |

说明:
1. 集合竞价抢板是以本地时间作为抢板开始尝试开始时间,即需要服务器做对时,开始尝试时间要早于沪深交易所的开盘时间;
2. 沪深交易所的敲门时间有差异,沪市早于深市;
3. 沪深策略的第一笔以最想达到成交的证券为主，其作为2个市场各自的敲门订单，时间掐的会更准；
4. 两市第一个证券会间隔200us尝试5次,在有一次确认后撤掉后续的4次委托,如果都是废单,继续尝试;后续的证券会在第一笔敲门成功后申报;


## 导出文件说明
[导出文件说明](md_export.html)

### 交易所行情文件(mktstore.csv)
交易所行情文件盘中保存为二进制文件data/store/mktstore.bin,可以通过xhisday工具转换为文本文件,转换后文件字段说明如下:

标识(order=1)，市场，证券代码，交易所时间，频道，业务编号，委托编号，委托序列，买卖类型，是否撤单，委托类型，委托价格，委托数量，本地时间
标识(trade=2)，市场，证券代码，交易所时间，频道，业务编号，成交编号，卖委托编号，买委托编号，是否撤单，成交金额，成交价格，成交数量，本地时间
标识（snapshot=3),市场，证券代码，交易所时间，卖一价，卖一量，最新价，买一价，买一量，成交数量，成交金额，最高价，最低价，本地时间，开盘价

## 操作手册
[操作手册](../量赢策略交易平台操作手册.doc)

## changelog
2024.7.17
1.增加行情信息及自定义指数计算;
2.修复抢板类策略问题;
3.新增异步消息信号,用于对策略复杂时延比较敏感的策略;
4.策略参数说明加强;
5.其它优化;