/* encode_geojson.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
 #include "twkb.h"


static int encode_point(POINT *g,TEXT_BUF *buf,buffer_collection *mem_buf);
static int encode_line(LINE *g,TEXT_BUF *buf,buffer_collection *mem_buf );
static int encode_polygon(POLY *g,TEXT_BUF *buf,buffer_collection *mem_buf );

static int encode_multi(COLLECTION *g,TEXT_BUF *buf,buffer_collection *mem_buf );




char* encode_geojson_start(GEOM *g,buffer_collection *mem_buf)
{
	TEXT_BUF buf;
	init_text_buf(&buf);
	return encode_geojson(g,mem_buf, &buf);
}

char* encode_geojson(GEOM *g,buffer_collection *mem_buf,TEXT_BUF *buf)
{
		switch (g->type)
		{
		case POINTTYPE:
				encode_point((POINT*) g,buf, mem_buf );
				break;
		case LINETYPE:
				encode_line((LINE*) g,buf, mem_buf );
				break;
		case POLYGONTYPE:
				encode_polygon((POLY*) g,buf, mem_buf );
				break;
		case MULTIPOINTTYPE:
		case MULTILINETYPE:
		case MULTIPOLYGONTYPE:
		case COLLECTIONTYPE:
				encode_multi((COLLECTION*) g,buf, mem_buf );
				break;

		}

		return buf->start_pos;

}

static int encode_point(POINT *g,TEXT_BUF *buf,buffer_collection *mem_buf )
{
		int ndims;
		POINTARRAY *pa = g->point;
	
		ndims = 2 + pa->has_z+pa->has_m;

		txt2buf(buf, "{\"type\":\"Point\",\"coordinates\":");
	
		coords2buf(buf, pa->serialized_pointlist,ndims, pa->n_decimals);
	
		txt2buf(buf, "}");
		return 0;
}



static int encode_line(LINE *g,TEXT_BUF *buf,buffer_collection *mem_buf )
{
		int i,ndims;
		POINTARRAY *pa = g->points;

		ndims = 2 + pa->has_z+pa->has_m;

		txt2buf(buf, "{\"type\":\"LineString\",\"coordinates\":[");
		for (i=0; i<pa->npoints; i++)
		{
			if(i!=0)
				txt2buf(buf, ",");
				coords2buf(buf, pa->serialized_pointlist+i*ndims,ndims, pa->n_decimals);
		}
		txt2buf(buf, "]}");
		return 0;
}


static int encode_polygon(POLY *g,TEXT_BUF *buf,buffer_collection *mem_buf)
{
		int i,r,ndims;
		POINTARRAY *pa;

		txt2buf(buf, "{\"type\":\"Polygon\",\"coordinates\":[");
		/*For now we are cheating some.
		It is possible in theory to have differnt number of dimmensions
		on different rings. But it is not possible in the PostGIS implementation
		of the twkb-spec. So here we use has_z and has_m from the first ring instead
		of checking all rings*/
		pa = g->rings[0];

		ndims = 2 + pa->has_z+pa->has_m;

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
		txt2buf(buf, "]}");
		return 0;
}
static int encode_multi(COLLECTION *g,TEXT_BUF *buf,buffer_collection *mem_buf )
{     
	int r;
	if(g->idlist && g->ngeoms>1)
	{
		txt2buf(buf, "{\"type\":\"FeatureCollection\",\"features\":[");
	}
	else if (g->ngeoms>1)
				txt2buf(buf, "{\"type\":\"GeometryCollection\",\"geometries\":[");	
			
		for (r=0; r<g->ngeoms; r++)
		{
			if(r!=0)
				txt2buf(buf, ",");	
			if(g->idlist)
			{
				txt2buf(buf, "{\"type\":\"Feature\",\"properties\":{\"id\":");
				int2buf(buf, g->idlist[r]);				
				txt2buf(buf, "},\"geometry\":");
			}
			encode_geojson(g->geoms[r],mem_buf,buf);
			if(g->idlist)
				txt2buf(buf, "}");	
		}
		if (g->ngeoms>1)
			txt2buf(buf, "}");	
		return 0;
}



