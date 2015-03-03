# TempHum
Humidity and temperature sensor reading system

This is a system used for humidity and temperature monitoring.
The system uses several technologies and has two parts (Hardware and Software part).

*********************
The software part
*********************
For now is made in C#. Although Windows is not my prefered platform, this was kind of a request from someone, so ... for now it is C#.
Techonologies used:
  - MSChart for display of the read data.
  - SQLite for background database
  - .NET Framework 4.0 or higher
  
***********************
The hardware part
***********************
This is done using microcontrollers, most specifically the AVR kind of microcontrollers
The technologies used here are:
 - I2C - used for the display of the sensor modules.
 - Modbus over 485 - used for data communication between the sensor modules and the main module
 - AVR-GCC - for the programming of the sensor modules
 - Arduino - for the programming of the main module (if time permits I would like to pass this one also directly on AVR-GCC)

********************************
********************************
Description
********************************
********************************
  The project has 1 to 32 sensor modules and one main module.
  *****
  Sensor modules
  *****
  The sensor modules have the function of reading the sensors, showing the read data, signaling an error condition both visualy and acousticaly
  They are comprised of a pcb that contains the microcontroller (Atmega8A), two LEDs, one for showing when the module is plugged (recommended green) and one for reading indicatin and error indication (recommended red), a RJ45 connection for 485, a power jack, 4 pins for sensor reading, 4 pins for data display.
  The sensor used is SHT11. The library used for reading the sensor is adapted after the one in Arduino. The four cables used for connecting the sensor are GND, VCC (5V), SDA and SCL
  The display is connected to the main PCB in the sensor module through I2C. Everything that the module writes to address 4 is displayed on the 7 segments display. This way the display can be used in sevral projects and we eliminate the flickering problem that could appear while the module reads the SHT11 sensor.
  *****
  Main module
  *****
  The main module reads the sensor modules through Modbus over 485.
  It stores the data on a SD card and deletes the files that are older then 2 months. Each data is stored in a file named with the date on which the data was read. Each write contains the hour and minute the reads was performed, the sensor that was read, the temperature value, the umidity value and the status of the read.
  The possible statuses are : 
  - I for information - this happens when everything went ok
  - A for attention - this happens when something went wrong, but there is no error condition signaled - for example when a time out happened while trying to read a sensor
  - E for error - this happens when an error condition happened. An error condition appears when one of the limits was  reached or when the sensor module can no longer read the sensor
  
  The date and time are kept using a DS1307. The main module communicates with this RTC through I2C (Wire library for Arduino).
  The main module communicates with the computer using a ft230xs chip. Thus the computer will see a virtual serial port. The communication with the computer uses a simple custom protocol. The protocol is somewhat similar to xml in the sense that it limits the messages between the module and the computer with tags. For example
  - <msg>hh:mm;a;t;u;s</msg> is a normal message from the main module towards the computer where:
    - hh is the hour the sensor was read  (can also be ?? if the time could not be read from RTC)
    - mm is the minute the sensor was read (can also be ?? if the time could not be read from RTC)
    - a is the address of the read sensor (can be a value beetwen 1 and 32)
    - t is the read temperature (can be a value or "to" for time out or "er" for error)
    - u is the read umidity     (can be a value or "to" for time out or "er" for error)
    - s is the status of the message (I, A or E)
  - <t>yyyy-MM-dd hh:mm:ss</t> - is a normal message from the computer towards the main module. This message will change the time in the main module to the value specified in the message. The main module will normally reply with a <t>OK</t> when the time was succesfully changed or with a <t_nok>incomming message</t_nok> when the time was not changed.
  
