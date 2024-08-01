import time
from machine import Pin, SPI, PWM
import oakdevtech_icepython
from ICM42688P import IMU

machine.freq(250000000)

IMU_CS = Pin(21, Pin.OUT) 
IMU_CS.value(1) 

FSYNC = PWM(Pin(22), freq=32000, duty_u16=32768)
spi = SPI(0, 5_000_000, sck=Pin(18), mosi=Pin(19), miso=Pin(16), polarity=1, phase=1)

iceprog = oakdevtech_icepython.Oakdevtech_icepython(
    spi, 17 , 27 , "main_bitmap.bin"
)
 
iceprog.program_fpga() 
spi.deinit()

#If the reading is triggered by interrupt and not busy loop, Thonny won't have the time to stop the code
#So the delay here is just in case you want to stop it.
time.sleep(5)

spi = SPI(0, 10_000_000, sck=Pin(18), mosi=Pin(19), miso=Pin(16), polarity=0, phase=0)

IMU = IMU(spi,IMU_CS,iceprog._chip_sel,rate=2) #100Hz


def callback(p):
    IMU.readIMU_CSV()
    #IMU.readIMU_average()

IMU_DRDY = Pin(28, Pin.IN)
IMU_DRDY.irq(trigger=Pin.IRQ_FALLING, handler=callback)

while True:
    #IMU.readIMU_CSV()
    time.sleep(1)

