/*
 * LM75.h
 *
 * Created: 27.11.2013 18:53:33
 *  Author: Julian
 *
 *	LM75 I2C Temperatur Sensor
 * 
 *
 *
 * 
 */ 


#ifndef LM75_H_
#define LM75_H_

/*	Register select	*/
#define RS_TEMP		0x00	//temperature		read only
#define RS_CONFIG	0x01	//configuration		r/w
#define RS_THYST	0x02	//hysterese			r/w
#define RS_TOS		0x03	//alarm temp		r/w

static int lm75Address;

void lm75_setAddress(uint8_t add);
int lm75_getTemp();
uint8_t lm75_readTemp(void);	//return MSB of Temperature
		//return LSB of Temp
		
void lm75_setPointer(uint8_t adress);
void lm75_write(uint8_t data);


/************************************************************************/
/*                                                                      */
/************************************************************************/

void lm75_setAddress(uint8_t add)
{
	//Adress: 1001.XXX(R/W)
	add &= 0x0F;		//clear bit7-4
	add |= 0b10010000;	//set factury adress bits 7-4
	lm75Address = add;
}
uint8_t lm75_getAdress()
{
	return lm75Address;
}

uint8_t lm75_readUint8Temp(void)
{
	lm75_setPointer(0x00);

	I2CReadByte(lm75Address | 0x01);	//set r/w bit to 'read'	
	lm75_setPointer(0x00);	
	uint8_t MSB = I2CReadByte(lm75Address | 0x01);	//set r/w bit to 'read'

	return MSB;
}
int lm75_getTemp()
{
	int currTemp;
	uint8_t tempRead = lm75_readUint8Temp();
	
	if((tempRead & 0x80))	//negative temperature
	{
		currTemp = 128 - (tempRead & 0x7F);	//clear sign bit
		currTemp *= -1;	//set sign
	}
	else	//positive temperature
	{
		currTemp = tempRead;
	}

	return currTemp;	
}

void lm75_setPointer(uint8_t adress)
{
	I2CWriteByte((lm75Address & 0xFE),RS_TEMP);
}

#endif /* LM75_H_ */