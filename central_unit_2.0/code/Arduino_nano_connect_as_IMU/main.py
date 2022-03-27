import time
import machine
from lsm6dsox import LSM6DSOX
import os
import sys

heading_x = 0
heading_y = 0
heading_z = 0

counter = 0
last_setup_pin = 1

def imu_int_handler(pin):
    global heading

    led.on()


def sign(x): return 1 if x >= 0 else -1

led       = machine.Pin( 6, machine.Pin.OUT, value=0)
int_pin   = machine.Pin(24, machine.Pin.IN,  machine.Pin.PULL_DOWN)       # int of IMU
setup_pin = machine.Pin(25, machine.Pin.IN,  machine.Pin.PULL_UP)         # D2
adc_pin   = machine.Pin(29)                                               # A3

int_pin.irq(handler = imu_int_handler, trigger = machine.Pin.IRQ_RISING)
adc = machine.ADC(adc_pin)
i2c = machine.I2C(0, scl=machine.Pin(13), sda=machine.Pin(12))
imu = LSM6DSOX(i2c, gyro_odr = 104, accel_odr = 104, gyro_scale = 250, accel_scale = 2)

debug = False

time.sleep(1) # wait for all power and IMU stable

try: # to read config values from setup file
    #os.chdir("/")
    f = open('IMU.cfg',"r")
    zero_x = float(f.readline())
    zero_y = float(f.readline())
    zero_z = float(f.readline())
    my_roll_axis = int(f.readline())
    my_roll_sign = int(f.readline())
    print(zero_x)
    print(zero_y)
    print(zero_z)
    print(my_roll_axis)
    print(my_roll_sign)
    f.close()
except:
    print("no setup file yet")
    zero_x, zero_y, zero_z = imu.read_accel()

while (True):

    # self-learning mounting orientation
    if last_setup_pin == 1 and setup_pin.value() == 0: # someone pressed the botton to set zero
        last_setup_pin = 0
        print("setting zero position")
        this_zero_x, this_zero_y, this_zero_z = imu.read_accel()
        cal_turnrate_x, cal_turnrate_y, cal_turnrate_z = imu.read_gyro()  # init rotation as well

    if last_setup_pin == 0 and setup_pin.value() == 1: # someone pressed the botton to set zero
        last_setup_pin = 1
        print("setting correct axis")
        
        front_up_x, front_up_y, front_up_z = imu.read_accel()

        modify_roll = False
        if abs(this_zero_z) > 0.9 and abs(front_up_x - this_zero_x) > 0.2:
            my_roll_axis = 1
            my_roll_sign = sign(front_up_x - this_zero_x) * sign(this_zero_z)
            print("PCB longside horrizontally")
            modify_roll = True

        if abs(this_zero_z) > 0.9 and abs(front_up_y - this_zero_y) > 0.2:
            my_roll_axis = 0
            my_roll_sign = - sign(front_up_y - this_zero_y) * sign(this_zero_z)
            print("PCB longside horrizontally")
            modify_roll = True

        if abs(this_zero_x) > 0.9 and abs(front_up_y - this_zero_y) > 0.2:
            my_roll_axis = 2
            my_roll_sign = sign(front_up_y - this_zero_y) * sign(this_zero_x)
            print("PCB longside horrizontally")
            modify_roll = True

        if abs(this_zero_x) > 0.9 and abs(front_up_z - this_zero_z) > 0.2:
            my_roll_axis = 1
            my_roll_sign = -sign(front_up_z - this_zero_z) * sign(this_zero_x)
            print("PCB longside horrizontally")
            modify_roll = True

        if abs(this_zero_y) > 0.9 and abs(front_up_z - this_zero_z) > 0.2:
            my_roll_axis = 0
            my_roll_sign = sign(front_up_z - this_zero_z) * sign(this_zero_y)
            print("PCB longside horrizontally")
            modify_roll = True

        if abs(this_zero_y) > 0.9 and abs(front_up_x - this_zero_x) > 0.2:
            my_roll_axis = 2
            my_roll_sign = -sign(front_up_x - this_zero_x) * sign(this_zero_y)
            print("PCB longside horrizontally")
            modify_roll = True

        if modify_roll:
            zero_x = this_zero_x
            zero_y = this_zero_y
            zero_z = this_zero_z

            f = open("IMU.cfg", "w")  # write config data to file
            f.write(str(zero_x))
            f.write("\n")
            f.write(str(zero_y))
            f.write("\n")
            f.write(str(zero_z))
            f.write("\n")
            f.write(str(my_roll_axis))
            f.write("\n")
            f.write(str(my_roll_sign))
            f.write("\n")
            f.close()
            print("written to file.")
        else:
            print("only rot calibrated. did you lift?")

    turnrate_x, turnrate_y, turnrate_z = imu.read_gyro()
    try:
        last_turnrate_x
    except: # init on power-up
        last_turnrate_x, last_turnrate_y, last_turnrate_z = turnrate_x, turnrate_y, turnrate_z
        cal_turnrate_x, cal_turnrate_y, cal_turnrate_z = turnrate_x, turnrate_y, turnrate_z
        
    # check for new gyro values (rate: 104 times per second)
    if last_turnrate_x != turnrate_x or last_turnrate_y != turnrate_y or last_turnrate_z != turnrate_z:
        counter += 1
        counter &= 0xff        

        last_turnrate_x, last_turnrate_y, last_turnrate_z = turnrate_x, turnrate_y, turnrate_z

        heading_x += turnrate_x - cal_turnrate_x
        heading_y += turnrate_y - cal_turnrate_y
        heading_z += turnrate_z - cal_turnrate_z
        
        if heading_x >= 18000:
            heading_x -= 36000
        if heading_x < -18000:
            heading_x += 36000
        if heading_y >= 18000:
            heading_y -= 36000
        if heading_y < -18000:
            heading_y += 36000
        if heading_z >= 18000:
            heading_z -= 36000
        if heading_z < -18000:
            heading_z += 36000
        
        accel_x, accel_y, accel_z = imu.read_accel()

        my_heading = 0
        my_roll = 18000
        
        if abs(accel_z) > 0.8:
            my_heading = (int) (-heading_z * sign(accel_z))
        
        if abs(accel_x) > 0.8:
            my_heading = (int) (-heading_x * sign(accel_x))

        if abs(accel_y) > 0.8:
            my_heading = (int) (-heading_y * sign(accel_y))

        try:
            if my_roll_axis == 0:
                x = (accel_x-zero_x) * my_roll_sign
                
            if my_roll_axis == 1:
                x = (accel_y-zero_y) * my_roll_sign

            if my_roll_axis == 2:
                x = (accel_z-zero_z) * my_roll_sign

            my_roll = (int) (5730 * x) # 100 * 180 * (x + x*x*x/6) / 3.141527)  # (short) Taylor row of arcsin; in 0.01 deg.
        except:
            pass

        yaw_low = my_heading & 0xff
        yaw_high = (my_heading & 0xff00) >> 8
        roll_low = my_roll & 0xff
        roll_high = (my_roll & 0xff00) >> 8
        Csum = (counter + yaw_low + yaw_high + roll_low + roll_high) & 0xff

        if not debug:
            #sys.stdout.write(chr(170)+chr(170)+chr(counter)+chr(yaw_low)+chr(yaw_high)+chr(0)+chr(0)+chr(roll_low)+chr(roll_high)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(0)+chr(Csum))
            data = [170, 170, counter, yaw_low, yaw_high, 0, 0, roll_low, roll_high, 0, 0, 0, 0, 0, 0, 0, 0, 0, Csum]
            sys.stdout.buffer.write(bytes(data))
#        else:
#            if counter == 128:
#                print(yaw_low)
#                print(yaw_high)
#                print(roll_low)
#                print(roll_high)
#                print(Csum)

        if (counter & 0x0f) == 0 and setup_pin.value() == 0:
            led.on()
        
        if (counter & 0x1f)== 0:
            led.on()
            try:
                my_roll_axis
            except:
                led.off()

            if debug:
                #reading = adc.read_u16()     # read anything just for test
                #print("ADC: ",reading)
                #print('Accelerometer: x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}'.format(accel_x, accel_y, accel_z))
                #print('Gyroscope:     x:{:>8.3f} y:{:>8.3f} z:{:>8.3f}'.format(turnrate_x, turnrate_y, turnrate_z))
                #print('Headings:      x:{:>8.1f} y:{:>8.1f} z:{:>8.1f}'.format(heading_x/100, heading_y/100, heading_z/100))
                try:
                    print('Heading: {:>8.1f}'.format(my_heading/100))
                    my_roll_axis
                    print('Roll: {:>8.1f}'.format(my_roll/100))
                except:
                    if setup_pin.value() == 1:
                        print ("roll axis undefined yet")
                    else:
                        print ("put front up")
                print("")
            
