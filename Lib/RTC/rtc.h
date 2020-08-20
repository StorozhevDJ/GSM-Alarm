#ifndef RTC_H_


typedef struct {
	unsigned short year;	/* 1..4095 */
	unsigned char  month;	/* 1..12 */
	unsigned char  mday;	/* 1.. 31 */
	unsigned char  wday;	/* 0..6, Sunday = 0*/
	unsigned char  hour;	/* 0..23 */
	unsigned char  min;		/* 0..59 */
	unsigned char  sec;		/* 0..59 */
	unsigned char  dst;		/* 0 Winter, !=0 Summer */
} RTC_t;

extern RTC_t time;


int  RTC_Init(void);
void RTC_GetTime (RTC_t*);			// Get time
void RTC_SetTime (const RTC_t*);		// Set time
void RTC_SetUnixTime (long utime);		//Set UnixTime
long RTC_GetUnixTime (void);			//Get UnixTime
void UnixTime_To_Time (long utime, RTC_t *t);
long Time_To_UnixTime (RTC_t *t);
long Time_To_UnixTime2 (int year, char month, char day, char hour, char min, char sec);
void MyRTC_SetAlarm(long time);

#endif
