#include "http.h"

char Methods[ MS_SIZE ][ MS_LEN ] =
	{
		"OPTIONS" ,
		"GET" ,
		"HEAD" ,
		"POST" ,
		"PUT" ,
		"DELETE" ,
		"TRACE" ,
		"CONNECT" ,
	} ;

char *
strnac( char **dest , char *source , uint size )
{
	if( !size )
		*dest = NULL ;
	else
	{
		*dest = malloc( CH_SZ * ( size + 1 ) ) ;
		memcpy( *dest , source , size * CH_SZ ) ;
		(*dest )[ size ] = '\0' ;
	}

	return *dest ;
}

char *
strconcat( char *dest , char *source , char *add_str )
{
	uint	len_add_str	,
		len_dest	;
	char	*result		;

	len_add_str = strlen( add_str ) ;
	
	if( len_add_str )
	{
		len_dest = strlen( dest ) + strlen( source ) + len_add_str + 1 ;
		result = malloc( CH_SZ * ( len_dest ) ) ;
		*result = '\0' ;

		result = strcat( result , dest ) ;
		result = strcat( result , add_str ) ;
	}
	else
	{
		len_dest = strlen( dest ) + strlen( source ) + 1 ;
		result = malloc( CH_SZ * ( len_dest ) ) ;
		*result = '\0' ;

		result = strcat( result , dest ) ;
	}

	result = strcat( result , source ) ;

	return result ;
}

uint
strtou( const char *number , uint len )
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
		result += ( *number - '0' ) * power ;

	return result ;
}

char *
utostr( uint number , uint *len )
{
	char		*buffer	,
			*pbuff	;
	uint		tens	,
			tnumber	;

	*len = 0 ;
	for( tnumber = number , tens = 1 ; tnumber ; tnumber /= 10 , tens *= 10 , ++( *len ) ) ;
	tens /= 10 ;

	if( !number )
		( *len )++ ;

	pbuff = buffer = malloc( CH_SZ * ( *len + 1 ) ) ;
	buffer[ *len ] = '\0' ;

	if( !number )
		*buffer = '0' ;

	while( number )
	{
		*pbuff = number / tens + '0' ;
		number -= ( ( number / tens ) * tens ) ;
		pbuff++ ;
	}

	return buffer ;
}


uint
readTill( char **content , char *buffer , char *skip , size_t size_skip , char *till , size_t size_till )
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
preprocess_str( char *content )
{
	for( ; !( !strcmp( content , "\n\n\0" ) || !strcmp( content , "\r\n\r\n\0" ) ) ; ++content )
	{
		if( *content == '\r' )
			*content = ' ' ;
	}
}


struct http_resp *
parse_http( char *content )
{
	struct http_resp	*resp		;
	struct field_parse	*root		,
				*parser		,
				*prev_field	,
				*cur_field	;
	char			*buffer		;
	uint			ui_cnt		,
				ui_iter		,
				ui_temp		,
				ui_cnt_fields	;

	buffer = malloc( sizeof( char ) * BF_SZ ) ;
	resp = calloc( 1 , sizeof( struct http_resp ) ) ;

	preprocess_str( content ) ;

	ui_cnt = readTill( &content , buffer , NULL , 0 , " " , 1 ) ;

	if( !strcmp( buffer , "HTTP\0" ) || ui_cnt < MIN_LEN_VER )
	{
		free( buffer ) ;
		free( resp ) ;
		return NULL ;
	}

	for( ui_iter = 0 ; ui_iter < ui_cnt && buffer[ ui_iter ] != '/' ; ++ui_iter ) ;
	for( ++ui_iter , ui_temp = ui_iter ; ui_iter < ui_cnt && buffer[ ui_iter ] != '.' ; ++ui_iter ) ;
	resp->version.major = strtou( &buffer[ ui_temp ] , ui_iter - ui_temp ) ;
	
	if( buffer[ ui_iter ] == '.' )
	{
		for( ++ui_iter , ui_temp = ui_iter ; ui_iter < ui_cnt && buffer[ ui_iter ] != ' ' ; ++ui_iter ) ;
		resp->version.minor = strtou( &buffer[ ui_temp ] , ui_iter - ui_temp ) ;
	}
	else
	{
		resp->version.minor = 0 ;
	}

	ui_cnt = readTill( &content , buffer , " " , 1 , " " , 1 ) ;
	resp->r_code = strtou( buffer , ui_cnt ) ;

	ui_cnt = readTill( &content , buffer , " " , 1 , "\n " , 2 ) ;
	strnac( &resp->r_text , buffer , ui_cnt ) ;

	root = prev_field = parser = calloc( 1 , sizeof( struct field_parse ) ) ;

	for( ui_cnt_fields = 0 ; prev_field ; )
	{
		ui_cnt = readTill( &content , buffer , " " , 1 , "\0\n:" , 3 ) ;
		if( ui_cnt )
		{
			strnac( &parser->name , buffer , ui_cnt ) ;
			++ui_cnt_fields ;

			ui_cnt = readTill( &content , buffer , " " , 1 , "\0\n" , 2 ) ;

			prev_field = parser ;
			if( ui_cnt )
			{
				strnac( &parser->value , buffer , ui_cnt ) ;
				
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

	ui_cnt = readTill( &content , buffer , "\n " , 2 , "\0" , 1 ) ;
	strnac( &resp->body , buffer , ui_cnt ) ;

	free( buffer ) ;

	for( prev_field = root ; prev_field != NULL ; prev_field = prev_field->next )
		for( parser = prev_field , cur_field = prev_field->next ; cur_field != NULL ; parser = cur_field , cur_field = cur_field->next )
			if( !strcmp( prev_field->name , cur_field->name ) )
			{
				buffer = prev_field->value ;
				prev_field->value = strconcat( prev_field->value , cur_field->value , ", \0" ) ;

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
			strnac( &resp->fields[ ui_iter ].name , parser->name , strlen( parser->name ) ) ;
			if( parser->value )
				strnac( &resp->fields[ ui_iter ].value , parser->value , strlen( parser->value ) ) ;
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
	char		*name	,
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
		name = va_arg( vl , char* ) ;
		value = va_arg( vl , char* ) ;

		strnac( &req->fields[ ui_iter ].name , name , strlen( name ) ) ;
		strnac( &req->fields[ ui_iter ].value , value , strlen( value ) ) ;
	}

	return cnt ;
}


uint
init_http_req( struct http_req *req , uint ver_major , uint ver_minor , char *path , char *method )
{
	req->version.major = ver_major ;
	req->version.minor = ver_minor ;
	strnac( &req->path , path , strlen( path ) ) ;
	strnac( &req->method , method , strlen( method ) ) ;

	return 0 ;
}

/*
	flags:
		- 1 bit - use minor version
*/
char *
req_to_text( struct http_req *req , uint flags )
{
	uint	len_text	,
		ui_iter		,
		len_major	,
		len_minor	;
	char	*text		,
		*ptext		,
		*major_ver	,
		*minor_ver	;

	minor_ver = major_ver = NULL ;

	len_text = strlen( req->method ) + 1 ; // Additional space
	len_text += strlen( req->path ) + 1 ; // Additional space

	len_text += 5 ; // "HTTP/"

	major_ver = utostr( req->version.major , &len_major ) ;

	len_text += len_major ;

	if( flags & 1 )
	{
		minor_ver = utostr( req->version.minor , &len_minor ) ;

		len_text += len_minor + 2 ; // . + {ver} + space
	}

	len_text += 1 ; // Additional '\n'

	for( ui_iter = 0 ; ui_iter < req->f_cnt ; ++ui_iter )
	{
		len_text += strlen( req->fields[ ui_iter ].name ) + 2 ; // Additional space and ':'
		len_text += strlen( req->fields[ ui_iter ].value ) + 1 ; // Additional '\n'
	}

	len_text += 3 ; // Additional "\n\n\0"

	ptext = text = malloc( CH_SZ * len_text ) ;
	*text = '\0' ;

	strcat( text , req->method ) ;
	ptext += strlen( req->method ) ;
	*ptext++ = ' ' ;
	*ptext = '\0' ;

	strcat( text , req->path ) ;
	ptext += strlen( req->path ) ;
	*ptext++ = ' ' ;
	*ptext = '\0' ;

	strcat( text , "HTTP/\0" ) ;
	ptext += 5 ;

	strcat( text , major_ver ) ;
	ptext += len_major ;

	if( flags & 1 )
	{
		*ptext++ = '.' ;
		*ptext = '\0' ;

		strcat( text , minor_ver ) ;
		ptext += len_minor ;
	}

	*ptext++ = '\n' ;
	*ptext = '\0' ;

	for( ui_iter = 0 ; ui_iter < req->f_cnt ; ++ui_iter )
	{
		strcat( text , req->fields[ ui_iter ].name ) ;
		ptext += strlen( req->fields[ ui_iter ].name ) ;

		*ptext++ = ':' ;
		*ptext++ = ' ' ;
		*ptext = '\0' ;

		strcat( text , req->fields[ ui_iter ].value ) ;
		ptext += strlen( req->fields[ ui_iter ].value ) ;

		*ptext++ = '\n' ;
		*ptext = '\0' ;
	}

	*ptext++ = '\n' ;
	*ptext = '\0' ;

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
