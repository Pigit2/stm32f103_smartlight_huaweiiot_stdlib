#include "adc.h"


float ADC_Value[2];
volatile uint16_t ADCConvertedValue[1000][2];//[10][3]�������ADCת�������Ҳ��DMA��Ŀ���ַ,3ͨ����ÿͨ���ɼ�10�κ���ȡƽ����
																					//A[0][0] A[0][1]
																					//A[1][0] A[1][1]
																					//A[2][0] A[2][1]
																					//A[3][0] A[3][1]
																					//   ...    ...
																					//   CH1    CH2

void ADC_int(void)
{
	GPIO_InitTypeDef GPIO_InitStrue;
	ADC_InitTypeDef ADC_InitStruct;
	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1|RCC_APB2Periph_GPIOA,ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//ʹ��ʱ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//����ADC1�ķ�Ƶ���� 72/6=12M<14M
	
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AIN;//PA1ģ������
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_0;
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	GPIO_InitStrue.GPIO_Mode=GPIO_Mode_AIN;//PA1ģ������
	GPIO_InitStrue.GPIO_Pin=GPIO_Pin_2;
	GPIO_Init(GPIOA,&GPIO_InitStrue);
	
	
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;//ADת��ģʽ
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;//ADC���ݶ���ģʽ���Ҷ���
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;//��������ģʽ
	ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;//ADC����ģʽ:����ģʽ
	ADC_InitStruct.ADC_NbrOfChannel = 2;//˳����й���ת����ADC ͨ������Ŀ1
	ADC_InitStruct.ADC_ScanConvMode = ENABLE;//AD ��ͨ��ģʽ 
	ADC_Init(ADC1,&ADC_InitStruct);
	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_0,1,ADC_SampleTime_239Cycles5);//ͨ��һת��������浽ADCConvertedValue[0~10][0]
	ADC_RegularChannelConfig(ADC1,ADC_Channel_2,2,ADC_SampleTime_239Cycles5);//ͨ����ת��������浽ADCConvertedValue[0~10][1]
	
	ADC_DMACmd(ADC1, ENABLE);//����DMA֧��
	ADC_Cmd(ADC1,ENABLE);//ʹ��ADC1
	
	ADC_ResetCalibration(ADC1);//ADCʹ�ܸ�λУ׼
	while(ADC_GetResetCalibrationStatus(ADC1));//�ȴ�ADCʹ�ܸ�λУ׼����
	
	ADC_StartCalibration(ADC1);//����ADУ׼
	while(ADC_GetCalibrationStatus(ADC1));//�ȴ�ADCУ׼����


	DMA_DeInit(DMA1_Channel1);    //��ͨ��һ�Ĵ�����ΪĬ��ֵ
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(ADC1->DR);//�ò������Զ���DMA�������ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADCConvertedValue;//�ò������Զ���DMA�ڴ����ַ(ת���������ĵ�ַ)
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//�ò����涨����������Ϊ���ݴ����Ŀ�ĵػ�����Դ���˴�����Ϊ��Դ
  DMA_InitStructure.DMA_BufferSize = 2*1000;//����ָ��DMAͨ����DMA����Ĵ�С,��λΪ���ݵ�λ������Ҳ����ADCConvertedValue�Ĵ�С
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�趨�����ַ�Ĵ����������,�˴���Ϊ���� Disable
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�����趨�ڴ��ַ�Ĵ����������,�˴���Ϊ������Enable
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//���ݿ��Ϊ16λ
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//���ݿ��Ϊ16λ
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //������ѭ������ģʽ
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;//DMAͨ��ӵ�и����ȼ� �ֱ�4���ȼ� �͡��С��ߡ��ǳ���
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//ʹ��DMAͨ�����ڴ浽�ڴ洫��
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);//����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��

  DMA_Cmd(DMA1_Channel1, ENABLE);//����DMAͨ��һ
	
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);//��ʼ�ɼ�
	
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
//	
//  
//  NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; 
//  NVIC_Init( & NVIC_InitStructure);
}

void ADC_get_voltage(void)
{
	
	float sum;
	uint16_t i,j;
	
	for(i=0;i<2;i++)
	{
		sum=0;
		for(j=0;j<1000;j++)
		{
			sum+=ADCConvertedValue[j][i];

		}
		ADC_Value[i]=(float)sum/(1000*4096)*3.3;//��ƽ��ֵ��ת���ɵ�ѹֵ	
		
	}
	
}


float Voltage_value(void)
{
	float temp = ADC_Value[1]*1220/220;
	if(temp < 0)
		return 0;
	else
		return temp;
}


float Current_value(void)
{
	float temp = (ADC_Value[0]-2.58)/0.185;
	if(temp < 0)
		return 0;
	else
		return temp;
}


float Power_value(void)
{
	return Voltage_value() * Current_value();
}


