
#include <string.h>
#include "driver_timer.h"
#include "driver_rtc.h"

#include "tuya_ble_stdlib.h"
#include "tuya_com_433.h"
#include "tuya_pin_def.h"
#include "tuya_log.h"
#include "tuya_hdl_gpio.h"

/*
*  1，RTC clock采用内部RC，1ticks == 16us（实测14.72us ≈ 15us）
*/
 

/*
*  芯片原厂：深圳前海维晟智能技术有限公司
*  芯片型号：WS480L
*  钥匙A4
*  引导码/起始码：
*    默认状态下 常高，8.8ms低电平表示 引导码/起始码
*
*  用户码、数据码：
*    码1：高电平-》800us  ，低电平-》300us
*    码0：高电平-》240us，低电平-》840us
*  结束码、重复码：
*    结束码同时也是重复码的起始
*    高电平-》240us，低电平-》8.68ms

*  钥匙D
*  引导码/起始码：
*    默认状态下 常高，12.60ms低电平表示 引导码/起始码
*
*  用户码、数据码：
*    码1：高电平-》1.28us  ，低电平-》420us
*    码0：高电平-》1.22us，低电平-》840us
*  结束码、重复码：
*    结束码同时也是重复码的起始
*    高电平-》400us，低电平-》12.48ms

*/

#define ONE_TICK_US_MUL		(4) 	//

//较长的低电平表示 引导码/起始码
#define SYNC_L_MAX     	 	(800)	//((8800 + 3000)>>ONE_TICK_US_MUL) //ticks  
#define SYNC_L_MIN    		(487)	//((8800 - 1000)>>ONE_TICK_US_MUL) //ticks


//1码高低电平时间
#define ONE_TIME_H_MAX		(80)	//((800 + 480)>>ONE_TICK_US_MUL) //ticks
#define ONE_TIME_H_MIN		(43)	//((800 - 100)>>ONE_TICK_US_MUL) //ticks

#define ONE_TIME_L_MAX    	(30)	//((300 + 180)>>ONE_TICK_US_MUL) //ticks
#define ONE_TIME_L_MIN		(12)	//((300 - 100)>>ONE_TICK_US_MUL) //ticks


//0码高低电平时间
#define ZERO_TIME_H_MAX		(30)	//((240 + 240)>>ONE_TICK_US_MUL) //ticks 
#define ZERO_TIME_H_MIN		(8)		//((240 - 100)>>ONE_TICK_US_MUL) //ticks

#define ZERO_TIME_L_MAX    	(80)	//((840 + 520)>>ONE_TICK_US_MUL) //ticks
#define ZERO_TIME_L_MIN		(46)	//((840 - 100)>>ONE_TICK_US_MUL) //ticks

#define INVALID_PULSE_PERIOD	(ZERO_TIME_H_MIN) //杂波

#define CODE_REPEAT_THRESHOLD	(15625) //250ms，在这个时间间隔周期内解析到同样的码值，说明遥控器长按，不给应用层上抛按键消息

typedef struct{
	uint8_t Key_Val;//后四位键值
	uint8_t Verify_Code[3];//前二十位校验码
	uint8_t Decode_Compile;
}tuya_com_433_t;



static uint32_t s_last_ticks = 0;
static uint32_t s_current_ticks = 0;
static uint32_t s_low_ticks = 0;
static uint32_t s_hight_ticks = 0;
static uint8_t  s_pulse_period = 0;//0->start,1->end
static uint8_t  s_start_recv = 0;//0->enable,1->disable
static int8_t   s_bit_index = 0;
static uint32_t s_code_record = 0;
static uint32_t s_last_decode_success_time = 0;
static uint32_t s_code_record_last = 0;


static tuya_com_433_t s_core_433 = {0};
static uint8_t s_code_bits_type;
static uint32_t s_data_pin;
static uint32_t s_shut_pin;



#define _433_PROTOCAL_DEBUGx

#ifdef _433_PROTOCAL_DEBUG
#define _433_TEST_ARRARY_MAX    20
typedef struct {
	uint32_t hight_ticks;
	uint32_t low_ticks;
}core_433_test_t;

static uint32_t s_sync_code_h = 0;
static uint32_t s_sync_code_l = 0;

static uint8_t s_h_perid_test[24] = {0};
static uint8_t s_l_perid_test[24] = {0};
static uint8_t s_print_index = 0;
static uint8_t s_433_arr_index = 0;
static core_433_test_t s_433_test_arr[_433_TEST_ARRARY_MAX] = {0};
#endif

static void core_433_irq_cb(void *param)
{
	//获取电平状态
	if(tuya_hdl_gpio_read_no_pmu(s_data_pin) == 0x00) //处理下降沿
	{
		s_current_ticks = rtc_get_tick();
		s_hight_ticks = s_current_ticks - s_last_ticks;
		s_pulse_period = 0;
		ext_int_set_type(EXTI_15, EXT_INT_TYPE_POS);
	}else{ //处理上升沿
		s_current_ticks = rtc_get_tick();
		s_low_ticks = s_current_ticks - s_last_ticks;
		s_pulse_period = 1;
		ext_int_set_type(EXTI_15, EXT_INT_TYPE_NEG);
	}
	s_last_ticks = s_current_ticks;

	//一个脉冲周期结束
	if(1 == s_pulse_period)
	{
		s_pulse_period = 0;
		//滤除杂波
		if(s_hight_ticks + s_low_ticks <= INVALID_PULSE_PERIOD) //
		{
			//co_printf("ER:%d\r\n",s_hight_ticks + s_low_ticks);
			return ;
		}
		
		//co_printf("%d^%d\r\n",s_hight_ticks,s_low_ticks);
		//同步（引导）码解析
		if(/*(s_hight_ticks >= SYNC_SHORT_H_TIME_PERIOD - SYNC_SHORT_H_FAULT_TOLERANT_D) 
			&& (s_hight_ticks <= SYNC_SHORT_H_TIME_PERIOD + SYNC_SHORT_H_FAULT_TOLERANT_R) 
			&& */(s_low_ticks >= SYNC_L_MIN ) 
			&& (s_low_ticks <= SYNC_L_MAX ))	//判断是否为同步码
		{
			//co_printf("S\r\n");  //调试的时候优先打开这句log
			s_start_recv = 1;
			s_bit_index = 24;
			s_code_record = 0;
			#ifdef _433_PROTOCAL_DEBUG
			s_sync_code_h = s_hight_ticks;
			s_sync_code_l = s_low_ticks;
			#endif
		}else if((1 == s_start_recv) && (s_hight_ticks > s_low_ticks) 
			&& (s_hight_ticks >= ONE_TIME_H_MIN && s_hight_ticks <= ONE_TIME_H_MAX)
			&& (s_low_ticks >= ONE_TIME_L_MIN && s_low_ticks <= ONE_TIME_L_MAX)){ //1码
		
			s_bit_index--;
			s_code_record |= (1 << s_bit_index);

			#ifdef _433_PROTOCAL_DEBUG
			s_h_perid_test[s_bit_index] = s_hight_ticks;
			s_l_perid_test[s_bit_index] = s_low_ticks;
			#endif
		}else if((1 == s_start_recv) && (s_hight_ticks < s_low_ticks) 
			&& (s_hight_ticks >= ZERO_TIME_H_MIN && s_hight_ticks <= ZERO_TIME_H_MAX)
			&& (s_low_ticks >= ZERO_TIME_L_MIN && s_low_ticks <= ZERO_TIME_L_MAX)){ //0码
		
			s_bit_index--;
			s_code_record |= (0 << s_bit_index);

			#ifdef _433_PROTOCAL_DEBUG
			s_h_perid_test[s_bit_index] = s_hight_ticks;
			s_l_perid_test[s_bit_index] = s_low_ticks;
			#endif
		}else{
			#if 1
			//static uint8_t ss_flag = 0;
			if(1 == s_start_recv/* && 0 == ss_flag*/)
			{
				//ss_flag = 1;
				//co_printf("E %d^%d\r\n",s_hight_ticks,s_low_ticks); //调试的时候优先打开这句log
				#ifdef _433_PROTOCAL_DEBUG
				s_433_arr_index = (s_433_arr_index + 1)%_433_TEST_ARRARY_MAX;
				s_433_test_arr[s_433_arr_index].hight_ticks = s_hight_ticks;
				s_433_test_arr[s_433_arr_index].low_ticks = s_low_ticks;
				#endif
			}
			#endif
			
			s_start_recv = 0;
		}

		if(!s_bit_index)
		{
			s_bit_index = 24;
			s_core_433.Verify_Code[0] = (s_code_record >> 16);
            s_core_433.Verify_Code[1] = ((s_code_record >> 8) & 0xff);
            s_core_433.Verify_Code[2] = ((s_code_record >> 4) & 0x0f);
            s_core_433.Key_Val = (s_code_record & 0x0f);
			co_printf("decode:%x^%x^%x^%x\r\n",s_core_433.Key_Val,s_core_433.Verify_Code[0],s_core_433.Verify_Code[1],s_core_433.Verify_Code[2]);
			#if 0
			if(s_code_record_last == s_code_record)
			{
				if(s_last_ticks - s_last_decode_success_time > CODE_REPEAT_THRESHOLD)
				{
					co_printf("P1\n");
					tuya_bll_433_msg_post(0,s_core_433.Verify_Code,s_core_433.Key_Val);
				}
			}else{
				//co_printf("P2\n");
				s_code_record_last = s_code_record;
				//tuya_bll_433_msg_post(0,s_core_433.Verify_Code,s_core_433.Key_Val);
			}
			
			s_last_decode_success_time = s_last_ticks;
			#else
			if(s_last_ticks - s_last_decode_success_time > CODE_REPEAT_THRESHOLD)
			{
				//co_printf("P1\n");
				tuya_bll_433_msg_post(0,s_core_433.Verify_Code,s_core_433.Key_Val);
			}
			s_last_decode_success_time = s_last_ticks;
			#endif
			
			#ifdef _433_PROTOCAL_DEBUG
			#if 1
			for(s_print_index = 0; s_print_index < sizeof(s_h_perid_test); s_print_index++)
			{
				co_printf("%d^%d::%d\r\n",s_h_perid_test[s_print_index],s_l_perid_test[s_print_index],
					s_h_perid_test[s_print_index] > s_l_perid_test[s_print_index] ? 1 : 0);
			}
			#endif
			for(s_print_index = 0; s_print_index < _433_TEST_ARRARY_MAX; s_print_index++)
			{
				co_printf("E %d^%d\r\n",s_433_test_arr[s_print_index].hight_ticks,s_433_test_arr[s_print_index].low_ticks);
			}
			//co_printf("H: %d^%d\r\n",s_sync_code_h,s_sync_code_l);
			#endif
		}
	}
}

void tuya_com_433_suspend(void)
{
	tuya_hdl_gpio_init(s_data_pin,GPIO_DIR_IN,false,TUYA_GPIO_LOW);
}

void tuya_com_433_resume_e(void)
{
	//433数据通讯
    tuya_hdl_gpio_irq_init(s_data_pin,TUYA_GPIO_IRQ_RISE_FALL,core_433_irq_cb);
}

void tuya_com_433_start(void)
{
	TUYA_LOG_I(MOD_RCU433, " enter",);
	tuya_hdl_gpio_write(s_shut_pin,TUYA_GPIO_LOW);
    tuya_hdl_gpio_irq_enable(s_data_pin);
}

void tuya_com_433_stop(void)
{
	TUYA_LOG_I(MOD_RCU433, " enter");
    tuya_hdl_gpio_write(s_shut_pin,TUYA_GPIO_HIGH);
    s_core_433.Decode_Compile = 0;
    memset(&s_core_433,0,sizeof(s_core_433));
    tuya_hdl_gpio_irq_disable(s_data_pin);
}

void tuya_com_433_resume(void)
{
    TUYA_LOG_I(MOD_RCU433, " enter");
    tuya_hdl_gpio_irq_enable(s_data_pin);
}

void tuya_com_433_pause(void)
{
    TUYA_LOG_I(MOD_RCU433, " enter");
    tuya_hdl_gpio_irq_disable(s_data_pin);
    s_core_433.Decode_Compile = 0;
    memset(&s_core_433,0,sizeof(s_core_433));
}

void tuya_com_433_set_code_bits(uint8_t bits_type)
{
	s_code_bits_type =  bits_type;
}

void tuya_com_433_init(uint32_t data_pin, uint32_t shut_pin)
{
	
    int32_t op_ret = 0;
	
    //433数据通讯
    tuya_hdl_gpio_irq_init(data_pin,TUYA_GPIO_IRQ_RISE_FALL,core_433_irq_cb);
	
    //433使能控制,高--》关闭接收器              低--》打开接收器
	tuya_hdl_gpio_init(shut_pin,TUYA_GPIO_DIR_OUTPUT,false,TUYA_GPIO_HIGH);
	
    s_data_pin = data_pin;
    s_shut_pin = shut_pin;
}


