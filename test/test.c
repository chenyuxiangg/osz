#include "test.h"

int main(int argc, char* argv[])
{
	test_uart();
	test_dlink();
	test_mem();
	test_printf();
	while(1);
	return 0;
}
