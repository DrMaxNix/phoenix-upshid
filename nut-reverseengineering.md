command used:
```console
# /lib/nut/usbhid-ups -a upsname -DDD
```



### report id 0x06
`06` **64** e4 08
RemainingCapacity, Offset: 0, Size: 8, Value: **100**
_0x64 => 100_

`06` 64 **e4 08**
RunTimeToEmpty, Offset: 8, Size: 16, Value: **2276**
_0x08e4 => 2276_



### report id 0x01
`01` **01** 00 01 00 01 00 00
ACPresent, Offset: 0, Size: 8, Value: **1**
_0x01 => 1 => true_

`01` 01 **00** 01 00 01 00 00
BelowRemainingCapacityLimit, Offset: 8, Size: 8, Value: **0**
_0x00 => 0 => false_

`01` 01 00 **01** 00 01 00 00
Charging, Offset: 16, Size: 8, Value: **1**
_0x01 => 1 => true_

`01` 01 00 01 **00** 01 00 00
Discharging, Offset: 24, Size: 8, Value: **0**
_0x00 => 0 => false_
