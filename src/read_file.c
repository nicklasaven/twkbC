/* read_file.c
 *
 * Copyright (C) 2015 Nicklas Avén
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
 #include <stdio.h>
#include <errno.h>
#include <string.h>
#include "twkb.h"

#define READ_CHUNKS_SIZE 1000

static FILE *the_file=NULL;

static int
readfile(TWKB_BUF *tb, FILE *read_file)
{
	if(read_file)
	{
		register int c;
		register uint8_t *cs;
		register size_t n;	

		cs = tb->start_pos;
		n = tb->end_pos - tb->start_pos;	
		if(n>READ_CHUNKS_SIZE)
			n = READ_CHUNKS_SIZE;

		memset(cs, 0, n);

		cs = tb->start_pos;

		while (n-- > 0 && (c = getc(read_file)) != EOF)
		{
			*cs++ = c;
		}
		tb->read_pos = tb->start_pos;

		//~ /*If n > 0 that means we have reached end of file
		//~ and we can decrease the twkb-buffer to resulting size
		//~ we will not use it again anyway, without reinitializing it*/

		tb->end_pos = cs;
		
		if(tb->start_pos == tb->end_pos)
		{
			fprintf(stderr,"Error: %s","There is nothing more to read in the file\n and more is requested. Shouldn't get here\n");
			exit(EXIT_FAILURE);
		}
		return 0;
	}
	fprintf(stderr,"Error: %s","Cannot find file to read\n");
	exit(EXIT_FAILURE);
}

int
readmore(TWKB_BUF *tb)
{
	if(the_file)
		return readfile(tb,the_file);
	else
		return 1;
}


int
buffer_from_file(char *file_name,TWKB_BUF *tb)
{
	tb->start_pos = tb->read_pos = malloc(READ_CHUNKS_SIZE);
	tb->end_pos=tb->start_pos + READ_CHUNKS_SIZE;
	the_file = fopen(file_name, "rb");
	if (the_file)
	{		
		tb->handled_buffer = 1;
		 readfile(tb,the_file);	
		return 0;		
	}
	
	char *errmsg = strerror(errno);
	fprintf(stderr,"Error: %s: \"%s\"\n",errmsg, file_name);
	exit(EXIT_FAILURE);
}

int
close_file()
{
	if(the_file)
		fclose(the_file);
		the_file = NULL;
	return 0;
}