EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Battery EEPROM Programmer V2"
Date "2021-03-05"
Rev "2.6"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
NoConn ~ 9350 1350
Text Label 9250 1200 1    60   ~ 0
IOREF
Text Label 8900 1200 1    60   ~ 0
Vin
Text Label 8900 2450 0    60   ~ 0
BattDiv
Text Label 8900 2550 0    60   ~ 0
Batt_SDA
Text Label 8900 2650 0    60   ~ 0
Batt_SCL
Text Label 8900 2750 0    60   ~ 0
THERM
Text Label 8900 2850 0    60   ~ 0
A4
Text Label 8900 2950 0    60   ~ 0
A5
Text Label 8900 3050 0    60   ~ 0
A6
Text Label 8900 3150 0    60   ~ 0
A7
Text Label 8900 3400 0    60   ~ 0
A8
Text Label 8900 3500 0    60   ~ 0
A9
Text Label 8900 3600 0    60   ~ 0
A10
Text Label 8900 3700 0    60   ~ 0
A11
Text Label 8900 3800 0    60   ~ 0
A12
Text Label 8900 3900 0    60   ~ 0
A13
Text Label 8900 4000 0    60   ~ 0
A14
Text Label 8900 4100 0    60   ~ 0
A15
Text Label 10500 4650 1    60   ~ 0
Therm_Rly
Text Label 10400 4650 1    60   ~ 0
SDA_Rly
Text Label 10300 4650 1    60   ~ 0
SCL_Rly
Text Label 10200 4650 1    60   ~ 0
Batt+_Rly
Text Label 10100 4650 1    60   ~ 0
30
Text Label 9800 4650 1    60   ~ 0
36
Text Label 9700 4650 1    60   ~ 0
38
Text Label 9600 4650 1    60   ~ 0
SCL_Test
Text Label 9400 4650 1    60   ~ 0
SDA_Test
Text Label 9300 4650 1    60   ~ 0
46
Text Label 10400 1650 0    60   ~ 0
WP
Text Label 9100 4650 1    60   ~ 0
MISO
Text Label 9000 4650 1    60   ~ 0
SCK
Text Label 10500 5650 1    60   ~ 0
23
Text Label 10400 5650 1    60   ~ 0
25
Text Label 10300 5650 1    60   ~ 0
27
Text Label 10100 5650 1    60   ~ 0
31
Text Label 10200 5650 1    60   ~ 0
29
Text Label 10000 5650 1    60   ~ 0
33
Text Label 9900 5650 1    60   ~ 0
35
Text Label 9800 5650 1    60   ~ 0
37
Text Label 9700 5650 1    60   ~ 0
39
Text Label 9600 5650 1    60   ~ 0
41
Text Label 9500 5650 1    60   ~ 0
43
Text Label 9400 5650 1    60   ~ 0
45
Text Label 9300 5650 1    60   ~ 0
47
Text Label 10400 1750 0    60   ~ 0
CD
Text Label 9100 5750 1    60   ~ 0
MOSI
Text Label 9000 5750 1    60   ~ 0
SS
Text Label 10400 4100 0    60   ~ 0
Mega_SCL
Text Label 10400 4000 0    60   ~ 0
Mega_SDA
Text Label 10400 3900 0    60   ~ 0
19(Rx1)
Text Label 10400 3600 0    60   ~ 0
16(Tx2)
Text Label 10400 3500 0    60   ~ 0
15(Rx3)
Text Label 10400 3400 0    60   ~ 0
14(Tx3)
Text Label 10400 1550 0    60   ~ 0
13(**)
Text Label 10400 2750 0    60   ~ 0
ACT_CLK_EN
Text Label 10400 3800 0    60   ~ 0
PAS_CLK_EN
Text Label 10400 2950 0    60   ~ 0
SDA_EN2
Text Label 10400 2850 0    60   ~ 0
SDA_EN1
Text Label 10400 2650 0    60   ~ 0
SDA_EN0
Text Label 9900 4650 1    60   ~ 0
SCL_EN2
Text Label 10000 4650 1    60   ~ 0
SCL_EN1
Text Label 10400 3700 0    60   ~ 0
SCL_EN0
Text Label 10400 2050 0    60   ~ 0
PULL_SEL1
Text Label 10400 1950 0    60   ~ 0
PULL_SEL0
Text Label 10400 2450 0    60   ~ 0
PULL_ENB*
Text Label 10400 3050 0    60   ~ 0
1(Tx0)
Text Label 10400 3150 0    60   ~ 0
0(Rx0)
Text Label 10400 1250 0    60   ~ 0
SDA
Text Label 10400 1150 0    60   ~ 0
SCL
Text Label 10400 1350 0    60   ~ 0
AREF
Text Notes 8375 575  0    60   ~ 0
Shield for Arduino Mega Rev 3
$Comp
L IO-Board-rescue:CONN_01X08-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue P2
U 1 1 56D71773
P 9550 1700
F 0 "P2" H 9550 2150 50  0000 C CNN
F 1 "Power" V 9650 1700 50  0000 C CNN
F 2 "Socket_Arduino_Mega:Socket_Strip_Arduino_1x08" H 9550 1700 50  0001 C CNN
F 3 "" H 9550 1700 50  0000 C CNN
	1    9550 1700
	1    0    0    -1  
$EndComp
Text Notes 9650 1350 0    60   ~ 0
1
Text Label 8600 1550 0    60   ~ 0
Reset
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR01
U 1 1 56D71D10
P 9000 1100
F 0 "#PWR01" H 9000 950 50  0001 C CNN
F 1 "+5V" H 9000 1240 50  0000 C CNN
F 2 "" H 9000 1100 50  0000 C CNN
F 3 "" H 9000 1100 50  0000 C CNN
	1    9000 1100
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR02
U 1 1 56D721E6
P 9250 2150
F 0 "#PWR02" H 9250 1900 50  0001 C CNN
F 1 "GND" H 9250 2000 50  0000 C CNN
F 2 "" H 9250 2150 50  0000 C CNN
F 3 "" H 9250 2150 50  0000 C CNN
	1    9250 2150
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:CONN_01X10-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue P5
U 1 1 56D72368
P 9950 1600
F 0 "P5" H 9950 2150 50  0000 C CNN
F 1 "PWM" V 10050 1600 50  0000 C CNN
F 2 "Socket_Arduino_Mega:Socket_Strip_Arduino_1x10" H 9950 1600 50  0001 C CNN
F 3 "" H 9950 1600 50  0000 C CNN
	1    9950 1600
	-1   0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR03
U 1 1 56D72A3D
P 10250 2150
F 0 "#PWR03" H 10250 1900 50  0001 C CNN
F 1 "GND" H 10250 2000 50  0000 C CNN
F 2 "" H 10250 2150 50  0000 C CNN
F 3 "" H 10250 2150 50  0000 C CNN
	1    10250 2150
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:CONN_01X08-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue P3
U 1 1 56D72F1C
P 9550 2800
F 0 "P3" H 9550 3250 50  0000 C CNN
F 1 "Analog" V 9650 2800 50  0000 C CNN
F 2 "Socket_Arduino_Mega:Socket_Strip_Arduino_1x08" H 9550 2800 50  0001 C CNN
F 3 "" H 9550 2800 50  0000 C CNN
	1    9550 2800
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:CONN_01X08-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue P6
U 1 1 56D734D0
P 9950 2800
F 0 "P6" H 9950 3250 50  0000 C CNN
F 1 "PWM" V 10050 2800 50  0000 C CNN
F 2 "Socket_Arduino_Mega:Socket_Strip_Arduino_1x08" H 9950 2800 50  0001 C CNN
F 3 "" H 9950 2800 50  0000 C CNN
	1    9950 2800
	-1   0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:CONN_01X08-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue P4
U 1 1 56D73A0E
P 9550 3750
F 0 "P4" H 9550 4200 50  0000 C CNN
F 1 "Analog" V 9650 3750 50  0000 C CNN
F 2 "Socket_Arduino_Mega:Socket_Strip_Arduino_1x08" H 9550 3750 50  0001 C CNN
F 3 "" H 9550 3750 50  0000 C CNN
	1    9550 3750
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:CONN_01X08-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue P7
U 1 1 56D73F2C
P 9950 3750
F 0 "P7" H 9950 4200 50  0000 C CNN
F 1 "Communication" V 10050 3750 50  0000 C CNN
F 2 "Socket_Arduino_Mega:Socket_Strip_Arduino_1x08" H 9950 3750 50  0001 C CNN
F 3 "" H 9950 3750 50  0000 C CNN
	1    9950 3750
	-1   0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:CONN_02X18-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue P1
U 1 1 56D743B5
P 9750 5100
F 0 "P1" H 9750 6050 50  0000 C CNN
F 1 "Digital" V 9750 5100 50  0000 C CNN
F 2 "Socket_Arduino_Mega:Socket_Strip_Arduino_2x18" H 9750 4050 50  0001 C CNN
F 3 "" H 9750 4050 50  0000 C CNN
	1    9750 5100
	0    -1   1    0   
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR04
U 1 1 56D758F6
P 8650 5750
F 0 "#PWR04" H 8650 5500 50  0001 C CNN
F 1 "GND" H 8650 5600 50  0000 C CNN
F 2 "" H 8650 5750 50  0000 C CNN
F 3 "" H 8650 5750 50  0000 C CNN
	1    8650 5750
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR05
U 1 1 56D75AB8
P 10750 4550
F 0 "#PWR05" H 10750 4400 50  0001 C CNN
F 1 "+5V" H 10750 4690 50  0000 C CNN
F 2 "" H 10750 4550 50  0000 C CNN
F 3 "" H 10750 4550 50  0000 C CNN
	1    10750 4550
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:MAX4618-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U4
U 1 1 59D7989D
P 1700 1350
F 0 "U4" H 1700 850 60  0000 C CNN
F 1 "MAX4618" H 1700 1850 60  0000 C CNN
F 2 "SMD_Packages:SO-16-N" H 1700 1700 60  0001 C CNN
F 3 "" H 1700 1700 60  0001 C CNN
	1    1700 1350
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:MAX4614-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U1
U 1 1 59D7A4B7
P 2600 2650
F 0 "U1" H 2600 2100 60  0000 C CNN
F 1 "MAX4614" H 2600 3000 60  0000 C CNN
F 2 "SMD_Packages:SOIC-14_N" H 2600 2650 60  0001 C CNN
F 3 "" H 2600 2650 60  0001 C CNN
	1    2600 2650
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:MAX4614-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U2
U 1 1 59D7A500
P 2600 3850
F 0 "U2" H 2600 3300 60  0000 C CNN
F 1 "MAX4614" H 2600 4200 60  0000 C CNN
F 2 "SMD_Packages:SOIC-14_N" H 2600 3850 60  0001 C CNN
F 3 "" H 2600 3850 60  0001 C CNN
	1    2600 3850
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:SN74LVC2G17-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U3
U 1 1 59D7A862
P 2050 4800
F 0 "U3" H 2050 4600 60  0000 C CNN
F 1 "SN74LVC2G17" H 2050 5000 60  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-23-6" H 2050 4650 60  0001 C CNN
F 3 "" H 2050 4650 60  0001 C CNN
	1    2050 4800
	1    0    0    -1  
$EndComp
Text Label 2200 1650 0    60   ~ 0
+5V
Text Label 2200 1750 0    60   ~ 0
GND
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R1
U 1 1 59DB890D
P 2950 1000
F 0 "R1" V 3030 1000 50  0000 C CNN
F 1 "100K" V 2950 1000 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 2880 1000 50  0001 C CNN
F 3 "" H 2950 1000 50  0001 C CNN
	1    2950 1000
	1    0    0    -1  
$EndComp
Text Label 3100 750  0    60   ~ 0
IOREF
Text Label 3150 1250 0    60   ~ 0
PULL_ENB*
Text Label 3150 1350 0    60   ~ 0
PULL_SEL0
Text Label 3150 1450 0    60   ~ 0
PULL_SEL1
Text Notes 4500 2050 0    60   ~ 0
Batt+
Text Notes 4600 2150 0    60   ~ 0
SCL
Text Notes 4600 2250 0    60   ~ 0
SDA
Text Notes 4600 2350 0    60   ~ 0
GND
Text Notes 4650 1850 0    60   ~ 0
Battery\nConnector
Text Label 7500 3400 0    60   ~ 0
Batt_SCL
Text Label 7500 4800 0    60   ~ 0
Batt_SDA
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR07
U 1 1 59DBE1A2
P 700 1750
F 0 "#PWR07" H 700 1500 50  0001 C CNN
F 1 "GND" H 700 1600 50  0000 C CNN
F 2 "" H 700 1750 50  0001 C CNN
F 3 "" H 700 1750 50  0001 C CNN
	1    700  1750
	1    0    0    -1  
$EndComp
Text Label 700  950  0    60   ~ 0
+3.3V
Text Label 700  1050 0    60   ~ 0
Batt+
Text Label 700  1150 0    60   ~ 0
+5V
Text Label 2150 950  0    60   ~ 0
PULL_SCL
Text Label 550  2500 0    60   ~ 0
SCL_EN0
Text Label 550  2700 0    60   ~ 0
SCL_EN1
Text Label 550  2900 0    60   ~ 0
SCL_EN2
Text Label 550  3100 0    60   ~ 0
PAS_CLK_EN
Text Label 600  3700 0    60   ~ 0
SDA_EN0
Text Label 600  3900 0    60   ~ 0
SDA_EN1
Text Label 600  4100 0    60   ~ 0
SDA_EN2
Text Label 600  4300 0    60   ~ 0
ACT_CLK_EN
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R5
U 1 1 59DC5169
P 3300 2400
F 0 "R5" V 3380 2400 50  0000 C CNN
F 1 "10K" V 3300 2400 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 3230 2400 50  0001 C CNN
F 3 "" H 3300 2400 50  0001 C CNN
	1    3300 2400
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R9
U 1 1 59DC51EA
P 3550 2550
F 0 "R9" V 3630 2550 50  0000 C CNN
F 1 "6.8K" V 3550 2550 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 3480 2550 50  0001 C CNN
F 3 "" H 3550 2550 50  0001 C CNN
	1    3550 2550
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R6
U 1 1 59DC5279
P 3300 2700
F 0 "R6" V 3380 2700 50  0000 C CNN
F 1 "5.1K" V 3300 2700 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 3230 2700 50  0001 C CNN
F 3 "" H 3300 2700 50  0001 C CNN
	1    3300 2700
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R7
U 1 1 59DC5335
P 3300 3600
F 0 "R7" V 3380 3600 50  0000 C CNN
F 1 "10K" V 3300 3600 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 3230 3600 50  0001 C CNN
F 3 "" H 3300 3600 50  0001 C CNN
	1    3300 3600
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R10
U 1 1 59DC5392
P 3600 3750
F 0 "R10" V 3680 3750 50  0000 C CNN
F 1 "6.8K" V 3600 3750 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 3530 3750 50  0001 C CNN
F 3 "" H 3600 3750 50  0001 C CNN
	1    3600 3750
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R8
U 1 1 59DC53E9
P 3300 3900
F 0 "R8" V 3380 3900 50  0000 C CNN
F 1 "5.1K" V 3300 3900 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 3230 3900 50  0001 C CNN
F 3 "" H 3300 3900 50  0001 C CNN
	1    3300 3900
	0    1    1    0   
$EndComp
Text Label 4050 3600 0    60   ~ 0
Batt_SDA
Text Label 3050 3000 0    60   ~ 0
+5V
Text Label 3050 3100 0    60   ~ 0
GND
Text Label 3050 4300 0    60   ~ 0
GND
Text Label 3050 4200 0    60   ~ 0
+5V
Text Label 2600 4950 0    60   ~ 0
GND
Text Label 1500 2400 0    60   ~ 0
PULL_SCL
Text Label 1500 3600 0    60   ~ 0
PULL_SDA
Text Label 3950 2400 0    60   ~ 0
HV_SCL
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R11
U 1 1 59DD8F81
P 700 1550
F 0 "R11" V 780 1550 50  0000 C CNN
F 1 "10K" V 700 1550 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 630 1550 50  0001 C CNN
F 3 "" H 700 1550 50  0001 C CNN
	1    700  1550
	1    0    0    -1  
$EndComp
Text Label 550  2250 0    60   ~ 0
IOREF
Text Label 950  5750 2    60   ~ 0
Mega_SCL
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R19
U 1 1 59DE7703
P 1100 3450
F 0 "R19" V 1180 3450 50  0000 C CNN
F 1 "100K" V 1100 3450 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 1030 3450 50  0001 C CNN
F 3 "" H 1100 3450 50  0001 C CNN
	1    1100 3450
	0    1    1    0   
$EndComp
Text Label 600  3450 0    60   ~ 0
IOREF
$Comp
L IO-Board-rescue:SD_Card-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue J2
U 1 1 59DEAFE6
P 2100 7000
F 0 "J2" H 1450 7550 50  0000 C CNN
F 1 "SD_Card" H 2700 6450 50  0000 C CNN
F 2 "Connectors:SD_Card_Receptacle" H 2300 7350 50  0001 C CNN
F 3 "" H 2100 7000 50  0001 C CNN
	1    2100 7000
	-1   0    0    1   
$EndComp
Text Label 3000 7000 0    60   ~ 0
+3.3V
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR08
U 1 1 59DEC551
P 1050 7550
F 0 "#PWR08" H 1050 7300 50  0001 C CNN
F 1 "GND" H 1050 7400 50  0000 C CNN
F 2 "" H 1050 7550 50  0001 C CNN
F 3 "" H 1050 7550 50  0001 C CNN
	1    1050 7550
	1    0    0    -1  
$EndComp
Text Label 550  7100 0    60   ~ 0
WP
Text Label 550  7200 0    60   ~ 0
CD
$Comp
L IO-Board-rescue:SN74LVC2G17-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U5
U 1 1 59DF3FC3
P 4500 6150
F 0 "U5" H 4500 5950 60  0000 C CNN
F 1 "SN74LVC2G17" H 4500 6350 60  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-23-6" H 4500 6000 60  0001 C CNN
F 3 "" H 4500 6000 60  0001 C CNN
	1    4500 6150
	-1   0    0    1   
$EndComp
Text Label 5050 6050 0    60   ~ 0
+3.3V
Text Label 3700 6050 0    60   ~ 0
GND
Text Label 3000 6700 0    60   ~ 0
MISO
Text Label 5050 6250 0    60   ~ 0
MOSI
Text Label 5050 6150 0    60   ~ 0
SCK
Text Label 3000 7100 0    60   ~ 0
GND
Text Label 3000 6800 0    60   ~ 0
GND
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R4
U 1 1 59E03026
P 750 6700
F 0 "R4" V 830 6700 50  0000 C CNN
F 1 "10K" V 750 6700 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 680 6700 50  0001 C CNN
F 3 "" H 750 6700 50  0001 C CNN
	1    750  6700
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R20
U 1 1 59E032E8
P 950 6700
F 0 "R20" V 1030 6700 50  0000 C CNN
F 1 "10K" V 950 6700 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 880 6700 50  0001 C CNN
F 3 "" H 950 6700 50  0001 C CNN
	1    950  6700
	1    0    0    -1  
$EndComp
Text Label 1050 6400 0    60   ~ 0
+3.3V
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R21
U 1 1 59E0C2FD
P 1100 4500
F 0 "R21" V 1180 4500 50  0000 C CNN
F 1 "100K" V 1100 4500 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 1030 4500 50  0001 C CNN
F 3 "" H 1100 4500 50  0001 C CNN
	1    1100 4500
	0    1    1    0   
$EndComp
Text Label 650  4500 0    60   ~ 0
IOREF
$Comp
L IO-Board-rescue:ATECC508A-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U6
U 1 1 59DCCE85
P 4100 5250
F 0 "U6" H 4100 5050 60  0000 C CNN
F 1 "ATECC508A" H 4100 5550 60  0000 C CNN
F 2 "SMD_Packages:SOIC-8-N" H 4150 5250 60  0001 C CNN
F 3 "" H 4150 5250 60  0001 C CNN
	1    4100 5250
	1    0    0    -1  
$EndComp
Text Label 4550 5050 0    60   ~ 0
+3.3V
Text Label 4550 5150 0    60   ~ 0
Mega_SCL
Text Label 4550 5250 0    60   ~ 0
Mega_SDA
Text Label 4550 5350 0    60   ~ 0
GND
$Comp
L IO-Board-rescue:+3.3V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR09
U 1 1 59DD96EA
P 9100 950
F 0 "#PWR09" H 9100 800 50  0001 C CNN
F 1 "+3.3V" H 9100 1090 50  0000 C CNN
F 2 "" H 9100 950 50  0001 C CNN
F 3 "" H 9100 950 50  0001 C CNN
	1    9100 950 
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C4
U 1 1 59DE1DC2
P 5500 7250
F 0 "C4" H 5525 7350 50  0000 L CNN
F 1 ".1uf" H 5525 7150 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 5538 7100 50  0001 C CNN
F 3 "" H 5500 7250 50  0001 C CNN
	1    5500 7250
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C1
U 1 1 59DE1E5B
P 6400 7250
F 0 "C1" H 6425 7350 50  0000 L CNN
F 1 ".1uf" H 6425 7150 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 6438 7100 50  0001 C CNN
F 3 "" H 6400 7250 50  0001 C CNN
	1    6400 7250
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C2
U 1 1 59DE1F12
P 6650 7250
F 0 "C2" H 6675 7350 50  0000 L CNN
F 1 ".1uf" H 6675 7150 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 6688 7100 50  0001 C CNN
F 3 "" H 6650 7250 50  0001 C CNN
	1    6650 7250
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C3
U 1 1 59DE203B
P 5750 7250
F 0 "C3" H 5775 7350 50  0000 L CNN
F 1 ".1uf" H 5775 7150 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 5788 7100 50  0001 C CNN
F 3 "" H 5750 7250 50  0001 C CNN
	1    5750 7250
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:+3.3V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR010
U 1 1 59DE2292
P 6400 7000
F 0 "#PWR010" H 6400 6850 50  0001 C CNN
F 1 "+3.3V" H 6400 7140 50  0000 C CNN
F 2 "" H 6400 7000 50  0001 C CNN
F 3 "" H 6400 7000 50  0001 C CNN
	1    6400 7000
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR011
U 1 1 59DE247C
P 6400 7500
F 0 "#PWR011" H 6400 7250 50  0001 C CNN
F 1 "GND" H 6400 7350 50  0000 C CNN
F 2 "" H 6400 7500 50  0001 C CNN
F 3 "" H 6400 7500 50  0001 C CNN
	1    6400 7500
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR012
U 1 1 59DE24EE
P 5500 7500
F 0 "#PWR012" H 5500 7250 50  0001 C CNN
F 1 "GND" H 5500 7350 50  0000 C CNN
F 2 "" H 5500 7500 50  0001 C CNN
F 3 "" H 5500 7500 50  0001 C CNN
	1    5500 7500
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR013
U 1 1 59DE2C5A
P 5500 7000
F 0 "#PWR013" H 5500 6850 50  0001 C CNN
F 1 "+5V" H 5500 7140 50  0000 C CNN
F 2 "" H 5500 7000 50  0001 C CNN
F 3 "" H 5500 7000 50  0001 C CNN
	1    5500 7000
	1    0    0    -1  
$EndComp
Text Label 9200 5650 1    60   ~ 0
49
Text Label 10400 2550 0    60   ~ 0
6
Text Label 2150 1050 0    60   ~ 0
PULL_SDA
Wire Wire Line
	9100 950  9100 1650
Wire Wire Line
	9250 1450 9250 1200
Wire Wire Line
	9350 1450 9250 1450
Wire Notes Line
	9850 650  9850 475 
Wire Notes Line
	8350 650  9850 650 
Wire Wire Line
	9100 1650 9350 1650
Wire Wire Line
	9000 1100 9000 1750
Wire Wire Line
	9000 1750 9350 1750
Wire Wire Line
	9350 2050 8900 2050
Wire Wire Line
	8900 2050 8900 1200
Wire Wire Line
	8600 1550 9350 1550
Wire Wire Line
	9350 1850 9250 1850
Wire Wire Line
	9250 1850 9250 1950
Wire Wire Line
	9350 1950 9250 1950
Connection ~ 9250 1950
Wire Wire Line
	10150 1150 10400 1150
Wire Wire Line
	10400 1250 10150 1250
Wire Wire Line
	10150 1350 10400 1350
Wire Wire Line
	10150 1550 10400 1550
Wire Wire Line
	10400 1650 10150 1650
Wire Wire Line
	10150 1750 10400 1750
Wire Wire Line
	10150 1850 10400 1850
Wire Wire Line
	10400 1950 10150 1950
Wire Wire Line
	10150 2050 10400 2050
Wire Wire Line
	10250 2150 10250 1450
Wire Wire Line
	10250 1450 10150 1450
Wire Wire Line
	9350 2450 8900 2450
Wire Wire Line
	8900 2550 9350 2550
Wire Wire Line
	9350 2650 8900 2650
Wire Wire Line
	8900 2750 9350 2750
Wire Wire Line
	9350 2850 8900 2850
Wire Wire Line
	8900 2950 9350 2950
Wire Wire Line
	9350 3050 8900 3050
Wire Wire Line
	8900 3150 9350 3150
Wire Wire Line
	10400 2450 10150 2450
Wire Wire Line
	10150 2550 10400 2550
Wire Wire Line
	10400 2650 10150 2650
Wire Wire Line
	10150 2750 10400 2750
Wire Wire Line
	10400 2850 10150 2850
Wire Wire Line
	10150 2950 10400 2950
Wire Wire Line
	10400 3050 10150 3050
Wire Wire Line
	10150 3150 10400 3150
Wire Wire Line
	9350 3400 8900 3400
Wire Wire Line
	8900 3500 9350 3500
Wire Wire Line
	9350 3600 8900 3600
Wire Wire Line
	8900 3700 9350 3700
Wire Wire Line
	9350 3800 8900 3800
Wire Wire Line
	8900 3900 9350 3900
Wire Wire Line
	9350 4000 8900 4000
Wire Wire Line
	8900 4100 9350 4100
Wire Wire Line
	10400 3400 10150 3400
Wire Wire Line
	10150 3500 10400 3500
Wire Wire Line
	10400 3600 10150 3600
Wire Wire Line
	10150 3700 10400 3700
Wire Wire Line
	10400 3800 10150 3800
Wire Wire Line
	10150 3900 10400 3900
Wire Wire Line
	10400 4000 10150 4000
Wire Wire Line
	10150 4100 10400 4100
Wire Wire Line
	10500 4850 10500 4650
Wire Wire Line
	10400 4850 10400 4650
Wire Wire Line
	10300 4850 10300 4650
Wire Wire Line
	10200 4850 10200 4650
Wire Wire Line
	10100 4850 10100 4650
Wire Wire Line
	10000 4850 10000 4650
Wire Wire Line
	9900 4850 9900 4650
Wire Wire Line
	9800 4850 9800 4650
Wire Wire Line
	9700 4850 9700 4650
Wire Wire Line
	9600 4850 9600 4650
Wire Wire Line
	9500 4850 9500 4650
Wire Wire Line
	9400 4850 9400 4650
Wire Wire Line
	9300 4850 9300 4650
Wire Wire Line
	9200 4850 9200 4650
Wire Wire Line
	9100 4850 9100 4650
Wire Wire Line
	9000 4850 9000 4650
Wire Wire Line
	10500 5350 10500 5650
Wire Wire Line
	10400 5350 10400 5650
Wire Wire Line
	10300 5350 10300 5650
Wire Wire Line
	10200 5350 10200 5650
Wire Wire Line
	10100 5350 10100 5650
Wire Wire Line
	10000 5350 10000 5650
Wire Wire Line
	9900 5350 9900 5650
Wire Wire Line
	9800 5350 9800 5650
Wire Wire Line
	9700 5350 9700 5650
Wire Wire Line
	9600 5350 9600 5650
Wire Wire Line
	9500 5350 9500 5650
Wire Wire Line
	9400 5350 9400 5650
Wire Wire Line
	9300 5350 9300 5650
Wire Wire Line
	9200 5350 9200 5650
Wire Wire Line
	9100 5350 9100 5750
Wire Wire Line
	9000 5350 9000 5750
Wire Wire Line
	8900 4850 8650 4850
Wire Wire Line
	8900 5350 8650 5350
Connection ~ 8650 5350
Wire Wire Line
	10750 5350 10600 5350
Wire Wire Line
	10750 4850 10600 4850
Connection ~ 10750 4850
Wire Wire Line
	10750 4550 10750 4850
Wire Wire Line
	8650 4850 8650 5350
Wire Notes Line
	11200 6050 8350 6050
Wire Notes Line
	8350 6050 8350 500 
Wire Wire Line
	2150 1650 2350 1650
Wire Wire Line
	2150 1750 2350 1750
Wire Wire Line
	2150 1250 2950 1250
Wire Wire Line
	2950 1250 2950 1150
Connection ~ 2950 1250
Wire Wire Line
	2150 1350 3150 1350
Wire Wire Line
	2150 1450 3150 1450
Wire Wire Line
	2950 850  2950 750 
Wire Wire Line
	2950 750  3100 750 
Wire Wire Line
	5050 2300 6150 2300
Wire Wire Line
	1250 1250 1000 1250
Wire Wire Line
	700  1250 700  1400
Wire Wire Line
	700  950  1150 950 
Wire Wire Line
	700  1050 1100 1050
Wire Wire Line
	700  1150 1050 1150
Wire Wire Line
	550  2500 2150 2500
Wire Wire Line
	550  2700 2150 2700
Wire Wire Line
	550  2900 2150 2900
Wire Wire Line
	600  3700 2150 3700
Wire Wire Line
	600  3900 2150 3900
Wire Wire Line
	600  4100 2150 4100
Wire Wire Line
	600  4300 1250 4300
Wire Wire Line
	1500 2400 2050 2400
Wire Wire Line
	2050 2400 2050 2600
Connection ~ 2050 2400
Wire Wire Line
	2150 2600 2050 2600
Connection ~ 2050 2600
Wire Wire Line
	2050 2800 2150 2800
Wire Wire Line
	3050 2400 3150 2400
Wire Wire Line
	3050 2550 3400 2550
Wire Wire Line
	3050 2700 3150 2700
Wire Wire Line
	3700 2550 3800 2550
Wire Wire Line
	3800 2400 3800 2550
Wire Wire Line
	3800 2700 3450 2700
Wire Wire Line
	3450 3600 3850 3600
Wire Wire Line
	3750 3750 3850 3750
Connection ~ 3850 3600
Wire Wire Line
	3850 3900 3450 3900
Connection ~ 3850 3750
Wire Wire Line
	3050 3900 3150 3900
Wire Wire Line
	3450 3750 3050 3750
Wire Wire Line
	3050 3600 3150 3600
Wire Wire Line
	2150 3000 2050 3000
Wire Wire Line
	3800 3250 2050 3250
Wire Wire Line
	3450 2400 3800 2400
Wire Wire Line
	3850 3600 3850 3750
Wire Wire Line
	1500 3600 2050 3600
Wire Wire Line
	2150 3800 2050 3800
Wire Wire Line
	2050 3600 2050 3800
Connection ~ 2050 3600
Wire Wire Line
	2050 4000 2150 4000
Connection ~ 2050 3800
Connection ~ 3800 2400
Wire Wire Line
	700  1750 700  1700
Wire Wire Line
	2050 3250 2050 3000
Wire Wire Line
	2050 4200 2150 4200
Wire Wire Line
	950  3450 600  3450
Wire Wire Line
	1250 3450 1250 4300
Connection ~ 1250 4300
Wire Wire Line
	1200 6900 1050 6900
Wire Wire Line
	1050 6800 1050 6900
Wire Wire Line
	1200 6800 1050 6800
Connection ~ 1050 6900
Wire Wire Line
	550  7100 750  7100
Wire Wire Line
	550  7200 950  7200
Wire Wire Line
	3500 7300 3000 7300
Wire Wire Line
	3950 6050 3700 6050
Wire Wire Line
	3350 6900 3000 6900
Wire Wire Line
	750  7100 750  6850
Connection ~ 750  7100
Wire Wire Line
	950  6850 950  7200
Connection ~ 950  7200
Wire Wire Line
	750  6550 750  6400
Wire Wire Line
	750  6400 950  6400
Wire Wire Line
	950  6400 950  6550
Connection ~ 950  6400
Wire Wire Line
	950  4500 650  4500
Wire Wire Line
	1250 4500 1250 4700
Connection ~ 1250 4700
Wire Wire Line
	5500 7000 5500 7050
Wire Wire Line
	5500 7400 5500 7450
Wire Wire Line
	6400 7000 6400 7050
Wire Wire Line
	6650 7050 6650 7100
Connection ~ 6400 7050
Wire Wire Line
	6400 7400 6400 7450
Wire Wire Line
	6650 7450 6650 7400
Connection ~ 6400 7450
Wire Wire Line
	6400 7050 6650 7050
Wire Wire Line
	6650 7450 6400 7450
Wire Wire Line
	5500 7050 5750 7050
Wire Wire Line
	5750 7050 5750 7100
Connection ~ 5500 7050
Wire Wire Line
	5500 7450 5750 7450
Wire Wire Line
	5750 7450 5750 7400
Connection ~ 5500 7450
Wire Wire Line
	1250 1450 1150 1450
Wire Wire Line
	1150 1450 1150 950 
Connection ~ 1150 950 
Wire Wire Line
	1250 1550 1100 1550
Wire Wire Line
	1100 1550 1100 1050
Connection ~ 1100 1050
Wire Wire Line
	1250 1650 1050 1650
Wire Wire Line
	1050 1650 1050 1150
Connection ~ 1050 1150
Wire Wire Line
	1250 1750 1000 1750
Wire Wire Line
	1000 1750 1000 1250
Connection ~ 1000 1250
Wire Wire Line
	3650 6250 3950 6250
Text Label 9200 4650 1    60   ~ 0
48
Wire Wire Line
	750  4700 1250 4700
Wire Wire Line
	2650 4500 2650 4800
Wire Wire Line
	2650 4800 2600 4800
Text Label 10400 1850 0    60   ~ 0
10
Text Label 5200 800  0    60   ~ 0
Batt+
Text Label 5200 900  0    60   ~ 0
Batt_SCL
Text Label 5200 1000 0    60   ~ 0
Batt_SDA
$Comp
L IO-Board-rescue:DT2042-04SO-7-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U7
U 1 1 59E5039F
P 4700 1150
F 0 "U7" H 4700 1100 60  0000 C CNN
F 1 "DT2042-04SO-7" H 4700 1600 60  0000 C CNN
F 2 "TO_SOT_Packages_SMD:SOT-23-6" H 4700 1150 60  0001 C CNN
F 3 "" H 4700 1150 60  0001 C CNN
	1    4700 1150
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C5
U 1 1 59E50BDF
P 3950 950
F 0 "C5" H 3975 1050 50  0000 L CNN
F 1 ".1uf" H 3975 850 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 3988 800 50  0001 C CNN
F 3 "" H 3950 950 50  0001 C CNN
	1    3950 950 
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR014
U 1 1 59E50BEB
P 3950 1200
F 0 "#PWR014" H 3950 950 50  0001 C CNN
F 1 "GND" H 3950 1050 50  0000 C CNN
F 2 "" H 3950 1200 50  0001 C CNN
F 3 "" H 3950 1200 50  0001 C CNN
	1    3950 1200
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR015
U 1 1 59E50BF1
P 3950 700
F 0 "#PWR015" H 3950 550 50  0001 C CNN
F 1 "+5V" H 3950 840 50  0000 C CNN
F 2 "" H 3950 700 50  0001 C CNN
F 3 "" H 3950 700 50  0001 C CNN
	1    3950 700 
	1    0    0    -1  
$EndComp
Wire Wire Line
	3950 700  3950 750 
Wire Wire Line
	3950 1100 3950 1150
Wire Wire Line
	3950 750  4200 750 
Connection ~ 3950 750 
Wire Wire Line
	3950 1150 4200 1150
Connection ~ 3950 1150
Wire Wire Line
	4250 800  4200 800 
Wire Wire Line
	4200 800  4200 750 
Wire Wire Line
	4250 1100 4200 1100
Wire Wire Line
	4200 1100 4200 1150
Wire Wire Line
	9250 1950 9250 2150
Wire Wire Line
	8650 5350 8650 5750
Wire Wire Line
	10750 4850 10750 5350
Wire Wire Line
	2950 1250 3150 1250
Wire Wire Line
	2050 2400 2150 2400
Wire Wire Line
	2050 2600 2050 2800
Wire Wire Line
	3850 3600 4050 3600
Wire Wire Line
	3850 3750 3850 3900
Wire Wire Line
	2050 3600 2150 3600
Wire Wire Line
	2050 3800 2050 4000
Wire Wire Line
	3800 2400 3950 2400
Wire Wire Line
	1250 4300 2150 4300
Wire Wire Line
	1050 6900 1050 7550
Wire Wire Line
	750  7100 1200 7100
Wire Wire Line
	950  7200 1200 7200
Wire Wire Line
	950  6400 1050 6400
Wire Wire Line
	1250 4700 1500 4700
Wire Wire Line
	6400 7050 6400 7100
Wire Wire Line
	6400 7450 6400 7500
Wire Wire Line
	5500 7050 5500 7100
Wire Wire Line
	5500 7450 5500 7500
Wire Wire Line
	1150 950  1250 950 
Wire Wire Line
	1100 1050 1250 1050
Wire Wire Line
	1050 1150 1250 1150
Wire Wire Line
	1000 1250 700  1250
Wire Wire Line
	3950 750  3950 800 
Wire Wire Line
	3950 1150 3950 1200
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R3
U 1 1 5C40CC5E
P 4100 3900
F 0 "R3" V 4180 3900 50  0000 C CNN
F 1 "10K" V 4100 3900 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 4030 3900 50  0001 C CNN
F 3 "" H 4100 3900 50  0001 C CNN
	1    4100 3900
	0    1    1    0   
$EndComp
Wire Wire Line
	3950 3900 3850 3900
Connection ~ 3850 3900
Wire Wire Line
	4250 3900 4350 3900
Text Label 4350 3900 0    50   ~ 0
SDA_Test
$Comp
L IO-Board-rescue:ISL21070DIH306Z-TK-Reference_Voltage-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U8
U 1 1 5C421C22
P 4550 7050
F 0 "U8" H 4320 7096 50  0000 R CNN
F 1 "NCP51460SN33T1G" H 4400 6700 50  0000 R CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 5050 6800 50  0001 C CIN
F 3 "http://www.intersil.com/content/dam/Intersil/documents/fn75/fn7599.pdf" H 4550 7050 50  0001 C CIN
	1    4550 7050
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR018
U 1 1 5C421FC3
P 4450 7500
F 0 "#PWR018" H 4450 7250 50  0001 C CNN
F 1 "GND" H 4450 7350 50  0000 C CNN
F 2 "" H 4450 7500 50  0001 C CNN
F 3 "" H 4450 7500 50  0001 C CNN
	1    4450 7500
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR017
U 1 1 5C42201C
P 4450 6700
F 0 "#PWR017" H 4450 6550 50  0001 C CNN
F 1 "+5V" H 4450 6840 50  0000 C CNN
F 2 "" H 4450 6700 50  0001 C CNN
F 3 "" H 4450 6700 50  0001 C CNN
	1    4450 6700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4850 7050 4900 7050
Text Label 5000 7050 0    50   ~ 0
AREF
Wire Wire Line
	4450 7350 4450 7450
Wire Wire Line
	4450 6750 4450 6700
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C6
U 1 1 5C45D730
P 6000 7250
F 0 "C6" H 6025 7350 50  0000 L CNN
F 1 ".1uf" H 6025 7150 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 6038 7100 50  0001 C CNN
F 3 "" H 6000 7250 50  0001 C CNN
	1    6000 7250
	1    0    0    -1  
$EndComp
Wire Wire Line
	5750 7050 6000 7050
Wire Wire Line
	6000 7050 6000 7100
Connection ~ 5750 7050
Wire Wire Line
	5750 7450 6000 7450
Wire Wire Line
	6000 7450 6000 7400
Connection ~ 5750 7450
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C7
U 1 1 5C486023
P 4900 7250
F 0 "C7" H 4925 7350 50  0000 L CNN
F 1 ".1uf" H 4925 7150 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 4938 7100 50  0001 C CNN
F 3 "" H 4900 7250 50  0001 C CNN
	1    4900 7250
	1    0    0    -1  
$EndComp
Wire Wire Line
	4900 7100 4900 7050
Connection ~ 4900 7050
Wire Wire Line
	4900 7050 5000 7050
Wire Wire Line
	4900 7400 4900 7450
Wire Wire Line
	4900 7450 4450 7450
Connection ~ 4450 7450
Wire Wire Line
	4450 7450 4450 7500
Text Label 9500 4650 1    60   ~ 0
13
$Comp
L BattProg:GL6 K1
U 1 1 5D8803F7
P 7650 1400
F 0 "K1" V 7083 1400 50  0000 C CNN
F 1 "G6L-1F" V 7174 1400 50  0000 C CNN
F 2 "BattProg:G6L-1F" H 8000 1350 50  0001 L CNN
F 3 "https://standexelectronics.com/wp-content/uploads/datasheet_reed_relay_DIP.pdf" H 7650 1400 50  0001 C CNN
	1    7650 1400
	0    1    1    0   
$EndComp
$Comp
L BattProg:GL6 K3
U 1 1 5D8804F9
P 7200 4600
F 0 "K3" V 6633 4600 50  0000 C CNN
F 1 "G6L-1F" V 6724 4600 50  0000 C CNN
F 2 "BattProg:G6L-1F" H 7550 4550 50  0001 L CNN
F 3 "https://standexelectronics.com/wp-content/uploads/datasheet_reed_relay_DIP.pdf" H 7200 4600 50  0001 C CNN
	1    7200 4600
	0    1    1    0   
$EndComp
$Comp
L BattProg:GL6 K2
U 1 1 5D880612
P 7200 3200
F 0 "K2" V 6633 3200 50  0000 C CNN
F 1 "G6L-1F" V 6724 3200 50  0000 C CNN
F 2 "BattProg:G6L-1F" H 7550 3150 50  0001 L CNN
F 3 "https://standexelectronics.com/wp-content/uploads/datasheet_reed_relay_DIP.pdf" H 7200 3200 50  0001 C CNN
	1    7200 3200
	0    1    1    0   
$EndComp
Wire Wire Line
	3350 6150 3950 6150
Wire Wire Line
	3000 7200 3650 7200
Wire Wire Line
	3650 6250 3650 7200
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q2
U 1 1 5D8BF25C
P 6550 2900
F 0 "Q2" V 6801 2900 50  0000 C CNN
F 1 "BSS806N" V 6892 2900 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 6750 2825 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 6550 2900 50  0001 L CNN
	1    6550 2900
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q3
U 1 1 5D8C052B
P 6550 4300
F 0 "Q3" V 6801 4300 50  0000 C CNN
F 1 "BSS806N" V 6892 4300 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 6750 4225 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 6550 4300 50  0001 L CNN
	1    6550 4300
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q4
U 1 1 5D9C4589
P 6550 5700
F 0 "Q4" V 6801 5700 50  0000 C CNN
F 1 "BSS806N" V 6892 5700 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 6750 5625 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 6550 5700 50  0001 L CNN
	1    6550 5700
	0    1    1    0   
$EndComp
Wire Wire Line
	7150 750  7100 750 
Wire Wire Line
	7450 2550 6800 2550
Wire Wire Line
	6800 2550 6800 3000
Wire Wire Line
	6800 3000 6900 3000
Wire Wire Line
	6750 3000 6800 3000
Connection ~ 6800 3000
Wire Wire Line
	6750 4400 6800 4400
Wire Wire Line
	7450 3850 6800 3850
Wire Wire Line
	6800 3850 6800 4400
Connection ~ 6800 4400
Wire Wire Line
	6800 4400 6900 4400
Wire Wire Line
	6750 5800 6800 5800
Wire Wire Line
	7450 5250 6800 5250
Wire Wire Line
	6800 5250 6800 5800
Connection ~ 6800 5800
Wire Wire Line
	6800 5800 6900 5800
Wire Wire Line
	7500 5800 8100 5800
Wire Wire Line
	8100 5800 8100 5250
Wire Wire Line
	8100 750  7450 750 
Wire Wire Line
	7950 1200 8100 1200
Wire Wire Line
	7500 3000 8100 3000
Connection ~ 8100 3000
Wire Wire Line
	8100 3000 8100 2550
Wire Wire Line
	7500 4400 8100 4400
Connection ~ 8100 4400
Wire Wire Line
	7750 5250 8100 5250
Connection ~ 8100 5250
Wire Wire Line
	8100 5250 8100 4400
Wire Wire Line
	7750 3850 8100 3850
Wire Wire Line
	8100 3000 8100 3850
Connection ~ 8100 3850
Wire Wire Line
	8100 3850 8100 4400
Wire Wire Line
	7750 2550 8100 2550
Connection ~ 8100 2550
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR020
U 1 1 5DC4A0C0
P 8100 700
F 0 "#PWR020" H 8100 550 50  0001 C CNN
F 1 "+5V" H 8100 840 50  0000 C CNN
F 2 "" H 8100 700 50  0000 C CNN
F 3 "" H 8100 700 50  0000 C CNN
	1    8100 700 
	1    0    0    -1  
$EndComp
Wire Wire Line
	6350 1600 6150 1600
Wire Wire Line
	6150 1600 6150 1750
Wire Wire Line
	6350 5800 6150 5800
Connection ~ 6150 5800
Wire Wire Line
	6150 5800 6150 5900
Wire Wire Line
	6350 4400 6150 4400
Connection ~ 6150 4400
Wire Wire Line
	6350 3000 6150 3000
Connection ~ 6150 3000
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR019
U 1 1 5DCB68BD
P 6150 6400
F 0 "#PWR019" H 6150 6150 50  0001 C CNN
F 1 "GND" H 6150 6250 50  0000 C CNN
F 2 "" H 6150 6400 50  0001 C CNN
F 3 "" H 6150 6400 50  0001 C CNN
	1    6150 6400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6900 3400 6050 3400
Wire Wire Line
	5950 2200 5950 4800
Wire Wire Line
	5950 4800 6900 4800
Wire Wire Line
	5050 2200 5950 2200
Wire Wire Line
	6550 1300 6550 1250
Text Label 5850 1250 2    50   ~ 0
Batt+_Rly
Wire Wire Line
	6550 2700 6550 2600
Text Label 5600 2600 2    50   ~ 0
SCL_Rly
Wire Wire Line
	6550 4100 6550 3700
Text Label 5600 5400 2    50   ~ 0
Therm_Rly
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R16
U 1 1 5DA9AEEB
P 5700 5650
F 0 "R16" V 5780 5650 50  0000 C CNN
F 1 "100K" V 5700 5650 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 5630 5650 50  0001 C CNN
F 3 "" H 5700 5650 50  0001 C CNN
	1    5700 5650
	1    0    0    -1  
$EndComp
Wire Wire Line
	6550 5500 6550 5400
Wire Wire Line
	6550 5400 5700 5400
Wire Wire Line
	5700 5500 5700 5400
Connection ~ 5700 5400
Wire Wire Line
	5700 5400 5600 5400
Wire Wire Line
	5700 5800 5700 5900
Wire Wire Line
	5700 5900 6150 5900
Connection ~ 6150 5900
Wire Wire Line
	6150 5900 6150 6400
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R15
U 1 1 5DBD7DCC
P 5700 3950
F 0 "R15" V 5780 3950 50  0000 C CNN
F 1 "100K" V 5700 3950 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 5630 3950 50  0001 C CNN
F 3 "" H 5700 3950 50  0001 C CNN
	1    5700 3950
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 4100 5700 4200
Wire Wire Line
	5700 4200 6150 4200
Wire Wire Line
	5600 3700 5700 3700
Wire Wire Line
	5700 3800 5700 3700
Connection ~ 5700 3700
Wire Wire Line
	5700 3700 6550 3700
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R14
U 1 1 5DD1A9EA
P 5700 2850
F 0 "R14" V 5780 2850 50  0000 C CNN
F 1 "100K" V 5700 2850 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 5630 2850 50  0001 C CNN
F 3 "" H 5700 2850 50  0001 C CNN
	1    5700 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 2600 5700 2600
Wire Wire Line
	5700 2700 5700 2600
Connection ~ 5700 2600
Wire Wire Line
	5700 2600 6550 2600
Wire Wire Line
	5700 3000 5700 3100
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R13
U 1 1 5DD964E8
P 5950 1500
F 0 "R13" V 6030 1500 50  0000 C CNN
F 1 "100K" V 5950 1500 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 5880 1500 50  0001 C CNN
F 3 "" H 5950 1500 50  0001 C CNN
	1    5950 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	5850 1250 5950 1250
Wire Wire Line
	5950 1350 5950 1250
Connection ~ 5950 1250
Wire Wire Line
	5950 1250 6550 1250
Wire Wire Line
	5950 1650 5950 1750
Wire Wire Line
	5950 1750 6150 1750
Connection ~ 6150 1750
$Comp
L BattProg:S1A D1
U 1 1 5DF403EA
P 7300 750
F 0 "D1" H 7300 966 50  0000 C CNN
F 1 "S1A" H 7300 875 50  0000 C CNN
F 2 "BattProg:DO-214AC" H 7300 750 50  0001 C CNN
F 3 "" H 7300 750 50  0001 C CNN
	1    7300 750 
	-1   0    0    1   
$EndComp
$Comp
L BattProg:S1A D2
U 1 1 5DF40690
P 7600 2550
F 0 "D2" H 7600 2766 50  0000 C CNN
F 1 "S1A" H 7600 2675 50  0000 C CNN
F 2 "BattProg:DO-214AC" H 7600 2550 50  0001 C CNN
F 3 "" H 7600 2550 50  0001 C CNN
	1    7600 2550
	-1   0    0    1   
$EndComp
$Comp
L BattProg:S1A D3
U 1 1 5DF40B55
P 7600 3850
F 0 "D3" H 7600 4066 50  0000 C CNN
F 1 "S1A" H 7600 3975 50  0000 C CNN
F 2 "BattProg:DO-214AC" H 7600 3850 50  0001 C CNN
F 3 "" H 7600 3850 50  0001 C CNN
	1    7600 3850
	-1   0    0    1   
$EndComp
$Comp
L BattProg:S1A D4
U 1 1 5E0835E6
P 7600 5250
F 0 "D4" H 7600 5466 50  0000 C CNN
F 1 "S1A" H 7600 5375 50  0000 C CNN
F 2 "BattProg:DO-214AC" H 7600 5250 50  0001 C CNN
F 3 "" H 7600 5250 50  0001 C CNN
	1    7600 5250
	-1   0    0    1   
$EndComp
Wire Wire Line
	6150 1750 6150 2150
Wire Wire Line
	5050 2100 6050 2100
Connection ~ 6150 2300
Wire Wire Line
	6150 2300 6150 2500
Wire Wire Line
	6900 6200 5850 6200
Text Label 7900 6100 2    50   ~ 0
THERM
Wire Wire Line
	6250 2150 6150 2150
Connection ~ 6150 2150
Wire Wire Line
	6150 2150 6150 2300
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C8
U 1 1 5E75D9DB
P 6600 2350
F 0 "C8" H 6625 2450 50  0000 L CNN
F 1 ".1uf" H 6625 2250 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 6638 2200 50  0001 C CNN
F 3 "" H 6600 2350 50  0001 C CNN
	1    6600 2350
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 2200 6600 2150
Connection ~ 6600 2150
Wire Wire Line
	6600 2150 6550 2150
Wire Wire Line
	6600 2500 6150 2500
Connection ~ 6150 2500
Wire Wire Line
	6150 2500 6150 3000
Text Label 7450 2150 0    50   ~ 0
BattDiv
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R17
U 1 1 5E6B2882
P 6400 2150
F 0 "R17" V 6480 2150 50  0000 C CNN
F 1 "6.81K" V 6300 2150 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 6330 2150 50  0001 C CNN
F 3 "" H 6400 2150 50  0001 C CNN
	1    6400 2150
	0    1    1    0   
$EndComp
Wire Wire Line
	7350 1200 7100 1200
Wire Wire Line
	7100 750  7100 1200
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q1
U 1 1 5D8C17C2
P 6550 1500
F 0 "Q1" V 6801 1500 50  0000 C CNN
F 1 "BSS806N" V 6892 1500 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 6750 1425 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 6550 1500 50  0001 L CNN
	1    6550 1500
	0    1    1    0   
$EndComp
Text Label 7150 1600 2    60   ~ 0
Batt+
Wire Wire Line
	8100 700  8100 750 
Connection ~ 8100 750 
Wire Wire Line
	8100 750  8100 1200
Connection ~ 8100 1200
Wire Wire Line
	8100 1200 8100 2550
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R22
U 1 1 5E6B1EA1
P 7250 1950
F 0 "R22" V 7330 1950 50  0000 C CNN
F 1 "3.65K" V 7150 1950 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 7180 1950 50  0001 C CNN
F 3 "" H 7250 1950 50  0001 C CNN
	1    7250 1950
	-1   0    0    1   
$EndComp
Wire Wire Line
	7250 1800 7250 1600
Wire Wire Line
	7250 1600 7350 1600
Wire Wire Line
	7250 2150 7250 2100
Wire Wire Line
	6600 2150 7250 2150
Wire Wire Line
	7250 2150 7450 2150
Connection ~ 7250 2150
Wire Wire Line
	7250 1600 7150 1600
Connection ~ 7250 1600
Wire Wire Line
	5050 2000 6950 2000
Wire Wire Line
	6950 2000 6950 1750
Wire Wire Line
	6950 1750 8000 1750
Wire Wire Line
	8000 1750 8000 1600
Wire Wire Line
	8000 1600 7950 1600
Wire Wire Line
	7100 1200 6800 1200
Wire Wire Line
	6800 1200 6800 1600
Wire Wire Line
	6800 1600 6750 1600
Connection ~ 7100 1200
Wire Wire Line
	6150 4400 6150 5800
Wire Wire Line
	3350 6150 3350 6900
Text Notes 6550 6500 0    60   ~ 0
Note: Relay contacts are rated at\n0.3 A at 125 VAC, 1 A at 24 VDC
Text Notes 4600 6800 0    50   ~ 0
3.3VDC 1%\nVoltage reference
Text Notes 4500 2450 0    60   ~ 0
THERM
$Comp
L IO-Board-rescue:Conn_01x05-Connector_Generic-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue J1
U 1 1 5EB5D224
P 4850 2200
F 0 "J1" H 4850 1900 50  0000 C CNN
F 1 "Conn_01x05" H 4850 2500 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x05_Pitch2.54mm" H 4850 2200 50  0001 C CNN
F 3 "~" H 4850 2200 50  0001 C CNN
	1    4850 2200
	-1   0    0    1   
$EndComp
Wire Wire Line
	6150 3000 6150 3100
Wire Wire Line
	6050 2100 6050 3400
Wire Wire Line
	5700 3100 6150 3100
Connection ~ 6150 3100
Wire Wire Line
	5050 2400 5850 2400
Wire Wire Line
	5850 2400 5850 6200
Wire Wire Line
	6150 3100 6150 4200
Connection ~ 6150 4200
Wire Wire Line
	6150 4200 6150 4400
Wire Wire Line
	7500 6200 7600 6200
$Comp
L BattProg:GL6 K4
U 1 1 5D8802E0
P 7200 6000
F 0 "K4" V 6633 6000 50  0000 C CNN
F 1 "G6L-1F" V 6724 6000 50  0000 C CNN
F 2 "BattProg:G6L-1F" H 7550 5950 50  0001 L CNN
F 3 "https://standexelectronics.com/wp-content/uploads/datasheet_reed_relay_DIP.pdf" H 7200 6000 50  0001 C CNN
	1    7200 6000
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R2
U 1 1 5EDE294A
P 7800 5900
F 0 "R2" V 7880 5900 50  0000 C CNN
F 1 "33K" V 7800 5900 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 7730 5900 50  0001 C CNN
F 3 "" H 7800 5900 50  0001 C CNN
	1    7800 5900
	0    1    1    0   
$EndComp
Wire Wire Line
	7650 5900 7600 5900
Wire Wire Line
	7600 5900 7600 6200
Connection ~ 7600 6200
Wire Wire Line
	7600 6200 7950 6200
Wire Wire Line
	7950 5900 8100 5900
Text Label 8100 5900 0    50   ~ 0
AREF
Text Label 5200 1100 0    60   ~ 0
THERM
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C9
U 1 1 5EAEEB28
P 8200 6200
F 0 "C9" V 8250 6250 50  0000 L CNN
F 1 ".1uf" V 8250 6000 50  0000 L CNN
F 2 "SMD_Packages:SMD-1206_Pol" H 8238 6050 50  0001 C CNN
F 3 "" H 8200 6200 50  0001 C CNN
	1    8200 6200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7950 6200 7950 6100
Wire Wire Line
	7950 6100 7900 6100
Connection ~ 7950 6200
Wire Wire Line
	7950 6200 8050 6200
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR06
U 1 1 5EBCAB11
P 8450 6300
F 0 "#PWR06" H 8450 6050 50  0001 C CNN
F 1 "GND" H 8450 6150 50  0000 C CNN
F 2 "" H 8450 6300 50  0000 C CNN
F 3 "" H 8450 6300 50  0000 C CNN
	1    8450 6300
	1    0    0    -1  
$EndComp
Wire Wire Line
	8350 6200 8450 6200
Wire Wire Line
	8450 6200 8450 6300
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q5
U 1 1 6044BDA4
P 1950 5650
F 0 "Q5" V 2201 5650 50  0000 C CNN
F 1 "BSS806N" V 2292 5650 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 2150 5575 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 1950 5650 50  0001 L CNN
	1    1950 5650
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q6
U 1 1 6044D142
P 2350 6000
F 0 "Q6" V 2601 6000 50  0000 C CNN
F 1 "BSS806N" V 2692 6000 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 2550 5925 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 2350 6000 50  0001 L CNN
	1    2350 6000
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R23
U 1 1 6044D78D
P 1350 5500
F 0 "R23" V 1430 5500 50  0000 C CNN
F 1 "100K" V 1350 5500 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 1280 5500 50  0001 C CNN
F 3 "" H 1350 5500 50  0001 C CNN
	1    1350 5500
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R24
U 1 1 6044DA97
P 1600 5500
F 0 "R24" V 1680 5500 50  0000 C CNN
F 1 "100K" V 1600 5500 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 1530 5500 50  0001 C CNN
F 3 "" H 1600 5500 50  0001 C CNN
	1    1600 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	550  3100 1200 3100
Connection ~ 1200 3100
Wire Wire Line
	1200 3100 2150 3100
Wire Wire Line
	900  2250 550  2250
Wire Wire Line
	3800 2700 3800 3250
Wire Wire Line
	3800 2550 3800 2700
Connection ~ 3800 2550
Connection ~ 3800 2700
Text Label 750  4700 0    60   ~ 0
SS
Wire Wire Line
	2050 4500 2650 4500
Wire Wire Line
	2050 4500 2050 4200
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R18
U 1 1 59DE1C33
P 1050 2250
F 0 "R18" V 1130 2250 50  0000 C CNN
F 1 "100K" V 1050 2250 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 980 2250 50  0001 C CNN
F 3 "" H 1050 2250 50  0001 C CNN
	1    1050 2250
	0    1    1    0   
$EndComp
Wire Wire Line
	1200 2250 1200 3100
Connection ~ 4700 4050
Wire Wire Line
	4700 4050 4700 2850
Connection ~ 4700 2850
Text Label 5600 3700 2    50   ~ 0
SDA_Rly
Text Label 5100 4050 0    50   ~ 0
SCL_Test
Wire Wire Line
	5050 4050 5100 4050
Wire Wire Line
	4750 4050 4700 4050
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R12
U 1 1 5C40CB84
P 4900 4050
F 0 "R12" V 4980 4050 50  0000 C CNN
F 1 "10K" V 4900 4050 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 4830 4050 50  0001 C CNN
F 3 "" H 4900 4050 50  0001 C CNN
	1    4900 4050
	0    1    1    0   
$EndComp
Wire Wire Line
	4700 2850 4800 2850
Wire Wire Line
	3050 4050 4700 4050
Wire Wire Line
	3050 2850 4700 2850
Text Label 4800 2850 0    60   ~ 0
Batt_SCL
Wire Wire Line
	2600 4700 3500 4700
Wire Wire Line
	3500 4700 3500 7300
$Comp
L IO-Board-rescue:+3.3V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0101
U 1 1 60D79DA4
P 1350 5200
F 0 "#PWR0101" H 1350 5050 50  0001 C CNN
F 1 "+3.3V" H 1350 5340 50  0000 C CNN
F 2 "" H 1350 5200 50  0001 C CNN
F 3 "" H 1350 5200 50  0001 C CNN
	1    1350 5200
	1    0    0    -1  
$EndComp
Wire Wire Line
	1350 5200 1350 5250
Wire Wire Line
	1350 5250 1500 5250
Connection ~ 1350 5250
Wire Wire Line
	1350 5250 1350 5350
Wire Wire Line
	1600 5350 1600 5250
Connection ~ 1600 5250
Wire Wire Line
	950  5750 1100 5750
Wire Wire Line
	1350 5650 1350 5750
Connection ~ 1350 5750
Wire Wire Line
	1350 5750 1750 5750
Wire Wire Line
	1600 5650 1600 6100
Wire Wire Line
	1600 6100 2150 6100
Wire Wire Line
	1950 5250 1950 5450
Wire Wire Line
	1950 5250 2350 5250
Wire Wire Line
	2350 5250 2350 5800
Connection ~ 1950 5250
Wire Wire Line
	2150 5750 2700 5750
Wire Wire Line
	2550 6100 2700 6100
Text Label 2700 6100 0    60   ~ 0
Batt_SDA
Text Label 2700 5750 0    60   ~ 0
HV_SCL
Text Label 950  6100 2    60   ~ 0
Mega_SDA
Connection ~ 1600 6100
Wire Wire Line
	1600 5250 1950 5250
Wire Wire Line
	1500 4900 1500 5250
Connection ~ 1500 5250
Wire Wire Line
	1500 5250 1600 5250
Wire Wire Line
	1600 6100 950  6100
Wire Wire Line
	1500 4800 1100 4800
Wire Wire Line
	1100 4800 1100 5750
Connection ~ 1100 5750
Wire Wire Line
	1100 5750 1350 5750
$EndSCHEMATC
