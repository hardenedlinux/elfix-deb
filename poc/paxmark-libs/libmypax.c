#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

void
doit() {
	size_t m = (size_t) mmap( NULL, 4096, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0 );
	if( m == (size_t) MAP_FAILED )
        	printf("%s\n", strerror(errno));
}
