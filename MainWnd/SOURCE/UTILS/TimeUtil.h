// TimeUtil.h : header file
//

#pragma once

typedef struct
{
  BYTE    bYear;
  BYTE    bMonth;
  BYTE    bDay;
  BYTE    bHour;
  BYTE    bMinute;
  BYTE    bSecond;
} CTOS_RTC;

// CTimeUtil
class CTimeUtil
{
// Construction
public:
	CTimeUtil();	// standard constructor

public:
	static CString GetCurrentTime();
	static CString CurrentDate();
	static CString CurrentTime();
	static CString CurrentTime1();
	USHORT GetCurrentDate(USHORT midnight, char *dateTime);
	void Yesterday(CTOS_RTC *now, CTOS_RTC *yesterday);
	void Tomorrow(CTOS_RTC *now, CTOS_RTC *tomorrow);
	INT CompareRTCs(CTOS_RTC *RTC1, CTOS_RTC *RTC2);
	ULONG CalculateJulianDate(ULONG iYear, ULONG iMonth, ULONG iDay);
	void GetSystemDateTime(char *pt);
	void GetSystemDateTimeLong(char *pt);
	void GetLocalDateTimeLong(char *pt);
	void GetSystemDate(char *pt);
	void GetSystemTime(char *pt);

	void SetSystemDate(char *date);
	void SetSystemTime(char *time);
	void SetDateAndTime(char *time);
private:
	int	 DaysInMonth(int month, int year);
	BYTE LastDayOfMonth(BYTE year, BYTE month);
	void CTOS_RTCGetLocal(CTOS_RTC *Time);
	void CTOS_RTCGetSystem(CTOS_RTC *Time);


};



