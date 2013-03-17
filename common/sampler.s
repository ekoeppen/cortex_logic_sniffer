        .syntax unified

        .thumb_func
        .global sampler
sampler:
        @ parameters:
        @ r0: samples array
        @ r1: size of samples array
        @ r2: reset_control flag
        @ r3: GPIO block to sample

        @ r0: working register
        @ r1: GPIOC IDR (input for sampling)
        @ r2: reset control (used to stop sampling)
        @ r3: SYSTICK counter
        @ r4: start of sample buffer
        @ r5: current sample
        @ r6: current SYSTICK value
        @ r7: previous sample
        @ r8: end of sample buffer
        @ r9: index to sample buffer

        push {r1-r9, lr}
        mov r4, r0
        mov r9, r0
        mov r8, r1
        lsl r8, #2
        add r8, r0
        mov r1, r3
        ldr r3, =0xe000e010

        @ reset reset control flag
        mov r0, #0
        str r0, [r2]

        @ wait for trigger (first changed sample)
        ldr r7, [r1]
        and r7, #0x7f
trigger:
        ldr r0, [r2]
        cmp r0, #0
        bne sample_end
        ldr r5, [r1]
        and r5, #0x7f
        teq r5, r7
        beq trigger
        mov r7, r5

        @ set up SYSTICK and store initial sample
        mov r0, #0
        str r0, [r3, #8]
        ldr r0, =0x00ffffff
        str r0, [r3, #4]

        lsl r5, #24
        orr r5, r0
        str r5, [r9], #4

        @ start SYSTICK
        mov r0, #7
        str r0, [r3]
sample_loop:
        cmp r9, r8
        bge sample_end
        ldr r5, [r2]
        cmp r5, #0
        bne sample_end
        ldr r5, [r1]
        and r5, #0x7f
        teq r5, r7
        beq sample_loop
        mov r7, r5
        lsl r5, #24
        ldr r6, [r3, #8]
        orr r6, r5
        str r6, [r9], #4
        cmp r9, r8
        blt sample_loop
sample_end:
        @ stop SYSTICK
        mov r0, #0
        str r0, [r3]

        lsl r9, #1
        lsr r9, #1
        sub r0, r9, r4
        lsr r0, #2
        pop {r1-r9, pc}

        .thumb_func
        .global SysTick_Handler
SysTick_Handler:
        cmp r9, r8
        bge irq_end
        mov r0, #1
        ror r0, #1
        str r0, [r9], #4
irq_end:
        bx lr
