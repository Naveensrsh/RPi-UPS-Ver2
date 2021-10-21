/*---------------------------------------------------------------------------*/
/*-----------------------------RPi-UPS-Ver-2.0-------------------------------*/
/*---------------------------------------------------------------------------*/
#include <16f884.h>         
#device ADC=10
#fuses INTRC,NOLVP,NOWDT,NOBROWNOUT,NOPUT,NODEBUG 
#use delay(clock=8000000)
/*----------------------------------Debug------------------------------------*/
#use rs232(baud=9600,parity=N,UART1,bits=8,ERRORS)
/*---------------------------------------------------------------------------*/ 
/*------------------------------Pin Details----------------------------------*/
/*---------------------------------------------------------------------------*/
#define Green_Led pin_D2
#define Red_Led pin_D3
#define Blue_Led pin_C4
#define Ext_Switch pin_B2
#define Bat_Switch pin_B4
#define Shutdown_Pin pin_D0    // Pin connected from PIC to RPi for shutdown
#define Reboot_Pin pin_C3     // Pin connected from PIC to RPi for reboot
#define HDD_Pin pin_D1       // For RPi to unmount HDD (if on Standby) to avoid HDD Spinup during no ext power & mount once power good
#define RPi_Reset pin_C2     // For Reset jumper of RPi
//#define RPi_WDT            // (Optional) Reset Pi if unresponsive for a fixed time
#define Buzzer pin_C5
#define Buzzer_Delay 100
#define Push_Switch pin_C1        // Push button to initiate shutdown. (Opt Feature): 2 Sec for reboot, 5 Sec for Shutdown with beep.
#define Ext_Analog pin_A2     // Read fn will not change 
#define Bat_Analog pin_A0
//#define Debug_Pin pin_B2
/*---------------------------------------------------------------------------*/
#define Over_Voltage 900         // If Ext supply > 14 V then turn of ext supply --> 14/3.2=4.375 --> 4.375/0.00488=896
#define Ext_High_Threshold 820   // 13/3.2=4.065/.00488=832 old --> new 12.8 so 820
#define Bat_High_Threshold 755   // 11.8/3.2=3.6875/.00488=755
#define Ext_Low_Threshold 755    // 12/3.2=3.75/.00488=768 old --> new 11.8 so 755
#define Bat_Low_Threshold 692    // 10.8/3.2=3.375/.00488=692
#define Init_delay 500
/*---------------------------------------------------------------------------*/
int State = 0;
int counter = 0;
int Mode = 0;
int OVP = 0;
int Beep_Flag = 0;
int Beep_Time = 50;
unsigned int Count = 0;
unsigned int16 Ext_Volt = 0;
unsigned int16 Bat_Volt = 0;
int Shutdown_Timer = 60;      // 1 Minute time for shutdown.
int OVP_Delay_Counter = 0;      // Use like millis() ; may need int16 and "if" value change
/*---------------------------------------------------------------------------*/
void initialise()
{
   setup_adc(ADC_CLOCK_INTERNAL);
   setup_adc_ports(0);
   setup_adc_ports(2);
   output_low(Ext_Switch);      // Ext Relay OFF
   output_low(Bat_Switch);      // Bat Relay OFF
   output_low(Shutdown_Pin);    //Shutdown to Pi Pin OFF 
   output_low(Reboot_Pin);
   output_low(HDD_Pin);
   output_high(Buzzer);
   delay_ms(250);
   output_low(Buzzer);
    //Test
  // delay_ms(Init_delay);
   output_low(Green_Led);
   output_low(Red_Led);      //OFF
   output_low(Blue_Led); 
   delay_ms(Init_delay);
   output_high(Green_Led);
   output_low(Red_Led);      //Green
   output_low(Blue_Led);
   delay_ms(Init_delay);
   output_high(Green_Led);
   output_high(Red_Led);      //Yellow
   output_low(Blue_Led); 
   delay_ms(Init_delay);
   output_low(Green_Led);
   output_high(Red_Led);      //Red
   output_low(Blue_Led); 
   delay_ms(Init_delay);
   output_low(Green_Led);
   output_high(Red_Led);      //Magenta
   output_high(Blue_Led); 
   delay_ms(Init_delay);
   output_low(Green_Led);
   output_low(Red_Led);      //Blue
   output_high(Blue_Led); 
   delay_ms(Init_delay);
   output_high(Green_Led);
   output_low(Red_Led);      //Cyan
   output_high(Blue_Led); 
   delay_ms(Init_delay);
   output_high(Green_Led);
   output_high(Red_Led);      //White
   output_high(Blue_Led); 
   delay_ms(500);
}
/*---------------------------------------------------------------------------*/
void Read_Voltage()
{
    set_adc_channel(0);
    delay_us(20);
    Bat_Volt = read_adc();
    delay_us(20);
    set_adc_channel(2);
    delay_us(20);
    Ext_Volt = read_adc();
}
/*---------------------------------------------------------------------------*/
void Switch_Action()
{
   if(input(Push_Switch) == 1)
   {
      delay_ms(1000);      // Big delay for Debounce
      while(input(Push_Switch) == 1)
      {
         delay_ms(100);
         //printf("%u\n",Mode);
         counter = counter + 1;
         if(counter == 10)      //Press switch for 2-4 sec reboot (1 beep)
         {
            output_high(Buzzer);
            delay_ms(Buzzer_Delay);
            output_low(Buzzer);
            Mode = 1;
         }
         else if (counter == 40)      //Press switch for 5-9 sec shutdown (2 beep)
         {
            output_high(Buzzer);
            delay_ms(Buzzer_Delay);
            output_low(Buzzer);
            delay_ms(Buzzer_Delay);
            output_high(Buzzer);
            delay_ms(Buzzer_Delay);
            output_low(Buzzer);
            Mode = 2;
         }
         else if (counter >= 90)      //Press switch for >10 sec Reset Pi (Long beep)
         {
            output_high(Buzzer);
            //delay_ms(2000);
            Mode = 3;
         }
      }
      delay_ms(500);
      output_low(Buzzer);
      counter = 0;       //Reset counter 
   }
   if(Mode == 1)
   {
      //Reboot RPi
      output_high(Reboot_Pin);
      delay_ms(500);
      output_low(Reboot_Pin);
      output_high(Buzzer);
      delay_ms(250);
      output_low(Buzzer);
   }
   else if( Mode == 2)
   {
      //Shutdown RPi
      output_high(Shutdown_Pin);
      delay_ms(500);
      output_low(Shutdown_Pin);
      output_high(Buzzer);
      delay_ms(250);
      output_low(Buzzer);
      delay_ms(250);
      output_high(Buzzer);
      delay_ms(250);
      output_low(Buzzer);
   }
   else if(Mode == 3)
   {
      //Reset RPi
      output_high(RPi_Reset);
      delay_ms(500);
      output_low(RPi_Reset);
   }
   Mode = 0;
}      
/*---------------------------------------------------------------------------*/
void Over_Voltage_Protection()
{
   if(Ext_Volt > Over_Voltage)
   {
      OVP = 1;
      output_low(Ext_Switch);//Switch off Ext Volt to protect circuit
      OVP_Delay_Counter++;
      if(OVP_Delay_Counter > 1 && OVP_Delay_Counter < 100)
      {
         output_high(Green_Led);
         //output_high(Blue_Led);
      }
      else if(OVP_Delay_Counter >= 100)
      {
         output_low(Green_Led);
         //output_low(Blue_Led);
      }
   }
   else
   {
      OVP = 0;
   }
   if(OVP_Delay_Counter == 200)
   {
      OVP_Delay_Counter = 0;
   }
   
}
/*---------------------------------------------------------------------------*/
void HDD_State()
{
   if(Ext_Volt < Ext_Low_Threshold || OVP == 1)
   {
      output_high(HDD_Pin);
   }
   else
   {
      output_low(HDD_Pin);
   }
}
/*---------------------------------------------------------------------------*/   
void Debug()
{
   //output_toggle(Debug_Pin);      // Max loop speed in Debug_Pin
   if(Count % 5 == 0)
   {
      printf("%Lu\t%Lu\t%u\t%u\n",Ext_Volt,Bat_Volt,State,OVP);
   }
   //printf("%Lu\t%Lu\n",Ext_Volt,Bat_Volt);
   Count++;
}
/*---------------------------------------------------------------------------*/   
void Shutdown()
{
   //Output shutdown pin.
   //Shutdown_Timer = Shutdown_Timer * 2;
   if(OVP == 0)
   {
      output_high(Ext_Switch);
   }
   output_high(Bat_Switch);
   //------------------------//
//!   output_high(Buzzer);
//!   delay_ms(500);
//!   output_low(Buzzer);
   //------------------------//
   output_low(Green_Led);
   output_low(Red_Led);
   //------------------------//
   output_high(Shutdown_Pin);
   delay_ms(500);
   output_low(Shutdown_Pin);
   //------------------------//
   while(Shutdown_Timer > 0)
   {
      output_high(Blue_Led);
      if(Shutdown_Timer % 5 == 0)
      {
      output_high(Buzzer);
      }
      delay_ms(500);
      output_low(Blue_Led);
      output_low(Buzzer);
      delay_ms(500);
      Shutdown_Timer--;
   }
   Shutdown_Timer = 60; // Reset 1 Minute time for shutdown.
   State = 0;
   output_low(Ext_Switch);
   output_low(Bat_Switch);
   output_low(Green_Led);
   output_low(Red_Led);
   output_low(Blue_Led);
   delay_ms(5000); // Wait for 5 sec 
}
/*---------------------------------------------------------------------------*/
void main()
{
   initialise(); //Done
   while(True)
   {
      delay_ms(10);
      Read_Voltage(); //Done
      Over_Voltage_Protection(); //Done
      HDD_State();
      Switch_Action();
      Debug();
      /*-----------------------------------A.1.a-----------------------------------*/
      if(Ext_Volt > Ext_High_Threshold && Bat_Volt > Bat_High_Threshold && OVP == 0 )  
      {
         State = 1;
         output_high(Ext_Switch);
         output_high(Bat_Switch);//check
         output_high(Green_Led);
         output_low(Red_Led);      // Green
         output_low(Blue_Led); 
      }
      /*-----------------------------------A.1.b-----------------------------------*/
      else if(Ext_Volt > Ext_High_Threshold && Bat_Volt > Bat_Low_Threshold && Bat_Volt < Bat_High_Threshold && OVP == 0)
      {
         State = 1;
         output_high(Ext_Switch);
         output_high(Bat_Switch);//check
         output_high(Green_Led);
         output_low(Red_Led);      //Cyan //Note : As long as Ext Supply is present Bat Volt will not go below 12.5V (charging circuit)
         output_high(Blue_Led); 
      }
      /*-----------------------------------A.1.c-----------------------------------*/
      else if(Ext_Volt > Ext_High_Threshold && Bat_Volt < Bat_Low_Threshold && OVP == 0)
      {
         State = 1;
         output_high(Ext_Switch);
         output_low(Bat_Switch);//check
         output_high(Green_Led);
         output_high(Red_Led);      //Yellow //Note : As long as Ext Supply is present Bat Volt will not go below 12.5V (charging circuit)
         output_low(Blue_Led); 
      }
      /*----------------------------------OVP-1-----------------------------------*/
      else if(Bat_Volt > Bat_High_Threshold && OVP == 1)
      {
         State = 1;
         output_high(Bat_Switch);
         output_low(Red_Led); 
         output_low(Blue_Led); //Green Blinks (in OVP Fn)
      }
      /*----------------------------------OVP-2-----------------------------------*/
      else if(Bat_Volt > Bat_Low_Threshold && Bat_Volt < Bat_High_Threshold && OVP == 1)
      {
         State = 1;
         output_high(Bat_Switch);
         output_low(Red_Led);
         output_high(Blue_Led); //Cyan and Blue Blink
      }
      /*----------------------------------OVP-3-----------------------------------*/
      else if(Bat_Volt < Bat_Low_Threshold && OVP == 1)      // Ext volt > OV & Bat volt < Low_Threshold
      {
         //output_low(Bat_Switch);
         output_high(Red_Led);        // Yellow and Red Blinks with Low Bat and OVP after shutdown is executed
         output_low(Blue_Led); 
         if(State == 1)
         {
            Shutdown();
         }
      }      
      /*---------------------------------------------------------------------------*/
      /*-----------------------------------A.2.a-----------------------------------*/
      if(Ext_Volt > Ext_Low_Threshold && Ext_Volt < Ext_High_Threshold && Bat_Volt > Bat_High_Threshold)
      {
         State = 1;
         output_high(Ext_Switch);
         output_high(Bat_Switch);
         output_high(Green_Led);
         output_low(Red_Led);      //Cyan
         output_high(Blue_Led); 
      }
      /*-----------------------------------A.2.b-----------------------------------*/
      else if(Ext_Volt > Ext_Low_Threshold && Ext_Volt < Ext_High_Threshold && Bat_Volt > Bat_Low_Threshold && Bat_Volt < Bat_High_Threshold)
      {
         State = 1;
         output_high(Ext_Switch);
         output_high(Bat_Switch);
         output_high(Green_Led);
         output_low(Red_Led);      //Cyan //Note : As long as Ext Supply is present Bat Volt will not go below 11.5V (charging circuit)(Min 12V for Ext then -0.5V in Charging Ckt)
         output_high(Blue_Led); 
      }
      /*-----------------------------------A.2.c-----------------------------------*/
      else if(Ext_Volt > Ext_Low_Threshold && Ext_Volt < Ext_High_Threshold && Bat_Volt < Bat_Low_Threshold)
      {
         State = 1;
         output_high(Ext_Switch);
         output_low(Bat_Switch);
         output_high(Green_Led);
         output_high(Red_Led);      //Yellow //Note : As long as Ext Supply is present Bat Volt will not go below 11.5V (charging circuit)(Min 12V for Ext then -0.5V in Charging Ckt)
         output_low(Blue_Led); 
      }
      /*---------------------------------------------------------------------------*/
      /*----------------------------------A.3.a------------------------------------*/
      if(Ext_Volt < Ext_Low_Threshold && Bat_Volt > Bat_High_Threshold && State == 0)
      {
         State = 1;
         output_low(Ext_Switch);
         output_high(Bat_Switch);
         output_low(Green_Led);
         output_high(Red_Led);      //Magenta
         output_high(Blue_Led); 
      }
      /*----------------------------------A.3.b------------------------------------*/
      else if(Ext_Volt < Ext_Low_Threshold && Bat_Volt > Bat_Low_Threshold && Bat_Volt < Bat_High_Threshold && State == 0)
      {
         State = 0;
         output_low(Ext_Switch);
         output_low(Bat_Switch);
         output_low(Green_Led);
         output_high(Red_Led);      //Red
         output_low(Blue_Led); 
      }
      /*----------------------------------A.3.c------------------------------------*/
      else if(Ext_Volt < Ext_Low_Threshold && Bat_Volt < Bat_Low_Threshold && State == 0)
      {
         State = 0;
         output_low(Ext_Switch);
         output_low(Bat_Switch);
         output_low(Green_Led);
         output_low(Blue_Led);
         output_high(Red_Led);      //Red Blinks 
         delay_ms(500);
         output_low(Red_Led);
         delay_ms(500);
          
      }
      /*---------------------------------------------------------------------------*/
      if(Ext_Volt < Ext_Low_Threshold && Bat_Volt > Bat_Low_Threshold && Beep_Flag == 0)
      {
         if(Beep_Time == 50)
         {
            output_high(Buzzer);
         }
         if(Beep_Time == 0)
         {
            output_low(Buzzer);
            Beep_Time = 51;
            Beep_Flag = 1;
         }
         Beep_Time--;
      }
      else if(Ext_Volt > Ext_High_Threshold)
      {
         Beep_Flag = 0;
         output_low(Buzzer);
      }
      /*---------------------------------------------------------------------------*/
      /*---------------------------------------------------------------------------*/
      /*----------------------------------B.3.a------------------------------------*/
      if(Ext_Volt < Ext_Low_Threshold && Bat_Volt > Bat_High_Threshold && State == 1)
      {
         State = 1;
         output_low(Ext_Switch);
         output_high(Bat_Switch);
         output_low(Green_Led);
         output_high(Red_Led);      //Magenta
         output_high(Blue_Led); 
      }
      /*----------------------------------B.3.b------------------------------------*/
      else if(Ext_Volt < Ext_Low_Threshold && Bat_Volt > Bat_Low_Threshold && Bat_Volt < Bat_High_Threshold && State == 1)
      {
         State = 1;
         output_low(Ext_Switch);
         output_high(Bat_Switch);
         output_low(Green_Led);
         output_high(Red_Led);      //Red 
         output_low(Blue_Led); 
      }
      /*----------------------------------B.3.c------------------------------------*/
      else if(Ext_Volt < Ext_Low_Threshold && Bat_Volt < Bat_Low_Threshold && State == 1) 
      {
         //Check whether its not caused due to a power surge. So wait for 1 sec and check again.
         delay_ms(50);      // Dont know whether its a good idea to delay in btw
         Read_Voltage();
         if(Bat_Volt < Bat_Low_Threshold)
         {
            //Keep Both Switches on during shutdown
//!            if(OVP == 0)
//!            {
//!            output_high(Ext_Switch); // So even if power comes in & Bat Volt too low , it will deliver current till shutdown is complete.
//!            }
//!            output_high(Bat_Switch);
            // Shutdown Procedure Starts
            Shutdown();
            //delay_ms(5000);
         }
         else
         {
            //Power surge... maintain previous state.
         }
      }
   }
}
/*---------------------------------------------------------------------------*/
/*Started 15 July 2020*/
/*---------------------------------------------------------------------------*/
/*------------------------------Extra Details--------------------------------*/
/*---------------------------------------------------------------------------*/
/*
A      --> At first power on
1.a      --> Ext power > & Bat power >    --> Both Ext & Bat Sw On   --> Green
1.b      --> Ext power > & Bat power <>   --> Both Ext & Bat Sw On   --> Cyan*
1.c      --> Ext power > & Bat power <   --> Ext On & Bat Off      --> Yellow*
2.a      --> Ext power <> & Bat power >   --> Both Ext & Bat Sw On   --> Cyan
2.b      --> Ext power <> & Bat power <>   --> Both Ext & Bat Sw On   --> Cyan
2.c      --> Ext power <> & Bat power <   --> Ext On & Bat Off      --> Yellow
3.a      --> Ext power < & Bat power >   --> Both Ext & Bat Sw On   --> Magenta
3.b      --> Ext power < & Bat power <>   --> Both Ext & Bat Sw On   -->   Red
3.c      --> Ext power < & Bat power <   --> Red Led Blinks indicating no power
B      --> Change in Bat levels with no Ext power
3.a      --> Ext power < & Bat power >    --> Both Ext & Bat Sw On
3.b      --> Ext power < & Bat power <>    --> Both Ext & Bat Sw On
3.c     --> Ext power < & Bat power <    --> So Shutdown signal given to RPi
/*---------------------------------------------------------------------------*/
