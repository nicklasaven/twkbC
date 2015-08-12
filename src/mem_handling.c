/* mem_handling.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
#include <stdio.h>
#include "twkb.h"
#define DEFAULT_MEM_CHUNK 1000


void init_res_buf(buffer_collection *res_buf)
{
    res_buf->nbuffers=0;
    res_buf->max_nbuffers=0;
    res_buf->buffers=NULL;
    return;
}

void* get_space(buffer_collection *res_buf, size_t needed_space)
{
    int i=0;
    void *res;
    while(i<(res_buf->nbuffers))
    {
        GEOM_BUF mb = res_buf->buffers[i];
        if((mb.buffer_end-mb.used_space_end)>=needed_space)
        {
            res = (void*) mb.used_space_end;
            res_buf->buffers[i].used_space_end+=needed_space;
            return res;
        }
        i++;
    }

    /*Ok, we didn't have space enough anywhere, let's allocate some*/

    res = create_new_buffer(res_buf, needed_space);

    return res;//create_new_buffer(res_buf, needed_space);
}

void* create_new_buffer(buffer_collection *res_buf, size_t needed_space)
{
    uint8_t *new_buffer;
    size_t size_to_get;
    //~ int i;

    if (res_buf->nbuffers == res_buf->max_nbuffers)
    {
        if(res_buf->buffers)
        {
            res_buf->buffers = realloc(res_buf->buffers, 2*res_buf->max_nbuffers*sizeof(GEOM_BUF));
            res_buf->max_nbuffers*=2;
        }
        else
        {
            res_buf->buffers = malloc(2*sizeof(GEOM_BUF));
            res_buf->max_nbuffers=2;
        }
    }
    size_to_get = DEFAULT_MEM_CHUNK>needed_space?DEFAULT_MEM_CHUNK:needed_space;
    new_buffer = malloc(size_to_get);

    res_buf->buffers[res_buf->nbuffers].buffer_start = (uint8_t*) new_buffer;
    res_buf->buffers[res_buf->nbuffers].used_space_end = new_buffer + needed_space;
    res_buf->buffers[res_buf->nbuffers].buffer_end = (uint8_t*) (new_buffer + size_to_get);
    res_buf->nbuffers++;
    return new_buffer;
}

void reset_buffer(buffer_collection *res_buf)
{
    int i, len;
    len = res_buf->nbuffers;
    for (i=0; i<len; i++)
    {
        res_buf->buffers[i].used_space_end=res_buf->buffers[i].buffer_start;
    }
}

void destroy_buffer(buffer_collection *res_buf)
{
    int i, len;
    len = res_buf->nbuffers;
    for (i=0; i<len; i++)
    {
        free(res_buf->buffers[i].buffer_start);
    }
    free(res_buf->buffers);
    return;
}
