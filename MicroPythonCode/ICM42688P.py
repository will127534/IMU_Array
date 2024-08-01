import time
import ustruct
import usys
from machine import Pin, SPI, PWM
ICM42688_DEVICE_CONFIG          =const(0x11)
ICM42688_DRIVE_CONFIG           =const(0x13)
ICM42688_SIGNAL_PATH_RESET      =const(0x4B)
ICM42688_PWR_MGMT0              =const(0x4E)
ICM42688_INT_CONFIG             =const(0x14)
ICM42688_INT_STATUS             =const(0x2D)
ICM42688_INT_STATUS2            =const(0x37)
ICM42688_INT_STATUS3            =const(0x38)
ICM42688_INT_CONFIG0            =const(0x63)
ICM42688_INT_CONFIG1            =const(0x64)
ICM42688_INT_SOURCE0            =const(0x65)
ICM42688_INT_SOURCE1            =const(0x66)
ICM42688_INT_SOURCE3            =const(0x68)
ICM42688_INT_SOURCE4            =const(0x69)
ICM42688_INT_SOURCE6            =const(0x4D)
ICM42688_INT_SOURCE7            =const(0x4E)
ICM42688_INT_SOURCE8            =const(0x4F)
ICM42688_INT_SOURCE9            =const(0x50)
ICM42688_INT_SOURCE10           =const(0x51)
ICM42688_TEMP_DATA1             =const(0x1D)
ICM42688_TEMP_DATA0             =const(0x1E)
ICM42688_ACCEL_DATA_X1          =const(0x1F)
ICM42688_ACCEL_DATA_X0          =const(0x20)
ICM42688_ACCEL_DATA_Y1          =const(0x21)
ICM42688_ACCEL_DATA_Y0          =const(0x22)
ICM42688_ACCEL_DATA_Z1          =const(0x23)
ICM42688_ACCEL_DATA_Z0          =const(0x24)
ICM42688_GYRO_DATA_X1           =const(0x25)
ICM42688_GYRO_DATA_X0           =const(0x26)
ICM42688_GYRO_DATA_Y1           =const(0x27)
ICM42688_GYRO_DATA_Y0           =const(0x28)
ICM42688_GYRO_DATA_Z1           =const(0x29)
ICM42688_GYRO_DATA_Z0           =const(0x30)
ICM42688_TMST_FSYNCH            =const(0x43)
ICM42688_TMST_FSYNCL            =const(0x44)
ICM42688_GYRO_CONFIG_STATIC2    =const(0x0B)
ICM42688_GYRO_CONFIG_STATIC3    =const(0x0C)
ICM42688_GYRO_CONFIG_STATIC4    =const(0x0D)
ICM42688_GYRO_CONFIG_STATIC5    =const(0x0E)
ICM42688_GYRO_CONFIG_STATIC6    =const(0x0F)
ICM42688_GYRO_CONFIG_STATIC7    =const(0x10)
ICM42688_GYRO_CONFIG_STATIC8    =const(0x11)
ICM42688_GYRO_CONFIG_STATIC9    =const(0x12)
ICM42688_GYRO_CONFIG_STATIC10   =const(0x13)
ICM42688_GYRO_CONFIG0           =const(0x4F)
ICM42688_ACCEL_CONFIG0          =const(0x50)
ICM42688_GYRO_CONFIG1           =const(0x51)
ICM42688_GYRO_ACCEL_CONFIG0     =const(0x52)
ICM42688_ACCEL_CONFIG1          =const(0x53)
ICM42688_TMST_CONFIG            =const(0x54)
ICM42688_SMD_CONFIG             =const(0x57)
ICM42688_FIFO_CONFIG            =const(0x16)
ICM42688_FIFO_COUNTH            =const(0x2E)
ICM42688_FIFO_COUNTL            =const(0x2F)
ICM42688_FIFO_DATA              =const(0x30)
ICM42688_FIFO_CONFIG1           =const(0x5F)
ICM42688_FIFO_CONFIG2           =const(0x60)
ICM42688_FIFO_CONFIG3           =const(0x61)
ICM42688_FIFO_LOST_PKT0         =const(0x6C)
ICM42688_FIFO_LOST_PKT1         =const(0x6D)
ICM42688_FSYNC_CONFIG           =const(0x62)
ICM42688_SELF_TEST_CONFIG       =const(0x70)
ICM42688_WHO_AM_I               =const(0x75)
ICM42688_REG_BANK_SEL           =const(0x76) 
ICM42688_SENSOR_CONFIG0         =const(0x03)
ICM42688_XG_ST_DATA             =const(0x5F)
ICM42688_YG_ST_DATA             =const(0x60)
ICM42688_ZG_ST_DATA             =const(0x61)
ICM42688_TMSTVAL0               =const(0x62)
ICM42688_TMSTVAL1               =const(0x63)
ICM42688_TMSTVAL2               =const(0x64)
ICM42688_INTF_CONFIG0           =const(0x4C)
ICM42688_INTF_CONFIG1           =const(0x4D)
ICM42688_INTF_CONFIG4           =const(0x7A)
ICM42688_INTF_CONFIG5           =const(0x7B)
ICM42688_INTF_CONFIG6           =const(0x7C)
ICM42688_ACCEL_CONFIG_STATIC2   =const(0x03)
ICM42688_ACCEL_CONFIG_STATIC3   =const(0x04)
ICM42688_ACCEL_CONFIG_STATIC4   =const(0x05)
ICM42688_XA_ST_DATA             =const(0x3B)
ICM42688_YA_ST_DATA             =const(0x3C)
ICM42688_ZA_ST_DATA             =const(0x3D)
ICM42688_APEX_DATA0             =const(0x31)
ICM42688_APEX_DATA1             =const(0x32)
ICM42688_APEX_DATA2             =const(0x33)
ICM42688_APEX_DATA3             =const(0x34)
ICM42688_APEX_DATA4             =const(0x35)
ICM42688_APEX_DATA5             =const(0x36)
ICM42688_APEX_CONFIG0           =const(0x56)
ICM42688_APEX_CONFIG1           =const(0x40)
ICM42688_APEX_CONFIG2           =const(0x41)
ICM42688_APEX_CONFIG3           =const(0x42)
ICM42688_APEX_CONFIG4           =const(0x43)
ICM42688_APEX_CONFIG5           =const(0x44)
ICM42688_APEX_CONFIG6           =const(0x45)
ICM42688_APEX_CONFIG7           =const(0x46)
ICM42688_APEX_CONFIG8           =const(0x47)
ICM42688_APEX_CONFIG9           =const(0x48)
ICM42688_ACCEL_WOM_X_THR        =const(0x4A)
ICM42688_ACCEL_WOM_Y_THR        =const(0x4B)
ICM42688_ACCEL_WOM_Z_THR        =const(0x4C)
ICM42688_OFFSET_USER0           =const(0x77)
ICM42688_OFFSET_USER1           =const(0x78)
ICM42688_OFFSET_USER2           =const(0x79)
ICM42688_OFFSET_USER3           =const(0x7A)
ICM42688_OFFSET_USER4           =const(0x7B)
ICM42688_OFFSET_USER5           =const(0x7C)
ICM42688_OFFSET_USER6           =const(0x7D)
ICM42688_OFFSET_USER7           =const(0x7E)
ICM42688_OFFSET_USER8           =const(0x7F)
ICM42688_STEP_DET_INT           =const(1<<5)
ICM42688_STEP_CNT_OVF_INT       =const(1<<4)
ICM42688_TILT_DET_INT           =const(1<<3)
ICM42688_WAKE_INT               =const(1<<2)
ICM42688_SLEEP_INT              =const(1<<1)
ICM42688_TAP_DET_INT            =const(1)
ICM42688_SMD_INT                =const(1<<3)
ICM42688_WOM_Z_INT              =const(1<<2)
ICM42688_WOM_Y_INT              =const(1<<1)
ICM42688_WOM_X_INT              =const(1)
ICM42688_STATUS_WALK            =const(1)
ICM42688_STATUS_RUN             =const(2)

FPGA_READ = const(0x55)

class IMU:
    def __init__(self, spi, cs, fpgacs, rate):
        self._spi = spi
        
        #Cache the functions directly to Speed up MicroPython
        self._cs_h = cs.on 
        self._cs_l = cs.off
        self._fpgacs_h = fpgacs.on
        self._fpgacs_l = fpgacs.off
        #Preallocate to Speed up MicroPython
        self.result = bytearray(512)
        self.result_short = bytearray(16)
        
        #Soft Reset
        self.write_reg(ICM42688_REG_BANK_SEL,[0x00])
        self.write_reg(ICM42688_DEVICE_CONFIG,[0x01])
        time.sleep(0.1)
        
        if rate == 1:
            self.write_reg(ICM42688_GYRO_CONFIG0,[0xE7]) # 15.625dps + 200Hz
            self.write_reg(ICM42688_ACCEL_CONFIG0,[0x67]) # 2g + 200Hz
        elif rate == 2:
            self.write_reg(ICM42688_GYRO_CONFIG0,[0xE8]) # 15.625dps + 100Hz
            self.write_reg(ICM42688_ACCEL_CONFIG0,[0x68]) # 2g + 100Hz
        else:
            self.write_reg(ICM42688_GYRO_CONFIG0,[0xE9]) # 15.625dps + 50Hz
            self.write_reg(ICM42688_ACCEL_CONFIG0,[0x69]) # 2g + 50Hz
        #Enable FIFO
        self.write_reg(ICM42688_FIFO_CONFIG1, [0x07]) #FIFO_TEMP_EN + FIFO_GYRO_EN + FIFO_ACCEL_EN
        self.write_reg(ICM42688_FIFO_CONFIG, [0x40]) #FIFO Enable
        self.write_reg(ICM42688_INTF_CONFIG1, [0x95]) # RTC_MODE
        self.write_reg(ICM42688_REG_BANK_SEL,[0x01])
        self.write_reg(ICM42688_INTF_CONFIG5, [0x04]) # CLKIN Enable
        self.write_reg(ICM42688_REG_BANK_SEL,[0x00])
        self.write_reg(ICM42688_TMST_CONFIG,[0x2B])
        self.write_reg(ICM42688_INT_CONFIG,[0x02]) # INT1 drive circuit = Push pull
        self.write_reg(ICM42688_INT_SOURCE0,[0x08]) # UI data ready interrupt routed to INT1
        self.write_reg(ICM42688_INT_CONFIG1,[0x00])
        self.start_measure()
    
    @micropython.native
    def write_reg(self,reg, data): 
        self._cs_l()
        self._spi.write(bytearray([reg,data[0]]))
        self._cs_h()
    
    @micropython.native
    def read_reg(self,reg): 
        self._cs_l()
        self._spi.write(bytearray([reg | 0x80]))
        result = bytearray(1)
        self._spi.readinto(result)
        self._cs_h()
        return result[0]
    
    @micropython.native
    def read_multi_reg(self,reg):
        self._cs_l()
        self._spi.write(bytearray([reg | 0x80]))
        self._spi.readinto(self.result_short)
        self._cs_h()
        return self.result_short
    
    def start_measure(self):
        self.write_reg(ICM42688_REG_BANK_SEL, [0x00])
        self.write_reg(ICM42688_PWR_MGMT0,[0x0F])
    
    @micropython.native
    def readIMU_CSV(self):
        self._spi = SPI(0, 24_000_000, sck=Pin(18), mosi=Pin(19), miso=Pin(16), polarity=0, phase=0)
        self._fpgacs_l()
        self.read_multi_reg(ICM42688_FIFO_DATA)
        self._fpgacs_h()
        self._fpgacs_l()
        self._spi.write(bytearray([FPGA_READ]))
        #Because of the verilog code the data always shift at the postive edge
        self._spi = SPI(0, 20_000_000, sck=Pin(18), mosi=Pin(19), miso=Pin(16), polarity=0, phase=1)
        self._spi.readinto(self.result)
        self._fpgacs_h()
        data = ustruct.unpack(">BhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBH", self.result)
        r = range(32)
        for no in r:
            header = data[0 + no * 9]
            acc_x = data[1 + no * 9]
            acc_y = data[2 + no * 9] 
            acc_z = data[3 + no * 9] 
            gyro_x = data[4 + no * 9]
            gyro_y = data[5 + no * 9]
            gyro_z = data[6 + no * 9]
            temp = data[7 + no * 9]
            timestamp = data[8 + no * 9]
            print("%d,%d,%d,%d,%d,%d,%d,%d,%d," % (header,timestamp,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z,temp),end="")
        print("\n",end="")

    @micropython.native
    def readIMU_average(self):
        self._spi = SPI(0, 24_000_000, sck=Pin(18), mosi=Pin(19), miso=Pin(16), polarity=0, phase=0)
        self._fpgacs_l()
        self.read_multi_reg(ICM42688_FIFO_DATA)
        self._fpgacs_h()
        self._fpgacs_l()
        self._spi.write(bytearray([FPGA_READ]))
        #Because of the verilog code the data always shift at the postive edge
        self._spi = SPI(0, 20_000_000, sck=Pin(18), mosi=Pin(19), miso=Pin(16), polarity=0, phase=1)
        self._spi.readinto(self.result)
        self._fpgacs_h()
        data = ustruct.unpack(">BhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBHBhhhhhhBH", self.result)
        r = range(32)
        acc_x = 0
        acc_y = 0
        acc_z = 0
        gyro_x = 0
        gyro_y = 0
        gyro_z = 0
        temp = 0
        for no in r:
            if no < 8:
                acc_x = acc_x + data[1 + no * 9]
                acc_y = acc_y + data[2 + no * 9]
                acc_z = acc_z + data[3 + no * 9]
                gyro_x = gyro_x - data[4 + no * 9]
                gyro_y = gyro_y - data[5 + no * 9]
                gyro_z = gyro_z + data[6 + no * 9]
            if no >=8 and no <16:
                acc_x = acc_x - data[2 + no * 9]
                acc_y = acc_y + data[1 + no * 9]
                acc_z = acc_z + data[3 + no * 9]
                gyro_x = gyro_x + data[5 + no * 9]
                gyro_y = gyro_y - data[4 + no * 9]
                gyro_z = gyro_z + data[6 + no * 9]
            if no >=16 and no <24:
                acc_x = acc_x - data[1 + no * 9]
                acc_y = acc_y - data[2 + no * 9]
                acc_z = acc_z + data[3 + no * 9]
                gyro_x = gyro_x + data[4 + no * 9]
                gyro_y = gyro_y + data[5 + no * 9]
                gyro_z = gyro_z + data[6 + no * 9]
            if no >=24:
                acc_x = acc_x + data[2 + no * 9]
                acc_y = acc_y - data[1 + no * 9]
                acc_z = acc_z + data[3 + no * 9]
                gyro_x = gyro_x - data[5 + no * 9]
                gyro_y = gyro_y + data[4 + no * 9]
                gyro_z = gyro_z + data[6 + no * 9]
            temp = temp + data[7 + no * 9]
        timestamp = data[8]
        header = data[0]
        #If data is valid (i.e FIFO not underrun)
        if header == 104:
            print("%d,%d,%d,%d,%d,%d,%d,%d,%d" % (timestamp,header,acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z,temp))


