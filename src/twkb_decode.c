/* twkb_decode.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
 #include "twkb.h"

static void init_decode(TWKB_PARSE_STATE *ts,TWKB_PARSE_STATE *old_ts);
static GEOM* decode_point(TWKB_PARSE_STATE *ts);
static GEOM* decode_line(TWKB_PARSE_STATE *ts);
static GEOM* decode_polygon(TWKB_PARSE_STATE *ts);
static GEOM* decode_multi(TWKB_PARSE_STATE *ts);
static POINTARRAY* read_pointarray(TWKB_PARSE_STATE *ts, uint32_t npoints);
int64_t* decode_id_list(TWKB_PARSE_STATE *ts, int ngeoms);

GEOM*
decode_twkb_start(uint8_t *buf, size_t buf_len)
{
 	TWKB_HEADER_INFO thi;
	TWKB_PARSE_STATE ts;
	TWKB_BUF tb;
	
	BBOX bbox;
	ts.thi = &thi;
	ts.thi->bbox=&bbox;
	


	tb.handled_buffer = 0; 
    tb.start_pos = tb.read_pos=buf;
    tb.end_pos=buf+buf_len;
    ts.tb=&tb;
    //~ ts.rb = res_buf;
    return decode_twkb(&ts);
    //~ return NULL;
}

GEOM*
decode_twkb(TWKB_PARSE_STATE *old_ts)
{    TWKB_PARSE_STATE ts;
    init_decode(&ts, old_ts);
    read_header (&ts);
    switch (ts.thi->type)
    {
    case POINTTYPE:
        return decode_point(&ts);
        break;
    case LINETYPE:
        return decode_line(&ts);
        break;
    case POLYGONTYPE:
        return decode_polygon(&ts);
        break;
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
        return decode_multi(&ts);
        break;
    default:
	fprintf(stderr,"Error: Unknown type number %d\n",ts.thi->type);
	exit(EXIT_FAILURE);
    }
    return NULL;
}


static void
init_decode(TWKB_PARSE_STATE *ts,TWKB_PARSE_STATE *old_ts )
{
    int i;

	ts->tb = old_ts->tb;
	//~ ts->rb = old_ts->rb;
	ts->thi = old_ts->thi;	
	ts->thi->has_bbox=0;
	ts->thi->has_size=0;
	ts->thi->has_idlist=0;
	ts->thi->has_z=0;
	ts->thi->has_m=0;
	ts->thi->is_empty=0;
	ts->thi->type=0;

    for (i=0; i<TWKB_IN_MAXCOORDS; i++)
    {
        ts->thi->factors[i]=0;
        ts->thi->coords[i]=0;
    }
    ts->thi->ndims=0;

    return;
}


int
read_header (TWKB_PARSE_STATE *ts)
{
    uint8_t flag;
    int8_t precision;
    uint8_t has_ext_dims;
    ts->thi->ndims=2;
	int i;
    /*Here comes a byte containing type info and precission*/
    flag = buffer_read_byte(ts->tb);
    ts->thi->type=flag&0x0F;
    precision=unzigzag64((flag&0xF0)>>4);
    ts->thi->factors[0]=ts->thi->factors[1]= pow(10, (double)precision);
    ts->thi->n_decimals[0]=ts->thi->n_decimals[1]= precision>0?precision:0; /*We save number of decimals too, to get it right in text based formats in a simple way*/

    //Flags for options

    flag = buffer_read_byte(ts->tb);
    ts->thi->has_bbox   =  flag & 0x01;
    ts->thi->has_size   = (flag & 0x02) >> 1;
    ts->thi->has_idlist = (flag & 0x04) >> 2;
    has_ext_dims = (flag & 0x08) >> 3;

    if ( has_ext_dims )
    {
        flag = buffer_read_byte(ts->tb);

	ts->thi->has_z    = (flag & 0x01);
	ts->thi->has_m    = (flag & 0x02) >> 1;
        /* If Z*/	    
        if(ts->thi->has_z)
        {
		ts->thi->ndims++;
		precision = (flag & 0x1C) >> 2;
		ts->thi->factors[2]= pow(10, (double)precision);
		ts->thi->n_decimals[2]=precision>0?precision:0; 
        }

        /* If M*/
        if(ts->thi->has_m)
        {
            ts->thi->ndims++;
            precision = (flag & 0xE0) >> 5;
            ts->thi->factors[2+ts->thi->has_z]= pow(10, (double)precision);
		ts->thi->n_decimals[2+ts->thi->has_z]=precision>0?precision:0; 
        }
    }

    if(ts->thi->has_size)
    {
		/*We need to first read the value and then add the position of the cursor in the file.
		The size we get from the twkb-file is the size from after the size-info to the end of the twkb*/
	    ts->thi->next_offset = buffer_read_uvarint(ts->tb);
	    ts->thi->next_offset+= getReadPos(ts->tb);
    }


    if(ts->thi->has_bbox)
    {
	for (i=0;i<ts->thi->ndims;i++)
	{
		ts->thi->bbox->bbox_min[i]=buffer_read_svarint(ts->tb)/ts->thi->factors[i];
		ts->thi->bbox->bbox_max[i]=buffer_read_svarint(ts->tb)/ts->thi->factors[i] + ts->thi->bbox->bbox_min[i];
	}	    	    
    }

    return 0;
}


static GEOM*
decode_point(TWKB_PARSE_STATE *ts)
{
    POINT *g;
    g = get_space(sizeof(POINT));
    g->type = POINTTYPE;
    g->point = read_pointarray(ts, 1);
    return (GEOM*) g;
}


static GEOM*
decode_line(TWKB_PARSE_STATE *ts)
{
    LINE *g;
    int npoints;
    g = get_space(sizeof(LINE));
    g->type = LINETYPE; 
	
    npoints = (int) buffer_read_uvarint(ts->tb);

    g->points = read_pointarray(ts, npoints);

    return (GEOM*) g;
}

static GEOM*
decode_polygon(TWKB_PARSE_STATE *ts)
{
    POLY *g;
    int npoints, i;
    g = get_space(sizeof(POLY));

    g->type = POLYGONTYPE;
    g->nrings = (int) buffer_read_uvarint(ts->tb);


    g->rings = get_space(g->nrings*sizeof(void*));
    for (i=0; i<g->nrings; i++)
    {
        npoints = (int) buffer_read_uvarint(ts->tb);
        g->rings[i] = read_pointarray(ts, npoints);
    }
    
    return (GEOM*) g;
}


static GEOM*
decode_multi(TWKB_PARSE_STATE *ts)
{
    COLLECTION *g;
    int i;
    parseFunctions_p pf;
    g = get_space(sizeof(COLLECTION));
    g->type = COLLECTIONTYPE;
    g->ngeoms = (int) buffer_read_uvarint(ts->tb);
    if(ts->thi->has_idlist)
        g->idlist = decode_id_list(ts, g->ngeoms);
    else
        g->idlist = NULL;

    g->geoms = get_space(g->ngeoms*sizeof(void*));

    switch (ts->thi->type)
    {
    case MULTIPOINTTYPE:
        pf=decode_point;
        break;
    case MULTILINETYPE:
        pf=decode_line;
        break;
    case MULTIPOLYGONTYPE:
        pf=decode_polygon;
        break;
    case COLLECTIONTYPE:
        pf=decode_twkb;
        break;
    default:
	    pf=NULL;
    }
    for (i=0; i<g->ngeoms; i++)
    {
        g->geoms[i] = pf(ts);
    }
    return (GEOM*) g;
}


static POINTARRAY*
read_pointarray(TWKB_PARSE_STATE *ts, uint32_t npoints)
{
    POINTARRAY *pa = NULL;
    uint32_t ndims = ts->thi->ndims;
    int64_t val;
    int i,j;
    double *dlist;
	
    pa = get_space(sizeof(POINTARRAY));
	
    /* Empty! */
    //~ if( npoints == 0 )
    //~ return pa;
	
    pa->npoints=npoints;
    pa->has_z=ts->thi->has_z;
    pa->has_m=ts->thi->has_m;
    
    dlist = get_space(npoints*ts->thi->ndims*sizeof(double));

    for( i = 0; i < npoints; i++ )
    {			 
        for( j = 0; j < ndims; j++ )
        {
            val = buffer_read_svarint(ts->tb);
            ts->thi->coords[j] += val;
            dlist[ndims*i + j] = ts->thi->coords[j] / ts->thi->factors[j];
        }
    }
    
	for( j = 0; j < ndims; j++ )
	{
		pa->n_decimals[j]=ts->thi->n_decimals[j];
	}		
	
    pa->serialized_pointlist = dlist;
    return pa;
}


int64_t* decode_id_list(TWKB_PARSE_STATE *ts, int ngeoms)
{
    int i;
    int64_t *idlist= get_space(ngeoms*sizeof(int64_t));

    for (i=0; i<ngeoms; i++)
    {
        idlist[i]=buffer_read_svarint(ts->tb);
    }
    return idlist;
}

