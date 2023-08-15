
#ifndef _HTTP_PARSER_H
#define _HTTP_PARSER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>

#define	uint		unsigned int
#define uchar		unsigned char
#define ull		unsigned long long
#define	BF_SZ		( 1024 * 1024 )
#define WCH_SZ		sizeof( wchar_t )
#define char_t		wchar_t
#define MS_SIZE		8
#define MS_LEN		10
#define MIN_LEN_VER	8

struct
field
{
	char_t	*name	,
		*value	;
} ;

struct
field_parse
{
	char_t			*name	,
				*value	;
	struct field_parse	*next	;
} ;

struct
http
{
	struct field	*fields		;
	struct
		{
			uchar	major	,
				minor	;
		}	version		;
	uint		r_code		,
			f_cnt		;
	char_t		*body		,
			*r_text		;

} ;

struct http *
parse_http( char_t * ) ;

void
free_http_resp( struct http * h ) ;

#endif
