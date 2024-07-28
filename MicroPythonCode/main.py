import time
from machine import Pin, SPI, PWM
import oakdevtech_icepython
from ICM42688P import IMU

machine.freq(150000000)

IMU_CS = Pin(21, Pin.OUT) 
IMU_CS.value(1) 
IMU_DRDY = Pin(28, Pin.IN)
spi = SPI(0, 1_000_000, sck=Pin(18), mosi=Pin(19), miso=Pin(16), polarity=1, phase=1)

#32Hz CLKIN for IMU
FSYNC = PWM(Pin(22), freq=32000, duty_u16=32768)

#Program FPGA
iceprog = oakdevtech_icepython.Oakdevtech_icepython(
    spi, 17 , 27 , "main_bitmap.bin"
)
iceprog.program_fpga() 
spi.deinit()


#Reconfig SPI
spi = SPI(0, 4_000_000, sck=Pin(18), mosi=Pin(19), miso=Pin(16), polarity=0, phase=0)

#Rate 1 = 200Hz, Enabled DRDY Interrupt, Enabled 32K CLKIN 
IMU = IMU(spi,IMU_CS,iceprog._chip_sel,rate=1)

def callback(p):
    IMU.readIMU_average()

IMU_DRDY.irq(trigger=Pin.IRQ_FALLING, handler=callback)

while True:
    time.sleep(0.1)
    

'''

while True:
    readIMUs()
    time.sleep(0.1)




IMU_CS.value = True
spi.unlock()
iceprog.program_fpga() 
while not spi.try_lock(): 
    pass



def readIMUs_FIFO():
    iceprog._chip_sel.value = False
    read_multi_reg(ICM42688_FIFO_DATA, 16)
    iceprog._chip_sel.value = True
    result = bytearray(512)
    iceprog._chip_sel.value = False
    spi.write(bytearray([0x55]))
    spi.readinto(result)
    iceprog._chip_sel.value = True
    data = unpack(">BhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBH", result)
    #print(data)
    for no in range(32):
        header = data[0 + no * 9]
        acc_x = data[1 + no * 9] * 0.488
        acc_y = data[2 + no * 9] * 0.488
        acc_z = data[3 + no * 9] * 0.488
        gyro_x = data[4 + no * 9] * 4000/65535.0
        gyro_y = data[5 + no * 9] * 4000/65535.0
        gyro_z = data[6 + no * 9] * 4000/65535.0
        temp = data[7 + no * 9]/2.07 + 25
        timestamp = data[8 + no * 9]
        print("[%d] IMU %d: %d" % (timestamp,no+1,header))
        print("Temperature: %f" % temp)
        print("ACC: x: %f, y: %f, z:%f" % (acc_x,acc_y,acc_z))
        print("Gyro: x: %f, y: %f, z:%f" % (gyro_x,gyro_y,gyro_z))

def readIMUs_old():
    iceprog._chip_sel.value = False
    read_multi_reg(ICM42688_TEMP_DATA1, 14)
    iceprog._chip_sel.value = True
    result = bytearray(28)
    iceprog._chip_sel.value = False
    spi.write(bytearray([0x55]))
    spi.readinto(result)
    iceprog._chip_sel.value = True
    data = unpack(">hhhhhhhhhhhhhh", result)
    print(data)
    temp = data[0]/132.48 + 25
    acc_x = data[1] * 0.488
    acc_y = data[2] * 0.488
    acc_z = data[3] * 0.488
    gyro_x = data[4] * 4000/65535.0
    gyro_y = data[5] * 4000/65535.0
    gyro_z = data[6] * 4000/65535.0
    print("IMU 1:")
    print("Temperature: %f" % temp)
    print("ACC: x: %f, y: %f, z:%f" % (acc_x,acc_y,acc_z))
    print("Gyro: x: %f, y: %f, z:%f" % (gyro_x,gyro_y,gyro_z))
    temp = data[7]/132.48 + 25
    acc_x = data[8] * 0.488
    acc_y = data[9] * 0.488
    acc_z = data[10] * 0.488
    gyro_x = data[11] * 4000/65535.0
    gyro_y = data[12] * 4000/65535.0
    gyro_z = data[13] * 4000/65535.0
    print("IMU 2:")
    print("Temperature: %f" % temp)
    print("ACC: x: %f, y: %f, z:%f" % (acc_x,acc_y,acc_z))
    print("Gyro: x: %f, y: %f, z:%f" % (gyro_x,gyro_y,gyro_z))


spi.configure(baudrate=5000000, phase=0, polarity=0)





write_reg(ICM42688_REG_BANK_SEL,[0x00])
write_reg(0x11,[0x01])
time.sleep(1)

IMU_id = read_reg(ICM42688_WHO_AM_I)

print("IMU ID:%d" % IMU_id)

start_measure()
time.sleep(0.1)

while True:
    data = read_multi_reg(ICM42688_TEMP_DATA1, 14)
    data = unpack(">hhhhhhh", data)
    print(data)
    temp = data[0]/132.48 + 25
    acc_x = data[1] * 0.488
    acc_y = data[2] * 0.488
    acc_z = data[3] * 0.488
    gyro_x = data[4] * 4000/65535.0
    gyro_y = data[5] * 4000/65535.0
    gyro_z = data[6] * 4000/65535.0

    print("Temperature: %f" % temp)
    print("ACC: x: %f, y: %f, z:%f" % (acc_x,acc_y,acc_z))
    print("Gyro: x: %f, y: %f, z:%f" % (gyro_x,gyro_y,gyro_z))

    time.sleep(1)
'''


