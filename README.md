# **ATTENTION: for crazy homemade metrologist only !!!**

# Keithley 2450 calibration software

## Introduction

My Agilent 34410 after 7 years usage became quite stable, I use it as a DMM for high precision measurements and calibrate him every 2-3 year.
I love when all my instruments are tuned with a small relative offset.
But calibration of my Keithley 2450 has expired and him show large offset :(
A few days ago I received Agilent 34410A from Keysight metrology lab with fresh calibration certificate. After a little and very painful thinking, I decided make software for transfer voltage/current calibration from Agilent 34410A to Keithley 2450.

That software design running on RPi based systems. That software based on [url=https://github.com/lxi-tools/liblxi]Liblxi library[/url].


## Usage

 1. Connect your Agilent/Keysight 34410A reference DMM and Keithley 2450 SMU via LXI to RPi 3B+, and connected 34410A and rear triax of 2450 together.
 2. Compile Liblxi and that software.
 3. Prepare popcorn/beer/coffee/some else. :)

That software included 2 mode: 
"Performance verification" - compare 34410A with 2450. 
"Adjustment" - transfer calibration from 34410A to 2450. 

Command line options:
-P Performance verification (default)
-A Adjustment
-D Request calibration data
-r Reference DMM IP address
-t Target 2450 IP address
-R Range
-V Voltage mode (default)
-C Current mode

Calibration transfer stand:
![Calibration transfer stand](https://misrv.com/wp-content/uploads/2020/06/DSC_0001.jpg)

**Warning: Keithley 2450 may calibrated on rear-triax inputs only, see: "Calibration considerations" (Calibration Manual 2450-905-01 Rev. A / June 2014)
ATTENTION: you can't make proper calibration with 34410A, see: "Recommended calibration equipment" (Calibration Manual 2450-905-01 Rev. A / June 2014)**

## Examples

Verification 200mV range: 2450_calibration -t 192.168.88.200 -r 192.168.88.203 -V -R 0.2 -P
Adjustment 2V range: 2450_calibration -t 192.168.88.200 -r 192.168.88.203 -V -R 0.2 -A
Verification 1mA range: 2450_calibration -t 192.168.88.200 -r 192.168.88.203 -C -R 1e-3 -P

Current verification 100uA range:
![Current verification 100uA range](https://misrv.com/wp-content/uploads/2020/06/curr_perf.png)

Current adjustment 100uA range:
![Current adjustment 100uA range](https://misrv.com/wp-content/uploads/2020/06/curr_adj.png)

## License and author

This code is released under GPL v3 license.

Author: Andrey Bykanov (aka Shodan)
E-Mail: adm@misrv.com
Location: Tula city, Russia.
