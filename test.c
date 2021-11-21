// // C program to illustrate
// // pipe system call in C
#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>
#define MSGSIZE 16
char* msg1 = "hello, world #1";
char* msg2 = "hello, world #2";
char* msg3 = "hello, world #3";

int main()
{
	char inbuf[MSGSIZE];
	int p[2], i;

	if (pipe(p) < 0)
		exit(1);

	/* continued */
	/* write pipe */

	write(p[1], msg1, MSGSIZE);
	write(p[1], msg2, MSGSIZE);
	write(p[1], msg3, MSGSIZE);

	for (i = 0; i < 3; i++) {
		/* read pipe */
		read(p[0], inbuf, MSGSIZE);
		printf("%s\n", inbuf);
	}
	return 0;
}
// #include <stdio.h>
// #include <string.h>


// int main( void )
// {
//     int array[] = {1, 6, 2, 4, 4};
//     char a[255] = {0};

//     /*do something*/
//     strcpy( a, "[" );
//     for( size_t i = 0; i < (sizeof(array)/sizeof(int)); i++ )
//     {
//         sprintf( &a[ strlen(a) ],  "%d, ", array[i] );
//     }

//     sprintf( &a[ strlen(a) ],  "%d", array[4] );
//     strcat( a, "]" );

//     printf("%s\n", a);
// }