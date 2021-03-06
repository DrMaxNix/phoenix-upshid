# phoenix-upshid
[![GitHub release](https://img.shields.io/badge/release-v1.0.2-orange)](https://github.com/DrMaxNix/phoenix-upshid/releases/tag/v1.0.2)
[![License](https://img.shields.io/badge/license-MIT-green)](https://github.com/DrMaxNix/phoenix-upshid/blob/main/LICENSE)
[![Maintaner](https://img.shields.io/badge/maintainer-DrMaxNix-blue)](https://www.drmaxnix.de)

Simple driver for reading data from Phoenixtec ("Amazon Basics") UPSes written in c

## Why?
A while ago I bought a UPS (Uninterruptible power supply) from Amazon Basics.
I tried multiple drivers for it, but they all had their problems:
- **NUT** tended to mess up the USB connection after some days to a point where you had to **unplug and re-plug the USB cable** of the UPS
- **UPower** had a stable connection, battery percentage was updated in real-time, but the **charging state was frozen**, displaying `fully-charged` when UPS was at 5% charge or displaying `charging` when UPS was already at 100%
- ..and many more that didn't work at all.

So I tried writing dirty fixes for both of those drivers, which would reset the USB connection when detecting such problem. But it wasn't as stable as I had hoped.

At the end I [reverse-engineered](https://github.com/DrMaxNix/phoenix-upshid/blob/main/nut-reverseengineering.md) the USB-HID protocol of the UPS and wrote my own driver for it in c, which has the reset-capability built-in. I'm not at all into c driver development, so use this at your own risk, as the license already states: "WITHOUT WARRANTY OF ANY KIND"!



## How to use
Download and extract source and compile:
```console
$ wget https://github.com/DrMaxNix/phoenix-upshid/archive/refs/tags/v1.0.2.tar.gz
$ tar xf v1.0.2.tar.gz
$ cd phoenix-upshid-1.0.2
$ gcc -o phoenix-upshid phoenix-upshid.c
```

OPTIONALLY copy to `$PATH`:
```console
# cp phoenix-upshid /usr/local/bin/
```

Find device id:
```console
$ lsusb
...
Bus 001 Device 005: ID 06da:ffff Phoenixtec Power Co., Ltd 
...
```

Print device data of device `06da:ffff` (and try usb reset):
```console
# ./phoenix-upshid -a 06da:ffff
```
