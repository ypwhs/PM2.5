# PM2.5

![](https://raw.githubusercontent.com/ypwhs/resources/master/pm2.5.jpeg)

http://pm.ypw.io

## 硬件

| 名称       | 型号        | 价格（元） | 通讯   | 图片                                       |
| -------- | --------- | ----- | ---- | ---------------------------------------- |
| NanoPi2  | NanoPi2   | 199   | ssh  | [<img width=400px src= https://img.alicdn.com/imgextra/i1/73291228/TB2aDndhpXXXXa9XXXXXXXXXXXX_!!73291228.jpg>](https://item.taobao.com/item.htm?id=524842034015) |
| PM2.5传感器 | PMS5003   | 110   | 串口   | [<img width=400px src=https://img.alicdn.com/imgextra/i1/2681925680/TB2W_i9nVXXXXXaXFXXXXXXXXXX_!!2681925680.jpg>](https://item.taobao.com/item.htm?id=524354539294) |
| 二氧化碳传感器  | DS-CO2-20 | 198   | 串口   | [<img width=400px src=https://img.alicdn.com/imgextra/i4/2681925680/TB2qhPAacgd61BjSZFPXXbVVFXa_!!2681925680.jpg>](https://item.taobao.com/item.htm?id=540405255399) |
| 7寸屏      | X710      | 160   | 排线   | [<img width=400px src= https://img.alicdn.com/imgextra/i3/73291228/TB2s.1FjpXXXXXQXXXXXXXXXXXX_!!73291228.jpg>](https://item.taobao.com/item.htm?id=526058309474) |

## 软件

传感器相关的程序用 C 写的，代码在 [sensors_server/sensors.c](sensors_server/sensors.c)

屏幕上的 UI 程序用 Qt 写的，代码在 [pm](pm)

服务器上的传感器记录程序用 python 写的，代码在 [log_server/server.py](log_server/server.py)

服务器上的前后端程序用 Flask 和 echarts 写的，代码在 [flask](flask)
