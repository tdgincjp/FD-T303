/* Include Files */
#include <windows.h>
#include "stdafx.h"
#include "Memory.h"
#include "String.h"

#pragma warning(disable: 4996)
/* Define */
#define to_lower_t(c)		((c >= TEXT('A') && c <= TEXT('Z')) ? (c - TEXT('A') + TEXT('a')) : c)
#define to_lower(c)			((c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c)

/* Global Variables */

/* Local Function Prototypes */
static int str_len_n(const TCHAR *buf, int len);

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    a2i															  */
/*                                                                            */
/* FUNCTION:    change char* to int			                                 */
/* RETURNS:     int value					                                   */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
int a2i(const char *str)
{
	int num = 0;
	int m = 1;

	if (*str == '-') {
		m = -1;
		str++;
	} else if (*str == '+') {
		str++;
	}

	for (; *str >= '0' && *str <= '9'; str++) {
		num = 10 * num + (*str - '0');
	}
	return num * m;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    delete_ctrl_char											  */
/*                                                                            */
/* FUNCTION:    delete "ctrl" letter from char*                               */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
void delete_ctrl_char(TCHAR *buf)
{
	TCHAR *p, *r;

	for (p = r = buf; *p != TEXT('\0'); p++) {
		if (*p == TEXT('\t')) {
			*(r++) = TEXT(' ');
		} else if (*p != TEXT('\r') && *p != TEXT('\n')) {
			*(r++) = *p;
		}
	}
	*r = TEXT('\0');
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    alloc_copy_t											  */
/*                                                                            */
/* FUNCTION:    copy TCHAR string		                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
TCHAR *alloc_copy_t(const TCHAR *buf)
{
	TCHAR *ret;

	if (buf == NULL) {
		return NULL;
	}
	ret = (TCHAR *)malloc(sizeof(TCHAR) * (lstrlen(buf) + 1));
	if (ret != NULL) {
		lstrcpy(ret, buf);
	}
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    alloc_copy											  */
/*                                                                            */
/* FUNCTION:    copy char string		                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
#ifdef UNICODE
char *alloc_copy(const char *buf)
{
	char *ret;

	if (buf == NULL) {
		return NULL;
	}
	ret = (char *)malloc(sizeof(char) * (tstrlen(buf) + 1));
	if (ret != NULL) {
		tstrcpy(ret, buf);
	}
	return ret;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    alloc_tchar_to_char											  */
/*                                                                            */
/* FUNCTION:    copy TCHAR string to char		                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
#ifdef UNICODE
char *alloc_tchar_to_char(TCHAR *str)
{
	char *cchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = tchar_to_char_size(str);
	cchar = (char *)malloc(len + 1);
	if (cchar == NULL) {
		return NULL;
	}
	tchar_to_char(str, cchar, len);
	return cchar;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    alloc_char_to_tchar											  */
/*                                                                            */
/* FUNCTION:    copy char string to TCHAR		                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/

#ifdef UNICODE
TCHAR *alloc_char_to_tchar(char *str)
{
	TCHAR *tchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = char_to_tchar_size(str);
	tchar = (TCHAR *)malloc(sizeof(TCHAR) * (len + 1));
	if (tchar == NULL) {
		return NULL;
	}
	char_to_tchar(str, tchar, len);
	return tchar;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    alloc_wchar_to_char											  */
/*                                                                            */
/* FUNCTION:    copy WCHAR string to char	                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/

#ifndef UNICODE
char *alloc_wchar_to_char(const UINT cp, WCHAR *str)
{
	char *cchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = WideCharToMultiByte(cp, 0, str, -1, NULL, 0, NULL, NULL);
	cchar = (char *)mem_alloc(len + 1);
	if (cchar == NULL) {
		return NULL;
	}
	WideCharToMultiByte(cp, 0, str, -1, cchar, len, NULL, NULL);
	return cchar;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    alloc_char_to_wchar											  */
/*                                                                            */
/* FUNCTION:    copy char string to WCHAR                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
#ifndef UNICODE
WCHAR *alloc_char_to_wchar(const UINT cp, char *str)
{
	WCHAR *tchar;
	int len;

	if (str == NULL) {
		return NULL;
	}
	len = MultiByteToWideChar(cp, 0, str, -1, NULL, 0);
	tchar = (WCHAR *)mem_alloc(sizeof(WCHAR) * (len + 1));
	if (tchar == NULL) {
		return NULL;
	}
	MultiByteToWideChar(cp, 0, str, -1, tchar, len);
	return tchar;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_join_t											  */
/*                                                                            */
/* FUNCTION:    join TCHAR string                                    */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
TCHAR * __cdecl str_join_t(TCHAR *ret, ... )
{
	va_list buf;
	TCHAR *str;

	va_start(buf, ret);

	str = va_arg(buf, TCHAR *);
	while (str != (TCHAR *)-1) {
		if (str != NULL) {
			while (*(ret++) = *(str++));
			ret--;
		}
		str = va_arg(buf, TCHAR *);
	}

	va_end(buf);
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_join											  */
/*                                                                            */
/* FUNCTION:    join char string                                      */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
#ifdef UNICODE
char * __cdecl str_join(char *ret, ... )
{
	va_list buf;
	char *str;

	va_start(buf, ret);

	str = va_arg(buf, char *);
	while (str != (char *)-1) {
		if (str != NULL) {
			while (*(ret++) = *(str++));
			ret--;
		}
		str = va_arg(buf, char *);
	}

	va_end(buf);
	return ret;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cpy_t											  */
/*                                                                            */
/* FUNCTION:    copy TCHAR string to TCHAR                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
TCHAR *str_cpy_t(TCHAR *ret, TCHAR *buf)
{
	if (buf == NULL) {
		*ret = TEXT('\0');
		return ret;
	}
 	while (*(ret++) = *(buf++));
	ret--;
	return ret;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cpy											  */
/*                                                                            */
/* FUNCTION:    copy char string                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
#ifdef UNICODE
char *str_cpy(char *ret, char *buf)
{
	if (buf == NULL) {
		*ret = '\0';
		return ret;
	}
	while (*(ret++) = *(buf++));
	ret--;
	return ret;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cpy_n_t											  */
/*                                                                            */
/* FUNCTION:    copy TCHAR string to TCHAR                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
void str_cpy_n_t(TCHAR *ret, TCHAR *buf, int len)
{
	while (--len && (*(ret++) = *(buf++)));
	*ret = TEXT('\0');
}

/*
 * str_cpy_n -
 */
#ifdef UNICODE
void str_cpy_n(char *ret, char *buf, int len)
{
	while (--len && (*(ret++) = *(buf++)));
	*ret = '\0';
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cpy_f_t											  */
/*                                                                            */
/* FUNCTION:    copy TCHAR string                                    */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
TCHAR *str_cpy_f_t(TCHAR *ret, TCHAR *buf, TCHAR c)
{
	TCHAR *p, *r;

	for (p = buf, r = ret; *p != c && *p != TEXT('\0'); p++, r++) {
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			*(r++) = *(p++);
		}
#endif
		*r = *p;
	}
	*r = TEXT('\0');
	return ((*p == c) ? p + 1 : p);
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cpy_f											  */
/*                                                                            */
/* FUNCTION:    copy char string to char                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
#ifdef UNICODE
char *str_cpy_f(char *ret, char *buf, char c)
{
	char *p, *r;

	for (p = buf, r = ret; *p != c && *p != '\0'; p++, r++) {
		*r = *p;
	}
	*r = '\0';
	return ((*p == c) ? p + 1 : p);
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cat_n											  */
/*                                                                            */
/* FUNCTION:    add char string to TCHAR string                                   */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
void str_cat_n(TCHAR *ret, char *buf, int len)
{
	TCHAR *p, *r;
	int i;

#ifdef UNICODE
	TCHAR *tBuf;

	p = tBuf = alloc_char_to_tchar(buf);
	if (p == NULL) {
		return;
	}
#else
	p = buf;
#endif

	i = lstrlen(ret);
	r = ret + i;
	while (*p != TEXT('\0') && i < len) {
		*(r++) = *(p++);
		i++;
	}
	*r = TEXT('\0');

#ifdef UNICODE
	free(&tBuf);
#endif
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cmp_i_t											  */
/*                                                                            */
/* FUNCTION:    compare TCHAR string                                      */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
int str_cmp_i_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	while (to_lower_t(*buf1) == to_lower_t(*buf2)) {
		if (*buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cmp_i											  */
/*                                                                            */
/* FUNCTION:    compare char string                                      */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
#ifdef UNICODE
int str_cmp_i(const char *buf1, const char *buf2)
{
	while (to_lower(*buf1) == to_lower(*buf2)) {
		if (*buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cmp_ni_t											  */
/*                                                                            */
/* FUNCTION:    compare TCHAR string                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
int str_cmp_ni_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	while (to_lower_t(*buf1) == to_lower_t(*buf2)) {
		len--;
		if (len <= 0 || *buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cmp_n_t											  */
/*                                                                            */
/* FUNCTION:    compare TCHAR string                                    */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
int str_cmp_n_t(const TCHAR *buf1, const TCHAR *buf2, int len)
{
	while (*buf1 == *buf2) {
		len--;
		if (len <= 0 || *buf1 == TEXT('\0')) {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}

#ifdef UNICODE
/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cmp_ni											  */
/*                                                                            */
/* FUNCTION:    compare char string                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
int str_cmp_ni(const char *buf1, const char *buf2, int len)
{
	while (to_lower(*buf1) == to_lower(*buf2)) {
		len--;
		if (len <= 0 || *buf1 == '\0') {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}
/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_cmp_n											  */
/*                                                                            */
/* FUNCTION:    compare char string                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
int str_cmp_n(const char *buf1, const char *buf2, int len)
{
	while (*buf1 == *buf2) {
		len--;
		if (len <= 0 || *buf1 == '\0') {
			return 0;
		}
		buf1++;
		buf2++;
	}
	return 1;
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_match_t											  */
/*                                                                            */
/* FUNCTION:    match char string                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
BOOL str_match_t(const TCHAR *ptn, const TCHAR *str)
{
	switch (*ptn) {
	case TEXT('\0'):
		return (*str == TEXT('\0'));
	case TEXT('*'):
		if (*(ptn + 1) == TEXT('\0')) {
			return TRUE;
		}
		if (str_match_t(ptn + 1, str) == TRUE) {
			return TRUE;
		}
		while (*str != TEXT('\0')) {
			str++;
			if (str_match_t(ptn + 1, str) == TRUE) {
				return TRUE;
			}
		}
		return FALSE;
	case TEXT('?'):
		return (*str != TEXT('\0')) && str_match_t(ptn + 1, str + 1);
	default:
		while (to_lower_t(*ptn) == to_lower_t(*str)) {
			if (*ptn == TEXT('\0')) {
				return TRUE;
			}
			ptn++;
			str++;
			if (*ptn == TEXT('*') || *ptn == TEXT('?')) {
				return str_match_t(ptn, str);
			}
		}
		return FALSE;
	}
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_match											  */
/*                                                                            */
/* FUNCTION:    match char string                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
#ifdef UNICODE
BOOL str_match(const char *ptn, const char *str)
{
	switch (*ptn) {
	case '\0':
		return (*str == '\0');
	case '*':
		if (*(ptn + 1) == '\0') {
			return TRUE;
		}
		if (str_match(ptn + 1, str) == TRUE) {
			return TRUE;
		}
		while (*str != '\0') {
			str++;
			if (str_match(ptn + 1, str) == TRUE) {
				return TRUE;
			}
		}
		return FALSE;
	case '?':
		return (*str != '\0') && str_match(ptn + 1, str + 1);
	default:
		while (to_lower(*ptn) == to_lower(*str)) {
			if (*ptn == '\0') {
				return TRUE;
			}
			ptn++;
			str++;
			if (*ptn == '*' || *ptn == '?') {
				return str_match(ptn, str);
			}
		}
		return FALSE;
	}
}
#endif

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_len_n											  */
/*                                                                            */
/* FUNCTION:    TCHAR length                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
static int str_len_n(const TCHAR *buf, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (*buf == TEXT('\0')) {
			break;
		}
		buf++;
	}
	return i;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    str_find											  */
/*                                                                            */
/* FUNCTION:    find TCHAR string in TCHAR string                                     */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
TCHAR *str_find(TCHAR *ptn, TCHAR *str, int case_flag)
{
	TCHAR *p;
	int len1, len2;

	len1 = lstrlen(ptn);
	for (p = str; *p != TEXT('\0'); p++) {
		len2 = str_len_n(p, len1);
		if (CompareString(MAKELCID(MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), SORT_DEFAULT),
			(case_flag) ? 0 : NORM_IGNORECASE, p, len2, ptn, len1) == 2) {
			break;
		}
#ifndef UNICODE
		if (IsDBCSLeadByte((BYTE)*p) == TRUE && *(p + 1) != TEXT('\0')) {
			p++;
		}
#endif
	}
	return p;
}

/******************************************************************************/
/*                                                                            */
/* RTN NAME:    word_find_ni_t											  */
/*                                                                            */
/* FUNCTION:    find TCHAR string in TCHAR string                             */
/* RETURNS:    																 */
/* NOTES:                                                                     */
/*                                                                            */
/******************************************************************************/
BOOL word_find_ni_t(const TCHAR *ptn, const TCHAR *str, const int len)
{
	const TCHAR *p;
	p = str;
	while (*p != '\0') {
		while(!IS_ALNUM_UM_T(*p)) p++;
		if (str_cmp_ni_t(ptn, p, len) == 0) {
			return TRUE;
		}
		while (*p != TEXT(' ') && *p != TEXT('\0')) p++;
	}
	return FALSE;
}


BOOL str_compare(const char * s1, const char * s2)
{
	int s1len = strlen(s1);
	int s2len = strlen(s2);
	if (s1len != s2len)
		return false;

	for(int i = 0; i < s1len; i++)
	{
		if (s1[i] != s2[i])
			return false;
	}
	return true;
}
/* End of source */
