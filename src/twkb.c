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
	TWKB_HEADER_INFO thi;
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	BBOX bbox;
	ts.thi = &thi;
	ts.thi->bbox=&bbox;
	
	//~ buffer_collection res_buf;
	init_res_buf(res_buf);

	GEOM *g;
	int first_run = 1;
	TEXT_BUF output_text;
	init_text_buf(&output_text);

	tb.handled_buffer = 0; 
	tb.start_pos = tb.read_pos=buf;
	tb.end_pos=buf+buf_len;
	ts.tb=&tb;
	//~ ts.rb = &res_buf;
	txt2buf(&output_text, "[");
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_geojson(g,res_buf, &output_text);
		reset_buffer();
	}
	txt2buf(&output_text, "]");
	destroy_buffer();
	return output_text.start_pos;
}


static void init_geom_array(GEOMS *gs)
{
	int n = 100;
	gs->g = malloc(n*sizeof(void*));
	gs->ngeoms=0;
	gs->maxgeoms = n;
}
static void destroy_geom_array(GEOMS *gs)
{
	free(gs->g);
	gs->ngeoms=0;
	gs->maxgeoms = 0;
}



extern char* 
twkb2geoJSON_fromIndexedFile2D(char *file_name, float xmin, float ymin, float xmax, float ymax) //BBOX *requestedBbox )
{
	TWKB_HEADER_INFO thi;
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	
	BBOX bbox, requestedBbox;
	
	requestedBbox.bbox_min[0]=xmin;
	requestedBbox.bbox_min[1]=ymin;
	requestedBbox.bbox_max[0]=xmax;
	requestedBbox.bbox_max[1]=ymax;
	
	ts.thi = &thi;
	ts.thi->bbox=&bbox;
	
	//~ buffer_collection res_buf;
	init_res_buf(res_buf);
	int n;
	/*TODO
		Make this proper*/
	GEOMS gs;
	init_geom_array(&gs);
	
	//~ gs.g = malloc(200*sizeof(void*));
	//~ gs.ngeoms=0;
	//~ gs.maxgeoms = 200;
	
	if(buffer_from_file(file_name,&tb))
		return NULL;	
	
	ts.tb=&tb;
	//~ ts.rb = &res_buf;	
	recursive_index_search(&ts,&requestedBbox,1, &gs);
	
	int first_run = 1;
	TEXT_BUF output_text;
	
	init_text_buf(&output_text);

	

	txt2buf(&output_text, "[");
	for (n=0;n<gs.ngeoms;n++)
	{
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_geojson(gs.g[n],res_buf,  &output_text);
		reset_buffer();
		
	}
	txt2buf(&output_text, "]");
	destroy_buffer();
	if(tb.handled_buffer)
		free(tb.start_pos);
	close_file();
	destroy_geom_array(&gs);
	return  output_text.start_pos;
}


extern char* 
twkb2geoJSON_fromFile(char *file_name)
{
	TWKB_HEADER_INFO thi;
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	BBOX bbox;
	ts.thi = &thi;
	ts.thi->bbox=&bbox;
	
	//~ buffer_collection res_buf;
	init_res_buf(res_buf);

	GEOM *g;
	int first_run = 1;
	TEXT_BUF output_text;
	
	init_text_buf(&output_text);
	
	if(buffer_from_file(file_name,&tb))
		return NULL;
	
	ts.tb=&tb;
	//~ ts.rb = &res_buf;
	txt2buf(&output_text, "[");
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_geojson(g,res_buf, &output_text);
		reset_buffer();
		
	}
	txt2buf(&output_text, "]");
	destroy_buffer();
	if(tb.handled_buffer)
		free(tb.start_pos);
	close_file();
	return  output_text.start_pos;
}



extern char* 
twkb2esriJSON(uint8_t *buf,int buf_len,int srid)
{
	TWKB_HEADER_INFO thi;
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	BBOX bbox;
	ts.thi = &thi;
	ts.thi->bbox=&bbox;
	
	//~ buffer_collection res_buf;
	init_res_buf(res_buf);

	GEOM *g;
	int first_run = 1;
	TEXT_BUF output_text;
	init_text_buf(&output_text);

	tb.handled_buffer = 0; 
	tb.start_pos = tb.read_pos=buf;
	tb.end_pos=buf+buf_len;
	ts.tb=&tb;
	//~ ts.rb = &res_buf;
	txt2buf(&output_text, "[");
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_esrijson(g,srid,res_buf, &output_text,NULL);
		reset_buffer();
	}
	txt2buf(&output_text, "]");
	destroy_buffer();
	return output_text.start_pos;
}

extern char* 
twkb2esriJSON_fromFile(char *file_name,int srid)
{
	TWKB_HEADER_INFO thi;
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	BBOX bbox;
	ts.thi = &thi;
	ts.thi->bbox=&bbox;
	//~ buffer_collection res_buf;
	init_res_buf(res_buf);

	GEOM *g;
	int first_run = 1;
	TEXT_BUF output_text;
	
	init_text_buf(&output_text);
	
	if(buffer_from_file(file_name,&tb))
		return NULL;
	
	ts.tb=&tb;
	//~ ts.rb = &res_buf;
	txt2buf(&output_text, "[");
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		encode_esrijson(g,srid,res_buf, &output_text,NULL);
		reset_buffer();
		
	}
	txt2buf(&output_text, "]");
	destroy_buffer();
	if(tb.handled_buffer)
		free(tb.start_pos);
	close_file();
	return  output_text.start_pos;
}


extern char* 
twkb2esriJSON_fromIndexedFile2D(char *file_name, int srid, float xmin, float ymin, float xmax, float ymax) //BBOX *requestedBbox )
{
	TWKB_HEADER_INFO thi;
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	
	BBOX bbox, requestedBbox;
	
	requestedBbox.bbox_min[0]=xmin;
	requestedBbox.bbox_min[1]=ymin;
	requestedBbox.bbox_max[0]=xmax;
	requestedBbox.bbox_max[1]=ymax;
	
	ts.thi = &thi;
	ts.thi->bbox=&bbox;
	
	//~ buffer_collection res_buf;
	init_res_buf(res_buf);
	int n;
	/*TODO
		Make this properly*/
	GEOMS gs;
	init_geom_array(&gs);
	
	//~ gs.g = malloc(200*sizeof(void*));
	//~ gs.ngeoms=0;
	//~ gs.maxgeoms = 200;
	
	if(buffer_from_file(file_name,&tb))
		return NULL;	
	
	ts.tb=&tb;
	//~ ts.rb = &res_buf;	
	recursive_index_search(&ts,&requestedBbox,1, &gs);
	
	int first_run = 1;
	TEXT_BUF output_text;
	
	init_text_buf(&output_text);

	

	txt2buf(&output_text, "[");
	for (n=0;n<gs.ngeoms;n++)
	{
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
		encode_esrijson(gs.g[n],srid,res_buf, &output_text,NULL);
		reset_buffer();
		
	}
	txt2buf(&output_text, "]");
	destroy_buffer();
	if(tb.handled_buffer)
		free(tb.start_pos);
	close_file();	
	destroy_geom_array(&gs);
	return  output_text.start_pos;
}


/*Get the twkb-buffer from SQLite and output as geoJSON
We have to find soomething faster than parsing to geoJSON*/
extern char* 
twkb2geoJSON_fromSQLite()
{
	/*twkb structures*/
	TWKB_HEADER_INFO thi;
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;

	/*Sqlite*/
	sqlite3 *db;
	sqlite3_stmt *res;	
	sqlite3_blob *db_res;

	/*twkb-buffer*/
	uint8_t *buf;	
	size_t buf_len;

	BBOX bbox;
	ts.thi = &thi;
	ts.thi->bbox=&bbox;
		
	/*Init the buffer to keep the internal format as result of twkb-parsing*/
	init_res_buf(res_buf);

	/*The internal geoemtry format*/
	GEOM *g;

	int first_run = 1;

	/*The resulting output text (geoJSON)*/
	TEXT_BUF output_text;
	init_text_buf(&output_text);


	//~ ts.rb = &res_buf;
	txt2buf(&output_text, "[");	


	char *err_msg = 0;

	int rc = sqlite3_open("/home/nicklas/Documents/test.sqlite", &db);

	if (rc != SQLITE_OK) {		
	fprintf(stderr, "Cannot open database: %s\n", 
		sqlite3_errmsg(db));
	sqlite3_close(db);		
	return NULL;
	}

	rc = sqlite3_blob_open(db,  "main", "eiendom", "twkb", 1, 0, &db_res);   
	int n = 0;
	tb.handled_buffer = 0; 
	if (rc == SQLITE_OK)
	{
	do 
	{
	    buf_len = sqlite3_blob_bytes(db_res);
	    buf = malloc(buf_len);
	    rc = sqlite3_blob_read(db_res, buf, buf_len, 0);
	    

		tb.start_pos = tb.read_pos=buf;
		tb.end_pos=buf+buf_len;
		ts.tb=&tb;
	    
	while (ts.tb->read_pos<ts.tb->end_pos)
	{
		g=decode_twkb(&ts);
		if(first_run)
			first_run = 0;
		else
			txt2buf(&output_text, ",");
			
		 encode_geojson(g,res_buf, &output_text);
		reset_buffer();
	}

	    free(buf);		
	    
	    n++;
	}while(!sqlite3_blob_reopen(db_res, n));
	} 


	    char *sql = "SELECT twkb, gnr, bnr FROM eiendom limit 10";
		
	    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
	    //rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
	    
	    if (rc != SQLITE_OK ) {
		
		fprintf(stderr, "Failed to select data\n");
		fprintf(stderr, "SQL error: %s\n", err_msg);

		sqlite3_free(err_msg);
		sqlite3_close(db);
		
		return NULL;
	    } 


	rc = sqlite3_step(res);

	if (rc == SQLITE_ROW) {
	printf("%s\n", sqlite3_column_text(res, 0));
	 tb.handled_buffer = 0; 
	tb.start_pos = tb.read_pos=buf;
	tb.end_pos=buf+buf_len;





	}

	sqlite3_finalize(res);
	sqlite3_close(db);

	txt2buf(&output_text, "]");
	destroy_buffer();
	return output_text.start_pos;
}


