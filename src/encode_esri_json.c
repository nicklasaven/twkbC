/* encode_esri_json.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
 #include "twkb.h"


static int encode_point(POINT *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf,int *id );
static int encode_mpoint(POINT *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf,int *id  );
static int encode_line(LINE *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf ,int *id );
static int encode_polygon(POLY *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf,int *id  );

static int encode_multi(COLLECTION *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf );




char* encode_esrijson_start(GEOM *g,int srid,buffer_collection *mem_buf)
{
	TEXT_BUF buf;
	init_text_buf(&buf);
	return encode_esrijson(g,srid,mem_buf, &buf,NULL);
}

char* encode_esrijson(GEOM *g,int srid,buffer_collection *mem_buf,TEXT_BUF *buf,int *id )
{
    switch (g->type)
    {
    case POINTTYPE:
        encode_point((POINT*) g,srid,buf, mem_buf,id );
        break;
    case LINETYPE:
        encode_line((LINE*) g,srid,buf, mem_buf,id );
        break;
    case POLYGONTYPE:
        encode_polygon((POLY*) g,srid,buf, mem_buf,id );
        break;
    case MULTIPOINTTYPE:
    case MULTILINETYPE:
    case MULTIPOLYGONTYPE:
    case COLLECTIONTYPE:
        encode_multi((COLLECTION*) g,srid,buf, mem_buf );
        break;

    }

    return buf->start_pos;

}

static int encode_point(POINT *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf,int *id )
{
    double x,y;
    POINTARRAY *pa = g->point;
    if(pa->has_z || pa-> has_m)
        return encode_mpoint((POINT*) g,srid, buf, mem_buf,id );

	if(id)
	{
		txt2buf(buf, "{\"GlobalId\":");
		int2buf(buf,*id);
		txt2buf(buf, ",\"geometry\":");
	}
	else
		txt2buf(buf, "{\"geometry\":");
    x = pa->serialized_pointlist[0];
    y = pa->serialized_pointlist[1];
    txt2buf(buf,"{\"x\":");


    num2buf(buf, x,pa->n_decimals[0]);
    txt2buf(buf, ",\"y\":");
    num2buf(buf, y,pa->n_decimals[1]);
    txt2buf(buf, ",\"spatialReference\":{\"wkid\":");
    int2buf(buf,srid);
    txt2buf(buf, "}}}");
    return 0;
}

static int 
set_zm(POINTARRAY *pa,TEXT_BUF *buf)
{
    if(pa->has_z)
        txt2buf(buf, "{\"hasZ\":true,");
    if(pa->has_m)
        txt2buf(buf, "\"hasM\":true,");
    return 0;
}


static int encode_mpoint(POINT *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf,int *id )
{
    int i;
    POINTARRAY *pa = g->point;
    int ndims = 2 + pa->has_z+pa->has_m;

	if(id)
	{
		txt2buf(buf, "{\"GlobalId\":");
		int2buf(buf,*id);
		txt2buf(buf, ",\"geometry\":");
	}
	else
		txt2buf(buf, "{\"geometry\":");

    set_zm(pa, buf);

    txt2buf(buf, "\"points\":[[");
    for (i=0; i<pa->npoints; i++)
    {
	if(i!=0)
		txt2buf(buf, ",");
        coords2buf(buf, pa->serialized_pointlist+i*ndims,ndims, pa->n_decimals);
    }
    txt2buf(buf, "],\"spatialReference\":{\"wkid\":");
    int2buf(buf,srid);
    txt2buf(buf, "}}}");
    return 0;
}


static int encode_line(LINE *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf ,int *id )
{
    int i,ndims;
    POINTARRAY *pa = g->points;

    ndims = 2 + pa->has_z+pa->has_m;

	if(id)
	{
		txt2buf(buf, "{\"GlobalId\":");
		int2buf(buf,*id);
		txt2buf(buf, ",\"geometry\":");
	}
	else
		txt2buf(buf, "{\"geometry\":");
    set_zm(pa, buf);

    txt2buf(buf, "{\"paths\":[[");

    for (i=0; i<pa->npoints; i++)
    {
	if(i!=0)
		txt2buf(buf, ",");
        coords2buf(buf, pa->serialized_pointlist+i*ndims,ndims, pa->n_decimals);
    }

    txt2buf(buf, "]],\"spatialReference\":{\"wkid\":");
    int2buf(buf,srid);
    txt2buf(buf, "}}}");
    return 0;
}


static int encode_polygon(POLY *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf,int *id )
{
    int i,r,ndims;
    POINTARRAY *pa;

	if(id)
	{
		txt2buf(buf, "{\"GlobalId\":");
		int2buf(buf,*id);
		txt2buf(buf, ",\"geometry\":");
	}
	else
		txt2buf(buf, "{\"geometry\":");
    /*For now we are cheating some.
    It is possible in theory to have differnt number of dimmensions
    on different rings. But it is not possible in the PostGIS implementation
    of the twkb-spec. So here we use has_z and has_m from the first ring instead
    of checking all rings*/
    pa = g->rings[0];

    set_zm(pa, buf);

    ndims = 2 + pa->has_z+pa->has_m;


    txt2buf(buf, "{\"rings\":[");

    for (r=0; r<g->nrings; r++)
    {
	if(r!=0)
	 txt2buf(buf, ",");
        pa = g->rings[r];
        txt2buf(buf, "[");
        for (i=0; i<pa->npoints; i++)
        {
		if(i!=0)
			txt2buf(buf, ",");
            coords2buf(buf, pa->serialized_pointlist+i*ndims,ndims, pa->n_decimals);
        }

        txt2buf(buf, "]");
    }
    txt2buf(buf, "],\"spatialReference\":{\"wkid\":");
    int2buf(buf,srid);
    txt2buf(buf, "}}}");
    return 0;
}
static int encode_multi(COLLECTION *g,int srid,TEXT_BUF *buf,buffer_collection *mem_buf )
{     
    int r;
	int64_t *id;
    for (r=0; r<g->ngeoms; r++)
    {
	if(r!=0)
		txt2buf(buf, ",");	
	if(g->idlist)
		id = g->idlist+r;
	else
		id = NULL;
	encode_esrijson(g->geoms[r],srid,mem_buf,buf,(int*) id );
    }
    return 0;
}



