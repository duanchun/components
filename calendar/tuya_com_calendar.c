
#include "tuya_com_calendar.h"
#include "tuya_log.h"

#define	BEGYEAR	        1970     // UTC started at 00:00:00 January 1, 1970
#define	DAY             86400UL  // 24 hours * 60 minutes * 60 seconds

static com_timestamp_set_cbk s_core_timestamp_set_cbk = NULL;
static com_timestamp_get_cbk s_core_timestamp_get_cbk = NULL;


static uint8_t is_leap_year(uint16_t year)
{  
    if((year%4==0 && year%100!=0) || year%400==0) 
    { 
        return 1;   
    }
    else 
    	return 0; 
}

/*  使用基姆拉尔森公式  */
static uint8_t caculate_week_day(uint16_t year,uint8_t month,uint8_t day)
{     
     uint8_t week_day = 0;
     if(month == 1 || month == 2)
     {
		month = month + 12;
		year = year-1;
     		
     }

     week_day = (day + 2*month + 3*(month+1)/5 + year + year/4 - year/100 + year/400 ) % 7 +1;
    
     return week_day;
}

/**
*@ Description:把UTC时间转换为SYSTEMTIME
*@ param 1.	tm：系统时间结构体指针
*		 2.	secTime:以秒为单位的时间
*@ return void:
*/
void tuya_com_utc_time_convert(tuya_com_time_t *tm, uint32_t secTime)
{
	const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};

	uint32_t temp_day=0;
	uint32_t temp_month=0;
	uint16_t temp_year=0;
	//if(tm == NULL || !secTime)
	if(NULL == tm)
		return;
		  
	temp_day = secTime % DAY; 
	tm->second = temp_day % 60UL;
	tm->miniute= (temp_day % 3600UL) / 60UL;
	tm->hour = temp_day / 3600UL;
	temp_day = secTime/DAY;   //Get the number of days(one day = 86400 sec)
	temp_year = BEGYEAR;	 

	while(temp_day>365)
	{                          
		if(is_leap_year(temp_year))//leap year
		{
		  temp_day-=366;
		}  
		else
		{
		  temp_day-=365; //ordinary year 
		} 

		temp_year++;  
	} 
	tm->year = temp_year;
	if((temp_day == 365) && !is_leap_year(temp_year))
	{
		temp_day-=365;
		tm->year++;
	}
	// SYSTEMTIME_DEBUG(("---liupan---convert_utc_time temp_day = %d\r\n",temp_day));

	while(temp_day >= 28)//More than a month
	{
		if( temp_month==1 && is_leap_year(tm->year) )//The year is not a leap year/February
		{

			if(temp_day>=29)
				temp_day-=29;//leap year day
			else 
				break; 
		}
		else 
		{
			if(temp_day >= mon_table[temp_month])
				temp_day -= mon_table[temp_month];//ordinary year

			else
				break;
		}
	    temp_month++;  	
	}

	tm->month = temp_month + 1;
	
	if(tm->month > 12)
	{
		tm->month = tm->month -12;
		tm->year++;
	}			 
	tm->day = temp_day + 1;
//	tm->format = HOUR_24;
	tm->week = caculate_week_day(tm->year,tm->month,tm->day);


	//SYSTEMTIME_DEBUG("%d  - %d - %d  ---weekDay-- = %d day= ",tm->year,tm->month,tm->date,tm->weekDay);
	//SYSTEMTIME_DEBUG("%d -%d -%d \r\n",tm->hours,tm->minutes,tm->seconds);
	 
}     

/**
*@ Description:把SYSTEMTIME时间转换为UTC时间
* @param tm：系统时间结构体指针
* @return UTCTime: 以秒为单位的时间
*/
static uint32_t convert_system_time(tuya_com_time_t *tm)
{
	const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	uint16_t t;
	uint32_t seccount=0;

	if((tm == NULL) || (tm->year< BEGYEAR) || (tm->year > 2099))
	return 0;       

	for(t=BEGYEAR;t<tm->year;t++) //The second sum of all years
	{
		if(is_leap_year(t))
			seccount+=31622400; //The number of seconds leap year

		else 
			seccount+=31536000; //The number of seconds ordinary year
	}

	for(t=0;t<tm->month-1;t++)         //The second number is added to the previous month
	{
		seccount+=(uint32_t)mon_table[t]*DAY;

		if((t==1) && is_leap_year(tm->year))
			seccount+=DAY;//Leap year day in February to increase the number of seconds  
	}
	seccount+=(uint32_t)(tm->day - 1)*DAY;//The second number is added to the front of date
	seccount+=(uint32_t)tm->hour * 3600UL;
	seccount+=(uint32_t)tm->miniute*60UL;

	seccount+=(uint32_t)tm->second;//all seconds

	return seccount;
}

void tuya_com_time_set(tuya_com_time_t value)
{
	if(NULL != s_core_timestamp_set_cbk)
    	s_core_timestamp_set_cbk(convert_system_time(&value));
}
	
tuya_com_time_t tuya_com_time_get(void)
{
	uint32_t timestamp;
	tuya_com_time_t sys_time = {0};

	if(NULL != s_core_timestamp_get_cbk)
    	timestamp = s_core_timestamp_get_cbk();
	
	tuya_com_utc_time_convert(&sys_time,timestamp);


	return sys_time;
}

uint32_t tuya_com_timestamp_get(void)
{
	if(NULL != s_core_timestamp_get_cbk)
		return s_core_timestamp_get_cbk();
	
	return 0;
}

void tuya_com_timestamp_set(uint32_t timestamp)
{
    if(NULL != s_core_timestamp_set_cbk)
		s_core_timestamp_set_cbk(timestamp);
}

void tuya_com_calendar_init(com_timestamp_set_cbk timestamp_set_cbk, com_timestamp_get_cbk timestamp_get_cbk)
{
	s_core_timestamp_set_cbk = timestamp_set_cbk;
	s_core_timestamp_get_cbk = timestamp_get_cbk;
}


