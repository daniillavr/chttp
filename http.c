#include "http.h"

char_t Methods[ MS_SIZE ][ MS_LEN ] =
	{
		L"OPTIONS" ,
		L"GET" ,
		L"HEAD" ,
		L"POST" ,
		L"PUT" ,
		L"DELETE" ,
		L"TRACE" ,
		L"CONNECT" ,
	} ;

char_t *
wcsnac( char_t **dest , char_t *source , uint size )
{
	if( !size )
		*dest = NULL ;
	else
	{
		*dest = malloc( WCH_SZ * ( size + 1 ) ) ;
		memcpy( *dest , source , size * WCH_SZ ) ;
		(*dest )[ size ] = L'\0' ;
	}

	return *dest ;
}

char_t *
wcsconcat( char_t *dest , char_t *source , char_t *add_str )
{
	uint	len_add_str	,
		len_dest	;
	char_t	*result		;

	len_add_str = wcslen( add_str ) ;
	
	if( len_add_str )
	{
		len_dest = wcslen( dest ) + wcslen( source ) + len_add_str + 1 ;
		result = malloc( WCH_SZ * ( len_dest ) ) ;
		*result = L'\0' ;

		result = wcscat( result , dest ) ;
		result = wcscat( result , add_str ) ;
	}
	else
	{
		len_dest = wcslen( dest ) + wcslen( source ) + 1 ;
		result = malloc( WCH_SZ * ( len_dest ) ) ;
		*result = L'\0' ;

		result = wcscat( result , dest ) ;
	}

	result = wcscat( result , source ) ;

	return result ;
}

uint
wstrtou( const char_t *number , uint len )
{
	uint	result	,
		power	,
		i	;

	if( !len )
		return 0 ;

	result = 0 ;

	power = 1 ;
	for( i = 1 ; i < len ; ++i )
		power *= 10 ;

	for( ; len > 0 ; --len , ++number , power /= 10 )
		result += ( *number - L'0' ) * power ;

	return result ;
}

char_t *
uinttowcs( uint number , uint *len )
{
	char_t		*buffer	,
			*pbuff	;
	uint		tens	,
			tnumber	;

	*len = 0 ;
	for( tnumber = number , tens = 1 ; tnumber ; tnumber /= 10 , tens *= 10 , ++( *len ) ) ;
	tens /= 10 ;

	if( !number )
		( *len )++ ;

	pbuff = buffer = malloc( WCH_SZ * ( *len + 1 ) ) ;
	buffer[ *len ] = L'\0' ;

	if( !number )
		*buffer = L'0' ;

	while( number )
	{
		*pbuff = number / tens + L'0' ;
		number -= ( ( number / tens ) * tens ) ;
		pbuff++ ;
	}

	return buffer ;
}


uint
readTill( char_t **content , char_t *buffer , char_t *skip , size_t size_skip , char_t *till , size_t size_till )
{
	uint	ui_iter	,
		ui_cnt	;
	uchar	found	;

	ui_cnt = 0 ;

	if( size_skip )
	{
		do
		{
			for( found = 0 , ui_iter = 0 ; ui_iter < size_skip ; ++ui_iter )
				if( skip[ ui_iter  ] == **content )
					found = 1 ;

			if( found )
				++( *content ) ;
		} while( found ) ;
	}

	if( size_till )
	{
		do
		{
			for( found = 0 , ui_iter = 0 ; ui_iter < size_till ; ++ui_iter )
				if( till[ ui_iter  ] == **content )
					found = 1 ;

			if( !found )
			{
				buffer[ ui_cnt++ ] = **content ;
				++( *content ) ;
			}
			else
				++( *content ) ;
		} while( !found ) ;
	}

	return ui_cnt ;
}


void
preprocess_wstr( char_t *content )
{
	for( ; !( !wcscmp( content , L"\n\n\0" ) || !wcscmp( content , L"\r\n\r\n\0" ) ) ; ++content )
	{
		if( *content == L'\r' )
			*content = L' ' ;
	}
}


struct http_resp *
parse_http( char_t *content )
{
	struct http_resp		*resp		;
	struct field_parse	*root		,
				*parser		,
				*prev_field	,
				*cur_field	;
	char_t			*buffer		;
	uint			ui_cnt		,
				ui_iter		,
				ui_temp		,
				ui_cnt_fields	;

	buffer = malloc( WCH_SZ * BF_SZ ) ;
	resp = calloc( 1 , sizeof( struct http_resp ) ) ;

	preprocess_wstr( content ) ;

	ui_cnt = readTill( &content , buffer , NULL , 0 , L" " , 1 ) ;

	if( wcsncmp( buffer , L"HTTP" , 4 ) || ui_cnt < MIN_LEN_VER )
	{
		free( buffer ) ;
		free( resp ) ;
		return NULL ;
	}

	for( ui_iter = 0 ; ui_iter < ui_cnt && buffer[ ui_iter ] != L'/' ; ++ui_iter ) ;
	for( ++ui_iter , ui_temp = ui_iter ; ui_iter < ui_cnt && buffer[ ui_iter ] != L'.' ; ++ui_iter ) ;
	resp->version.major = wstrtou( &buffer[ ui_temp ] , ui_iter - ui_temp ) ;
	
	if( buffer[ ui_iter ] == L'.' )
	{
		for( ++ui_iter , ui_temp = ui_iter ; ui_iter < ui_cnt && buffer[ ui_iter ] != L' ' ; ++ui_iter ) ;
		resp->version.minor = wstrtou( &buffer[ ui_temp ] , ui_iter - ui_temp ) ;
	}
	else
	{
		resp->version.minor = 0 ;
	}

	ui_cnt = readTill( &content , buffer , L" " , 1 , L" " , 1 ) ;
	resp->r_code = wstrtou( buffer , ui_cnt ) ;

	ui_cnt = readTill( &content , buffer , L" " , 1 , L"\n " , 2 ) ;
	wcsnac( &resp->r_text , buffer , ui_cnt ) ;

	root = prev_field = parser = calloc( 1 , sizeof( struct field_parse ) ) ;

	for( ui_cnt_fields = 0 ; prev_field ; )
	{
		ui_cnt = readTill( &content , buffer , L" " , 1 , L"\0\n:" , 34 ) ;
		if( ui_cnt )
		{
			wcsnac( &parser->name , buffer , ui_cnt ) ;
			++ui_cnt_fields ;

			ui_cnt = readTill( &content , buffer , L" " , 1 , L"\0\n" , 2 ) ;

			prev_field = parser ;
			if( ui_cnt )
			{
				wcsnac( &parser->value , buffer , ui_cnt ) ;
				
				parser->next = calloc( 1 , sizeof( struct field_parse ) ) ;
				parser = parser->next ;
			}
			else
			{
				parser->value = NULL ;
			}
		}
		else
		{
			prev_field->next = NULL ;
			prev_field = NULL ;
		}
	}

	ui_cnt = readTill( &content , buffer , L"\n " , 1 , L"\0\n" , 2 ) ;
	wcsnac( &resp->body , buffer , ui_cnt ) ;

	free( buffer ) ;

	for( prev_field = root ; prev_field != NULL ; prev_field = prev_field->next )
		for( parser = prev_field , cur_field = prev_field->next ; cur_field != NULL ; parser = cur_field , cur_field = cur_field->next )
			if( !wcscmp( prev_field->name , cur_field->name ) )
			{
				buffer = prev_field->value ;
				prev_field->value = wcsconcat( prev_field->value , cur_field->value , L", \0" ) ;

				parser->next = cur_field->next ;
				
				if( cur_field->value )
					free( cur_field->value ) ;
				if( cur_field->name )
					free( cur_field->name ) ;
				free( buffer ) ;
				
				--ui_cnt_fields ;
				cur_field = prev_field ;
			}

	if( ui_cnt_fields )
	{
		resp->f_cnt = --ui_cnt_fields ;
		resp->fields = malloc( sizeof( struct field ) * ui_cnt_fields ) ;

		for( parser = root , ui_iter = 0 ; ui_iter < ui_cnt_fields ; parser = parser->next , ui_iter++ )
		{
			wcsnac( &resp->fields[ ui_iter ].name , parser->name , wcslen( parser->name ) ) ;
			if( parser->value )
				wcsnac( &resp->fields[ ui_iter ].value , parser->value , wcslen( parser->value ) ) ;
		}
	}

	if( root )
		do
		{
			parser = root->next ;
			if( root->name )
				free( root->name ) ;
			if( root->value )
				free( root->value ) ;
			free( root ) ;
			root = parser ;
		} while( root ) ;

	return resp ;
}


uint
add_fields( int cnt , ... )
{
	struct http_req	*req	;
	va_list		vl	;
	uint		ui_iter	;
	char_t		*name	,
			*value	;

	va_start( vl , cnt ) ;

	--cnt ;

	if( cnt & 1 || cnt < 0 )
	{
		return 0 ;
	}

	req = va_arg( vl , struct http_req* ) ;

	cnt /= 2 ;

	req->f_cnt = cnt ;
	req->fields = malloc( sizeof( struct field ) * cnt ) ;

	for( ui_iter = 0 ; ui_iter < cnt ; ++ui_iter )
	{
		name = va_arg( vl , char_t* ) ;
		value = va_arg( vl , char_t* ) ;

		wcsnac( &req->fields[ ui_iter ].name , name , wcslen( name ) ) ;
		wcsnac( &req->fields[ ui_iter ].value , value , wcslen( value ) ) ;
	}

	return cnt ;
}


uint
init_http_req( struct http_req *req , uint ver_major , uint ver_minor , char_t *path , char_t *method )
{
	req->version.major = ver_major ;
	req->version.minor = ver_minor ;
	wcsnac( &req->path , path , wcslen( path ) ) ;
	wcsnac( &req->method , method , wcslen( method ) ) ;

	return 0 ;
}

/*
	flags:
		- 1 bit - use minor version
*/
char_t *
req_to_text( struct http_req *req , uint flags )
{
	uint	len_text	,
		ui_iter		,
		len_major	,
		len_minor	;
	char_t	*text		,
		*ptext		,
		*major_ver	,
		*minor_ver	;

	minor_ver = major_ver = NULL ;

	len_text = wcslen( req->method ) + 1 ; // Additional space
	len_text += wcslen( req->path ) + 1 ; // Additional space

	len_text += 5 ; // "HTTP/"

	major_ver = uinttowcs( req->version.major , &len_major ) ;

	len_text += len_major ;

	if( flags & 1 )
	{
		minor_ver = uinttowcs( req->version.minor , &len_minor ) ;

		len_text += len_minor + 2 ; // . + {ver} + space
	}

	len_text += 1 ; // Additional '\n'

	for( ui_iter = 0 ; ui_iter < req->f_cnt ; ++ui_iter )
	{
		len_text += wcslen( req->fields[ ui_iter ].name ) + 2 ; // Additional space and ':'
		len_text += wcslen( req->fields[ ui_iter ].value ) + 1 ; // Additional '\n'
	}

	len_text += 3 ; // Additional "\n\n\0"

	ptext = text = malloc( WCH_SZ * len_text ) ;
	*text = L'\0' ;

	wcscat( text , req->method ) ;
	ptext += wcslen( req->method ) ;
	*ptext++ = L' ' ;
	*ptext = L'\0' ;

	wcscat( text , req->path ) ;
	ptext += wcslen( req->path ) ;
	*ptext++ = L' ' ;
	*ptext = L'\0' ;

	wcscat( text , L"HTTP/\0" ) ;
	ptext += 5 ;

	wcscat( text , major_ver ) ;
	ptext += len_major ;

	if( flags & 1 )
	{
		*ptext++ = L'.' ;
		*ptext = L'\0' ;

		wcscat( text , minor_ver ) ;
		ptext += len_minor ;
	}

	*ptext++ = L'\n' ;
	*ptext = L'\0' ;

	for( ui_iter = 0 ; ui_iter < req->f_cnt ; ++ui_iter )
	{
		wcscat( text , req->fields[ ui_iter ].name ) ;
		ptext += wcslen( req->fields[ ui_iter ].name ) ;

		*ptext++ = L':' ;
		*ptext++ = L' ' ;
		*ptext = L'\0' ;

		wcscat( text , req->fields[ ui_iter ].value ) ;
		ptext += wcslen( req->fields[ ui_iter ].value ) ;

		*ptext++ = L'\n' ;
		*ptext = L'\0' ;
	}

	*ptext++ = L'\n' ;
	*ptext = L'\0' ;

	free( major_ver ) ;
	if( minor_ver )
		free( minor_ver ) ;

	return text ;
}


void
free_http_resp( struct http_resp * h )
{
	uint	i	;

	if( !h )
		return ;

	if( h->body )
		free( h->body ) ;

	if( h->r_text )
		free( h->r_text ) ;
	
	for( i = 0 ; i < h->f_cnt ; ++i )
	{
		free( h->fields[ i ].name ) ;
		if( h->fields[ i ].value )
			free( h->fields[ i ].value ) ;
	}

	if( h->fields )
		free( h->fields ) ;

	free( h ) ;
}

void
free_http_req( struct http_req * h )
{
	uint	i	;

	if( !h )
		return ;

	if( h->method )
		free( h->method ) ;

	if( h->path )
		free( h->path ) ;
	
	for( i = 0 ; i < h->f_cnt ; ++i )
	{
		free( h->fields[ i ].name ) ;
		if( h->fields[ i ].value )
			free( h->fields[ i ].value ) ;
	}

	if( h->fields )
		free( h->fields ) ;

	free( h ) ;
}
