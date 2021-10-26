EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Connector_Generic:Conn_01x03 J1
U 1 1 5EE70748
P 3900 2100
F 0 "J1" H 3818 2417 50  0000 C CNN
F 1 "TouchWheel" H 3818 2326 50  0000 C CNN
F 2 "" H 3900 2100 50  0001 C CNN
F 3 "~" H 3900 2100 50  0001 C CNN
	1    3900 2100
	-1   0    0    -1  
$EndComp
$Comp
L Device:Antenna_Loop AE1
U 1 1 5EE71048
P 3900 2650
F 0 "AE1" V 4267 2618 50  0000 C CNN
F 1 "Antenna_Loop" V 4176 2618 50  0000 C CNN
F 2 "Saintcon2020Front:SpiralAnt3_254000_508000" H 3900 2650 50  0001 C CNN
F 3 "~" H 3900 2650 50  0001 C CNN
	1    3900 2650
	0    -1   1    0   
$EndComp
$Comp
L Connector:TestPoint TP1
U 1 1 5EE71950
P 4250 2000
F 0 "TP1" V 4250 2200 50  0000 L CNN
F 1 "TestPoint" V 4250 2400 50  0000 L CNN
F 2 "Saintcon2020Front:SPRING-120220-0315" H 4450 2000 50  0001 C CNN
F 3 "~" H 4450 2000 50  0001 C CNN
	1    4250 2000
	0    1    1    0   
$EndComp
$Comp
L Connector:TestPoint TP2
U 1 1 5EE71FA3
P 4250 2100
F 0 "TP2" V 4250 2300 50  0000 L CNN
F 1 "TestPoint" V 4250 2500 50  0000 L CNN
F 2 "Saintcon2020Front:SPRING-120220-0315" H 4450 2100 50  0001 C CNN
F 3 "~" H 4450 2100 50  0001 C CNN
	1    4250 2100
	0    1    1    0   
$EndComp
$Comp
L Connector:TestPoint TP3
U 1 1 5EE7213D
P 4250 2200
F 0 "TP3" V 4250 2400 50  0000 L CNN
F 1 "TestPoint" V 4250 2600 50  0000 L CNN
F 2 "Saintcon2020Front:SPRING-120220-0315" H 4450 2200 50  0001 C CNN
F 3 "~" H 4450 2200 50  0001 C CNN
	1    4250 2200
	0    1    1    0   
$EndComp
$Comp
L Connector:TestPoint ANT1
U 1 1 5EE722FC
P 4250 2750
F 0 "ANT1" V 4250 2950 50  0000 L CNN
F 1 "TestPoint" V 4350 2750 50  0000 L CNN
F 2 "Saintcon2020Front:SPRING-120220-0315" H 4450 2750 50  0001 C CNN
F 3 "~" H 4450 2750 50  0001 C CNN
	1    4250 2750
	0    1    1    0   
$EndComp
$Comp
L Connector:TestPoint ANT2
U 1 1 5EE72741
P 4250 2650
F 0 "ANT2" V 4250 2850 50  0000 L CNN
F 1 "TestPoint" V 4150 2650 50  0000 L CNN
F 2 "Saintcon2020Front:SPRING-120220-0315" H 4450 2650 50  0001 C CNN
F 3 "~" H 4450 2650 50  0001 C CNN
	1    4250 2650
	0    1    1    0   
$EndComp
Wire Wire Line
	4100 2000 4250 2000
Wire Wire Line
	4250 2100 4100 2100
Wire Wire Line
	4100 2200 4250 2200
Wire Wire Line
	4250 2650 4100 2650
Wire Wire Line
	4100 2750 4250 2750
$EndSCHEMATC
