/* twkb.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
 #include "twkb.h"

extern char* 
twkb2geoJSON(uint8_t *buf,int buf_len)
{
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	buffer_collection res_buf;
	init_res_buf(&res_buf);

	GEOM *g;
	int first_run = 1;
	TEXT_BUF output_text;
	init_text_buf(&output_text);

	tb.handled_buffer = 0; 
	tb.start_pos = tb.read_pos=buf;
	tb.end_pos=buf+buf_len;
	ts.tb=&tb;
	ts.rb = &res_buf;
	txt2buf(&output_text, "[");
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_geojson(g,&res_buf, &output_text);
		reset_buffer(&res_buf);
	}
	txt2buf(&output_text, "]");
	destroy_buffer(&res_buf);
	return output_text.start_pos;
}

extern char* 
twkb2geoJSON_fromFile(char *file_name)
{
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	buffer_collection res_buf;
	init_res_buf(&res_buf);

	GEOM *g;
	int first_run = 1;
	TEXT_BUF output_text;
	
	init_text_buf(&output_text);
	
	if(buffer_from_file(file_name,&tb))
		return NULL;
	
	ts.tb=&tb;
	ts.rb = &res_buf;
	txt2buf(&output_text, "[");
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_geojson(g,&res_buf, &output_text);
		reset_buffer(&res_buf);
		
	}
	txt2buf(&output_text, "]");
	destroy_buffer(&res_buf);
	if(tb.handled_buffer)
		free(tb.start_pos);
	close_file();
	return  output_text.start_pos;
}



extern char* 
twkb2esriJSON(uint8_t *buf,int buf_len,int srid)
{
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	buffer_collection res_buf;
	init_res_buf(&res_buf);

	GEOM *g;
	int first_run = 1;
	TEXT_BUF output_text;
	init_text_buf(&output_text);

	tb.handled_buffer = 0; 
	tb.start_pos = tb.read_pos=buf;
	tb.end_pos=buf+buf_len;
	ts.tb=&tb;
	ts.rb = &res_buf;
	txt2buf(&output_text, "[");
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_esrijson(g,srid,&res_buf, &output_text,NULL);
		reset_buffer(&res_buf);
	}
	txt2buf(&output_text, "]");
	destroy_buffer(&res_buf);
	return output_text.start_pos;
}

extern char* 
twkb2esriJSON_fromFile(char *file_name,int srid)
{
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	buffer_collection res_buf;
	init_res_buf(&res_buf);

	GEOM *g;
	int first_run = 1;
	TEXT_BUF output_text;
	
	init_text_buf(&output_text);
	
	if(buffer_from_file(file_name,&tb))
		return NULL;
	
	ts.tb=&tb;
	ts.rb = &res_buf;
	txt2buf(&output_text, "[");
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_esrijson(g,srid,&res_buf, &output_text,NULL);
		reset_buffer(&res_buf);
		
	}
	txt2buf(&output_text, "]");
	destroy_buffer(&res_buf);
	if(tb.handled_buffer)
		free(tb.start_pos);
	close_file();
	return  output_text.start_pos;
}



