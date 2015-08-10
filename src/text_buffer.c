/* text_buffer.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
 #include "twkb.h"

int 
init_text_buf(TEXT_BUF *buf)
{

    buf->start_pos = buf->write_pos = malloc(DEFAULT_TEXT_BUF);
    buf->end_pos = buf->start_pos + DEFAULT_TEXT_BUF ;
    return 0;
}

int 
increase_buf(TEXT_BUF *buf)
{
    int len = buf->end_pos - buf->start_pos;
    int used_size = buf->write_pos - buf->start_pos;
    buf->start_pos = realloc(buf->start_pos, 2*len);
    buf->write_pos = buf->start_pos + used_size ;
    buf->end_pos = buf->start_pos + 2*len ;
    return 0;
}

int num2buf(TEXT_BUF *buf, double num,int ndecimals)
{
    int avail_space = buf->end_pos-buf->write_pos;
    int len = snprintf((char*) buf->write_pos, avail_space, "%.*f",ndecimals, num);
    if(len<avail_space)
    {
        buf->write_pos+=len;
        return 0;
    }
    else
    {
        increase_buf(buf);
        return num2buf(buf,num,ndecimals);
    }
}
int coords2buf(TEXT_BUF *buf,double *num,int ndims, uint8_t *ndecimals)
{
    int avail_space = buf->end_pos-buf->write_pos;
    int len;
	
/*Ok, this is very ugly but works as a start*/
    if(ndims==2)
        len = snprintf((char*) buf->write_pos, avail_space, "[%.*f,%.*f]",ndecimals[0], num[0],ndecimals[1], num[1]);
    else if(ndims==3)
        len = snprintf((char*) buf->write_pos, avail_space, "[%.*f,%.*f,%.*f]",ndecimals[0], num[0],ndecimals[1], num[1],ndecimals[2], num[2]);
    else if(ndims==4)
        len = snprintf((char*) buf->write_pos, avail_space, "[%.*f,%.*f,%.*f,%.*f]" ,ndecimals[0], num[0],ndecimals[1], num[1],ndecimals[2], num[2],ndecimals[3], num[3]);
    else
        return 1;

    if(len<avail_space)
    {
        buf->write_pos+=len;
        return 0;
    }
    else
    {
        increase_buf(buf);
        return coords2buf(buf,num,ndims, ndecimals);
    }
}
int int2buf(TEXT_BUF *buf, int num)
{
    int avail_space = buf->end_pos-buf->write_pos;
    int len = snprintf((char*) buf->write_pos, avail_space, "%d", num);
    if(len<avail_space)
    {
        buf->write_pos+=len;
        return 0;
    }
    else
    {
        increase_buf(buf);
        return int2buf(buf,num);
    }
}

int txt2buf(TEXT_BUF *buf, char *txt)
{
    int avail_space = buf->end_pos-buf->write_pos;
    int len = snprintf((char*) buf->write_pos, avail_space, "%s", txt);
    if(len<avail_space)
    {
        buf->write_pos+=len;
        return 0;
    }
    else
    {
        increase_buf(buf);
        return txt2buf(buf,txt);
    }
}

