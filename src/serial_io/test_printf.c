#include "test_printf.h"

void
test_print(
)
{
	//serial_init(9600);
	char * s = "hurray";	
	printf("%s\r\n",s);	
	printf("this is a redirected printf with the number %i!\r\n",10);
	printf("and this is another number (with the last value missing) %i %i %d\r\n",10,20);
	printf("and this is another number %i %i %d\r\n",10,20,30);	
	//serial_close();
}
