EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr USLetter 11000 8500
encoding utf-8
Sheet 1 1
Title "Battery EEPROM Programmer V2"
Date "2021-03-23"
Rev "1.2"
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L IO-Board-rescue:MAX4614-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U1
U 1 1 59D7A4B7
P 2100 3100
F 0 "U1" H 2100 2550 60  0000 C CNN
F 1 "MAX4614" H 2100 3450 60  0000 C CNN
F 2 "SO:SOIC-14_3.9x8.7mm_P1.27mm" H 2100 3100 60  0001 C CNN
F 3 "" H 2100 3100 60  0001 C CNN
	1    2100 3100
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:MAX4614-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U2
U 1 1 59D7A500
P 2100 4300
F 0 "U2" H 2100 3750 60  0000 C CNN
F 1 "MAX4614" H 2100 4650 60  0000 C CNN
F 2 "SO:SOIC-14_3.9x8.7mm_P1.27mm" H 2100 4300 60  0001 C CNN
F 3 "" H 2100 4300 60  0001 C CNN
	1    2100 4300
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:SN74LVC2G17-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U3
U 1 1 59D7A862
P 2100 6100
F 0 "U3" H 2100 5900 60  0000 C CNN
F 1 "SN74LVC2G17" H 2100 6300 60  0000 C CNN
F 2 "TO/SOT SMD:SOT-23-6_Handsoldering" H 2100 5950 60  0001 C CNN
F 3 "" H 2100 5950 60  0001 C CNN
	1    2100 6100
	1    0    0    -1  
$EndComp
Text Label 2100 1200 0    60   ~ 0
PULL_SEL
Text Notes 4500 3200 0    60   ~ 0
Batt+
Text Notes 4550 3300 0    60   ~ 0
SCL
Text Notes 4550 3400 0    60   ~ 0
SDA
Text Notes 4550 3500 0    60   ~ 0
GND
Text Notes 4650 3000 0    60   ~ 0
Battery\nConnector
Text Label 7400 3350 0    60   ~ 0
Batt_SCL
Text Label 7400 4750 0    60   ~ 0
Batt_SDA
Text Label 900  1050 2    60   ~ 0
+3.3V
Text Label 900  1150 2    60   ~ 0
Batt+
Text Label 2100 1050 0    60   ~ 0
PULL_VOLT
Text Label 1250 2950 2    60   ~ 0
PU_EN0
Text Label 1250 3150 2    60   ~ 0
PU_EN1
Text Label 1250 3550 2    60   ~ 0
PAS_CLK_EN
Text Label 1250 4150 2    60   ~ 0
PU_EN0
Text Label 1250 4350 2    60   ~ 0
PU_EN1
Text Label 1250 4550 2    60   ~ 0
PU_EN2
Text Label 1250 4750 2    60   ~ 0
ACT_CLK_EN
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R4
U 1 1 59DC5169
P 2750 2850
F 0 "R4" V 2830 2850 50  0000 C CNN
F 1 "10K" V 2750 2850 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 2680 2850 50  0001 C CNN
F 3 "" H 2750 2850 50  0001 C CNN
	1    2750 2850
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R5
U 1 1 59DC51EA
P 2950 3000
F 0 "R5" V 3030 3000 50  0000 C CNN
F 1 "6.8K" V 2950 3000 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 2880 3000 50  0001 C CNN
F 3 "" H 2950 3000 50  0001 C CNN
	1    2950 3000
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R6
U 1 1 59DC5279
P 2750 3150
F 0 "R6" V 2830 3150 50  0000 C CNN
F 1 "5.1K" V 2750 3150 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 2680 3150 50  0001 C CNN
F 3 "" H 2750 3150 50  0001 C CNN
	1    2750 3150
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R7
U 1 1 59DC5335
P 2750 4050
F 0 "R7" V 2830 4050 50  0000 C CNN
F 1 "10K" V 2750 4050 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 2680 4050 50  0001 C CNN
F 3 "" H 2750 4050 50  0001 C CNN
	1    2750 4050
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R8
U 1 1 59DC5392
P 2950 4200
F 0 "R8" V 3030 4200 50  0000 C CNN
F 1 "6.8K" V 2950 4200 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 2880 4200 50  0001 C CNN
F 3 "" H 2950 4200 50  0001 C CNN
	1    2950 4200
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R9
U 1 1 59DC53E9
P 2750 4350
F 0 "R9" V 2830 4350 50  0000 C CNN
F 1 "5.1K" V 2750 4350 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 2680 4350 50  0001 C CNN
F 3 "" H 2750 4350 50  0001 C CNN
	1    2750 4350
	0    1    1    0   
$EndComp
Text Label 3300 4050 0    60   ~ 0
Batt_SDA
Text Label 2550 3450 0    60   ~ 0
+5V
Text Label 2550 3550 0    60   ~ 0
GND
Text Label 2550 4750 0    60   ~ 0
GND
Text Label 2550 4650 0    60   ~ 0
+5V
Text Label 1250 2800 2    60   ~ 0
PULL_VOLT
Text Label 1250 4000 2    60   ~ 0
PULL_VOLT
Text Label 3300 2850 0    60   ~ 0
HV_SCL
Text Label 900  7050 2    60   ~ 0
SCL
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R10
U 1 1 59DE7703
P 1350 5000
F 0 "R10" V 1430 5000 50  0000 C CNN
F 1 "100K" V 1350 5000 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 1280 5000 50  0001 C CNN
F 3 "" H 1350 5000 50  0001 C CNN
	1    1350 5000
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:ATECC508A-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue U6
U 1 1 59DCCE85
P 9300 4950
F 0 "U6" H 9300 4750 60  0000 C CNN
F 1 "ATECC508A" H 9300 5250 60  0000 C CNN
F 2 "SO:SOIC-8_3.9x4.9mm_P1.27mm" H 9350 4950 60  0001 C CNN
F 3 "" H 9350 4950 60  0001 C CNN
	1    9300 4950
	1    0    0    -1  
$EndComp
Text Label 9750 4750 0    60   ~ 0
+3.3V
Text Label 9750 4850 0    60   ~ 0
SCL
Text Label 9750 4950 0    60   ~ 0
SDA
Text Label 9750 5050 0    60   ~ 0
GND
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C4
U 1 1 59DE1DC2
P 4750 7350
F 0 "C4" H 4775 7450 50  0000 L CNN
F 1 ".1uf" H 4775 7250 50  0000 L CNN
F 2 "CapSMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 4788 7200 50  0001 C CNN
F 3 "" H 4750 7350 50  0001 C CNN
	1    4750 7350
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C1
U 1 1 59DE1E5B
P 5650 7350
F 0 "C1" H 5675 7450 50  0000 L CNN
F 1 ".1uf" H 5675 7250 50  0000 L CNN
F 2 "CapSMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 5688 7200 50  0001 C CNN
F 3 "" H 5650 7350 50  0001 C CNN
	1    5650 7350
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C2
U 1 1 59DE1F12
P 5900 7350
F 0 "C2" H 5925 7450 50  0000 L CNN
F 1 ".1uf" H 5925 7250 50  0000 L CNN
F 2 "CapSMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 5938 7200 50  0001 C CNN
F 3 "" H 5900 7350 50  0001 C CNN
	1    5900 7350
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:+3.3V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR010
U 1 1 59DE2292
P 5650 7100
F 0 "#PWR010" H 5650 6950 50  0001 C CNN
F 1 "+3.3V" H 5650 7240 50  0000 C CNN
F 2 "" H 5650 7100 50  0001 C CNN
F 3 "" H 5650 7100 50  0001 C CNN
	1    5650 7100
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR011
U 1 1 59DE247C
P 5650 7600
F 0 "#PWR011" H 5650 7350 50  0001 C CNN
F 1 "GND" H 5650 7450 50  0000 C CNN
F 2 "" H 5650 7600 50  0001 C CNN
F 3 "" H 5650 7600 50  0001 C CNN
	1    5650 7600
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR012
U 1 1 59DE24EE
P 4750 7600
F 0 "#PWR012" H 4750 7350 50  0001 C CNN
F 1 "GND" H 4750 7450 50  0000 C CNN
F 2 "" H 4750 7600 50  0001 C CNN
F 3 "" H 4750 7600 50  0001 C CNN
	1    4750 7600
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR013
U 1 1 59DE2C5A
P 4750 7100
F 0 "#PWR013" H 4750 6950 50  0001 C CNN
F 1 "+5V" H 4750 7240 50  0000 C CNN
F 2 "" H 4750 7100 50  0001 C CNN
F 3 "" H 4750 7100 50  0001 C CNN
	1    4750 7100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5000 3450 5350 3450
Wire Wire Line
	1250 2950 1650 2950
Wire Wire Line
	1250 3150 1650 3150
Wire Wire Line
	1250 3350 1650 3350
Wire Wire Line
	1250 4150 1650 4150
Wire Wire Line
	1250 4350 1650 4350
Wire Wire Line
	1250 4550 1650 4550
Wire Wire Line
	1550 2850 1550 3050
Wire Wire Line
	1650 3050 1550 3050
Connection ~ 1550 3050
Wire Wire Line
	1550 3250 1650 3250
Wire Wire Line
	2550 2850 2600 2850
Wire Wire Line
	2550 3000 2800 3000
Wire Wire Line
	2550 3150 2600 3150
Wire Wire Line
	3100 3000 3200 3000
Wire Wire Line
	3200 2850 3200 3000
Wire Wire Line
	3200 3150 2900 3150
Wire Wire Line
	2900 4050 3200 4050
Wire Wire Line
	3100 4200 3200 4200
Connection ~ 3200 4050
Wire Wire Line
	3200 4350 2900 4350
Connection ~ 3200 4200
Wire Wire Line
	2550 4350 2600 4350
Wire Wire Line
	2800 4200 2550 4200
Wire Wire Line
	2550 4050 2600 4050
Wire Wire Line
	1650 3450 1550 3450
Wire Wire Line
	3200 3700 1550 3700
Wire Wire Line
	2900 2850 3200 2850
Wire Wire Line
	3200 4050 3200 4200
Wire Wire Line
	1650 4250 1600 4250
Wire Wire Line
	1600 4050 1600 4250
Wire Wire Line
	1600 4450 1650 4450
Connection ~ 1600 4250
Connection ~ 3200 2850
Wire Wire Line
	1550 3700 1550 3450
Wire Wire Line
	1550 4650 1650 4650
Wire Wire Line
	1350 5150 1350 5200
Wire Wire Line
	1350 4850 1350 4750
Wire Wire Line
	4750 7100 4750 7150
Wire Wire Line
	4750 7500 4750 7550
Wire Wire Line
	5650 7100 5650 7150
Wire Wire Line
	5900 7150 5900 7200
Connection ~ 5650 7150
Wire Wire Line
	5650 7500 5650 7550
Wire Wire Line
	5900 7550 5900 7500
Connection ~ 5650 7550
Wire Wire Line
	5650 7150 5900 7150
Wire Wire Line
	5900 7550 5650 7550
Connection ~ 4750 7150
Connection ~ 4750 7550
Wire Wire Line
	2750 5300 2750 6100
Wire Wire Line
	2750 6100 2650 6100
Wire Wire Line
	1550 2850 1650 2850
Wire Wire Line
	1550 3050 1550 3250
Wire Wire Line
	3200 4050 3300 4050
Wire Wire Line
	3200 4200 3200 4350
Wire Wire Line
	1600 4050 1650 4050
Wire Wire Line
	1600 4250 1600 4450
Wire Wire Line
	3200 2850 3300 2850
Wire Wire Line
	5650 7150 5650 7200
Wire Wire Line
	5650 7550 5650 7600
Wire Wire Line
	4750 7150 4750 7200
Wire Wire Line
	4750 7550 4750 7600
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR020
U 1 1 5DC4A0C0
P 7900 2300
F 0 "#PWR020" H 7900 2150 50  0001 C CNN
F 1 "+5V" H 7900 2440 50  0000 C CNN
F 2 "" H 7900 2300 50  0000 C CNN
F 3 "" H 7900 2300 50  0000 C CNN
	1    7900 2300
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR019
U 1 1 5DCB68BD
P 5350 3750
F 0 "#PWR019" H 5350 3500 50  0001 C CNN
F 1 "GND" H 5350 3600 50  0000 C CNN
F 2 "" H 5350 3750 50  0001 C CNN
F 3 "" H 5350 3750 50  0001 C CNN
	1    5350 3750
	1    0    0    -1  
$EndComp
Wire Wire Line
	5500 3350 5500 4650
Wire Wire Line
	5500 4650 6750 4650
Wire Wire Line
	5000 3350 5500 3350
Wire Wire Line
	6200 2150 6200 1900
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R13
U 1 1 5DD964E8
P 5700 2150
F 0 "R13" V 5780 2150 50  0000 C CNN
F 1 "100K" V 5700 2150 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 5630 2150 50  0001 C CNN
F 3 "" H 5700 2150 50  0001 C CNN
	1    5700 2150
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 1900 5700 1900
Wire Wire Line
	5700 2000 5700 1900
Connection ~ 5700 1900
Wire Wire Line
	5700 1900 6200 1900
Wire Wire Line
	5700 2300 5700 2450
$Comp
L BattProg:S1A D1
U 1 1 5DF403EA
P 7050 3650
F 0 "D1" H 7050 3750 50  0000 C CNN
F 1 "S1A" H 7050 3550 50  0000 C CNN
F 2 "CapSMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 7050 3650 50  0001 C CNN
F 3 "" H 7050 3650 50  0001 C CNN
	1    7050 3650
	-1   0    0    1   
$EndComp
Wire Wire Line
	6750 5050 5200 5050
Text Label 7500 5350 0    50   ~ 0
THERM
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C6
U 1 1 5E75D9DB
P 6100 4000
F 0 "C6" H 6125 4100 50  0000 L CNN
F 1 ".1uf" H 6125 3900 50  0000 L CNN
F 2 "CapSMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 6138 3850 50  0001 C CNN
F 3 "" H 6100 4000 50  0001 C CNN
	1    6100 4000
	1    0    0    -1  
$EndComp
Text Label 6150 3800 0    50   ~ 0
BattDiv
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R15
U 1 1 5E6B2882
P 5800 4000
F 0 "R15" V 5880 4000 50  0000 C CNN
F 1 "6.81K" V 5700 4000 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 5730 4000 50  0001 C CNN
F 3 "" H 5800 4000 50  0001 C CNN
	1    5800 4000
	-1   0    0    1   
$EndComp
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q1
U 1 1 5D8C17C2
P 6200 2350
F 0 "Q1" V 6451 2350 50  0000 C CNN
F 1 "BSS806N" V 6542 2350 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 6400 2275 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 6200 2350 50  0001 L CNN
	1    6200 2350
	0    1    1    0   
$EndComp
Text Label 7400 2950 0    60   ~ 0
Batt+
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R14
U 1 1 5E6B1EA1
P 5800 3550
F 0 "R14" V 5880 3550 50  0000 C CNN
F 1 "3.65K" V 5700 3550 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 5730 3550 50  0001 C CNN
F 3 "" H 5800 3550 50  0001 C CNN
	1    5800 3550
	-1   0    0    1   
$EndComp
Wire Wire Line
	6550 2450 6400 2450
Text Notes 4450 3600 0    60   ~ 0
THERM
$Comp
L IO-Board-rescue:Conn_01x05-Connector_Generic-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue J1
U 1 1 5EB5D224
P 4800 3350
F 0 "J1" H 4800 3050 50  0000 C CNN
F 1 "Conn_01x05" H 4800 3650 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical" H 4800 3350 50  0001 C CNN
F 3 "~" H 4800 3350 50  0001 C CNN
	1    4800 3350
	-1   0    0    1   
$EndComp
Wire Wire Line
	7350 5150 7400 5150
Wire Wire Line
	7500 5150 7400 5150
Connection ~ 7400 5150
Wire Wire Line
	7400 5150 7400 5350
Wire Wire Line
	7800 5150 7900 5150
Text Label 7900 5150 0    50   ~ 0
AREF
Connection ~ 7400 5350
Wire Wire Line
	7400 5350 7400 5500
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR06
U 1 1 5EBCAB11
P 7400 5850
F 0 "#PWR06" H 7400 5600 50  0001 C CNN
F 1 "GND" H 7400 5700 50  0000 C CNN
F 2 "" H 7400 5850 50  0000 C CNN
F 3 "" H 7400 5850 50  0000 C CNN
	1    7400 5850
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q3
U 1 1 6044BDA4
P 1900 6950
F 0 "Q3" V 2151 6950 50  0000 C CNN
F 1 "BSS806N" V 2242 6950 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 2100 6875 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 1900 6950 50  0001 L CNN
	1    1900 6950
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:2N7002-Transistor_FET-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue Q2
U 1 1 6044D142
P 2300 7300
F 0 "Q2" V 2551 7300 50  0000 C CNN
F 1 "BSS806N" V 2642 7300 50  0000 C CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 2500 7225 50  0001 L CIN
F 3 "https://www.fairchildsemi.com/datasheets/2N/2N7002.pdf" H 2300 7300 50  0001 L CNN
	1    2300 7300
	0    1    1    0   
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R12
U 1 1 6044D78D
P 1300 6800
F 0 "R12" V 1380 6800 50  0000 C CNN
F 1 "20K" V 1300 6800 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 1230 6800 50  0001 C CNN
F 3 "" H 1300 6800 50  0001 C CNN
	1    1300 6800
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R11
U 1 1 6044DA97
P 1550 6800
F 0 "R11" V 1630 6800 50  0000 C CNN
F 1 "20K" V 1550 6800 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 1480 6800 50  0001 C CNN
F 3 "" H 1550 6800 50  0001 C CNN
	1    1550 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 3150 3200 3700
Wire Wire Line
	3200 3000 3200 3150
Connection ~ 3200 3000
Connection ~ 3200 3150
Wire Wire Line
	1550 5300 2750 5300
Wire Wire Line
	1550 5300 1550 4650
Wire Wire Line
	3750 4500 3750 3300
Wire Wire Line
	3750 4500 3850 4500
Wire Wire Line
	2550 4500 3750 4500
Wire Wire Line
	2550 3300 3750 3300
Text Label 3850 4500 0    60   ~ 0
Batt_SCL
$Comp
L IO-Board-rescue:+3.3V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0101
U 1 1 60D79DA4
P 1300 6500
F 0 "#PWR0101" H 1300 6350 50  0001 C CNN
F 1 "+3.3V" H 1300 6640 50  0000 C CNN
F 2 "" H 1300 6500 50  0001 C CNN
F 3 "" H 1300 6500 50  0001 C CNN
	1    1300 6500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 6500 1300 6550
Connection ~ 1300 6550
Wire Wire Line
	1300 6550 1300 6650
Wire Wire Line
	1550 6650 1550 6550
Connection ~ 1550 6550
Wire Wire Line
	900  7050 1050 7050
Wire Wire Line
	1300 6950 1300 7050
Connection ~ 1300 7050
Wire Wire Line
	1300 7050 1700 7050
Wire Wire Line
	1550 6950 1550 7400
Wire Wire Line
	1550 7400 2100 7400
Wire Wire Line
	1900 6550 1900 6750
Wire Wire Line
	1900 6550 2300 6550
Wire Wire Line
	2300 6550 2300 7100
Connection ~ 1900 6550
Wire Wire Line
	2100 7050 2650 7050
Wire Wire Line
	2500 7400 2650 7400
Text Label 2650 7400 0    60   ~ 0
Batt_SDA
Text Label 2650 7050 0    60   ~ 0
HV_SCL
Text Label 900  7400 2    60   ~ 0
SDA
Connection ~ 1550 7400
Wire Wire Line
	1550 6550 1900 6550
Wire Wire Line
	1550 7400 900  7400
Wire Wire Line
	1550 6100 1050 6100
Connection ~ 1050 7050
Wire Wire Line
	1050 7050 1300 7050
Text Label 1250 3350 2    60   ~ 0
PU_EN2
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0102
U 1 1 60583D9A
P 1350 5200
F 0 "#PWR0102" H 1350 4950 50  0001 C CNN
F 1 "GND" H 1350 5050 50  0000 C CNN
F 2 "" H 1350 5200 50  0001 C CNN
F 3 "" H 1350 5200 50  0001 C CNN
	1    1350 5200
	1    0    0    -1  
$EndComp
Text Label 8750 3200 2    60   ~ 0
SCL
Text Label 8750 3100 2    60   ~ 0
SDA
Text Label 8900 1900 2    60   ~ 0
ACT_CLK_EN
Text Label 8900 2000 2    60   ~ 0
PAS_CLK_EN
Text Label 8900 1800 2    60   ~ 0
PU_EN2
Text Label 8900 1600 2    60   ~ 0
PU_EN0
Text Label 8900 1500 2    60   ~ 0
PULL_SEL
Text Label 10150 1900 0    60   ~ 0
AREF
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR01
U 1 1 56D71D10
P 9850 1000
F 0 "#PWR01" H 9850 850 50  0001 C CNN
F 1 "+5V" H 9850 1140 50  0000 C CNN
F 2 "" H 9850 1000 50  0000 C CNN
F 3 "" H 9850 1000 50  0000 C CNN
	1    9850 1000
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR02
U 1 1 56D721E6
P 9550 3650
F 0 "#PWR02" H 9550 3400 50  0001 C CNN
F 1 "GND" H 9550 3500 50  0000 C CNN
F 2 "" H 9550 3650 50  0000 C CNN
F 3 "" H 9550 3650 50  0000 C CNN
	1    9550 3650
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:+3.3V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR09
U 1 1 59DD96EA
P 9650 950
F 0 "#PWR09" H 9650 800 50  0001 C CNN
F 1 "+3.3V" H 9650 1090 50  0000 C CNN
F 2 "" H 9650 950 50  0001 C CNN
F 3 "" H 9650 950 50  0001 C CNN
	1    9650 950 
	1    0    0    -1  
$EndComp
Text Label 10150 2200 0    60   ~ 0
THERM
Wire Wire Line
	9550 3600 9550 3650
Wire Wire Line
	9650 1100 9650 950 
Wire Wire Line
	9750 1100 9750 1050
Wire Wire Line
	9750 1050 9850 1050
Wire Wire Line
	9850 1050 9850 1000
Text Label 10150 2100 0    50   ~ 0
BattDiv
Wire Wire Line
	10050 2100 10150 2100
Wire Wire Line
	10150 2200 10050 2200
Wire Wire Line
	10050 1900 10150 1900
Wire Wire Line
	8750 3100 9050 3100
Wire Wire Line
	8750 3200 9050 3200
Text Label 8900 1700 2    60   ~ 0
PU_EN1
Wire Wire Line
	8900 2000 9050 2000
Wire Wire Line
	9050 1900 8900 1900
Wire Wire Line
	8900 1800 9050 1800
Wire Wire Line
	9050 1700 8900 1700
Wire Wire Line
	8900 1600 9050 1600
Wire Wire Line
	9050 1500 8900 1500
$Comp
L Modules:Adafruit_Feather_M4_Express A1
U 1 1 604929CD
P 9550 2300
F 0 "A1" H 9250 3450 50  0000 C CNN
F 1 "Adafruit_Feather_M4_Express" H 8700 3800 50  0000 C CNN
F 2 "Feather:Adafruit_Feather" H 9650 950 50  0001 L CNN
F 3 "https://cdn-learn.adafruit.com/downloads/pdf/adafruit-feather.pdf" H 9550 1500 50  0001 C CNN
	1    9550 2300
	1    0    0    -1  
$EndComp
Text Label 8900 2100 2    60   ~ 0
Batt_Rly
Wire Wire Line
	9050 2100 8900 2100
Text Notes 8550 4550 0    60   ~ 0
Note:Generic 508A setup as config\nstorage and "Math coprocessor"
Text Notes 8000 1050 0    60   ~ 0
120MHz Cotex M4, 512KB Flash,\n192KB RAM, 2MB QSPI Flash
Wire Wire Line
	1250 4750 1350 4750
Connection ~ 1350 4750
Wire Wire Line
	1350 4750 1650 4750
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0105
U 1 1 608EC656
P 2750 6300
F 0 "#PWR0105" H 2750 6050 50  0001 C CNN
F 1 "GND" H 2750 6150 50  0000 C CNN
F 2 "" H 2750 6300 50  0001 C CNN
F 3 "" H 2750 6300 50  0001 C CNN
	1    2750 6300
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 6200 2750 6200
Wire Wire Line
	2750 6200 2750 6300
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R16
U 1 1 5EDE294A
P 7650 5150
F 0 "R16" V 7730 5150 50  0000 C CNN
F 1 "33K" V 7650 5150 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 7580 5150 50  0001 C CNN
F 3 "" H 7650 5150 50  0001 C CNN
	1    7650 5150
	0    -1   -1   0   
$EndComp
Wire Wire Line
	7400 5350 7500 5350
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C5
U 1 1 5EAEEB28
P 7400 5650
F 0 "C5" V 7450 5700 50  0000 L CNN
F 1 ".1uf" V 7450 5450 50  0000 L CNN
F 2 "CapSMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 7438 5500 50  0001 C CNN
F 3 "" H 7400 5650 50  0001 C CNN
	1    7400 5650
	1    0    0    -1  
$EndComp
Text Notes 5650 5500 0    60   ~ 0
Note: Relay contacts are rated\n      at 1 A at 250 VAC/220 VDC
Wire Wire Line
	7400 5800 7400 5850
Wire Wire Line
	5000 3550 5200 3550
Wire Wire Line
	5200 3550 5200 5050
$Comp
L Diode:BAT54SW D2
U 1 1 607C61D2
P 9900 6050
F 0 "D2" V 9946 6137 50  0000 L CNN
F 1 "BAS70-04" V 9855 6137 50  0000 L CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 9975 6175 50  0001 L CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/BAT54W_SER.pdf" H 9780 6050 50  0001 C CNN
	1    9900 6050
	0    -1   -1   0   
$EndComp
$Comp
L Diode:BAT54SW D3
U 1 1 607C8E35
P 8950 6050
F 0 "D3" V 8996 6137 50  0000 L CNN
F 1 "BAS70-04" V 8905 6137 50  0000 L CNN
F 2 "TO/SOT SMD:SOT-23_Handsoldering" H 9025 6175 50  0001 L CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/BAT54W_SER.pdf" H 8830 6050 50  0001 C CNN
	1    8950 6050
	0    -1   -1   0   
$EndComp
Text Label 9150 6050 0    50   ~ 0
THERM
Text Label 10100 6050 0    50   ~ 0
BattDiv
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0106
U 1 1 608B3B3D
P 8950 6500
F 0 "#PWR0106" H 8950 6250 50  0001 C CNN
F 1 "GND" H 8950 6350 50  0000 C CNN
F 2 "" H 8950 6500 50  0000 C CNN
F 3 "" H 8950 6500 50  0000 C CNN
	1    8950 6500
	1    0    0    -1  
$EndComp
Wire Wire Line
	8950 6500 8950 6450
Wire Wire Line
	8950 6450 9900 6450
Wire Wire Line
	9900 6450 9900 6350
Connection ~ 8950 6450
Wire Wire Line
	8950 6450 8950 6350
Wire Wire Line
	8950 5750 8950 5650
Wire Wire Line
	8950 5650 9900 5650
Wire Wire Line
	9900 5650 9900 5750
$Comp
L IO-Board-rescue:+3.3V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0107
U 1 1 608DE1C0
P 8950 5550
F 0 "#PWR0107" H 8950 5400 50  0001 C CNN
F 1 "+3.3V" H 8950 5690 50  0000 C CNN
F 2 "" H 8950 5550 50  0001 C CNN
F 3 "" H 8950 5550 50  0001 C CNN
	1    8950 5550
	1    0    0    -1  
$EndComp
Connection ~ 8950 5650
Wire Wire Line
	8950 5550 8950 5650
Text Notes 900  750  0    50   ~ 0
I2C Pull up voltage selection
Text Notes 1600 2650 0    50   ~ 0
I2C Clock pull up resistor selection
Text Notes 1600 3850 0    50   ~ 0
I2C Data pull up resistor selection
Text Notes 1650 5800 0    50   ~ 0
Active clock drive buffer
Text Notes 2400 6800 0    50   ~ 0
I2C Level shifting
Wire Wire Line
	1300 6550 1550 6550
Text Notes 4950 6950 0    50   ~ 0
Bypass Caps
Text Notes 9000 5600 0    50   ~ 0
Analog input protection
Wire Wire Line
	1050 6100 1050 7050
Wire Wire Line
	1550 6200 1550 6550
Wire Wire Line
	5350 3450 5350 3750
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0108
U 1 1 60568CAB
P 5700 2550
F 0 "#PWR0108" H 5700 2300 50  0001 C CNN
F 1 "GND" H 5700 2400 50  0000 C CNN
F 2 "" H 5700 2550 50  0001 C CNN
F 3 "" H 5700 2550 50  0001 C CNN
	1    5700 2550
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 2450 6000 2450
Wire Wire Line
	5700 2550 5700 2450
Connection ~ 5700 2450
Wire Wire Line
	5800 3700 5800 3800
Wire Wire Line
	6100 3850 6100 3800
Wire Wire Line
	6100 3800 5800 3800
Connection ~ 5800 3800
Wire Wire Line
	5800 3800 5800 3850
Wire Wire Line
	6100 4150 6100 4250
Wire Wire Line
	6100 4250 5800 4250
Wire Wire Line
	5800 4250 5800 4150
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0109
U 1 1 60626DBA
P 5800 4300
F 0 "#PWR0109" H 5800 4050 50  0001 C CNN
F 1 "GND" H 5800 4150 50  0000 C CNN
F 2 "" H 5800 4300 50  0001 C CNN
F 3 "" H 5800 4300 50  0001 C CNN
	1    5800 4300
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 4300 5800 4250
Connection ~ 5800 4250
Wire Wire Line
	6100 3800 6150 3800
Connection ~ 6100 3800
$Comp
L IO-Board-rescue:C-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue C3
U 1 1 60737DFC
P 4400 7350
F 0 "C3" H 4425 7450 50  0000 L CNN
F 1 "10uf" H 4425 7250 50  0000 L CNN
F 2 "CapSMD:C_1206_3216Metric_Pad1.42x1.75mm_HandSolder" H 4438 7200 50  0001 C CNN
F 3 "" H 4400 7350 50  0001 C CNN
	1    4400 7350
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 7500 4400 7550
Wire Wire Line
	4400 7550 4750 7550
Wire Wire Line
	4400 7200 4400 7150
Wire Wire Line
	4400 7150 4750 7150
$Comp
L BattProg:EC2-3NU K1
U 1 1 60772C2B
P 7050 2850
F 0 "K1" V 6283 2850 50  0000 C CNN
F 1 "EB2-5NU" V 6374 2850 50  0000 C CNN
F 2 "BattProg:Relay_DPDT_Omron_G6H-2F" H 7050 2850 50  0001 C CNN
F 3 "https://content.kemet.com/datasheets/KEM_R7002_EC2_EE2.pdf" H 7050 2850 50  0001 C CNN
	1    7050 2850
	0    1    1    0   
$EndComp
$Comp
L BattProg:EC2-3NU K2
U 1 1 607743A4
P 7050 4650
F 0 "K2" V 6283 4650 50  0000 C CNN
F 1 "EB2-5NU" V 6374 4650 50  0000 C CNN
F 2 "BattProg:Relay_DPDT_Omron_G6H-2F" H 7050 4650 50  0001 C CNN
F 3 "https://content.kemet.com/datasheets/KEM_R7002_EC2_EE2.pdf" H 7050 4650 50  0001 C CNN
	1    7050 4650
	0    1    1    0   
$EndComp
Wire Wire Line
	6550 2450 6750 2450
Connection ~ 6550 2450
Wire Wire Line
	7350 2450 7900 2450
Connection ~ 7900 2450
Wire Wire Line
	7900 2450 7900 3650
Wire Wire Line
	7350 2950 7400 2950
Wire Wire Line
	7350 3350 7400 3350
Wire Wire Line
	6550 2450 6550 3650
Wire Wire Line
	6750 4250 6550 4250
Wire Wire Line
	7350 4250 7900 4250
Wire Wire Line
	7350 4750 7400 4750
Connection ~ 3750 4500
$Comp
L IO-Board-rescue:R-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue R1
U 1 1 59DB890D
P 2000 1500
F 0 "R1" V 2080 1500 50  0000 C CNN
F 1 "100K" V 2000 1500 50  0000 C CNN
F 2 "ResistorSMD:R_1206_3216Metric_Pad1.42x1.75mm_HandSolder" V 1930 1500 50  0001 C CNN
F 3 "" H 2000 1500 50  0001 C CNN
	1    2000 1500
	1    0    0    -1  
$EndComp
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR0103
U 1 1 60542BE3
P 2000 1800
F 0 "#PWR0103" H 2000 1550 50  0001 C CNN
F 1 "GND" H 2000 1650 50  0000 C CNN
F 2 "" H 2000 1800 50  0001 C CNN
F 3 "" H 2000 1800 50  0001 C CNN
	1    2000 1800
	1    0    0    -1  
$EndComp
Wire Wire Line
	1850 1350 1900 1350
Wire Wire Line
	1850 1200 2000 1200
Wire Wire Line
	2000 1200 2000 1350
Connection ~ 2000 1200
Wire Wire Line
	2000 1200 2100 1200
Wire Wire Line
	2000 1650 2000 1750
Wire Wire Line
	2000 1750 1850 1750
Wire Wire Line
	1850 1750 1850 1450
Connection ~ 2000 1750
Wire Wire Line
	2000 1750 2000 1800
$Comp
L BattProg:MAX4624 U4
U 1 1 6052BA30
P 1400 1450
F 0 "U4" H 1400 1300 60  0000 C CNN
F 1 "MAX4624" H 1400 1991 60  0000 C CNN
F 2 "TO/SOT SMD:SOT-23-6_Handsoldering" H 1400 1800 60  0001 C CNN
F 3 "" H 1400 1800 60  0001 C CNN
	1    1400 1450
	1    0    0    -1  
$EndComp
Wire Wire Line
	900  1150 950  1150
Wire Wire Line
	900  1050 950  1050
Wire Wire Line
	1850 1050 2100 1050
Wire Wire Line
	1250 4000 1600 4000
Wire Wire Line
	1600 4000 1600 4050
Connection ~ 1600 4050
Wire Wire Line
	1250 2800 1550 2800
Wire Wire Line
	1550 2800 1550 2850
Connection ~ 1550 2850
$Comp
L IO-Board-rescue:+5V-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR?
U 1 1 6068FDC6
P 1900 1000
F 0 "#PWR?" H 1900 850 50  0001 C CNN
F 1 "+5V" H 1900 1140 50  0000 C CNN
F 2 "" H 1900 1000 50  0000 C CNN
F 3 "" H 1900 1000 50  0000 C CNN
	1    1900 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	1900 1350 1900 1000
Text Notes 6500 2000 0    50   ~ 0
External connection isolation
Wire Wire Line
	7900 2300 7900 2450
Wire Wire Line
	6900 3650 6550 3650
Connection ~ 6550 3650
Wire Wire Line
	6550 3650 6550 4250
Wire Wire Line
	7200 3650 7900 3650
Connection ~ 7900 3650
Wire Wire Line
	7900 3650 7900 4250
Wire Wire Line
	5000 3150 5800 3150
Wire Wire Line
	5800 3150 5800 3400
Wire Wire Line
	5800 3150 5800 2850
Wire Wire Line
	5800 2850 6750 2850
Connection ~ 5800 3150
Wire Wire Line
	5000 3250 6750 3250
Wire Wire Line
	1250 3550 1650 3550
Text Notes 3600 800  0    50   ~ 0
ESD Protection
Wire Wire Line
	3350 1300 3350 1350
Wire Wire Line
	3350 1300 3450 1300
$Comp
L BattProg:DT2042-04SO-7 U5
U 1 1 59E5039F
P 3900 1350
F 0 "U5" H 3900 1300 60  0000 C CNN
F 1 "DT2042-04SO-7" H 3900 1800 60  0000 C CNN
F 2 "TO/SOT SMD:SOT-23-6_Handsoldering" H 3900 1350 60  0001 C CNN
F 3 "" H 3900 1350 60  0001 C CNN
	1    3900 1350
	1    0    0    -1  
$EndComp
Text Label 4400 1300 0    50   ~ 0
THERM
Text Label 5600 1900 2    50   ~ 0
Batt_Rly
$Comp
L IO-Board-rescue:GND-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue-IO-Board-rescue #PWR014
U 1 1 59E50BEB
P 3350 1350
F 0 "#PWR014" H 3350 1100 50  0001 C CNN
F 1 "GND" H 3350 1200 50  0000 C CNN
F 2 "" H 3350 1350 50  0001 C CNN
F 3 "" H 3350 1350 50  0001 C CNN
	1    3350 1350
	1    0    0    -1  
$EndComp
Text Label 4400 1200 0    50   ~ 0
Batt_SDA
Text Label 4400 1100 0    50   ~ 0
Batt_SCL
Text Label 4400 1000 0    50   ~ 0
Batt+
$EndSCHEMATC
