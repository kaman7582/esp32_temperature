/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"


#define GPIO_OUTPUT_IO_0    18
#define GPIO_DATA   GPIO_OUTPUT_IO_0
#define GPIO_TEST    19
#define GPIO_BLACK_GROUND
#define GPIO_RED_VCC
//delayMicroseconds

//ets_delay_us
int data_gpio = 0;
int test_gpio = 0;
int sys_init = 0;

void temp_sensor_init()
{
	data_gpio = GPIO_DATA;
	//set gpio
	gpio_pad_select_gpio(data_gpio);
	sys_init = 1;
}

void check_out_pin()
{
	test_gpio = GPIO_TEST;
	gpio_pad_select_gpio(test_gpio);
	

}
int reset_sensor()
{
	unsigned char count = 0;
	int ret = 0;
	gpio_set_direction(data_gpio, GPIO_MODE_OUTPUT);
	gpio_set_level(data_gpio,1);
	//delay
	ets_delay_us(100);
	
	gpio_set_level(data_gpio,0);
	
	ets_delay_us(600);
	//set it to high
	gpio_set_level(data_gpio,1);
	
	ets_delay_us(45);
	//
	gpio_set_direction(data_gpio, GPIO_MODE_INPUT);

	do
	{
		ret= gpio_get_level(data_gpio);
		ets_delay_us(1);			
		count++;
	}while(ret != 0 && count<100);

	gpio_set_direction(data_gpio, GPIO_MODE_OUTPUT);
	//wait for 30us
	//ets_delay_us(30);
	//response = (gpio_get_level(data_gpio) == 0) ?1:0;
	//ets_delay_us(470);
	//response = (gpio_get_level(data_gpio) == 1) ?1:0;
	ets_delay_us(400);
	
	gpio_set_level(data_gpio,1);
	return ret;
}

void gpio_send_bit(char bit)
{
	gpio_set_direction(data_gpio, GPIO_MODE_OUTPUT);
	gpio_set_level(data_gpio,0);
	ets_delay_us(5);
	if(bit == 1)
		gpio_set_level(data_gpio,1);
	ets_delay_us(55);
	gpio_set_level(data_gpio,1);
}

void gpio_send_byte(char data)
{
	unsigned char x;
	int i = 0;
	for(i =0 ; i < 8; i++)
	{
		x = ((data>>i)&0x01);
		gpio_send_bit(x);
	}
	ets_delay_us(100);
}

unsigned char gpio_read_bit()
{
	unsigned char re_data;
	gpio_set_direction(data_gpio, GPIO_MODE_OUTPUT);
	gpio_set_level(data_gpio,0);
	ets_delay_us(2);
	gpio_set_level(data_gpio,1);
	ets_delay_us(15);
	gpio_set_direction(data_gpio, GPIO_MODE_INPUT);
	if(gpio_get_level(data_gpio)==1) re_data=1; else re_data=0;
	return re_data;
}

unsigned char gpio_read_byte()
{
	unsigned char x=0;
	int i =0 ;
	for(i =0 ; i < 8 ; i++)
	{
		if(gpio_read_bit())
			x |= 1<<i;
		ets_delay_us(15);
	}
	return x;
}


void task_main()
{
    	char temp1=0, temp2=0;
	while(1)
	{
		if(sys_init)
		{
			int check_out = 0;
			check_out = reset_sensor();
			if(check_out == 0)
			{
				gpio_send_byte(0xCC);
				gpio_send_byte(0x44);
				vTaskDelay(750 / portTICK_RATE_MS);
				//ets_delay_us(4);
				if( reset_sensor())
					continue;
				gpio_send_byte(0xCC);
				gpio_send_byte(0xBE);
				temp1=gpio_read_byte();
				temp2=gpio_read_byte();
				check_out=reset_sensor();
				float temp=0;
				temp=(float)(temp1+(temp2*256))/16;
				printf("temp:%.2f \n",temp);
			}
			else
				continue;
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		//ets_delay_us(1000000);
	}
}

void test_main()
{
	int gpio=0;
	while(1)
	{
		//vTaskDelay(5000 / portTICK_PERIOD_MS);
		gpio = gpio_get_level(test_gpio);
		printf("read out is:%d \n",gpio);
	}
}

void app_main()
{
	temp_sensor_init();
	//check_out_pin();
	xTaskCreate(task_main, "apptask", 1024*3, NULL, 10, NULL);
	//xTaskCreate(test_main, "apptask", 1024*3, NULL, 10, NULL);
	//task_main();
}

