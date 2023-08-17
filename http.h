
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

struct
field
{
	char	*name	,
		*value	;
} ;

struct
field_parse
{
	char			*name	,
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
	char		*r_text		,
			*body		;
			

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
	char		*method		,
			*path		;
} ;

struct http_resp *
parse_http( char * ) ;

char *
req_to_text( struct http_req * , uint ) ;

uint
init_http_req( struct http_req * , uint , uint , char * , char * ) ;
uint
add_fields( int , ... ) ;

void
free_http_resp( struct http_resp * ) ;
void
free_http_req( struct http_req * ) ;

#endif
