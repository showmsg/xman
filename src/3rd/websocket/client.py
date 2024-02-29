#!/usr/bin/python3
# -*- coding:utf-8 -*-

# 安装: pip3 install websocket-client
# 注意: 不推荐用pyhon2版本,运行时会报'SSLSocket'错误
# 参考: https://blog.csdn.net/weixin_37989267/article/details/87928934
import websocket
import threading
import time

def on_message(ws, message):
    print('on_message', message)

def on_error(ws, error):
    global isOpen
    print('on_error', error)
    isOpen = False

def on_close(ws):
    global isOpen
    print('on_close')
    isOpen = False

def on_open(ws):
    global isOpen
    print('on_open')
    isOpen = True

def ws_loop(ws):
    # 注意该'run_forever()'函数是阻塞的
    ws.run_forever()
    # 定时ping服务器
    # ws.run_forever(ping_interval = 10, ping_timeout = 5)

if __name__ == "__main__":
    # main函数里定义的变量默认为全局变量
    isOpen = False
    # 打印后台信息
    websocket.enableTrace(False)
    # 装载接口
    ws = websocket.WebSocketApp(
        "ws://127.0.0.1:9999/",
        on_message=on_message,
        on_error=on_error,
        on_close=on_close,
        on_open=on_open)
    # 开线程维护连接(避免阻塞在这里)
    threading.Thread(target=ws_loop, args=(ws,)).start()
    # 周期发送心跳
    while True:
        time.sleep(3)
        if isOpen:
            try:
                ws.send(
                    'Heart from client ' + 
                    time.strftime("%H:%M:%S", time.localtime()))
            except:
                print('ws.send error !!')
                isOpen = false
