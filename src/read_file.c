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
readfromfile(TWKB_BUF *tb, FILE *read_file, size_t len)
{
	if(read_file)
	{
		register int c;
		register uint8_t *cs;
		register size_t n;	

		
		/*Register where in the file our new buffer starts*/
		tb->BufOffsetFromBof=ftell(read_file);
		
		cs = tb->start_pos;
		n = tb->max_end_pos - tb->start_pos;	
		if(n<len)
		{
			tb->start_pos =tb->read_pos = realloc(tb->start_pos, len);
				
			tb->end_pos=tb->max_end_pos = tb->start_pos + len;
		}
		n = len;


		cs = tb->start_pos;
		memset(cs, 0, n);
		
		while (n-- > 0 && (c = getc(read_file)) != EOF)
		{
			*cs++ = c;
		}
		tb->read_pos = tb->start_pos;

		//~ /*If n > 0 that means we have reached end of file
		//~ and we can decrease the twkb-buffer to resulting size
		//~ we will not use it again anyway, without reinitializing it*/

		tb->end_pos = cs;
		
		//~ if(tb->start_pos == tb->end_pos)
		//~ {
			//~ fprintf(stderr,"Error: %s","There is nothing more to read in the file\n and more is requested. Shouldn't get here\n");
			//~ exit(EXIT_FAILURE);
		//~ }
		return 0;
	}
	fprintf(stderr,"Error: %s","Cannot find file to read\n");
	exit(EXIT_FAILURE);
}

long int getReadPos(TWKB_BUF *tb)
{
	return tb->BufOffsetFromBof + tb->read_pos - tb->start_pos;
}

int setReadPos(TWKB_BUF *tb,long int read_pos, size_t len)
{

	if(the_file)
	{

		
		if(read_pos>=tb->BufOffsetFromBof && (read_pos + len) <= (tb->BufOffsetFromBof + (tb->end_pos - tb->start_pos)))
		{
			/*Ok, we already have the buffer we need
			Just place the cursor and return*/
			tb->read_pos = tb->start_pos + (read_pos-tb->BufOffsetFromBof);
			return 0;
		}
		else
		{		
			if(fseek(the_file, read_pos, SEEK_SET))
				return 1;
		
			return readfromfile(tb,the_file, len);
		}
	}
	
	return 1;
	
}


int
readmore(TWKB_BUF *tb )
{
	if(the_file)
		return readfromfile(tb,the_file,READ_CHUNKS_SIZE);
	else
		return 1;
}


int
jumpandread(TWKB_BUF *tb,size_t jump, size_t len )
{
	size_t unread_in_buffer;
	if(the_file)
	{
		//Do we have what we need in the buffer already?
		unread_in_buffer = tb->end_pos - tb->read_pos;
		
		if(unread_in_buffer>=jump + len)
		{
			/*Ok, we already have the buffer we need
			Just place the cursor and return*/
			tb->read_pos = tb->read_pos + jump;
			return 0;
		}
		else
		{		
			if(fseek(the_file, jump - unread_in_buffer, SEEK_CUR))
			{
				
				printf("problems to jump %d\n",(int) (jump - unread_in_buffer));
				return 1;
			}
		
			return readfromfile(tb,the_file, len);
		}
	}
	
	printf("%s\n","Couldn't find file");
	return 1;
}






int
buffer_from_file(char *file_name,TWKB_BUF *tb)
{
	tb->start_pos = tb->read_pos = malloc(READ_CHUNKS_SIZE);
	tb->end_pos=tb->max_end_pos = tb->start_pos + READ_CHUNKS_SIZE;
	the_file = fopen(file_name, "rb");
	if (the_file)
	{		
		tb->handled_buffer = 1;
		readfromfile(tb,the_file,READ_CHUNKS_SIZE);
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