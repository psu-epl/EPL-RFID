import time
import pigpio

SDA=18
SCL=19

I2C_ADDR=9

file = open("data.txt", "w+")

def i2c(id, tick):
 global pi

 s, b, d = pi.bsc_i2c(I2C_ADDR)

 if b:
     file.write(d)
     print(d)

pi = pigpio.pi()

if not pi.connected:
 exit()

# Add pull-ups in case external pull-ups haven't
# been added

pi.set_pull_up_down(SDA, pigpio.PUD_UP)
pi.set_pull_up_down(SCL, pigpio.PUD_UP)

# Respond to BSC slave activity

e = pi.event_callback(pigpio.EVENT_BSC, i2c)

pi.bsc_i2c(I2C_ADDR) # Configure BSC as I2Cslave

time.sleep(60)

file.close()
e.cancel()

pi.bsc_i2c(0) # Disable BSC peripheral

pi.stop()
