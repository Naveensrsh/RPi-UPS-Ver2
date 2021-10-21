#------------------------------------------------------------------------------#
#-------------------------------RPi-UPS-Ver-2----------------------------------#
#------------------------------------------------------------------------------#
from datetime import datetime
import RPi.GPIO as GPIO
from time import sleep
import os
import subprocess
#------------------------------------------------------------------------------#
FAN_Pin = 19
Switch_LED_Pin = 26
Shutdown_Pin = 23
Reboot_Pin = 6
HDD_Pin = 13
Temp_Lower_Cutoff = 50.0
Temp_Upper_Cutoff = 60.0
Pi_State = 'ON'
#Mount_Flag = 0
#Unmount_Flag = 0
#------------------------------------------------------------------------------#
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(Shutdown_Pin,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(Reboot_Pin,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(HDD_Pin,GPIO.IN,pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(Switch_LED_Pin,GPIO.OUT)
GPIO.setup(FAN_Pin,GPIO.OUT)
GPIO.output(Switch_LED_Pin,1)
GPIO.output(FAN_Pin,0)
#------------------------------------------------------------------------------#
def Update_Log():
    global Pi_State
    CurDateTime = datetime.now().strftime('%H:%M:%S %d-%m-%Y')
    data = "  " + Pi_State + "  " + str(CurDateTime) + "  "
    with open("/home/pi/RPi-UPS/Pi-Log.txt", "a") as myfile:
        if "OFF" in Pi_State:
            myfile.write(data)
        else:
            myfile.write("\n" + data)
#------------------------------------------------------------------------------#
def Shutdown():
    print "Shutdown"
    global Pi_State
    Pi_State = 'OFF - Shutdown'
    Update_Log()
    sleep(1)
    os.system("sudo shutdown -h now")
#------------------------------------------------------------------------------#
def Reboot():
    print "Rebooting"
    global Pi_State
    Pi_State = 'OFF - Reboot'
    Update_Log()
    sleep(1)
    os.system("sudo reboot")
#------------------------------------------------------------------------------#
def Temp():
    t = os.popen('vcgencmd measure_temp').readline()
    return(t.replace("temp=","").replace("'C\n",""))
#------------------------------------------------------------------------------#
##def HDD_Unmount():
##    process = subprocess.Popen("sudo hdparm -C /dev/sdaX", shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
##    Output = process.communicate()[0]    
##    print(Output)
##    if "standby" in str(Output):
##        print("Unmounting HDD")
##        Out = subprocess.call('/home/pi/RPi-UPS-Ver2/Unmount.sh',shell=True)
##        if Out == 0:
##            print("\nUnmounting Successful")
##        else:
##            print("Unmount Error!!!")
##    else:
##        print("HDD Currently in Use")
#------------------------------------------------------------------------------#
##def HDD_Mount():
##    
#------------------------------------------------------------------------------#
print "RPi-UPS-Ver2 Code Executing ..."
Update_Log()
GPIO.add_event_detect(Shutdown_Pin, GPIO.RISING, callback = Shutdown)
GPIO.add_event_detect(Reboot_Pin, GPIO.RISING, callback = Reboot)
#GPIO.add_event_detect(HDD_Pin, GPIO.RISING, callback = HDD_Unmount)
#GPIO.add_event_detect(HDD_Pin, GPIO.FALLING, callback = HDD_Mount)
#------------------------------------------------------------------------------#
while(True):
    try:
        temperature = float(Temp())
        print "Temp =",temperature
        if temperature > Temp_Upper_Cutoff:
            GPIO.output(FAN,1)
        elif temperature < Temp_Lower_Cutoff:
            GPIO.output(FAN,0)
        sleep(10)
    except:
        GPIO.output(LED,0)
        sleep(1)
        GPIO.output(LED,1)
        sleep(1)
#------------------------------------------------------------------------------#
