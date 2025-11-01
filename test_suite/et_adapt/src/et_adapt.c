#include "et.h"
#include "hal_uart.h"
#include "schedule.h"

unsigned int et_group_start = (unsigned int)&_et_group_start;
unsigned int et_group_end = (unsigned int)&_et_group_end;

void ET_onInit(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    return;
}

void ET_onPrintChar(char const ch)
{
    hal_uart_putc(ch);
}

void ET_onExit(int err)
{
    (void)err;
    while(1);
}

void app_main(void *data)
{
    ET_onInit(0, NULL);

    print_str("\nET embedded test " ET_VERSION
              ", https://github.com/QuantumLeaps/ET\n");

    et_curr_group = ET_FIRST_GROUP;
    int group_cnt = ET_GROUP_COUNT;
    for (int i = 0; i < group_cnt; ++i) {
        print_str("---------------- group: ");
        print_str(et_curr_group->name);
        print_str(" -----------------\n");
        if (et_curr_group->op_func.setup) {
            et_curr_group->op_func.setup();
        }
        if (et_curr_group->op_func.entry) {
            et_curr_group->op_func.entry();
            test_end();
        } else {
            print_str("No test entry for group.\n");
            et_curr_group = (et_tc_group_t *)((unsigned int)et_curr_group + sizeof(et_tc_group_t));
            continue;
        }
        if (et_curr_group->op_func.clean) {
            et_curr_group->op_func.clean();
        }
        et_curr_group = (et_tc_group_t *)((unsigned int)et_curr_group + sizeof(et_tc_group_t));
    }
    et_curr_group = (et_tc_group_t*)0;

    print_summary(1U);

    ET_onExit(0); // success
}