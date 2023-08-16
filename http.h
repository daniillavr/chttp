
#ifndef _HTTP_PARSER_H
#define _HTTP_PARSER_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <wctype.h>
#include <wchar.h>
#include <locale.h>
#include <stdarg.h>

#define	uint		unsigned int
#define uchar		unsigned char
#define ull		unsigned long long
#define	BF_SZ		( 1024 * 1024 )
#define WCH_SZ		sizeof( wchar_t )
#define char_t		wchar_t
#define MS_SIZE		8
#define MS_LEN		10
#define MIN_LEN_VER	6

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
http_resp
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

struct
http_req
{
	struct field	*fields		;
	struct
		{
			uchar	major	,
				minor	;
		}	version		;
	uint		f_cnt		;
	char_t		*method		,
			*path		;
} ;

struct http_resp *
parse_http( char_t * ) ;

char_t *
uinttowcs( uint , uint * ) ;
char_t *
req_to_text( struct http_req * , uint ) ;

uint
init_http_req( struct http_req * , uint , uint , char_t * , char_t * ) ;
uint
add_fields( int , ... ) ;

void
free_http_resp( struct http_resp * ) ;
void
free_http_req( struct http_req * ) ;

#endif
