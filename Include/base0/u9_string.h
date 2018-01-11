#ifndef __U9_STRING_H
#define __U9_STRING_H

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4706)	// assignment within conditional expression
#endif

///////////////////////////////////////
//区分是否使用Unicode方式
///////////////////////////////////////
#ifdef UNICODE

typedef wchar_t		U9_CHAR;
typedef wstring		u9_string;
typedef wfstream	u9_fstream;

#ifdef WIN32
#include <tchar.h>
#else
#ifndef _T
#define _T(x)		L ## x
#endif
#endif

#ifdef WIN32
#define u9_wtoi _wtoi
#else
#define u9_wtoi(wstr) wcstol(wstr,NULL,10)
#endif //WIN32

#else	//UNICODE

typedef char		U9_CHAR;
#ifdef EGUI_UNICODE
typedef wstring u9_string;
#else
typedef string	u9_string;
#endif

typedef fstream		u9_fstream;

#ifndef _T
#ifdef EGUI_UNICODE
#define _T(x)       L ## x
#else
#define _T(x)		x
#endif
#endif

#endif 

#ifndef WIN32
inline char *_strlwr(char *s)
{
	if (!s) return NULL;
	while ( *s ) 
	{
		*s = (char) tolower( *s ); 
		++s;
	}
	return s;
}

inline wchar_t * u9_wcsrchr (
						   const wchar_t * string,
						   wchar_t ch
						   )
{
	wchar_t *start = (wchar_t *)string;

	while (*string++)                       /* find end of string */
		;
	/* search towards front */
	while (--string != start && *string != (wchar_t)ch)
		;

	if (*string == (wchar_t)ch)             /* wchar_t found ? */
		return( (wchar_t *)string );

	return(NULL);
}

#endif // !WIN32

//////////////////////////////////////////////////////////////////////////
// 下面这些函数是从vc copy来的。linux也使用这些函数。但是不支持big_endor
inline size_t u9_wcslen (const wchar_t * wcs)
{
	const wchar_t *eos = wcs;
	while( *eos++ ) ;
	return( (size_t)(eos - wcs - 1) );
}


inline size_t u9_wcsrtombs(char *_Dest, const wchar_t *_PSource, size_t _Count)
{
	if (!_Dest || !_PSource || _Count <= 0)
	{
		return 0;
	}

	size_t  count;
	size_t srclen;
	const wchar_t *src = _PSource;

	srclen= u9_wcslen(_PSource);
	if (_Count > srclen)
	{
		_Count = srclen;
	}
	// [xqh] 下面的*src++ 让人费解, 只需要src++就够了,BTW:用前置++效率更高
	for (count = 0; count < _Count; count++, *src++)
	{
		if (*src == '\0')
			break;

		_Dest[count] = (char)*src;
	}

	_Dest[count] = '\0';
	return count;
}

inline wchar_t * u9_wcsncpy (
									wchar_t * dest,
									const wchar_t * source,
									size_t count
									)
{
	wchar_t *start = dest;

	while (count && (*dest++ = *source++))    /* copy string */
		count--;

	if (count)                              /* pad out with zeroes */
		while (--count)
			*dest++ = L'\0';

	return(start);
}

inline int u9_wcscmp (
					const wchar_t * src,
					const wchar_t * dst
					)
{
	int ret = 0 ;

	while( ((ret = (int)(*src - *dst)) == 0) && *dst)
	{
		++src;
		++dst;
	}

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}

inline int u9_wcsicmp (
					 const wchar_t * src,
					 const wchar_t * dst
					 )
{
	int ret = 0 ;
	wchar_t lCmpA=*src;
	wchar_t lCmpB=*dst;

	if( lCmpA >= 0x41 &&  lCmpA <= 0x5a)
		 lCmpA += 0x20;

	if( lCmpB >= 0x41 &&  lCmpB <= 0x5a)
		lCmpB += 0x20;

	while( ((ret = (int)(lCmpB - lCmpA)) == 0) && lCmpB)
	{
		++src;
		++dst;
		lCmpA=*src;
		lCmpB=*dst;

		if( lCmpA >= 0x41 &&  lCmpA <= 0x5a)
			lCmpA += 0x20;

		if( lCmpB >= 0x41 &&  lCmpB <= 0x5a)
			lCmpB += 0x20;

	}

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}
inline wchar_t * u9_wcsncat (
						   wchar_t * front,
						   const wchar_t * back,
						   size_t count
						   )
{
	wchar_t *start = front;

	while (*front++)
		;
	front--;

	while (count--)
		if (!(*front++ = *back++))
			return(start);

	*front = L'\0';
	return(start);
}


inline int u9_wcsncmp (
					 const wchar_t * first,
					 const wchar_t * last,
					 size_t count
					 )
{
	if (!count)
		return(0);

	while (--count && *first && *first == *last)
	{
		first++;
		last++;
	}

	return((int)(*first - *last));
}

inline wchar_t * u9_wcscpy(wchar_t * dst, const wchar_t * src)
{
	wchar_t * cp = dst;

	while( *cp++ = *src++ )
		;               /* Copy src over dst */

	return( dst );
}
inline wchar_t * u9_wcscat (
						  wchar_t * dst,
						  const wchar_t * src
						  )
{
	wchar_t * cp = dst;

	while( *cp )
		cp++;                   /* find end of dst */

	while( *cp++ = *src++ ) ;       /* Copy src to end of dst */

	return( dst );                  /* return dst */

}

inline wchar_t * u9_wcschr (
						  const wchar_t * string,
						  wchar_t ch
						  )
{
	while (*string && *string != (wchar_t)ch)
		string++;

	if (*string == (wchar_t)ch)
		return((wchar_t *)string);
	return(NULL);
}

inline wchar_t * u9_wcsstr (
						  const wchar_t * wcs1,
						  const wchar_t * wcs2
						  )
{
	wchar_t *cp = (wchar_t *) wcs1;
	wchar_t *s1, *s2;

	if ( !*wcs2)
		return (wchar_t *)wcs1;

	while (*cp)
	{
		s1 = cp;
		s2 = (wchar_t *) wcs2;

		while ( *s1 && *s2 && !(*s1-*s2) )
			s1++, s2++;

		if (!*s2)
			return(cp);

		cp++;
	}

	return(NULL);
}

inline wchar_t * u9_wcslwr( wchar_t *string )
{
	int lilen = (int)u9_wcslen(string);
	for(int i = 0 ; i< lilen ; ++i)
	{
		string[i] = towlower(string[i]);
	}
	return string;
}

////////////////////////////////////////////
//	以为为字符串操作定义
////////////////////////////////////////////
#ifdef UNICODE

#define u9_strlen	u9_wcslen		// 取长度
#define u9_strcpy	u9_wcscpy		// 拷贝
#define u9_strcpy_s	wcscpy_s		// 拷贝
#define u9_strncpy	u9_wcsncpy		// 带长度拷贝
#define u9_strcat	u9_wcscat		// 合并
#define u9_strncat	u9_wcsncat		// 带长度合并
#define u9_strncpy_s	wcsncpy_s		// 带长度拷贝

#define u9_strncat_s	wcsncat_s		// 带长度合并
#define u9_strcmp	u9_wcscmp		// 比较
#define u9_strncmp	u9_wcsncmp		// 带长度比较
#define u9_strchr	u9_wcschr		// 在一字符串中查找另一字符串第一次出现的位置
#define u9_strstr	u9_wcsstr		// 替换字符串
#define u9_strspn	wcsspn			// 返回包含第二个字符串的初始数目
#define u9_strcspn	wcscspn			// 返回不包含第二个字符串的的初始数目
#define u9_stricmp	_wcsicmp		// 不区分大小写比较
#define u9_strcoll	wcscoll			// 
#define u9_strlwr	u9_wcslwr
#define u9_strupr	wcsupr
#define u9_strtok	wcstok 
#define u9_atoi		u9_wtoi


#define u9_itoa		_itow
#define u9_atol		_wtol
#define u9_ltoa		_ltow
#define u9_ultoa		_ultow
#define u9_atoul		wcstoul
#define u9_i64toa	_i64tow
#define u9_atoi64	_wtoi64

#define u9_sprintf	swprintf

#ifdef WIN32
#define u9_a_snprintf _snprintf
#define u9_strcat_s	wcscat_s		// 合并
#define u9_snprintf	_snwprintf
#define u9_sprintf_s	swprintf_s
#define u9_strrchr	wcsrchr			// 从尾部开始查找子字符串出现的第一个位置
#else
#define u9_a_snprintf snprintf
#define u9_strcat_s	wcscat			// 合并
#define u9_snprintf	swprintf
#define u9_sprintf_s	swprintf
#define u9_strrchr	u9_wcsrchr			// 从尾部开始查找子字符串出现的第一个位置
#endif // WIN32

#define u9_snprintf_s _snwprintf_s
#define u9_printf	wprintf
#define u9_scanf		wscanf
#define u9_fprintf	fwprintf
#define u9_fprintf_s	fwprintf_s

#define u9_fopen		_wfopen
#define u9_fopen_s	_wfopen_s

#define u9_strftime	wcsftime
#define u9_asctime	_wasctime

#define ZeroString(str,len) \
	memset(str,0,(len)*sizeof(wchar_t))

#define u9_streambuf	wstreambuf
#define u9_istream wistream
#define u9_ostream wostream
#define u9_iostream wiostream

  
#define u9_filebuf wfilebuf
#define u9_ifstream wifstream
#define u9_ofstream wofstream
#define u9_fstream wfstream

#define _u9_vsnprintf _vsnwprintf
#define u9_fputs		fputws
#else //UNICODE

#define u9_strlen	strlen
#define u9_strcpy	strcpy
#define u9_strncpy	strncpy
#define u9_strcat	strcat
#define u9_strncat	strncat			//带长度合并
#define u9_strcpy_s	strcpy_s
#define u9_strncpy_s	strncpy_s
#define u9_strcat_s	strcat_s
#define u9_strncat_s	strncat_s			//带长度合并
#define u9_strcmp	strcmp
#define u9_strncmp	strncmp
#define u9_strchr	strchr
#define u9_strstr	strstr
#define u9_strrchr	strrchr
#define u9_strspn	strspn
#define u9_strcspn	strcspn			//返回不包含第二个字符串的的初始数目
#define u9_stricmp	stricmp			//不区分大小写比较
#define u9_strcoll	strcoll
#define u9_strlwr	strlwr
#define u9_strupr	strupr
#define u9_strtok	strtok 


#define u9_atoi		atoi
#define u9_itoa		_itoa
#define u9_atol		atol
#define u9_ltoa		_ltoa
#define u9_ultoa		_ultoa
#define u9_atoul		strtoul
#define u9_i64toa	_i64toa
#define u9_atoi64	_atoi64

#define u9_sprintf	sprintf

#ifdef WIN32
#define u9_snprintf	_snprintf
#define u9_sprintf_s	sprintf_s
#else
#define u9_snprintf	snprintf
#define u9_sprintf_s	sprintf
#endif 

#define u9_printf	printf
#define u9_scanf		scanf
#define u9_fprintf	fprintf
#define u9_fprintf_s	fprintf_s

#define u9_fopen		fopen
#define u9_fopen_s	fopen_s

#define u9_strftime	strftime
#define u9_asctime	asctime

#define U9_ZeroString(str,len) \
	memset(str,0,len)

#define u9_streambuf	streambuf
#define u9_istream istream
#define u9_ostream ostream
#define u9_iostream iostream

#define u9_filebuf filebuf
#define u9_ifstream ifstream
#define u9_ofstream ofstream
#define u9_fstream fstream

#define _u9_vsnprintf _vsnprintf

#define u9_fputs		fputs
#endif

#define U9_AFXMESSAGEBOX(x)	AfxMessageBox(_T(x))

//-----------------------------------------------------------------------------
// 拷贝字符串宏 -- 将字符串拷贝到一个字符串指针上
#define CopyStringToStringPointer(dst, src)	\
	if(dst)	SAFE_DELETE_ARRAY(dst);	\
	if(src)	\
	{	\
		size_t sz = u9_strlen(src);	\
		dst = new U9_CHAR[sz+1];	\
		u9_strncpy(dst,src,sz);	\
		dst[sz]=0;\
	}

//-----------------------------------------------------------------------------
// 比较两个字符串的内容，如果内容相等返回0，不同返回非0
inline int CompareStringContents(const U9_CHAR* src, const U9_CHAR* dst)
{
	if(src && dst)
		return u9_strcmp(src, dst);
	if(src)
		return 1;

	if(dst)
		return -1;

	return 0;
}
#ifdef WIN32
//-----------------------------------------------------------------------------
// 检查文件的扩展名, apszExt的格式为 .xxx
inline bool CheckFileExt(const U9_CHAR* apszFile, const U9_CHAR* apszExt)
{
	if(!apszFile || !*apszFile || !apszExt || !*apszExt) return false;

	// 获取文件字符串的长度
	size_t s = u9_strlen(apszFile)-1;
	U9_CHAR* p2 = (U9_CHAR*)apszExt;
	U9_CHAR* p1 = (U9_CHAR*)(apszFile+s);
	// 从右搜索的一个 "." 字符
	while(p1!=apszFile)
	{
		if(*p1==_T('.'))
		{
			return (u9_stricmp(p1, p2)==0);
		}
		p1--;
	}
	return false;
}
#endif 

//-----------------------------------------------------------------------------

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif
