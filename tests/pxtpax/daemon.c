#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	pid_t p = fork();

	if(p)
		printf("%d\n", p);
	else
	{
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		for(;;) sleep(1);
	}

	exit(0) ;
}
