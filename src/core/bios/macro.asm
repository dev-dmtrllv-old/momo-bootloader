%macro BIOS_ROUTINE 1
[bits 16]
section .bios

global %1

%1:
%endmacro

%define REGISTERS bios_routine_registers
