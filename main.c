/*
 * File:   main.c
 * Author: DELL
 *
 * Created on 8 April, 2022, 6:39 AM
 */


#include <xc.h>
#include "main.h"
#pragma config WDTE = OFF // Watchdog timer enable bit-WDT disabled
static void init_config(void)
{   
    //Initialization of CLCD Module
    init_clcd();
    
    //Initialization of I2C Module
     init_i2c(100000);//100k
     
    //Initialization of RTC Module
     init_ds1307();
     
    //Initialization of ADC Module
     init_adc();
     
    //Initialization of Digital Keypad Module
     init_digital_keypad();
     
    //Initialization of timer2 Module
    init_timer2();
    PEIE=1;
    GIE=1;
   
    
}
void main(void) {
    char event[3]="ON";
    unsigned char speed=0;
    unsigned char control_flag=DASH_BOARD_FLAG; //for dashboard
    unsigned char key, reset_flag, menu_pos;
    char *gear[]={"GN","GR","G1","G2","G3","G4"};
    //        gear[0]  [1]  [2]  [3]  [4]  [5]
    unsigned char gr= 0;
    
    init_config();
    log_car_event(event, speed);
    //Password for access= 2424- first 4 addresses used for password in eeprom
    eeprom_write(0x00,'2');
    eeprom_write(0x01,'4');
    eeprom_write(0x02,'2');
    eeprom_write(0x03,'4');
    while(1)
    {   speed= read_adc()/10; //0 to 1023  after /10-> 0 10 102
    if (speed>99) //100 101 102
    { 
        speed=99;
    }
    key =read_digital_keypad(STATE);
    
    //IF DKP SWITCH 1 PRESSED
    if (key == SW1)
    {
        strcpy(event,"C ");
        log_car_event(event, speed);
    }
    //IF DKP SWITCH 2 PRESSED
    else if (key == SW2 && gr<=5)
    {
       strcpy(event, gear[gr]);
       log_car_event(event, speed);
       gr++;
    }
    //IF DKP SWITCH 3 PRESSED
    else if (key == SW3 && gr>0)
    {
        gr--;
        strcpy(event, gear[gr]);
        log_car_event(event, speed);
    }
    //screen is dashboard and key-> SW4 or SW5 login screen
    else if((control_flag== DASH_BOARD_FLAG) && (key==SW4 || key==SW5))
    {   
        clear_screen();
        clcd_print(" ENTER PASSWORD",LINE1(0));
        clcd_write(CURSOR_POS,INST_MODE);
        clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
         __delay_us(100);
        control_flag= LOGIN_FLAG;
        reset_flag = RESET_PASSWORD;
        //Switching on timer
        TMR2ON=1;

    }
    else if(control_flag == LOGIN_MENU_FLAG && (key==SW6))
    {
          switch (menu_pos)  
          {
              case 0://logic for view log menu
                  clear_screen();
                  clcd_print("# TIME     E  SP",LINE1(0));
                  control_flag=VIEW_LOG_FLAG;
                  reset_flag=VIEW_LOG_RESET;
                  
                  break;
              case 1://logic for Clear log menu
                  log_car_event("CL", speed);
                  clear_screen();
                  control_flag=CLEAR_LOG_FLAG;
                  reset_flag=RESET_MEMORY;
                  break;
              case 2://logic for Change Passwrd menu
                  log_car_event("CP", speed);
                  clear_screen();
                  control_flag = CHANGE_PASSWORD_FLAG;
                  reset_flag = RESET_CHANGE_PASSWORD;
                  break;
          }
    }
    
    else if((control_flag==VIEW_LOG_FLAG) && (key == SW6))
    {
        control_flag = DASH_BOARD_FLAG;
    }
  
    switch(control_flag)
    {
        case DASH_BOARD_FLAG: //dashboard
            display_dash_board(event, speed);
            break;
        case LOGIN_FLAG:
            switch(login(key, reset_flag))
            {
                case RETURN_BACK://See time is over
                    control_flag=DASH_BOARD_FLAG;
                    TMR2ON=0;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                     __delay_us(100);
                    break;
                case TASK_SUCCESS:
                    control_flag=LOGIN_MENU_FLAG;
                    reset_flag=RESET_LOGIN_MENU;
                    clear_screen();
                    TMR2ON=0;
                    clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                     __delay_us(100);
                    
                    continue;
                    
                    break;
                
            }
            break;
        case LOGIN_MENU_FLAG:
            menu_pos=login_menu(key,reset_flag);
            break;
        case VIEW_LOG_FLAG:
            view_log(key, reset_flag);
            break;
        case CLEAR_LOG_FLAG:
           if(clear_log(reset_flag)== TASK_SUCCESS)
           {   clear_screen();
               control_flag= LOGIN_MENU_FLAG;
               reset_flag= RESET_LOGIN_MENU;
               continue;
           }
            break;
        case CHANGE_PASSWORD_FLAG:
            
            switch(change_password(key, reset_flag))
            { case TASK_SUCCESS:
               clear_screen();
               clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
               __delay_us(100);    
               control_flag= LOGIN_MENU_FLAG;
               reset_flag= RESET_LOGIN_MENU;
               continue;
            
            } break;
            
    }
      reset_flag=RESET_NOTHING;
  }
    
}   