/*
 * SunriseAlarm.c
 *
 * RedLabs
 * Created: 22.10.2014 20:02:46
 *  Author: Julian
 */ 

#define F_CPU 8000000

#include <avr/io.h>
#include <inttypes.h>
#include <stdlib.h>
#include <util/delay.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <string.h>
	
#include "SunriseAlarm.h"

#include "rs232_init.h"
#include "i2c_basic.h"

#include "lcd-routines.h"
#include "MCP79410.h"
#include "LM75.h"

#define DEGBUG 1

uint8_t current_screen = SCREEN_RESET;
uint8_t userAction = 100;

uint8_t new_action = 0;

uint8_t alarms[1][5];
char isAlarm = 0;

int main(void)
{	
	init();
	
	#if DEGBUG
		sende(NL NL NL "*** RedLabs Sunrise Alarm ***");
	#endif
	
	RTC_start(TRUE);	
 	RTC_enableBat(TRUE);	
	 
	RTC_start_min_interrupt();
	DisplayWriteCurrDate();
	
	LCD_BL_ON
	
 	sei();
	 
    while(1)
    {
		if(!userAction)
		{
			LCD_BL_OFF
		}
		else
		{
			userAction--;
			LCD_BL_ON
		}
       	   
		
       if(new_action != 0)
	   {
		if(new_action == ACT_BUTTON3)
		{
			LED_ON;
			_delay_ms(10000);
		}	
		   
		ActionSwitch(new_action);
		new_action = 0;
		
				
	   }
	   
	   if(isAlarm)
	   {
		  //fade up LED
		  
		  //test: switch on ligh
		  LED_ON;
		   
		   
	   }
	   else
	   {
		   LED_OFF;			
	   }
	   
	   _delay_ms(300);
	   
    }
}


void init(void)
{
	
	/* LED */
	LED_DDR |= (1<<LED_NR);
	
	/*	LCD Backlight	*/
	LCD_BL_DDR |= (1<<LCD_BL_PIN);
	
	/*	Buttons */
	BUTTON_DDR &= ~(1<<BUTTON1_NR);	//B1 input
	BUTTON_PORT |= (1<<BUTTON1_NR);	//B1 pullup
	BUTTON_DDR &= ~(1<<BUTTON2_NR);	//B2 input
	BUTTON_PORT |= (1<<BUTTON2_NR);	//B2 pullup
	BUTTON_DDR &= ~(1<<BUTTON3_NR);	//B3 input
	BUTTON_PORT |= (1<<BUTTON3_NR);	//B3 pullup
	
	/* Encoder (PCINT21-23)	*/
	ENCODER_DDR &= ~(1<<ENCODER_A_NR);
	ENCODER_DDR &= ~(1<<ENCODER_B_NR);
	ENCODER_DDR &= ~(1<<ENCODER_BUTTON_NR);
	ENCODER_PORT |= (1<<ENCODER_A_NR) | (1<<ENCODER_B_NR) | (1<<ENCODER_BUTTON_NR);	//pullup
	
	
	PCICR |= (1<<PCIE1);	//enable PCINT 8-14
	PCICR |= (1<<PCIE2);	//enable PCINT 16-23
	PCIFR |= (1<<PCIF1);	//enable interrupt request PCINT 8-14
	PCIFR |= (1<<PCIF2);	//enable interrupt request PCINT 16-23
	PCMSK1 |= (1<<PCINT8) | (1<<PCINT10) | (1<<PCINT11); //Buttons PCINT8 PCINT10 PCINT11
	PCMSK2 |= (1<<PCINT21) | (1<<PCINT22) | (1<<PCINT23); //Encoder (PCINT21-23)
	
	
	/*	UART	*/
	DDRD |= (1<<1); // TX as Output
	DDRD &= ~(1<<0); //RX as Input 
	USARTInit(MYUBRR);  //15 == UBRR
	
	/* I2C	*/
	DDRC |= (1<<5);
	I2CInit();	
	
	/*	MCP79410 RTC	*/
	RTC_INT_DDR &= ~(1<<RTC_INT_NR);	//input
	RTC_INT_PORT |= (1<<RTC_INT_NR);	//int. pullup
	EICRA |= (1<<ISC10);
	EIMSK |= (1<<INT1);
	
	/*	LM75 Temperatur Sensor	*/
	lm75_setAddress(0x00); //A0-A2 'low'
	
	/*	Summer	*/
	SUMMER_DDR |= (1<<SUMMER_NR);
	
	/* LCD	*/
	lcd_init();
	
}



/************************************************************************/
/*                                ISR                                   */
/************************************************************************/

ISR(INT1_vect)	//Interrupt from RTC every minute
{
	cli();
	
	/*	reset ALM0IF bit	*/
	uint8_t configByte = SRAMReadByte(0x0D);
	configByte &= ~(1<<ALM0IF);	
	SRAMWriteByte(0x0D,configByte);
	
	uint8_t currMin = decodeBCD(SRAMReadByte(ADD_time_min),0x7F);
	uint8_t currHour = decodeBCD(SRAMReadByte(ADD_time_hour),0x3F);
	uint8_t currDay = decodeBCD(SRAMReadByte(ADD_time_day),0x07);
	
	check_alarms(currMin,currHour,currDay);
	
	ActionHandle(ACT_MINUTE_START);
	
	sei();
}

ISR(PCINT1_vect)	//Interrupt from Buttons
{
	cli();
	if(!(BUTTON_PIN & (1<<BUTTON1_NR)))//0 = active
	{
		// button clicked
		ActionHandle(ACT_BUTTON1);
		#if DEGBUG
			sende(NL"B1");
		#endif
	}
	else if(!(BUTTON_PIN & (1<<BUTTON2_NR)))//0 = active
	{
		// button clicked
		ActionHandle(ACT_BUTTON2);
		#if DEGBUG
			sende(NL"B2");
		#endif
	}
	else if(!(BUTTON_PIN & (1<<BUTTON3_NR)))//0 = active
	{
		// button clicked
		ActionHandle(ACT_BUTTON3);
		#if DEGBUG
			sende(NL"B3");
		#endif
	}
	
	_delay_ms(300);
	sei();
}	

ISR(PCINT2_vect)	//Interrupt from encoder and encoder-button
{
	cli();
	static char lastA = 0;
	static char lastB = 0;

	_delay_us(500);
		
	
	if(!(ENCODER_PIN & (1<<ENCODER_BUTTON_NR)))//0 = active
	{
		// encoder button clicked
		ActionHandle(ACT_ENCODER_BUTTON);
		#if DEGBUG
			sende(NL"enc button");
		#endif
		_delay_ms(300);
	}
	
	if(!(ENCODER_PIN & (1<<ENCODER_A_NR)))	//0 = active
	{
		lastA = (lastA<<1);
		lastA |= (1<<0);
	}
	else
	{
		lastA = (lastA<<1);
		lastA &= ~(1<<0);
	}
	
	if(!(ENCODER_PIN & (1<<ENCODER_B_NR)))
	{
		lastB = (lastB<<1);
		lastB |= (1<<0);
	}
	else
	{
		lastB = (lastB<<1);
		lastB &= ~(1<<0);
	}
	
	if(((lastA & 0x03) == 3 && (lastB & 0x03) == 1)) //||((lastA & 0x03) == 0 && (lastB & 0x03) == 2) )
	{
		//encoder left turn
		ActionHandle(ACT_ENCODER_LEFT);
		#if DEGBUG
			sende(NL"l");
		#endif
	}
	else if (((lastB & 0x03) == 3 && (lastA & 0x03) == 1))
	{
		//encoder right turn
		ActionHandle(ACT_ENCODER_RIGHT);
		#if DEGBUG
			sende(NL"r");
		#endif
	}
	sei();
}

ISR(__vector_default)
{
	#if DEGBUG
			sende(NL"ISR error");
		#endif
	_delay_ms(300);
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

void ActionHandle(uint8_t action)
{
	new_action = action;
}

void ActionSwitch(uint8_t action)
{
	
	if(action == ACT_MINUTE_START) //aktualisiere LCD
	{
		DisplayWriteCurrDate();
	}	
	else
	{
		//user action or alarm
		userAction = 100;
	}
	
	switch(current_screen)
	{
		case SCREEN_RESET:
			current_screen = SCREEN_HOME;
		break;
		case SCREEN_HOME:
			DisplayWriteCurrDate();	
			if(action == ACT_BUTTON1)	
			{
				current_screen = SCREEN_SET_TIME;
				menu_setTime(action);
			}				
			else if(action == ACT_BUTTON2)	
			{
				menu_setAlarm(action);
				current_screen = SCREEN_SET_ALARM;
			}	
			else if(action == ACT_BUTTON3)	
			{
				//menu_configAlarm(action);
				//current_screen = SCREEN_CONFIG_ALARM;
			}				
		break;
		case SCREEN_SET_TIME:
			menu_setTime(action);
		break;
		case SCREEN_SET_ALARM:
			menu_setAlarm(action);
		break;
		default:
			sende("default");
		break;		
	}			
			
// 	switch(menu_status)
// 	{
// 		case 0:
// 			if(action == ACT_ENCODER_LEFT && (menu_index[0]>0))	menu_index[0]--;
// 			if(action == ACT_ENCODER_RIGHT && menu_index[0]<6)	menu_index[0]++;
// 			if(action == ACT_ENCODER_BUTTON) menu_status = menu_index[0];
// 		break;
// 		case 2:
// 		break;
// 		case 3:
// 			menu_setTime(action);
// 		break;
// 		case 4:
// 			menu_setAlarm1(action);
// 		break;
// 		case 5:
// 			menu_setAlarm1_stop(action);
// 		break;
// 		case 6:
// 			menu_setAlarm1_enable(action);
// 		break;
// 		
// 		default:
// 		break;
// 	}
// 	
// 	if(menu_status < 3 ) setDisplayMain(0);
	return TRUE;
}


// 	 uint8_t new_Hour = 0;
// 	 uint8_t new_Min = 0;
// 	 uint8_t new_WDay = 1;
// 	 uint8_t new_Day = 1;
// 	 uint8_t new_Month = 1;
// 	 uint8_t new_Year = 14; //year+2000 will be set	
	 

	
	

/************************************************************************/
/*                                                                      */
/************************************************************************/
void menu_setTime(uint8_t action)
{
	static uint8_t new_Hour = 0;
	static uint8_t new_Min = 0;
	static uint8_t new_WDay = 1;
	static uint8_t new_Day = 1;
	static uint8_t new_Month = 1;
	static uint8_t new_Year = 14; //year+2000 will be set	
	
	static uint8_t state = 0;

	if(state == 0)
	{
		  new_Hour = RTC_getTime_Hour();
		  new_Min = RTC_getTime_Minute();
		  new_WDay = RTC_getTime_WeekDay();
		  new_Day = RTC_getTime_Day();
		  new_Month = RTC_getTime_Month();
		  new_Year = RTC_getTime_Year(); //year+2000 will be set	
		  state = 1;
	}	  
	
	
	switch(state)
	{
		case STATE_SET_HOUR:
			if(action == ACT_ENCODER_RIGHT)
			{
				new_Hour++;				
			}
			else if(action == ACT_ENCODER_LEFT)
			{
				if(new_Hour == 0)
					new_Hour = 23;
				else
				new_Hour--;
			}
			else if(action == ACT_ENCODER_BUTTON)
			{
				state ++;				
			}
		break;
		case STATE_SET_MINUTE:
			if(action == ACT_ENCODER_RIGHT)
			{
				new_Min++;				
			}
			else if(action == ACT_ENCODER_LEFT)
			{
				if(new_Min == 0)
					new_Min = 59;
				else
					new_Min--;
			}
			else if(action == ACT_ENCODER_BUTTON)
			{
				state ++;				
			}
		break;
		case STATE_SET_WEEKDAY:
			if(action == ACT_ENCODER_RIGHT)
			{
				new_WDay++;				
			}
			else if(action == ACT_ENCODER_LEFT)
			{
				if(new_WDay == 1)
					new_WDay = 7;
				else
					new_WDay--;
			}
			else if(action == ACT_ENCODER_BUTTON)
			{
				state ++;				
			}
		break;
		case STATE_SET_DAY:
			if(action == ACT_ENCODER_RIGHT)
			{
				new_Day++;				
			}
			else if(action == ACT_ENCODER_LEFT)
			{
				if(new_Day == 0)
					new_Day = 32;
				else
					new_Day--;
			}
			else if(action == ACT_ENCODER_BUTTON)
			{
				state ++;				
			}
		break;
		case STATE_SET_MONTH:
			if(action == ACT_ENCODER_RIGHT)
			{
				new_Month++;				
			}
			else if(action == ACT_ENCODER_LEFT)
			{
				if(new_Month == 1)
					new_Month = 12;
				else
					new_Month--;
			}
			else if(action == ACT_ENCODER_BUTTON)
			{
				state ++;				
			}
		break;
		case STATE_SET_YEAR:
			if(action == ACT_ENCODER_RIGHT)
			{
				new_Year++;				
			}
			else if(action == ACT_ENCODER_LEFT)
			{
				new_Year--;
			}
			else if(action == ACT_ENCODER_BUTTON)
			{
				//state = STATE_INPUT_OVER;		
				current_screen = SCREEN_HOME;		
				state = 0;		
				
				RTC_setTime(new_Hour,new_Min,0);
				RTC_setDate(new_Day,new_Month,new_Year);
				RTC_setDay(new_WDay);
				RTC_start(TRUE);
			}
		break;
		case STATE_INPUT_OVER:
			//send new time to RTC
			 
			
		break;
	}
	
	if(new_Hour > 23)
		new_Hour = 0;
	if(new_Min > 59)
		new_Min = 0;
	if(new_WDay > 7)
		new_WDay = 1;
	if(new_Day > 32)
		new_Day = 1;
	if(new_Month > 12)
		new_Month = 1;
				
	lcd_clear();
	lcd_string("set time   ");
	if(new_Hour<10)
		lcd_string("0");
	lcd_int(new_Hour);
	lcd_string(":");
	if(new_Min<10)
		lcd_string("0");
	lcd_int(new_Min);
	lcd_setcursor(0,2); //go to 2. line	
	lcd_sendWeekday(new_WDay);
	lcd_string("    ");	
	if(new_Day<10)
		lcd_string("0");
	lcd_int(new_Day);
	lcd_string(".");
	if(new_Month<10)
		lcd_string("0");
	lcd_int(new_Month);
	lcd_string(".");
	lcd_int(new_Year+2000);

}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void menu_setAlarm(uint8_t action)
{
	static uint8_t new_Alarm_Hour = 0;
	static uint8_t new_Alarm_Min = 0;
	static uint8_t new_Week = 0;
	
	static uint8_t state = 1;
	
	switch(state)
	{
		case STATE_SET_HOUR:
			if(action == ACT_ENCODER_RIGHT)
			{
				new_Alarm_Hour++;				
			}
			else if(action == ACT_ENCODER_LEFT)
			{
				if(new_Alarm_Hour == 0)
					new_Alarm_Hour = 23;
				else
					new_Alarm_Hour--;
			}
			else if(action == ACT_ENCODER_BUTTON)
			{
				state ++;				
			}
		break;
		case STATE_SET_MINUTE:
			if(action == ACT_ENCODER_RIGHT)
			{
				new_Alarm_Min++;				
			}
			else if(action == ACT_ENCODER_LEFT)
			{
				if(new_Alarm_Min == 0)
					new_Alarm_Min = 59;
				else
					new_Alarm_Min--;
			}
			else if(action == ACT_ENCODER_BUTTON)
			{
				state ++;				
			}
		break;
		case STATE_SET_WEEKDAY:
			if(action == ACT_ENCODER_RIGHT)
				{
					new_Week++;				
				}
				else if(action == ACT_ENCODER_LEFT)
				{
					new_Week--;
				}
				else if(action == ACT_ENCODER_BUTTON)
				{
					state == STATE_INPUT_OVER;		
					current_screen = SCREEN_HOME;		
					state = STATE_SET_HOUR;		
					
					//write alarm to alarm array
					alarms[0][0] = 1;
					alarms[0][1] = new_Alarm_Hour;
					alarms[0][2] = new_Alarm_Min; 
					alarms[0][3] = new_Alarm_Hour;
					alarms[0][4] = new_Alarm_Min+10;
					alarms[0][5] = 0xFF; //every day
				}
		break;
		case STATE_INPUT_OVER:
			//send new alarm time to RTC
			
		break;
	}	
	
	if(new_Alarm_Hour > 23)
		new_Alarm_Hour = 0;
	if(new_Alarm_Min > 59)
		new_Alarm_Min = 0;
	if(new_Week > 3)
		new_Week = 1;

				
	lcd_clear();
	lcd_string("set alarm  ");
	if(new_Alarm_Hour<10)
		lcd_string("0");
	lcd_int(new_Alarm_Hour);
	lcd_string(":");
	if(new_Alarm_Min<10)
		lcd_string("0");
	lcd_int(new_Alarm_Min);
	lcd_setcursor(0,2); //go to 2. line	
	if(new_Week == 1)
	{
		lcd_string("Mo-Fr");
	}
	else if(new_Week ==2)
	{
		lcd_string("Sa-So");
	}
	else
	{
		lcd_string("Mo-So");
	}
}	

/************************************************************************/
/*                                                                      */
/************************************************************************/

uint8_t Sunrise_Intensity = 200;

// void menu_configAlarm(uint8_t action)
// {
// 	static uint8_t Sunrise_Min = 1;
// 	static uint8_t Sunrise_Min = 1;
// 	
// 	static uint8_t state = 2;
// 	
// 	switch(state)
// 	{
// 		case STATE_SET_MINUTE:
// 			if(action == ACT_ENCODER_RIGHT)
// 			{
// 				Sunrise_Min++;				
// 			}
// 			else if(action == ACT_ENCODER_LEFT)
// 			{
// 				Sunrise_Min--;
// 			}
// 			else if(action == ACT_ENCODER_BUTTON)
// 			{
// 				state = STATE_SET_INTENSITY;				
// 			}
// 		break;
// 		case STATE_SET_INTENSITY:
// 			if(action == ACT_ENCODER_RIGHT)
// 				{
// 					Sunrise_Intensity++;				
// 				}
// 				else if(action == ACT_ENCODER_LEFT)
// 				{
// 					Sunrise_Intensity--;
// 				}
// 				else if(action == ACT_ENCODER_BUTTON)
// 				{
// 					state == STATE_INPUT_OVER;		
// 					current_screen = SCREEN_HOME;		
// 					state = STATE_SET_MINUTE;		
// 					
// 					//write alarm to alarm array
// 					alarms[0][0] = 1;
// 					alarms[0][1] = new_Alarm_Hour;
// 					alarms[0][2] = new_Alarm_Min; 
// 					alarms[0][3] = new_Alarm_Hour;
// 					alarms[0][4] = new_Alarm_Min+5;
// 					alarms[0][5] = 0xFF; //every day
// 				}
// 		break;
// 		case STATE_INPUT_OVER:
// 			//send new alarm time to RTC
// 			
// 		break;
// 	}	
// 	
// 	if(new_Alarm_Hour > 23)
// 		new_Alarm_Hour = 0;
// 	if(new_Alarm_Min > 59)
// 		new_Alarm_Min = 0;
// 	if(new_Week > 3)
// 		new_Week = 1;
// 
// 				
// 	lcd_clear();
// 	lcd_string("set alarm  ");
// 	if(new_Alarm_Hour<10)
// 		lcd_string("0");
// 	lcd_int(new_Alarm_Hour);
// 	lcd_string(":");
// 	if(new_Alarm_Min<10)
// 		lcd_string("0");
// 	lcd_int(new_Alarm_Min);
// 	lcd_setcursor(0,2); //go to 2. line	
// 	if(new_Week == 1)
// 	{
// 		lcd_string("Mo-Fr");
// 	}
// 	else if(new_Week ==2)
// 	{
// 		lcd_string("Sa-So");
// 	}
// 	else
// 	{
// 		lcd_string("Mo-So");
// 	}
// }	
/************************************************************************/
/*                                                                      */
/************************************************************************/

void lcd_sendWeekday(uint8_t weekday)
{
	switch(weekday)
	{
		case 1:
			lcd_string("Mo");
		break;
		case 2:
			lcd_string("Di");
		break;
		case 3:
			lcd_string("Mi");
		break;
		case 4:
			lcd_string("Do");
		break;
		case 5:
			lcd_string("Fr");
		break;
		case 6:
			lcd_string("Sa");
		break;
		case 7:
			lcd_string("So");
		break;
		default:
			lcd_string("er");
		break;
	}
}


void RTC_start_min_interrupt()
{	
	//1 second
	SRAMWriteByte(0x0A,0x01);	
	
	//set to Second Match (1s)
	uint8_t configByte = SRAMReadByte(0x0D);
	configByte &= ~(1<<4);
	configByte &= ~(1<<5);
	configByte &= ~(1<<5);
	SRAMWriteByte(0x0D,configByte);	
	
	//enable alarm0
	configByte = SRAMReadByte(0x07);
	configByte |= (1<<4);
	SRAMWriteByte(0x07,configByte);	
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
uint8_t check_alarms(uint8_t minute, uint8_t hour, uint8_t day)
{
	sende(NL"check alarms "NL);
// 	if(check_day(alarms[0][5],day))
// 		sende(NL"Day match");
// 	else
// 		sende(NL"no Day match");

	uint8_t alarmMax = 1; //number of alarms
	
	for(int i=0;i<alarmMax;i++)
	{
		//check start condition
		if ( (alarms[i][1] == hour) && (alarms[i][2] == minute) && (alarms[i][0]) ) //&& check_day(alarms[i][5],day) 
		{
			//set alarm ON!
			alarm_on();
		}
		else
		{
			//no match
		}
		
		//check stop condition
		if ( (alarms[i][3] == hour) && (alarms[i][4] == minute))
		{
			//set alarm OFF!
			alarm_off();
		}
		else
		{
			//no match
		}
	}
	
	return TRUE;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void alarm_on(void)
{
	isAlarm = 1;
}
void alarm_off(void)
{
	SUMMER_OFF;
	
	isAlarm = 0;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
void DisplayWriteCurrDate(void)
{
	uint8_t minute,hour,date,month,day;
	//uint8_t second, year;
	
	//read data
		//second = decodeBCD(SRAMReadByte(ADD_time_sec),0x7F);
		minute = decodeBCD(SRAMReadByte(ADD_time_min),0x7F);
		hour = decodeBCD(SRAMReadByte(ADD_time_hour),0x3F);
		
		date = decodeBCD(SRAMReadByte(ADD_time_date),0x3F);
		month = decodeBCD(SRAMReadByte(ADD_time_month),0x1F);
		//year = decodeBCD(SRAMReadByte(ADD_time_year),0xFF);
		day = decodeBCD(SRAMReadByte(ADD_time_day),0x07);
		
		currentTime[0] = hour;
		currentTime[1] = minute;
	//clear display
		lcd_clear();
	//display data 
		switch(day)
		{
			case 1:
				lcd_string("Mo");
			break;
			case 2:
				lcd_string("Di");
			break;
			case 3:
				lcd_string("Mi");
			break;
			case 4:
				lcd_string("Do");
			break;
			case 5:
				lcd_string("Fr");
			break;
			case 6:
				lcd_string("Sa");
			break;
			case 7:
				lcd_string("So");
			break;
			default:
				lcd_string("er");
			break;
		}
		lcd_string(" ");
		
		if(hour<10) lcd_string("0");
		lcd_int(hour);
		lcd_string(":");
		if(minute<10) lcd_string("0");
		lcd_int(minute);
		
		lcd_string("  ");	
		
	/*
		//write temperature
		lcd_string(" ");
		lcd_int(lm75_getTemp());
		lcd_data(0b11011111); //°
		lcd_string("C");
	*/
		lcd_setcursor(0,2);
		lcd_string("Alarm: ");
		
		lcd_int(alarms[0][1]);
		lcd_string(":");
		lcd_int(alarms[0][2]);
		
		if(alarms[0][0])
			lcd_string("  ON");
		else
			lcd_string(" OFF");
		
		
// 		lcd_string(" ");
// 		if(date<10) lcd_string("0");
// 		lcd_int(date);
// 		lcd_string(".");
// 		if(month<10) lcd_string("0");
// 		lcd_int(month);
		

}