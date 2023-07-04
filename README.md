# Description
    - The system includes: 8051 microcontroller, DS1621 sensor, 16x2 LCD, 74LS373 address-latch.
    - 8051 reads temperature from DS1621 by I2C protocol, then displays on LCD.

# Source:
    - main:         folder contains source code written in Keil C IDE.
    - DS1621_LCD:   simulates operation of system by Proteus.

# Connection:
    - Communicate to LCD:
        + Port 0 is used as a multiplexed address/bus (lower 8-bit address and data bus). 
        + The external latch and the ALE signal provided by the 8051 are used to latch the 8-bit address. 
        + WR, RD: write/read external data memory.

    - Communicate to DS1621:
        + Pin P3.3: SCL
        + Pin P3.4: SDA
        + A2 A1 A0 is address of sensor
        + TOUT: warning when value exceeds threshold 

# Reference:
    - Operation of I2C protocol.
    - Datasheets of DS1621, LCD to know more details about commands.
    - Datasheet of 74LS373.