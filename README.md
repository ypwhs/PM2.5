# PM2.5

![](https://raw.githubusercontent.com/ypwhs/resources/master/pm2.5.jpeg)

http://pm.ypw.io

## 硬件

硬件使用了：
* PMS5003 和	DS-CO2-20 传感器。
* NanoPi2
* NanoPi2 7寸屏

## 软件

传感器相关的程序用 C 写的，代码在 [sensors_server/sensors.c](sensors_server/sensors.c)

屏幕上的 UI 程序用 Qt 写的，代码在 [pm](pm)

服务器上的传感器记录程序用 python 写的，代码在 [log_server/server.py](log_server/server.py)

服务器上的前后端程序用 Flask 和 echarts 写的，代码在 [flask](flask)
