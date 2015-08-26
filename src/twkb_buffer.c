/* twkb_buffer.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
#include "twkb.h"

/**
Reads an unsigned varInt value
*/
static uint64_t
varint_u64_read(TWKB_BUF *tb)
{
	//~ printf("buffer to read = %d\n",tb->end_pos - tb->read_pos);
    uint64_t nVal = 0;
    int nShift = 0;
    uint8_t nByte;	
	
    /* Check so we don't read beyond the twkb 
	and if we do; try reading more from file 
	if there is one*/
    while( tb->read_pos < tb->end_pos  || !readmore(tb) )
    {
	nByte = *(tb->read_pos);
        /* Hibit is set, so this isn't the last byte */
        if (nByte & 0x80)
        {
            /* We get here when there is more to read in the input varInt */
            /* Here we take the least significant 7 bits of the read */
            /* byte and put it in the most significant place in the result variable. */
            nVal |= ((uint64_t)(nByte & 0x7f)) << nShift;
            /* move the "cursor" of the input buffer step (8 bits) */
            (tb->read_pos)++;
            /* move the cursor in the resulting variable (7 bits) */
            nShift += 7;
        }
        else
        {
            /* move the "cursor" one step */
            (tb->read_pos)++;
            return nVal | ((uint64_t)nByte << nShift);
        }
    }
    fprintf(stderr,"TWKB-buffer seems to be corrupt. We have read beyond the buffer last byte is %p\n",tb->read_pos);
    exit(EXIT_FAILURE);
}

/**
makes the unsigned value signed (if the input vas signed)
*/
int64_t
unzigzag64(uint64_t val)
{
    if ( val & 0x01 )
        return -1 * (int64_t)((val+1) >> 1);
    else
        return (int64_t)(val >> 1);
}



/**
Just a wrapper. Have no function now
It is the function exposed for reading unsigned varint
*/
uint64_t
buffer_read_uvarint(TWKB_BUF *tb)
{
    return varint_u64_read(tb);
}

/**
Just a wrapper. Have no function now
It is the function exposed for reading signed varint
*/
int64_t
buffer_read_svarint(TWKB_BUF *tb)
{
    uint64_t val,uval ;
    uval= varint_u64_read(tb);
    val =  unzigzag64(uval);

    return val;
}

/**
Read 1 byte from the twkb-buffer
*/
uint8_t
buffer_read_byte(TWKB_BUF *tb)
{
	if(tb->end_pos - tb->read_pos < 1)
	{
		readmore(tb);
	}
	uint8_t r= *(tb->read_pos++);

	return r;
	
}

/**
Jump n VarInts in the buffer.
A little more efficient than reading them properly
*/
void
buffer_jump_varint(TWKB_BUF *tb,int n)
{
    int i;
    for (i=0; i<n; i++)
    {
        do 
	{
		if(tb->read_pos == tb->end_pos)
			readmore(tb);
	}while(*(tb->read_pos++) & 0x80) ;
    }
    return;
}






