[bits 32]

%include "interrupt.mac"

[EXTERN debug_kernelpanic]

isr_define_ep isr_timerTick, isr_timerTick_c

isr_dkp_ep isr_keyboard, 2
