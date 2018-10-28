/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mgos.h"
#include "mgos_gpio.h"

#if 0

#define GPIO_OUTPUT_IO_0    18
#define GPIO_DATA   GPIO_OUTPUT_IO_0
#define GPIO_TEST    19
#define GPIO_BLACK_GROUND
#define GPIO_RED_VCC
//delayMicroseconds

enum gpio_dir
{
	gpio_dir_in,
	gpio_dir_out
};

int set_esp32_gpio_init(int pin)
{
#if 1
	gpio_pad_select_gpio(pin);
#else
	
#endif
	return 0;
}

int set_esp32_gpio_mode(int pin,int mode)
{
	int set_val= 0;
#ifdef ESP32_SDK
	set_val = (mode == gpio_dir_in)?GPIO_MODE_INPUT:GPIO_MODE_OUTPUT;
	gpio_set_direction(pin,set_val);
#else
	 set_val = (mode == gpio_dir_in)?MGOS_GPIO_MODE_INPUT:MGOS_GPIO_MODE_OUTPUT;
	mgos_gpio_set_mode(pin, set_val);	
#endif
	return 0;
}

int esp32_gpio_read(int pin)
{
#ifdef ESP32_SDK
	return gpio_get_level(pin);
#else
	return mgos_gpio_read(pin);    
#endif
}

int esp32_gpio_write(int pin,int val)
{
#ifdef ESP32_SDK
	return gpio_set_level(pin,val);
#else
	 mgos_gpio_write(pin, val);
#endif
	return 0;
}

void esp32_delay_ms(int msec)
{
#if ESP32_SDK
	 ets_delay_us(msec);
#else
	 mgos_msleep(msec);
#endif
}

typedef struct gpio_driver{
	int (*set_gpio_init)(int pin);
	int (*set_gpio_mode)(int pin,int mode);
	int (*dr_gpio_read)(int pin);
	int (*dr_gpio_write)(int pin,int val);
	void (*dr_udelay)(int usec);
}gpio_driver_t;

gpio_driver_t esp32_dr=
{
	set_esp32_gpio_init,
	set_esp32_gpio_mode,
	esp32_gpio_read,
	esp32_gpio_write,
	esp32_delay_ms
};

//ets_delay_us
int data_gpio = 0;
int test_gpio = 0;
int my_sys_init = 0;

void temp_sensor_init()
{
	data_gpio = GPIO_DATA;
	//set gpio
	esp32_dr.set_gpio_init(data_gpio);
	my_sys_init = 1;
}

int reset_sensor(int pin)
{
	unsigned char count = 0;
	int ret = 0;
	//gpio_set_direction(data_gpio, GPIO_MODE_OUTPUT);
	esp32_dr.set_gpio_mode(pin,gpio_dir_out);
	
	//gpio_set_level(data_gpio,1);
	esp32_dr.dr_gpio_write(pin,1);
	//delay
	esp32_dr.dr_udelay(100);
	
	esp32_dr.dr_gpio_write(pin,0);
	
	esp32_dr.dr_udelay(600);
	//set it to high
	esp32_dr.dr_gpio_write(pin,1);
	
	esp32_dr.dr_udelay(45);
	//
	//gpio_set_direction(data_gpio, GPIO_MODE_INPUT);
	esp32_dr.set_gpio_mode(pin,gpio_dir_in);

	do
	{
		ret= esp32_dr.dr_gpio_read(pin);
		esp32_dr.dr_udelay(1);			
		count++;
	}while(ret != 0 && count<100);

	esp32_dr.set_gpio_mode(pin,gpio_dir_out);
	//wait for 30us
	//ets_delay_us(30);
	//response = (gpio_get_level(data_gpio) == 0) ?1:0;
	//ets_delay_us(470);
	//response = (gpio_get_level(data_gpio) == 1) ?1:0;
	esp32_dr.dr_udelay(400);
	
	esp32_dr.dr_gpio_write(pin,1);
	return ret;
}

void gpio_send_bit(int pin,char bit)
{
	//gpio_set_direction(data_gpio, GPIO_MODE_OUTPUT);
	esp32_dr.set_gpio_mode(pin,gpio_dir_out);
	//gpio_set_level(data_gpio,0);
	esp32_dr.dr_gpio_write(pin,0);
	esp32_dr.dr_udelay(5);
	if(bit == 1)
		esp32_dr.dr_gpio_write(pin,1);
	esp32_dr.dr_udelay(55);
	esp32_dr.dr_gpio_write(pin,1);
}

void gpio_send_byte(int pin,char data)
{
	unsigned char x;
	int i = 0;
	for(i =0 ; i < 8; i++)
	{
		x = ((data>>i)&0x01);
		gpio_send_bit(pin,x);
	}
	esp32_dr.dr_udelay(100);
}

unsigned char gpio_read_bit(int pin)
{
	unsigned char re_data;
	esp32_dr.set_gpio_mode(pin,gpio_dir_out);
	//gpio_set_level(data_gpio,0);
	
	esp32_dr.dr_gpio_write(pin,0);
	esp32_dr.dr_udelay(2);
	esp32_dr.dr_gpio_write(pin,1);
	esp32_dr.dr_udelay(15);
	esp32_dr.set_gpio_mode(pin,gpio_dir_in);
	if(esp32_dr.dr_gpio_read(pin)==1) re_data=1; else re_data=0;
	return re_data;
}

unsigned char gpio_read_byte(int pin)
{
	unsigned char x=0;
	int i =0 ;
	for(i =0 ; i < 8 ; i++)
	{
		if(gpio_read_bit(pin))
			x |= 1<<i;
		esp32_dr.dr_udelay(15);
	}
	return x;
}


void task_main()
{
    	char temp1=0, temp2=0;
	while(1)
	{
		if(my_sys_init)
		{
			int check_out = 0;
			int pin = data_gpio;
			check_out = reset_sensor(pin);
			if(check_out == 0)
			{
				gpio_send_byte(pin,0xCC);
				gpio_send_byte(pin,0x44);
				//vTaskDelay(750 / portTICK_RATE_MS);
				//ets_delay_us(4);
				esp32_dr.dr_udelay(4);
				if( reset_sensor(pin))
					continue;
				gpio_send_byte(pin,0xCC);
				gpio_send_byte(pin,0xBE);
				temp1=gpio_read_byte(pin);
				temp2=gpio_read_byte(pin);
				check_out=reset_sensor(pin);
				float temp=0;
				temp=(float)(temp1+(temp2*256))/16;
				printf("temp:%.2f \n",temp);
			}
			else
				continue;
		}
		//vTaskDelay(5000 / portTICK_PERIOD_MS);
		//ets_delay_us(1000000);
		esp32_dr.dr_udelay(1000000);
	}
}

#else
#include "mgos.h"

#define GPIO_OUTPUT_IO_0    18
#define GPIO_DATA   GPIO_OUTPUT_IO_0
#define GPIO_TEST    19
#define GPIO_BLACK_GROUND
#define GPIO_RED_VCC
//delayMicroseconds

//ets_delay_us
int data_gpio = 0;
int test_gpio = 0;
static int sys_init = 0;
float cur_temp = 0;

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
				//check_out=reset_sensor();
				float temp=0;
				temp=(float)(temp1+(temp2*256))/16;
				//printf("temp:%.2f \n",temp);
				cur_temp = temp;
			}
			else
				continue;
		}
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		//ets_delay_us(1000000);
	}
}

static float read_temp(void)
{
	char temp1=0, temp2=0;

	if(reset_sensor())
	{
		printf("step1,reset_ds18b20 error!\n");
		return 0;
	}

	gpio_send_byte(0xcc);
	gpio_send_byte(0x44);
	ets_delay_us(4);

	if(reset_sensor())
	{
		printf("step2,reset_ds18b20 error!\n");
		return 0;
	}

	gpio_send_byte(0xcc);
	gpio_send_byte(0xbe);

	temp1=gpio_read_byte();
	temp2=gpio_read_byte();
	float temp=0;
	temp=(float)(temp1+(temp2*256))/16;

	return temp;
}

#endif

static void timer_cb(void *arg) {
	printf(" timer call %.02f\n",read_temp());
  	//get_sensor_temp();
}


enum mgos_app_init_result mgos_app_init(void) {
          LOG(LL_INFO, ("Hi there"));
         temp_sensor_init();
         //xTaskCreate(task_main, "apptask", 1024*3, NULL, 10, NULL);
	 //use mongoose timer
	 
	// struct mgos_dht *dht = mgos_dht_create(mgos_sys_config_get_app_pin(), DHT22);
	mgos_set_timer(1000,true,timer_cb,NULL);
         return MGOS_APP_INIT_SUCCESS;
  	
}
