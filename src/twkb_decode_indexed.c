/* twkb_decode.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
 #include "twkb.h"
#define NON_OVERLAPPING 1
#define CONTAINING 2
#define INTERSECTING 3


static int parseThis(TWKB_PARSE_STATE *ts, GEOMS *res);
static uint8_t compareBbox(BBOX *requestedBbox, BBOX *foundBbox,uint8_t ndims);

//~ GEOM*
//~ decode_twkb_start(uint8_t *buf, size_t buf_len,buffer_collection *res_buf)
//~ {
	//~ TWKB_HEADER_INFO thi;
	//~ TWKB_PARSE_STATE ts;
	//~ TWKB_BUF tb;
	
	//~ BBOX bbox;
	//~ ts.thi = &thi;
	//~ ts.thi->bbox=&bbox;
    //~ //buffer_collection res_buf;

	//~ tb.handled_buffer = 0; 
    //~ tb.start_pos = tb.read_pos=buf;
    //~ tb.end_pos=buf+buf_len;
    //~ ts.tb=&tb;
    //~ ts.rb = res_buf;
	
    //~ return decode_twkb(&ts);
    //~ return NULL;
//~ }


long int recursive_index_search(TWKB_PARSE_STATE *ts,BBOX *bbox,int nBoxes, GEOMS *res)
{
	int n,nextNBboxes, bboxCheck;
	long int save_pos;
	for (n=0;n<nBoxes;n++)
	{		
		if(jumpandread(ts->tb,0, 100 ))
		{
			fprintf(stderr,"%s","Problems reading from file\n");
			exit(EXIT_FAILURE);
		}		
		save_pos = getReadPos(ts->tb); //record where we are since we might want to find back
		
		read_header (ts);

		if(!(ts->thi->has_size))
		{
			setReadPos(ts->tb, save_pos, 1000);
			parseThis(ts, res);
		}
		else
		{
			bboxCheck = compareBbox(bbox, ts->thi->bbox, ts->thi->ndims);
			if( bboxCheck == NON_OVERLAPPING)
					setReadPos(ts->tb, ts->thi->next_offset, 100);
			else if (bboxCheck == CONTAINING || (bboxCheck == INTERSECTING && ts->thi->type != COLLECTIONTYPE))
			{
					setReadPos(ts->tb, save_pos, ts->thi->next_offset-save_pos);
		//			printf("(st_makebox2d(st_point(%f, %f), st_point(%f, %f))),\n",ts->thi->bbox->bbox_min[0],ts->thi->bbox->bbox_min[1],ts->thi->bbox->bbox_max[0],ts->thi->bbox->bbox_max[1]);
					parseThis(ts, res);	
			}
			else if (bboxCheck == INTERSECTING)
			{
					nextNBboxes = buffer_read_uvarint(ts->tb);
					buffer_jump_varint(ts->tb,nextNBboxes); //We jump over IDs for the subtiles for now. In the future we might use them
					setReadPos(ts->tb, recursive_index_search(ts, bbox,nextNBboxes,res), 100);				
			}
			else
			{
				fprintf(stderr,"Strange result from BBox comparing: %d\n",bboxCheck);
				exit(EXIT_FAILURE);				
			}
			
		}
	}
	return getReadPos(ts->tb);	
}


static uint8_t compareBbox(BBOX *requestedBbox, BBOX *foundBbox,uint8_t ndims)
{
		
	int n=0;
	int fullywithin=1;
	while (n<ndims)
	{
		if(requestedBbox->bbox_min[n]>foundBbox->bbox_max[n] || requestedBbox->bbox_max[n]<foundBbox->bbox_min[n])
			return NON_OVERLAPPING;				
		
		if(!(requestedBbox->bbox_min[n]<foundBbox->bbox_min[n] && requestedBbox->bbox_max[n]>foundBbox->bbox_max[n] && fullywithin))
			fullywithin=0;
		
		n++;
	}
	
	if(fullywithin)	
		return CONTAINING;
	else
		return INTERSECTING;
} 

static int add_to_geom_array(GEOMS *gs, GEOM *g)
{
	if(gs->ngeoms==gs->maxgeoms)
	{
		int new_n = gs->maxgeoms*2;
		gs->g = realloc(gs->g,new_n*sizeof(void*));
		gs->maxgeoms = new_n;
	}
	
	gs->g[gs->ngeoms++] = g;
	return 0;	
}

static int parseThis(TWKB_PARSE_STATE *ts, GEOMS *res)
{
	//~ while (ts->tb->read_pos<ts->tb->end_pos)
	//~ {
		/*TODO 
			Check if there is space enough for more geometries*/
		//~ res->g[res->ngeoms++]=decode_twkb(ts);
		add_to_geom_array(res, decode_twkb(ts));
	//~ }
	return 0;
	
}
