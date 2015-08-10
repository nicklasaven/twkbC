/* twkb.h
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 *
 ************************************************************************
*This is the main header file where all exposed fuctions is declared
*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/*Maximum number of dimmensions that a twkb geoemtry 
can hold according to the specification*/
#define TWKB_IN_MAXCOORDS 4

/*twkb types*/
#define	POINTTYPE			1
#define	LINETYPE			2
#define	POLYGONTYPE			3
#define	MULTIPOINTTYPE		4
#define	MULTILINETYPE		5
#define	MULTIPOLYGONTYPE	6
#define	COLLECTIONTYPE		7
#define DEFAULT_TEXT_BUF 1024

/**************************************************************
This program bas 3 types of memory buffers
1)	TWKB_BUF		Holding a buffer with 
					twkb-data while it is read
2)	TEXT_BUF			Holds a buffer handling text-based results
					This buffer reallocs when nessecery
3)	GEOM_BUF		buffers storing the internal geoemtries
					This bufer never reallocs. It just mallocs
					new chunks and uses, when needed
					This buffer is part of another struct called 
					buffer_collection
***************************************************************/

/***************************************************************
			INTERNAL GEOEMTRIES						*/


/*Point array*/
typedef struct
{ 
	int npoints; 
	int has_z;
	int has_m;
	uint8_t n_decimals[TWKB_IN_MAXCOORDS];
	double *serialized_pointlist;
}
POINTARRAY;


typedef struct
{
	uint8_t type;
	void *data;
}
GEOM;

/* POINTYPE */
typedef struct
{
	uint8_t type; /* POINTTYPE */
	POINTARRAY *point;
}
POINT; /* "light-weight point" */
 
/* LINETYPE */
typedef struct
{
	uint8_t type; /* LINETYPE */
	POINTARRAY *points;
}
LINE; /* "light-weight line" */

/* POLYGONTYPE */
typedef struct
{
	uint8_t type; /* POLYGONTYPE */
	int nrings;
	POINTARRAY **rings; /* list of rings (list of points) */
}
POLY; /* "light-weight polygon" */

/* COLLECTIONTYPE */
typedef struct
{
    uint8_t type;
	int64_t *idlist;
	int ngeoms;   /* how many geometries we are currently storing */
	GEOM **geoms;
}
COLLECTION;

			
/*a buffer handling memory for our internal geoemtries*/
typedef struct
{
        uint8_t *buffer_start;
        uint8_t *used_space_end;	
	uint8_t *buffer_end;
}
GEOM_BUF;

/*Keeping track of many memory buffers*/
typedef struct
{
        GEOM_BUF *buffers;
        int nbuffers;	
	int max_nbuffers;
}
buffer_collection;



void init_res_buf(buffer_collection *res_buf);
void* get_space(buffer_collection *res_buf, size_t needed_space);
void* create_new_buffer(buffer_collection *res_buf, size_t needed_space);
void destroy_buffer(buffer_collection *res_buf);



/***************************************************************
			DECODING TWKB						*/
/*Holds a buffer with the twkb-data during read*/
typedef struct
{	
	uint8_t handled_buffer; /*Indicates if this program is resposible for freeing*/
	uint8_t *start_pos;
	uint8_t *read_pos;
	uint8_t *end_pos;
}TWKB_BUF;


/* Used for passing the parse state between the parsing functions.*/
typedef struct
{
	TWKB_BUF *tb; /* Points to start of TWKB */
	buffer_collection *rb;
	uint8_t has_bbox;
	uint8_t has_size;
	uint8_t has_idlist;
	uint8_t has_z;
	uint8_t has_m;
	uint8_t is_empty;
	uint8_t type;
	
	/* Precision factors to convert ints to double */
	uint8_t n_decimals[TWKB_IN_MAXCOORDS];
	/* Precision factors to convert ints to double */
	double factors[TWKB_IN_MAXCOORDS];

	uint32_t ndims; /* Number of dimensions */
	
 /* An array to keep delta values from 4 dimensions */
	int64_t coords[TWKB_IN_MAXCOORDS];
	
} TWKB_PARSE_STATE;


/*functions for handling the twkb input buffer
and reading varint*/

char* twkb2esriJSON(uint8_t *buf,int buf_len,int srid);
int64_t unzigzag64(uint64_t val);
uint64_t buffer_read_uvarint(TWKB_BUF *tb);
int64_t buffer_read_svarint(TWKB_BUF *tb);
uint8_t buffer_read_byte(TWKB_BUF *tb);
void buffer_jump_varint(TWKB_BUF *tb,int n);

GEOM* decode_twkb_start(uint8_t *buf, size_t buf_len,buffer_collection *res_buf);
GEOM* decode_twkb(TWKB_PARSE_STATE *old_ts);


/*a type holding pointers to our parsing functions*/
typedef GEOM* (*parseFunctions_p)(TWKB_PARSE_STATE*);


/*For reading twkb from file*/
int buffer_from_file(char *file_name,TWKB_BUF *tb);
int readmore(TWKB_BUF *tb);
int close_file();

/***************************************************************
				OUTPUT TEXT FORMATS*/



/*a struct for handling the textbuffer of the result*/
typedef struct
{
	char *start_pos;
	char *write_pos;	
	char *end_pos;
}
TEXT_BUF;


int init_text_buf(TEXT_BUF *buf);
int txt2buf(TEXT_BUF *buf, char *txt);
void reset_buffer(buffer_collection *res_buf);

int increase_buf(TEXT_BUF *buf);
int num2buf(TEXT_BUF *buf, double num,int ndecimals);
int coords2buf(TEXT_BUF *buf,double *num,int ndims, uint8_t *ndecimals);
int int2buf(TEXT_BUF *buf, int num);
int txt2buf(TEXT_BUF *buf, char *txt);


char* encode_geojson_start(GEOM *g,buffer_collection *mem_buf);
char* encode_geojson(GEOM *g,buffer_collection *mem_buf,TEXT_BUF *buf);
char* encode_esrijson_start(GEOM *g,int srid,buffer_collection *mem_buf);
char* encode_esrijson(GEOM *g,int srid,buffer_collection *mem_buf,TEXT_BUF *buf,int *id );


/*Functions exposed to other programs*/
extern char*  twkb2geoJSON_fromFile(char *file_name);
extern char* twkb2geoJSON(uint8_t *buf,int buf_len);
extern char*  twkb2esriJSON_fromFile(char *file_name,int srid);
extern char* twkb2esriJSON(uint8_t *buf,int buf_len,int srid);

