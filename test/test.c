#include "test.h"

int main(int argc, char* argv[])
{
	test_uart();
	test_dlink();
	test_mem();
	while(1);
	return 0;
}
