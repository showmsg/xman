## 导出文件


###  trade.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | trdId | 成交编号 | | |
| 3 | customerId | 客户号 | | |
| 4 | market | 市场 | 1：上海，2：深圳 | |
| 5 | securityId | 证券代码 | | |
| 6 | investid | 股东账户 | | |
| 7 | trdSide | 成交方向 |  1：买，2：卖 | |
| 8 | trdTime | 成交时间 | 格式HHMMSSsss | |
| 9 | trdQty | 成交数量 | | |
| 10 | trdPrice | 成交价格 | |  |
| 11 | trdAmt | 成交金额 | | |
| 12 | ordid | 成交的委托订单编号 | | |


###  strategy.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | plotid | 策略编号 | | |
| 3 | frontid |前端报单序号 | 必须，前段可以根据sessionId+frontId确定订单唯一 | |
| 4 | market | 市场 | 1：上海，2：深圳 | |
| 5 | securityId | 证券代码 |  | |
| 6 | bs | 买卖 | | |
| 7 | status | 策略执行状态 | | |
| 8 | investId | 投资者账户 | |  |
| 9 | ordPx | 委托价格 |  -1代表系统自动计划涨停价，其他价格 * 10000(int) | |
| 10 | confPx | 条件价格 | | |
| 11 | qtyType |数量类型 | 0：数量，1：资金 | |
| 12 | ordQty | 委托数量 | qtyType为0时必须填写;如果是qtyType为1，在响应时实际数量展示在此字段 | |
| 13 | money | 委托金额 | qtyType为1时必须填写 | |
| 14 | askQty | 卖一未成交量 | -1代表不控制卖一未成交数量,0为无卖单 | |
| 15 | buyMoney | 封单金额(万元) | | |
| 16 | cdl | 买一撤单率 | | |
| 17 | buyCtrlMoney | 封单不足撤单(万元) | | |
| 18 | ordGapTime | 委托间隔时间 | | |
| 19 | ctrGapTime | 撤单间隔时间 | | |
| 20 | sign | 标识 | 0：抢板，1：低吸，2：半路 | |
| 21 | beginTime | 开始时间 | 格式：HHMMSSsss | |
| 22 | endTime | 结束时间 | 格式：HHMMSSsss | |
| 23 | isUpperStop | 涨停打开后是否继续买 | | |
| 24 | isCtrlStop | 撤单后策略是否停止 | | |

###  stock.csv  ###
|序号|字段|字段说明|取值|说明 |
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | market | 市场 | 1：上海，2：深圳 | |
| 3 | securityId | 证券代码 | | |
| 4 | securityName | 证券名称 | | |
| 5 | secStatus | 证券状态 | | |
| 6 | secType | 证券类型 | | |
| 7 | subSecType | 证券子类型 | | |
| 8 | baseSecurityId | 关联证券 | 如securityId为可转债，则baseSecurityId为其对应正股代码 | |
| 9 | prdType | 产品类型 | | |
| 10 | isPriceLimit | 是否是涨停价 | | |
| 11 | isDayTrading | 是否是日例回转 | | |
| 12 | buyUnit | 买基本单元 | | |
| 13 | sellUnit | 卖基本单元 | | |
| 14 | preClose | 前收盘价 | | |
| 15 | priceTick | 价格tick | | |
| 16 | HighPrice| 连续竞价涨停价 | | |
| 17 | LowPrice | 连续竞价跌停价 | | |
| 18 | outstandingShare | 总股本 | | |
| 19 | publicfloatShare | 流通股本 | | |
| 20 | maturityDate | 可转债日期 | | |
| 21 | lmtBuyMinQty | 现价最小买数量 | | |
| 22 | lmtBuyMaxQty | 现价最大买数量 | | |
| 23 | convPx | 转股价值 | | |

###  snapshot.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | tradeDate | 交易日期 | 年月日 | |
| 3 | market | 市场 | 1：上海，2：深圳 | |
| 4 | securityId | 证券代码 | | |
| 5 | secStatus | 证券状态 | | |
| 6 | preClosePx | 昨收价 | | |
| 7 | openPx | 开盘价 | | |
| 8 | highPx | 最高价 | | |
| 9 | lowPx | 最低价 | | |
| 10 | tradePx | 最新价 | | |
| 11 | numTrades | 成交笔数 | | |
| 12 | volumeTrade | 成交量 | | |
| 13 | amountTrade | 成交金额 | | |
| 14 | updateTime | 更新时间 | | |
| 15 | locTime | 落地行情时间 | 精确到纳秒 | |
| 16 | ask5 | 卖五 | | |
| 17 | askqty5 | 卖五未成交量 | | |
| 18 | ask4 | 卖四 | | |
| 19 | askqty4 | 卖四未成交量 | | |
| 20 | ask3 | 卖三 | | |
| 21 | askqty3 | 卖三未成交量 | | |
| 22 | ask2 | 卖二 | | |
| 23 | askqty2 | 卖二未成交量 | | |
| 24 | ask1 | 卖一 | | |
| 25 | askqty1 | 卖一未成交量 | | |
| 26 | bid1 | 买一 | | |
| 27 | bidqty1 | 买一未成交量 | | |
| 28 | bid2 | 买二 | | |
| 29 | bidqty2 | 买二未成交量 | | |
| 30 | bid3 | 买三 | |  |
| 31 | bidqty3 | 买三未成交量 | | |
| 32 | bid4 | 买四 | | |
| 33 | bidqty4 | 买四未成交量 | | |
| 34 | bid5 | 买五 | | |
| 35 | bidqty5 | 买五未成交量 | | |
| 36 | gapTime(ns) | 本地收到行情到处理完快照时间 | 精确到纳秒 | |

###  rsnapshot.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | tradeDate | 交易日期 | 年月日 | |
| 3 | market | 市场 | 1：上海，2：深圳 | |
| 4 |securityId | 证券代码 | | |
| 5 | type | 类型 | | |
| 6 | secStatus | 证券状态 | | |
| 7 | preClosePx | 昨收价 | | |
| 8 | openPx | 开盘价 | | |
| 9 | highPx | 最高价 | | |
| 10 | lowPx | 最低价 | | |
| 11 | tradePx | 最新价 | | |
| 12 | numTrades | 成交笔数 | | |
| 13 | volumeTrade | 成交量 | | |
| 14 | amountTrade | 成交金额 | | |
| 15 | updateTime(ms) | 更新时间 | 精确到毫秒 | |
| 16 | locTime(ns) | 落地行情时间|精确到纳秒 | |
| 17 | recvTime(ns) | 精确到纳秒 | | |
| 18 | ask5 | 卖五 | | |
| 19 | askqty5 | 卖五未成交量 | | |
| 20 | askcqty5 | 卖五撤单量 | | |
| 21 | ask4 | 卖四 | | |
| 22 | askqty4 | 卖四未成交量 | | |
| 23 | askcqty4 | 卖四撤单量 | | |
| 24 | ask3 | 卖三 | | |
| 25 | askqty3 | 卖三未成交量 | | |
| 26 | askcqty3 | 卖三撤单量 | | |
| 27 | ask2 | 卖二 | | |
| 28 | askqty2 | 卖二未成交量 | | |
| 29 | askcqty2 | 卖二撤单量 | | |
| 30 | ask1 | 卖一 | | |
| 31 | askqty1 | 卖一未成交量 | | |
| 32 | askcqty1 | 卖一撤单量 | | |
| 33 | bid1 | 买一 | | |
| 34 | bidqty1 | 买一未成交量 | | |
| 35 | bidcqty1 | 买一撤单量 | | |
| 36 | bid2 | 买二 | | |
| 37 | bidqty2 | 买二未成交量 | | |
| 38 | bidcqty2 | 买二撤单量 | | |
| 39 | bid3 | 买三 | | |
| 40 | bidqty3 | 买三未成交量 | | |
| 41 | bidcqty3 | 买三撤单量 | | |
| 42 | bid4 | 买四 | | |
| 43 | bidqty4 | 买四未成交量 | | |
| 44 | bidcqty4 | 买四撤单量 | | |
| 45 | bid5 | 买五 | | |
| 46 | bidqty5 | 买五未成交量 | | |
| 47 | bidcqty5 | 买五撤单量 | | |
| 48 | gapTimes(ns) | 本地收到行情到处理完快照时间|精确到纳秒 | |
| 49 | _channel | 频道号 | | |
| 50 | version | 版本 | | |
| 51 | upperPx | 涨停价 | | |
| 52 | lowerPx | 跌停价 | | |
| 53 | driveAskPx | 主动卖价 | | |
| 54 | driveBidPx | 主动买价 | | |
| 55 | totalBidQty | 累计买入数量 | | |
| 56 | totalAskQty | 累计卖一未成交量 | | |
| 57 | curUpBidQty | 当前买量 | | |
| 58 | curUpBidCQty | 当前撤单量 | | |
| 59 | bigBuyOrdAmt | 大单买入金额 | | |
| 60 | bigSellOrdAmt | 大单卖出金额 | | |
| 61 | bigBuyOrdQty | 大单买数量 | | |
| 62 | bigSellOrdQty | 大单卖数量 | | |
| 63 | bigBuyTrdAmt | 大单买入成交金额 | | |
| 64 | bigSellTrdAmt | 大单卖出成交金额 | | |
| 65 | driveBuyAmt | 主动买入金额 | | |
| 66 | driverSellTrdAmt | 主动卖单金额 | | |
| 67 | totalBuyOrdCnt | 累计买次数 | | |
| 68 | totalSellOrdCnt | 累计买次数 | | |
| 69 | sealTime| | | |
| 70 | pchg| | | |
| 71 | upperOfferOrdQty | 累计涨停价卖委托数量，撤单扣减 | | |
| 72 | upperOfferOrdCnt | 累计涨停价卖委托次数，撤单扣减 | | |


###  order.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | reqid | 请求编号|订单与请求对应 | |
| 3 | plotid | 策略编号 | | |
| 4 | localId | 客户委托流水号 | | |
| 5 | clocalid | 撤单委托编号 | | |
| 6 | ordid | 订单编号 | | |
| 7 | customerId | 客户号 | | |
| 8 | market | 市场|1：上海，2：深圳 | |
| 9 | securityId | 证券代码 | | |
| 10 | Investid | 股东账号 | | |
| 11 | envno | 环境号 | | |
| 12 | bsType | 买卖类型 | 1：买，2：卖 | |
| 13 | isCancel | 是否允许撤单 | 1：撤单 | |
| 14 | ordType| 订单类型 | 1：限价，2：市价 | |
| 15 | ordQty| 委托数量 | qtyType为0时必须填写;如果是qtyType为1，在响应时实际数量展示在此字段 | |
| 16 | ordPrice| 委托价格 | qtyType为1时必须填写 | |
| 17 | orgEnvno | 环境号 | | |
| 18 | orgLocalId | 撤单时原始订单编号 | 撤单时必须，orgLocalId和orgOrdId必须填一个 | |
| 19 | orgOrdid | 撤单时原始订单编号 | 撤单时必须，orgLocalId和orgOrdId必须填一个 |  |
| 20 | mktPx | 市场价格 | | |
| 21 | mktTime | 该笔行情本地时间 | | |
| 22 | trdQty | 成交数量 | | |
| 23 | trdMoney | 成交金额 | | |
| 24 | ordStatus | 订单状态 | | |
| 25 | sendLocTime | 本地发出时间 | | |
| 26 | cnfLocTime | 收到柜台响应时间 | | |
| 27 | gapCnterRsp(ns) | 柜台响应时间|精确到纳秒 | |
| 28 | gapExchRsp(ns) | 交易所首次响应时间|精确到纳秒 | |
| 29 | gapMktRsp(ns) | 收到行情到委托出订单收到交易所响应时间 | 精确到纳秒 | |
| 30 | sendTime | 柜台发出时间 | | |
| 31 | cnfTime | 交易所响应时间 | | |
| 32 | gapCntExch(ms) | 柜台收到交易响应-柜台发出时间 | | |
| 33 | errorId|错误码 | | |
| 34 | errorMsg | 错误消息 | | |
| 35 | locFrzQty | 冻结数量 | | |
| 36 | locFrzMoney | 冻结资金，发单时记录，收到确认和拒绝时解冻，同时根据此更新资金中locFrz | | |
| 37 | bizIndex | 下单时带入逐笔中委托和成交编号，用于跟踪 | | |
| 38 | exeStatus | 撤单时，如果因某系统未交易失败需要继续撤单 | | |


###  invest.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | customerId | 客户号 | | |
| 2 | acctType | 资金类型 | 1：现货，2：两融，3：期权，4：期货，5：黄金 | |
| 3 | market | 市场 | 1：上海，2：深圳 | |
| 4 | investId | 股东账户 | | |
| 5 | quota | 市值 | | |


###  hold.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | customerId | 客户号 | | |
| 3 | investid | 股东账户 | | |
| 4 | market | 市场 | 1：上海，2：深圳 | |
| 5 | securityId | 产品代码 | | |
| 6 | orgHld | 日初持仓 | | |
| 7 | orgAvlHld | 日初可用持仓 | | |
| 8 | orgCostAmt | 日初持仓成本 | | |
| 9 | totalBuyHld | 累计买 | | |
| 10 | totalSellHld | 累计卖 | | | 
| 11 | sumHld | 总持仓 | | |
| 12 | sellAvlHld | 可卖出持仓 | | |
| 13 | countSellAvlHld | 柜台可以持仓 | | |
| 14 | etfAvlHld | 可用于etf申购的持仓 | | |
| 15 | costPrice | 持仓成本 | | |
| 16 | locFrz | 记录本地冻结持仓，系统重启时为0;发单时在Order记录冻结持仓数量,收到确认和拒绝时解冻 | | |
| 17 | tradePx | 最新价 | | |
| 18 | sellFrzHold | 卖冻结持仓 | | |
| 19 | totalBuyAmt | 累计买入金额 | | |
| 20 | totalSellAmt | 累计卖出金额 | |  |




###  difsnapshot.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | tradeDate | 交易日期 | 年月日 | |
| 3 | market | 市场 | 1：上海，2：深圳 | |
| 4 | securityId | 证券代码 | | |
| 5 | type | 类型 | 1：交易，2：行情，3：交易和行情 | |
| 6 | secStatus | 证券状态 | | |
| 7 | preClosePx | 昨收价 | | |
| 8 | openPx | 开盘价 | | |
| 9 | highPx | 最高价 | | |
| 10 | lowPx | 最低价 | | |
| 11 | tradePx | 最新价 | | |
| 12 | numTrades | 成交笔数 | | |
| 13 | volumeTrade | 成交量 | | |
| 14 | amountTrade | 成交金额 | | |
| 15 | updateTime(ms) | 更新时间 | | |
| 16 | locTime(ns) | 落地行情时间 | 精确到纳秒 | |
| 17 | ask5 | 卖五 | | |
| 18 | askqty5 | 卖五未成交量 | | |
| 19 | askcqty5 | 卖五撤单量 | | |
| 20 | ask4 | 卖四 | | |
| 21 | askqty4 | 卖四未成交量 | | |
| 22 | askcqty4 | 卖四撤单量 | | |
| 23 | ask3 | 卖三 | | |
| 24 | askqty3 | 卖三未成交量 | | |
| 25 | askcqty3 | 卖三撤单量 | | |
| 26 | ask2 | 卖二 | | |
| 27 | askqty2 | 卖二未成交量 | | |
| 28 | askcqty2 | 卖二撤单量 | | |
| 29 | ask1 | 卖一 | | |
| 30 | askqty1 | 卖一未成交量 | | |
| 31 | askcqty1 | 卖一撤单量 | | |
| 32 | bid1 | 买一 | | |
| 33 | bidqty1 | 买一未成交量 | | |
| 34 | bidcqty1 | 买一撤单量 | | |
| 35 | bid2 | 买二 | | |
| 36 | bidqty2 | 买二未成交量 | | |
| 37 | bidcqty2 |买二撤单量 | | |
| 38 | bid3 | 买三 | | |
| 39 | bidqty3 | 买三未成交量 | | |
| 40 | bidcqty3 | 买三撤单量 | | |
| 41 | bid4 | 买四 | | |
| 42| bidqty4 | 买四未成交量 | | |
| 43| bidcqty4| 买四撤单量 | | |
| 44 | bid5 | 买五 | | |
| 45 | bidqty5 | 买五未成交量 | | |
| 46 | bidcqty5 | 买五撤单量 | | |
| 47 | gapTimes(ns) | 本地收到行情到处理完快照时间 | 精确到纳秒| |
| 48 | _channel | 频道号 | | |
| 49 | version | 版本 | | |
| 50 | upperPx | 涨停价 | | |
| 51 | lowerPx | 跌停价 | | |
| 52 | driveAskPx | 主动卖价 | | |
| 53 | driveBidPx |主动买价 | | |
| 54 | totalBidQty | 累计买入数量 | | |
| 55 | totalBidCQty | 累计撤单量 | | |
| 56 | curUpBidQty | 当前买量 | | |
| 57 | curUpBidCQty | 当前撤单量 | | |


###  cash.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | id | 编号 | | |
| 2 | customerId| 客户号 | | |
| 3 | acctType | 资金类型 | 0:现货，1：两融，2：期权 | |
| 4 | accountId | 资金账户 | | |
| 5 | beginBalance | 日初资金 | | |
| 6 | beginAvailable | 初始可用资金 | | |
| 7 | beginDrawable | 初始可取资金 | | |
| 8 | curAvailable |当前可用资金 | | |
| 9 | totalBuy | 累计买金额 | | |
| 10 | totalSell | 累计卖金额 | | |
| 11 | locFrz | 本地资金冻结，委托返回后解冻 | | |


###  blockinfo.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | blockno | 板块编号 | | |
| 2 | securityid |证券代码 | | |
| 3 | market | 市场 | 1：上海，2：深圳 | |
| 4 | zdf | 个股涨跌幅 | | |
| 5 | securityName | 证券名称 | | |


###  block.csv  ###
|序号|字段|字段说明|取值|说明|
|-|-|-|-|-|
| 1 | blockno | 板块编号 | | |
| 2 | blockname | 板块名称 | | |
| 3 | Count | 板块数量 | | |
| 4 | zdf | 涨跌幅 | | |