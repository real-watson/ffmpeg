#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void generate_file_name(int index,char *filename)
{
	if (!index)
		return;
	memset(filename,0,strlen(filename));
	sprintf(filename,"jpg/no_%d.jpg",index);
}


