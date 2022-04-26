command used:
```console
# /lib/nut/usbhid-ups -a upsname -DDD
```



## report id 0x06
```
RAW RESPONSE:		06 [64] e4 08
NUT DEBUG LOG:		RemainingCapacity, Offset: 0, Size: 8, Value: 100
BYTE(S) DECODED:	0x64 => 100
```

```
RAW RESPONSE:		06 64 [e4 08]
NUT DEBUG LOG:		RunTimeToEmpty, Offset: 8, Size: 16, Value: 2276
BYTE(S) DECODED:	0x08e4 => 2276
```



## report id 0x01
```
RAW RESPONSE:		01 [01] 00 01 00 01 00 00
NUT DEBUG LOG:		ACPresent, Offset: 0, Size: 8, Value: 1
BYTE(S) DECODED:	0x01 => 1 => true
```

```
RAW RESPONSE:		01 01 [00] 01 00 01 00 00
NUT DEBUG LOG:		BelowRemainingCapacityLimit, Offset: 8, Size: 8, Value: 0
BYTE(S) DECODED:	0x00 => 0 => false
```

```
RAW RESPONSE:		01 01 00 [01] 00 01 00 00
NUT DEBUG LOG:		Charging, Offset: 16, Size: 8, Value: 1
BYTE(S) DECODED:	0x01 => 1 => true
```

```
RAW RESPONSE:		01 01 00 01 [00] 01 00 00
NUT DEBUG LOG:		Discharging, Offset: 24, Size: 8, Value: 0
BYTE(S) DECODED:	0x00 => 0 => false
```
