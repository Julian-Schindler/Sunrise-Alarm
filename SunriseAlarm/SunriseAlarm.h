/*
 * SunriseAlarm.h
 *
 * Created: 22.10.2014 20:03:40
 *  Author: Julian
 */ 


#ifndef SUNRISEALARM_H_
#define SUNRISEALARM_H_

#define FALSE 0
#define TRUE 1

#define BAUD 19200
#define MYUBRR F_CPU/16/BAUD-1

int currentTime[2];	//hour/minute

#define ACT_RESTART 0

#define ACT_ALARM_START 2
#define ACT_ALARM_STOP 3
#define ACT_MINUTE_START 4

#define ACT_ENCODER_LEFT 10
#define ACT_ENCODER_RIGHT 11
#define ACT_ENCODER_BUTTON 12

#define ACT_BUTTON1 13
#define ACT_BUTTON2 14
#define ACT_BUTTON3 15

#define STATE_SET_HOUR 1
#define STATE_SET_MINUTE 2
#define STATE_SET_WEEKDAY 3
#define STATE_SET_DAY 4
#define STATE_SET_MONTH 5
#define STATE_SET_YEAR 6
#define STATE_SET_INTENSITY 6

#define STATE_INPUT_OVER 20

#define SCREEN_RESET 0
#define SCREEN_HOME 1
#define SCREEN_SET_TIME 2
#define SCREEN_SET_ALARM 3
#define SCREEN_CONFIG_ALARM 4








/*	LCD Backlight	*/
#define LCD_BL_DDR DDRD
#define LCD_BL_PORT PORTD
#define LCD_BL_PIN 4
	#define LCD_BL_ON LCD_BL_PORT |= (1<<LCD_BL_PIN);
	#define LCD_BL_OFF LCD_BL_PORT &= ~(1<<LCD_BL_PIN);

/*	Buttons	*/
#define BUTTON_DDR DDRC
#define BUTTON_PIN PINC
#define BUTTON_PORT PORTC

#define BUTTON1_NR 3			//push button 1
#define BUTTON2_NR 2			//push button 2
#define BUTTON3_NR 0			//push button 3

/*	Encoder	*/
#define ENCODER_DDR DDRD
#define ENCODER_PIN PIND
#define ENCODER_PORT PORTD
#define ENCODER_BUTTON_NR 7	//encoder button
#define ENCODER_A_NR 6		//A
#define ENCODER_B_NR 5		//B

/*	Input from RTC	*/
#define RTC_INT_DDR DDRD
#define RTC_INT_PIN PIND
#define RTC_INT_NR 3
#define RTC_INT_PORT PORTD
#define RTC_INT_ACT (RTC_INT_PIN & (1<<RTC_INT_NR))

/*	Piezo summer (DC)	*/
#define SUMMER_DDR DDRB
#define SUMMER_PORT PORTB
#define SUMMER_NR 7

#define SUMMER_ON (SUMMER_PORT |= (1<<SUMMER_NR))
#define SUMMER_OFF (SUMMER_PORT &= ~(1<<SUMMER_NR))
#define SUMMER_TOG (SUMMER_PORT ^= (1<<SUMMER_NR))


/* TEST LED */

#define LED_DDR DDRB
#define LED_PORT PORTB
#define LED_NR 6

#define LED_ON (LED_PORT |= (1<<LED_NR))
#define LED_OFF (LED_PORT &= ~(1<<LED_NR))
#define LED_TOG (LED_PORT ^= (1<<LED_NR))


void ActionHandle(uint8_t action);
void RTC_start_min_interrupt();
void DisplayWriteCurrDate(void);
void menu_setTime(uint8_t action);
uint8_t check_alarms(uint8_t minute, uint8_t hour, uint8_t day);

#endif /* SUNRISEALARM_H_ */