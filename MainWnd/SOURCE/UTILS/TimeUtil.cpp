#include "stdafx.h"

//#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "TimeUtil.h"
#include "..\\UTILS\\string.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning(disable: 4996)

CTimeUtil::CTimeUtil()
{
}
/******************************************************************************/
/*  DaysInMonth :  the number of days in the specified month                                */
/******************************************************************************/

CString CTimeUtil::GetCurrentTime()
{
	SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
//	::GetLocalTime( &LocalTime );
	CTime LocalT( LocalTime );
	return CString(LocalT.Format( L"%x %X\n" ) );
}

CString CTimeUtil::CurrentTime1()
{
	SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
	CTime LocalT( LocalTime );
	return CString(LocalT.Format( L"%H:%M:%S" ) );
}

CString CTimeUtil::CurrentTime()
{
	SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
	CTime LocalT( LocalTime );
	return CString(LocalT.Format( L"%I:%M %p" ) );
}

CString CTimeUtil::CurrentDate()
{
	SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
//	::GetLocalTime( &LocalTime );
	CTime LocalT( LocalTime );
	return CString(LocalT.Format( L"%Y/%m/%d" ) );
}
void CTimeUtil::CTOS_RTCGetLocal(CTOS_RTC *Time)
{
	SYSTEMTIME LocalTime;
	::GetLocalTime( &LocalTime );
	CTime LocalT( LocalTime );
	Time->bYear		=(BYTE) LocalT.GetYear() - 2000;
	Time->bMonth	=(BYTE) LocalT.GetMonth();
	Time->bDay		=(BYTE) LocalT.GetDay();
	Time->bHour		=(BYTE) LocalT.GetHour();
	Time->bMinute	=(BYTE) LocalT.GetMinute();
	Time->bSecond	=(BYTE) LocalT.GetSecond();
}

void CTimeUtil::CTOS_RTCGetSystem(CTOS_RTC *Time)
{
	SYSTEMTIME SystemTime;
	::GetSystemTime( &SystemTime );
	CTime SystemT( SystemTime);
	Time->bYear		=(BYTE) SystemT.GetYear() - 2000;
	Time->bMonth	=(BYTE) SystemT.GetMonth();
	Time->bDay		=(BYTE) SystemT.GetDay();
	Time->bHour		=(BYTE) SystemT.GetHour();
	Time->bMinute	=(BYTE) SystemT.GetMinute();
	Time->bSecond	=(BYTE) SystemT.GetSecond();
}

int CTimeUtil::DaysInMonth(int month, int year)
{
	switch (month)
	{
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
			return 31;

		case 4:
		case 6:
		case 9:
		case 11:
			return 30;

		case 2:
			/* leap years occur in years exactly divisible by four,
			 except that years ending in 00 are leap years
			 only if they are divisible by 400.
			 So, 1700, 1800, 1900, 2100, and 2200 are not leap years.
			 But 1600, 2000, and 2400 are leap years.
			 Since we are only interested in the year 1901 to 2099,
			 just check for division by four. */
			if (year % 4 == 0)
				return 29;
			else
				return 28;
	}
	return 0; // never ends here, this is only for rid of warning
}

/******************************************************************************/
BYTE CTimeUtil::LastDayOfMonth(BYTE year, BYTE month)
{
	BYTE lastDay;

	if (month == 1 || month == 3 || month == 5 || month == 7 || 
			month == 8 || month == 10 || month == 12)
	{
		lastDay = 31;
	}
	else if (month == 4 || month == 6 || month == 9 || month == 11)
	{
		lastDay = 30;
	}
	else if (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))
	{
		lastDay = 29;
	}
	else
	{
		lastDay = 28;
	}

	return lastDay;
}


/******************************************************************************/
/*  Get current date:  Output format is MMDD.                                 */
/******************************************************************************/
USHORT CTimeUtil::GetCurrentDate(USHORT midnight, char *dateTime)
{
	USHORT day, month, year, hour, minute;
	CTOS_RTC stRTC; 
 
	/* If the current hour and minutes are earlier than midnight,
		 then the prior day is used. Otherwise, the current day is used.
		 midnight is MMHH */

	if (dateTime == NULL)
	{
		CTOS_RTCGetLocal(&stRTC);
		day = (USHORT) stRTC.bDay;
		month = (USHORT) stRTC.bMonth;
		year = (USHORT) stRTC.bYear + 2000;
		hour = (USHORT) stRTC.bHour;
		minute = (USHORT) stRTC.bMinute;
	}
	else
	{
		sscanf(dateTime, "%2d%2d%2d%2d%2d", (INT *)&year, (INT *)&month, 
														(INT *)&day, (INT *)&hour, (INT *)&minute);
		year += 2000;
	}

	if (hour * 100 + minute < midnight)
	{
		/* Use prior day */
		if (--day < 1)
		{
			if (--month < 1)
			{
				year--;
				month = 12;
			}
			day = DaysInMonth(month, year);
		}
	}
	return (USHORT)(month * 100 + day);
}

/******************************************************************************/
void CTimeUtil::Yesterday(CTOS_RTC *now, CTOS_RTC *yesterday)
{
	BYTE year, month, day, hour, minute, second;

	year = now->bYear;
	month = now->bMonth;
	day = now->bDay;
	hour = now->bHour;
	minute = now->bMinute;
	second = now->bSecond;
	
	if (hour < 3)
	{
		day--;
		if (day == 0)
		{
			month--;
			if (month == 0)
			{
				if (year > 0)
					year--;
				month = 1;
			}
			day = LastDayOfMonth(year, month);
		}
	}
	yesterday->bYear = year;
	yesterday->bMonth = month;
	yesterday->bDay = day;
	yesterday->bHour = 3;
	yesterday->bMinute = 0;
	yesterday->bSecond = 0;
}

/******************************************************************************/
void CTimeUtil::Tomorrow(CTOS_RTC *now, CTOS_RTC *tomorrow)
{
	BYTE year, month, day, hour, minute, second;

	year = now->bYear;
	month = now->bMonth;
	day = now->bDay;
	hour = now->bHour;
	minute = now->bMinute;
	second = now->bSecond;

	if (hour >= 3)
	{
		day++;
		if (day > LastDayOfMonth(year, month))
		{
			day = 1;
			month++;
			if (month == 12)
			{
				year++;
			}
		}
	}

	tomorrow->bYear = year;
	tomorrow->bMonth = month;
	tomorrow->bDay = day;
	tomorrow->bHour = 3;
	tomorrow->bMinute = 0;
	tomorrow->bSecond = 0;
}

/******************************************************************************/
INT CTimeUtil::CompareRTCs(CTOS_RTC *RTC1, CTOS_RTC *RTC2)
{
	ULONG date1, date2, time1, time2;
	unsigned long long datetime1, datetime2;
	
	date1 = (RTC1->bYear << 16) | (RTC1->bMonth << 8) | RTC1->bDay;
	time1 = (RTC1->bHour << 16) | (RTC1->bMinute << 8) | RTC1->bSecond;
	date2 = (RTC2->bYear << 16) | (RTC2->bMonth << 8) | RTC2->bDay;
	time2 = (RTC2->bHour << 16) | (RTC2->bMinute << 8) | RTC2->bSecond;
	
	datetime1 = ((unsigned long long)date1 << 32) | (unsigned long long)time1;
	datetime2 = ((unsigned long long)date2 << 32) | (unsigned long long)time2;
	
	if (datetime1 == datetime2)
	{
		return 0;
	}
	else if (datetime1 > datetime2)
	{
		return 1;
	}
	else
	{
		return -1;
	}
	return 0;
}

/******************************************************************************/
ULONG CTimeUtil::CalculateJulianDate(ULONG iYear, ULONG iMonth, ULONG iDay)
{
  ULONG  julian_date, temp;
	static const int days_month[12] =
  {   0,		// # Julian Days at start of January
      31,		// # Julian Days at start of February
      59,		// # Julian Days at start of March
      90,		// # Julian Days at start of April
      120,  // # Julian Days at start of May
      151,  // # Julian Days at start of June
      181,  // # Julian Days at start of July
      212,  // # Julian Days at start of August
      243,  // # Julian Days at start of September
      273,  // # Julian Days at start of October
      304,  // # Julian Days at start of November
      334   // # Julian Days at start of December
  };
	
  // Get Julian year 
  julian_date = ( iYear%100 * 1000);

  // Add the julian date of the specified day
  temp = iMonth-1;
  julian_date += days_month[temp] + iDay;

  /* If a Leap Year and the date is Mar. 1st or later,
  increment julian date by 1 to account for leap day.  A
  year is a Leap Year if the last two digits are evenly
  dividable by 4. Century year (eg. 1900, 2000, 2100) is
  considered Leap Year if it is evenly divisible by 400:
  1800, 1900, 2100 are NOT leap, but 2000 is.   */
  if ( (iYear % 4 == 0) && (iMonth    >= 3) )
  {
    julian_date++;
  }

  return (julian_date);
}

/******************************************************************************/
/*  Get system date and time.  Output format is YYMMDDHHMMSS.                 */
/******************************************************************************/
void CTimeUtil::GetSystemDateTime(char *pt)
{
	CTOS_RTC stRTC; 
 
	CTOS_RTCGetLocal(&stRTC); 
	sprintf(pt, "%02d%02d%02d%02d%02d%02d", stRTC.bYear, stRTC.bMonth, stRTC.bDay,
											stRTC.bHour, stRTC.bMinute, stRTC.bSecond);		
}

/******************************************************************************/
/*  Get system date and time.  Output format is YYYYMMDDHHMMSS.                 */
/******************************************************************************/
void CTimeUtil::GetSystemDateTimeLong(char *pt)
{
	CTOS_RTC stRTC; 
 
	CTOS_RTCGetSystem(&stRTC); 
	sprintf(pt, "20%02d%02d%02d%02d%02d%02d", stRTC.bYear, stRTC.bMonth,  stRTC.bDay,
										      stRTC.bHour, stRTC.bMinute, stRTC.bSecond);		
}

/******************************************************************************/
/*  Get system date and time.  Output format is YYYYMMDDHHMMSS.                 */
/******************************************************************************/
void CTimeUtil::GetLocalDateTimeLong(char *pt)
{
	CTOS_RTC stRTC; 
 
	CTOS_RTCGetLocal(&stRTC); 
	sprintf(pt, "20%02d%02d%02d%02d%02d%02d", stRTC.bYear, stRTC.bMonth,  stRTC.bDay,
										      stRTC.bHour, stRTC.bMinute, stRTC.bSecond);		
}

/******************************************************************************/
/*  Get system date.  Output format is YYMMDD.                                */
/******************************************************************************/
void CTimeUtil::GetSystemDate(char *pt)
{
	SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
	CTime LocalT( LocalTime );
	CString str = LocalT.Format(L"%Y%m%d");
	str = str.Mid(2);
		
	char *p = alloc_tchar_to_char(str.GetBuffer(str.GetLength()));
	strcpy(pt,p);
	free(p);
	str.ReleaseBuffer();
}

/******************************************************************************/
/*  Get system time.  Output format is HHMMSS.                                */
/******************************************************************************/
void CTimeUtil::GetSystemTime(char *pt)
{
	SYSTEMTIME LocalTime;
	::GetSystemTime( &LocalTime );
	CTime LocalT( LocalTime );
	CString str = LocalT.Format(L"%H%M%S");
	char *p = alloc_tchar_to_char(str.GetBuffer(str.GetLength()));
	strcpy(pt,p);
	free(p);
	str.ReleaseBuffer();
}

void CTimeUtil::SetSystemTime(char *time) //*time = "HHmmSS"
{
	SYSTEMTIME LocalTime;
	::GetLocalTime( &LocalTime );
	char* p = time;
	char temp[3];
	temp[2] = 0;

	//hour
	temp[0] = *(p);
	temp[1] = *(p+1);
	LocalTime.wHour = atoi(temp);
	//Minute
	temp[0] = *(p+2);
	temp[1] = *(p+3);
	LocalTime.wMinute = atoi(temp);

	//Second
	temp[0] = *(p+4);
	temp[1] = *(p+5);
	LocalTime.wSecond = atoi(temp);
	::SetSystemTime(&LocalTime);
	Sleep(1);
}

void CTimeUtil::SetSystemDate(char *date) //*date = "YYMMDD"
{
	SYSTEMTIME LocalTime;
	::GetLocalTime( &LocalTime );
	char* p = date;
	char temp[3];
	temp[2] = 0;

	//Year
	temp[0] = *(p);
	temp[1] = *(p+1);
	LocalTime.wYear = atoi(temp)+2000;
	//Month
	temp[0] = *(p+2);
	temp[1] = *(p+3);
	LocalTime.wMonth = atoi(temp);

	//day
	temp[0] = *(p+4);
	temp[1] = *(p+5);
	LocalTime.wDay = atoi(temp);
	::SetSystemTime(&LocalTime);
	Sleep(1);

}

void CTimeUtil::SetDateAndTime(char *time)
{
	SYSTEMTIME LocalTime;
	::GetLocalTime( &LocalTime );
	char* p = time;
	char temp[3];
	temp[2] = 0;

	//Year
	temp[0] = *(p);
	temp[1] = *(p+1);
	LocalTime.wYear = atoi(temp)+2000;

	//Month
	temp[0] = *(p+2);
	temp[1] = *(p+3);
	LocalTime.wMonth = atoi(temp);

	//day
	temp[0] = *(p+4);
	temp[1] = *(p+5);
	LocalTime.wDay = atoi(temp);

	//hour
	temp[0] = *(p+6);
	temp[1] = *(p+7);
	LocalTime.wHour = atoi(temp);

	//Minute
	temp[0] = *(p+8);
	temp[1] = *(p+9);
	LocalTime.wMinute = atoi(temp);

	//Second
	temp[0] = *(p+10);
	temp[1] = *(p+11);
	LocalTime.wSecond = atoi(temp);
	::SetSystemTime(&LocalTime);
	Sleep(1);
}
