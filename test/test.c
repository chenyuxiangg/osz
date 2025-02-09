#include "test.h"

int main(int argc, char* argv[])
{
	test_uart();
	test_dlink();
	test_mem();
	test_printf();
	test_base_operator();
	test_interrupt();
	// test_mip();
	// test_mtime();
	// test_trap();
	// test_sched();
	// Faild, because qemi virt not surport f extension.
	// test_float_operator();
	while(1);
	return 0;
}
