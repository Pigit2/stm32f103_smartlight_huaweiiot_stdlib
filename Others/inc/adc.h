#ifndef __ADC_H
#define __ADC_H 			   
#include "sys.h"  


void ADC_int(void);
void ADC_get_voltage(void);//����ʱmain��float* a; a=ADC_get_voltage(); a[0],a[1]
float Voltage_value(void);
float Current_value(void);
float Power_value(void);


#endif
