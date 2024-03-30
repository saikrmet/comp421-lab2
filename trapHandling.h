#include <comp421/hardware.h>
#include <comp421/yalnix.h>
#pragma once

void get_pid_handler(ExceptionInfo *exInfo);
void wait_handler(ExceptionInfo *exInfo);
void exit_handler(ExceptionInfo *exInfo, int wasProgramErr);
void exec_handler(ExceptionInfo *exInfo);
void fork_handler(ExceptionInfo *exInfo);
void delay_handler(ExceptionInfo *exInfo);
void tty_read_handler(ExceptionInfo *exInfo);
void tty_write_handler(ExceptionInfo *exInfo);
void trap_kernel_handler(ExceptionInfo *exInfo);
void trap_tty_receive_handler(ExceptionInfo *exInfo);
void trap_tty_transmit_handler(ExceptionInfo *exInfo);
void trap_clock_handler(ExceptionInfo *exInfo);
void trap_illegal_handler(ExceptionInfo *exInfo);
void trap_math_handler(ExceptionInfo *exInfo);
void trap_memory_handler(ExceptionInfo *exInfo);
