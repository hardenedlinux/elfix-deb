#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

int
main()
{
	if( mmap(NULL, 4096, PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) != MAP_FAILED )
	{
		printf("mmap(): succeeded\n");
		return 0;
	}
	else
	{
		printf("mmap(): %s\n", strerror(errno));
		return 1;
	}
}
