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
	{
		*dest = malloc( WCH_SZ * ( size + 1 ) ) ;
		memcpy( *dest , source , size * WCH_SZ ) ;
		(*dest )[ size ] = L'\0' ;
	}

	return *dest ;
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


struct http *
parse_http( char_t *content )
{
	struct http		*resp		;
	struct field_parse	*root		,
				*parser		;
	char_t			*buffer		;
	uint			ui_cnt		,
				ui_iter		,
				ui_temp		,
				ui_cnt_fields	;

	buffer = malloc( WCH_SZ * BF_SZ ) ;
	resp = calloc( 1 , sizeof( struct http ) ) ;

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
	
	for( ++ui_iter , ui_temp = ui_iter ; ui_iter < ui_cnt && buffer[ ui_iter ] != L' ' ; ++ui_iter ) ;
	resp->version.minor = wstrtou( &buffer[ ui_temp ] , ui_iter - ui_temp ) ;

	ui_cnt = readTill( &content , buffer , L" " , 1 , L" " , 1 ) ;
	resp->r_code = wstrtou( buffer , ui_cnt ) ;

	ui_cnt = readTill( &content , buffer , L" " , 1 , L"\r " , 2 ) ;
	wcsnac( &resp->r_text , buffer , ui_cnt ) ;

	root = parser = calloc( 1 , sizeof( struct field_parse ) ) ;

	for( ui_cnt_fields = 0 ; ui_cnt ; )
	{
		ui_cnt = readTill( &content , buffer , L"\n " , 2 , L"\0\r:" , 3 ) ;
		if( ui_cnt )
		{
			wcsnac( &parser->name , buffer , ui_cnt ) ;
			++ui_cnt_fields ;
		}
	
		ui_cnt = readTill( &content , buffer , L" " , 1 , L"\0\r" , 2 ) ;
		readTill( &content , buffer , L"\n" , 1 , NULL , 0 ) ;
		if( ui_cnt )
		{
			wcsnac( &parser->value , buffer , ui_cnt ) ;
			
			parser->next = calloc( 1 , sizeof( struct field_parse ) ) ;
			parser = parser->next ;

			ui_cnt = 0 ;
		}
		else
			parser->value = NULL ;
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

	free( buffer ) ;

	return resp ;
}

void
free_http_resp( struct http * h )
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
