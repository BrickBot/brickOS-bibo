/*
 * foss-rom.s
 * Replacement for LEGO MindStorm RCX ROM.
 * mostly address compatible
 * 
 * Copyright (c) 2003-2006 Jochen Hoenicke
 *   - Originally bundled with brickEmu
 *   - May be used under the GNU GPL v2 or (at your option) any later version
 *   - Original website:  https://jochen-hoenicke.de/rcx/brickemu.html
 * 
 * May 7, 2025 - Matthew Sheets
 *   - Update to be able to use binutils assembler instead of GCC
 */
.globl _start
_start:
	. = 0
irq_vectors:	
	.word	reset_trampoline	/*  0 */
	.word	0
	.word	0
	.word	nmi_trampoline
	.word	irq0_trampoline
	.word	irq1_trampoline		/*  5 */
	.word	irq2_trampoline
	.word	0
	.word	0
	.word	0
	.word	0			/* 10 */
	.word	0
	.word	icia_trampoline
	.word	icib_trampoline
	.word	icic_trampoline
	.word	icid_trampoline		/* 15 */
	.word	ocia_trampoline
	.word	ocib_trampoline
	.word	fovi_trampoline
	.word	cmi0a_trampoline
	.word	cmi0b_trampoline	/* 20 */
	.word	ovi0_trampoline
	.word	cmi1a_trampoline
	.word	cmi1b_trampoline
	.word	ovi1_trampoline
	.word   0			/* 25 */
	.word   0
	.word	eri_trampoline
	.word	rxi_trampoline
	.word	txi_trampoline
	.word	tei_trampoline		/* 30 */
	.word	0
	.word	0
	.word	0
	.word	0
	.word	adi_trampoline		/* 35 */
	.word	wovf_trampoline

	. = 0x4a
softirq_vectors:
	.word	umulhi3
	.word	umodhi3
	.word	udivmodhi3
	.word	modhi3
	.word	divhi3
	.word	mulsi3
	.word	udivmodsi3
	.word	divmodsi3


	. = 0x130
umulhi3:
	/* calculate r6 := r6 * r5,  clobber r5 */
	push	r0
	mov.w	r6,  r0
	mulxu.b	r5h, r0      /* r0 = r5h * r6l */
	mov.b	r6h, r0h
	mulxu.b	r5l, r6      /* r6 = r5l * r6l */
	add.b	r0l, r6h     /* r6 += r0 << 8 */
	mulxu.b	r0h, r5      /* r5 = r5l * r6h */
	add.b	r5l, r6h     /* r6 += r5 << 8 */
	pop	r0
	rts
	. = 0x14a
umodhi3:
	/* calculate r6 := r6 % r5 */
	bsr	udivmodhi3
	mov.w	r5, r6
	rts

	. = 0x156
divhi3:
	/* calculate r6 := r6 / r5 (signed) */
	mov.w	r6, r6
	bmi	1f
	mov.w	r5, r5		/* r6 positive */
	bpl	udivmodhi3

	not.b	r5h
	not.b	r5l
	adds	#1, r5
2:
	bsr	udivmodhi3
	not.b	r6h
	not.b	r6l
	adds	#1, r6
	rts
1:				/* r6 negative */
	not.b	r6h
	not.b	r6l
	adds	#1, r6
	mov.w	r5, r5
	bpl	2b
	not.b	r5h
	not.b	r5l
	adds	#1, r5
	bra	udivmodhi3

	. = 0x188
modhi3:
	/* calculate r6 := r6 % r5 (signed) */
	mov.w	r5, r5
	bpl	1f
	not.b	r5h
	not.b	r5l
	adds	#1, r5
1:	
	mov.w	r6, r6            /* r5 is now positive */
	bpl	umodhi3
	not.b	r6h               /* r6 negative */
	not.b	r6l
	adds	#1, r6
	bsr	udivmodhi3
	not.b	r5h
	not.b	r5l
	adds	#1, r5
	mov.w	r5, r6
	rts

	. = 0x1be
udivmodhi3:	
	/* r6, r5 := r6 / r5 , r6 % r5  (unsigned) */
	push	r0
	mov.b	#0, r0h
	mov.b	r6h, r0l	  /* r0h:r0l:r6l = dividend */
	mov.b	r5h, r5h	  /* Test for r5h == 0 */
	bne	1f
	divxu.b	r5l, r0           /* r0l,r0h = r6 >> 8 (/,%) r5l */
	mov.b	r0l, r6h          /* r6h = high part of dividend */
	mov.b	r6l, r0l
	divxu.b	r5l, r0
	mov.b	r0l, r6l          /* r6l = low part of dividend */
	mov.b	r0h, r5l          /* r5l = modulo */
	pop	r0
	rts
1:
	mov.b	r6h, r0l          
	                          /* result is also built into r6l */
	mov.b   #8, r6h
2:	rotxl	r6l               /* store bit of result and rotate dividend */
	rotxl	r0l
	rotxl	r0h
	cmp.w	r0, r5            /* test if we can subtract.
	bcs	1f
	sub.w	r0, r5
	dec	r6l
	bne	2b
	rotxl	r6l               /* put last bit into r6 */
	not.b	r6l
	mov.b	#0, r6h           /* clear high byte of result */
	mov	r0, r5            /* store modulo into r5 */
	pop	r0
	rts
	
	
	. = 0x1fe
mulsi3:
	/*  r5h r5l r6h r6l *  r3h r3l r4h r4l */

	/*     r3h*r6l r3l*r6l r4h*r6l r4l*r6l */
	/*     r3l*r6h r4h*r6h r4l*r6h         */
	/*     r4h*r5l r4l*r5l                 */
	/*     r4l*r5h                         */

	/* r5r6 = r5r6 * r3r4 */
	push	r0
	push	r1
	mov.w	r6, r0
	mulxu.b	r3h, r0         /* r0 = r6l*r3h */
	mov.w	r3, r1
	mulxu.b	r6h, r1         /* r1 = r6h*r3l */
	add.b	r1l, r0l
	mulxu.b r6l, r3         /* r3 = r6l*r3l */
	add.b	r1l, r3h
	add.b   r0l, r3h        /* r3 = r6*r3 */

	mov.w	r4, r0
	mulxu.b	r5h, r0         /* r0 = r4l*r5h */
	mov.w	r5, r1
	mulxu.b	r4h, r1         /* r1 = r4h*r5l */
	add.b	r1l, r0l
	mulxu.b r4l, r5         /* r5 = r4l*r5l */
	add.b	r1l, r5h
	add.b   r0l, r5h        /* r5 = r4*r5 */
	add.w	r3, r5          /* r5 = r4*r5 + r6*r3 */

	mov.b	r4h, r1l
	mulxu.b	r6h, r1         /* r1 = r4h*r6h */
	add.w	r1, r5

	mov.w	r6, r0
	mulxu.b	r4h, r0		/* r0 = r4h*r6l */
	mov.w	r4, r1
	mulxu.b	r6h, r1		/* r1 = r4l*r6h */
	mulxu.b r4l, r6		/* r6 = r4l*r6l */
	add.b	r1l, r6h
	addx	r1h, r5l
	addx	#0, r5h
	add.b	r0l, r6h
	addx	r0h, r5l
	addx	#0, r5h
	pop	r1
	pop	r0
	rts

	. = 0x26e
divmodsi3:
	/* divide r5:r6 / r3:r4   (signed)
	 * quotient  in r5:r6
	 * remainder in r3:r4
	 */
	mov.w	r5, r5          /* check sign of dividend */
	bpl	2f

	not.b	r6l
	not.b	r6h
	not.b	r5l
	not.b	r5h
	add.b	#1, r6l
	addx    #0, r6h
	addx    #0, r5l
	addx    #0, r5h

	mov.w	r3, r3		/* check sign of divisor */
	bpl	1f

	/*  -x / -y */

	not.b	r4l
	not.b	r4h
	not.b	r3l
	not.b	r3h
	add.b	#1, r4l
	addx    #0, r4h
	addx    #0, r3l
	addx    #0, r3h
	bsr	udivmodsi3
	bra	3f
1:	bsr	udivmodsi3

	not.b	r6l              /* negate result */
	not.b	r6h
	not.b	r5l
	not.b	r5h
	add.b	#1, r6l
	addx    #0, r6h
	addx    #0, r5l
	addx    #0, r5h

3:
	not.b	r4l            /* negate modulo */
	not.b	r4h
	not.b	r3l
	not.b	r3h
	add.b	#1, r4l
	addx    #0, r4h
	addx    #0, r3l
	addx    #0, r3h
	rts
	


2:	/* dividend positive */
	mov.w	r3, r3		/* check sign of divisor */
	bpl	udivmodsi3      /* divisor positive, too */

	not.b	r4l
	not.b	r4h
	not.b	r3l
	not.b	r3h
	add.b	#1, r4l
	addx    #0, r4h
	addx    #0, r3l
	addx    #0, r3h

	bsr	udivmodsi3

	not.b	r6l              /* negate result */
	not.b	r6h
	not.b	r5l
	not.b	r5h
	add.b	#1, r6l
	addx    #0, r6h
	addx    #0, r5l
	addx    #0, r5h
	rts
	

	. = 0x306
udivmodsi3:
	/* divide r5:r6 / r3:r4  
	 * quotient  in r5:r6
	 * remainder in r3:r4
	 */	 	
	mov.w	r3, r3
	bne	6f
	/* Simple case: r3 is zero */
	cmp.b   #0, r4h
	bne	5f

	/* Most simple case: only r4l; we can clobber r3 */
	mov.b	r5h, r3l
	bne	1f
	/* r5h is zero, skip it */
	mov.b   r5l, r3l
	bne	2f
	/* r5 is zero skip it */
	mov.b   r6h, r3l
	bne	3f
	bra	4f
1:	divxu.b	r4l, r3
	mov.b	r3l, r5h
	mov.b	r5l, r3l
2:	divxu.b	r4l, r3
	mov.b	r3l, r5l
	mov.b	r6h, r3l
3:	divxu.b	r4l, r3
	mov.b	r3h, r6h
4:	divxu.b r4l, r6
	mov.b	r6h, r4l
	mov.b   r3l, r6h
	sub.w	r3, r3
	rts

5:      /* r3 = 0, r4h != 0 */
	
	mov.b	r5h, r3l          /* dividend is in r3:r5l:r6 */
	mov.b   #24, r5h

4:	rotxl	r6l               /* store bit of result and rotate dividend*/
	rotxl   r6h
	rotxl   r5l
	rotxl	r3l
	rotxl	r3h
	cmp.w	r4, r3            /* test if we can subtract. */
	bcs	1f
	sub.w	r4, r3
1:	dec	r5h
	bne	4b
	rotxl	r6l               /* put last bit into r5:r6 */
	rotxl	r6h
	rotxl	r5l
	not.b	r6l
	not.b	r6h
	not.b	r5l
	mov.b	#0, r5h           /* clear high byte of result */
	mov	r3, r4            /* store modulo into r4 */
	sub.w	r3, r3
	rts

6:	/* r3 != 0, r4 != 0 */

	push    r0
	push    r1
	sub.w	r0, r0            /* dividend is in r0:r5:r6 */
	mov.b	#16, r1l
4:	rotxl   r6l
	rotxl   r6h
	rotxl   r5l
	rotxl   r5h
	rotxl   r0l
	rotxl   r0h
	sub.w	r4, r5
	subx	r3l, r0l
	subx	r3h, r0h
	bcc	1f
	add.w	r4, r5
	addx	r3l, r0l
	addx	r3h, r0h
1:	dec	r1l
	bne	4b
	rotxl	r6l
	rotxl   r6h
	not.b	r6h
	not.b	r6l
	mov.w	r5, r4
	mov.w	r0, r3
	sub.w	r5, r5
	pop	r1
	pop	r0
	rts


	. = 0x3ae
reset_trampoline:	
	mov.w	#0xff7e, r7
	jsr	@init_ports
	bsr	init_vectors
	jsr	@main
	jmp	halt

	. = 0x3ca
init_vectors:
	mov.w	#rom_main_loop_state, r0
	mov.w	#0xf000, r1
	bsr	memclr
	mov.w	#dummy, r0
	mov.w	#nmi_vector, r1
	mov.w	#wovf_vector+2, r2
1:
	mov.w	r0, @-r2
	cmp.w	r1, r2
	bne	1b
	rts	

	. = 0x42a - 6
1:	mov.b	@r0+,r3l
	mov.b	r3l, @r2
	adds	#1, r2
	. = 0x42a
	/* used by brickOS library */
memcopy:
	/*  r0 = start, r1 = end, r2 = dest */
	cmp.w	r0, r1
	bne	1b
	rts

	. = 0x436
memclr:
	/*  r0 = start, r1 = end */
	mov.b  #0, r2l
2:
	cmp.w  r0, r1
	beq    1f
	mov.b   r2l, @-r1
	bra    2b
1:	rts


	. = 0x442
init_ports:
	bset	#2, @_PORT5_DDR:8
	bclr	#2, @_PORT5:8
	mov.b	#0xff, r6l
	mov.b	r6l, @_PORT1_DDR:8
	mov.b	r6l, @_PORT2_DDR:8
	mov.b	r6l, @PORT1_DDR_SHADOW:16
	mov.b	r6l, @PORT2_DDR_SHADOW:16
	mov.b	#2, r6l
	mov.b	r6l, @0x8001
	mov.b	#1, r6l
	mov.b	r6l, @0x8000
	rts

	. = 0x468
halt:
	bra	halt

	. = 0x46a
dummy:	rts

	. = 0x46c
nmi_trampoline:
	push	r6
	mov.w	@nmi_vector:16, r6
	jsr	@r6
	pop	r6
	rte
irq0_trampoline:	
	push	r6
	mov.w	@irq0_vector:16, r6
	jsr	@r6
	pop	r6
	rte
irq1_trampoline:
	push	r6
	mov.w	@irq1_vector:16, r6
	jsr	@r6
	pop	r6
	rte
irq2_trampoline:
	push	r6
	mov.w	@irq2_vector:16, r6
	jsr	@r6
	pop	r6
	rte
icia_trampoline:
	push	r6
	mov.w	@icia_vector:16, r6
	jsr	@r6
	pop	r6
	rte
icib_trampoline:	
	push	r6
	mov.w	@icib_vector:16, r6
	jsr	@r6
	pop	r6
	rte
icic_trampoline:	
	push	r6
	mov.w	@icic_vector:16, r6
	jsr	@r6
	pop	r6
	rte
icid_trampoline:	
	push	r6
	mov.w	@icid_vector:16, r6
	jsr	@r6
	pop	r6
	rte
ocia_trampoline:	
	push	r6
	mov.w	@ocia_vector:16, r6
	jsr	@r6
	pop	r6
	rte
ocib_trampoline:	
	push	r6
	mov.w	@ocib_vector:16, r6
	jsr	@r6
	pop	r6
	rte
fovi_trampoline:
	push	r6
	mov.w	@fovi_vector:16, r6
	jsr	@r6
	pop	r6
	rte
cmi0a_trampoline:
	push	r6
	mov.w	@cmi0a_vector:16, r6
	jsr	@r6
	pop	r6
	rte
cmi0b_trampoline:
	push	r6
	mov.w	@cmi0b_vector:16, r6
	jsr	@r6
	pop	r6
	rte
ovi0_trampoline:
	push	r6
	mov.w	@ovi0_vector:16, r6
	jsr	@r6
	pop	r6
	rte
cmi1a_trampoline:	
	push	r6
	mov.w	@cmi1a_vector:16, r6
	jsr	@r6
	pop	r6
	rte
cmi1b_trampoline:	
	push	r6
	mov.w	@cmi1b_vector:16, r6
	jsr	@r6
	pop	r6
	rte
ovi1_trampoline:	
	push	r6
	mov.w	@ovi1_vector:16, r6
	jsr	@r6
	pop	r6
	rte
eri_trampoline:	
	push	r6
	mov.w	@eri_vector:16, r6
	jsr	@r6
	pop	r6
	rte
rxi_trampoline:	
	push	r6
	mov.w	@rxi_vector:16, r6
	jsr	@r6
	pop	r6
	rte
txi_trampoline:	
	push	r6
	mov.w	@txi_vector:16, r6
	jsr	@r6
	pop	r6
	rte
tei_trampoline:	
	push	r6
	mov.w	@tei_vector:16, r6
	jsr	@r6
	pop	r6
	rte
adi_trampoline:	
	push	r6
	mov.w	@adi_vector:16, r6
	jsr	@r6
	pop	r6
	rte
wovf_trampoline:	
	push	r6
	mov.w	@wovf_vector:16, r6
	jsr	@r6
	pop	r6
	rte

	. = 0x580
.globl _main
_main:
main:
	mov.b	@_T0_CNT:8, r6l
	beq	1f
	mov.b	#0, r6l
	mov.b	r6l, @rom_main_loop_state:16
	bra	2f
1:	mov.b	#8, r6l
	mov.b	r6l, @rom_main_loop_state:16
	mov.b	#1, r6l
	mov.b	r6l, @rom_ready_to_sleep:16
	mov.w	#15, r6
	mov.w	r6, @power_off_timer:16
	mov.w	#rom_handler_struct, r6
	push	r6
	add.b	#0x10, r6l
	jsr	@init_timer
	adds	#0x2, r7
	mov.w	#rom_main_loop_state, r6
	jsr	@init_handlers
	mov.w	#rom_main_loop_state, r6
	jsr	@init_builtin_programs
2:
main_loop:
	mov.b	@rom_main_loop_state:16, r6l
	cmp.b	#0x13, r6l
	bne	1f

	mov.w	@0xee82, r6
	bne	main_loop
	jsr	@shutdown_handlers
	jsr	@shutdown_timer
	mov.w	@firmware_entry, r6
	jsr	@r6
	bra	main_loop

1:	cmp.b	#8, r6l
	bne	1f
	mov.w	#rom_main_loop_state, r6
	jsr	@update
	mov.b   @sound_playing, r6l      /* get sound playing flag */
	cmp.b	#0, r6l
	bne	main_loop
	mov.b	@rom_ready_to_sleep, r6l
	beq	main_loop
	jsr	@shutdown_handlers
	jsr	@shutdown_timer
	jsr	@rom_power_off
	mov.b	#0,r6l
	mov.b	r6l, @rom_main_loop_state:16

1:	cmp.b	#0, r6l
	bne	1f
	mov.w	#0xf, r6
	mov.w	r6, @power_off_timer
	mov.w	#rom_handler_struct, r6
	push	r6
	add.b	#0x10, r6l
	jsr	@init_timer
	adds	#2, r6
	mov.w	#0xee5e, r6
	bsr	init_handlers
	mov.w	#0xee5e, r6
	jsr	@init_builtin_programs
	mov.b	#0xd, r6l
	mov.b	r6l, @rom_main_loop_state

1:	mov.w	#rom_main_loop_state, r6
	jsr	@update
	mov.b	@0xee64:16, r6l
	btst	#7, r6l		           /* New sensor values? */
	beq	1f
	mov.w	#rom_main_loop_state, r6
	jsr	@run_builtin_programs
1:	jmp     main_loop

	. = 0x688
/* needed by fast download rom */
init_handlers:
	jsr	@init_sensors
	jsr	@init_motors
	jsr	@init_buttons
	jsr	@init_power

	mov.b	#1, r6l
	push	r6
	push	r6
	mov.w	#rom_timer_struct, r6
	push	r6
	mov.w	#rom_handler_struct, r6
	jsr	@init_serial
	adds	#2,r7
	adds	#2,r7
	adds	#2,r7

	jsr	@set_port_6_bit_3_output
	mov	#0x3c, r6l
	mov.b	r6l, @0xee82
	jsr	@lcd_clear
	jsr	@lcd_refresh
	mov.w	#50, r6
	mov.b	r6h, @rom_update_state
	mov.b	r6l, @rom_key_state
	rts

	. = 0x70c
update:
	push	r5
	mov.b	@rom_update_state, r6l
	bne	1f
	
	/* rom_update_state == 0: START */
	/* original ROM checks memory here but that code is broken anyway. */
	
	mov.w	#man_stand, r6    /* show standing man */
	jsr	@lcd_show
	mov.b	#7, r6l    /* state = 7 (beep twice) */
	mov.b	r6l, @rom_update_state
	bra	end_switch_1

1:	dec	r6l
	bne	1f

	push	r3
	push	r4
	sub.w	r3, r3
	mov.w	r3, r4
	/* rom_update_state == 1: CHECK FIRMWARE */
	/* We do not check for "Do you byte when I knock"  magic */
	mov.w	#0x8000, r5   /* start address */
	mov.w	#0xcc00, r6   /* end address */
2:	mov.b	@r5+, r4l
	add.w	r4, r3
	cmp.w	r5, r6
	bne	2b
	mov.w	@high_firmware_checksum, r4
	cmp.w	r4, r3
	bne	3f
	mov.w	@high_firmware_checksum_inv, r4
	adds	#1, r4
	add.w	r3, r4
	bne	3f

	mov.b	#3, r6l      /* send unlock ok */
	bra	4f
3:	mov.w   #4, r6       /* low buzz */
	push	r6
	mov.w	#0x4004, r6
	jsr	play_system_sound
	adds	#2, r7
	mov.b	#0xff, r6l
	mov.b	r6l, @last_received_opcode
	mov.b	#2, r6l      /* wait for message */

4:	pop	r4
	pop	r3
	mov.b	r6l, @rom_update_state
	bra	end_switch_1

1:	dec	r6l
	bne	1f

	/* rom_update_state == 2: WAIT FOR MESSAGE */
	mov.b	@serial_valid_incoming_data, r5l
	beq	end_switch_1
	mov.w	#15, r6
	mov.w	r6, @power_off_timer:16
	mov.b	#4, r6l
	mov.b	r6l, @rom_update_state
end_switch_1:
	jmp	end_switch
	
1:	dec	r6l
	bne	1f

	/* rom_update_state == 3: SEND UNLOCK OK */
	mov.w	#0x19, r6
	push	r6
	mov.w	#Just_a_bit, r6
	push	r6
	mov.b	@rom_receive_buffer+0, r6l
	push	r6
	mov.w	#0x1776, r6      /* long packet */
	jsr	@send_packet
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	cmp.b	#0x4c, r6l
	beq	end_switch_1

	mov.w	#30, r6
	mov.w	r6, @firmware_holdoff_timer
	mov.w   @high_firmware_entry, r6
	mov.w   r6, @firmware_entry
	mov.b	#0x13, r6l
	mov.b	r6l, @rom_main_loop_state
	mov.b	#8, r6l
	mov.b	r6l, @rom_update_state
	jmp	end_switch
	
1:	dec	r6l
	beq	2f
	jmp	1f

2:
	/* rom_update_state == 4: PROCESS MESSAGE */
	subs	#2, r7
	push	r7
	mov.w	#10, r6
	push	r6
	mov.w	#rom_receive_buffer, r6
	jsr	@receive_data
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	mov.b	@rom_receive_buffer, r6l
	mov.b	@last_received_opcode, r6h
	cmp.b	r6l, r6h
	beq	send_reply

	mov.b	r6l, @last_received_opcode
	and.b	#0xf7, r6l
	cmp.b	#0x10, r6l
	bne	2f

	/* opcode = 0x10 (alive) */
send_simple_reply:
	mov.b	#1, r6l
send_inverse_opcode:
	mov.b	r6l, @rom_send_packet_length
	mov.b	@rom_receive_buffer, r6l
	not.b	r6l
	mov.b	r6l, @rom_send_buffer
send_reply:
	mov.b	#5, r6l
	mov.b	r6l, @rom_update_state
	jmp	end_switch

2:	cmp.b	#0x15, r6l
	bne	2f

	/* opcode = 0x15 (get_version) */
	mov.w	#3, r6
	mov.b	r6h, @rom_send_buffer+1
	mov.b	r6l, @rom_send_buffer+2
	mov.b	#1, r6l
	mov.b	r6h, @rom_send_buffer+3
	mov.b	r6l, @rom_send_buffer+4
	mov.b	r6h, @rom_send_buffer+5
	mov.b	r6h, @rom_send_buffer+6
	mov.b	r6h, @rom_send_buffer+7
	mov.b	r6h, @rom_send_buffer+8
	mov.b	#9, r6l
	bra	send_inverse_opcode

2:	cmp.b	#0x65, r6l
	beq	send_simple_reply
	/* opcode = 0x65 (delete firmware (nothing to do)) */

2:	cmp.b	#0x75, r6l
	bne	2f
	/* opcode = 0x75 (start firmware download) */
	mov.b	@rom_receive_buffer+1, r6l
	mov.b	@rom_receive_buffer+2, r6h
	mov.w	r6, @high_firmware_entry	 /* firmware enty */
	mov.b	@rom_receive_buffer+3, r6l
	mov.b	@rom_receive_buffer+4, r6h
	mov.w	r6, @high_firmware_checksum	 /* firmware checksum */
	not.b	r6l
	not.b	r6h
	mov.w	r6, @high_firmware_checksum_inv	 /* inverted firmware checksum */

	mov.w	#0xffff, r6
	mov.w	r6, @serial_transfer_sequence_number
	mov.w	#0x8000, r6
	mov.w	r6, @serial_incoming_dataptr
	push	r0
	push	r1
	mov.w	r6, r0
	mov.w	#0xcc00, r1
	jsr	@memclr
	pop	r1
	pop	r0
	mov.w	#1, r6
	mov.w	r6, @rom_transfer_data_index
	mov.b	#0, r6l
send_two_byte_reply:
	mov.b   r6l, @rom_send_buffer+1
	mov.b	#2, r6l
	jmp	send_inverse_opcode

2:	cmp.b	#0x45, r6l
	bne	2f
	/* opcode = 0x45 (transfer firmware) */

	mov.b	@rom_receive_buffer+1, r6l
	bne	3f
	mov.b	@rom_receive_buffer+2, r6h
	beq	transfer_done
3:
	mov.b	@rom_receive_buffer+2, r6h	
	mov.w	@rom_transfer_data_index, r5
	cmp.w	r6, r5
	bne	wait_for_message_1
	adds	#1, r6
	mov.w	r6, @rom_transfer_data_index
	mov.b	@rom_receive_buffer+5, r6l
	mov.b	@rom_receive_buffer+6, r6h
	sub.b	r6h, r6l
	beq	send_two_byte_reply   /* send 0: okay */
	mov.b	#3, r6l
	bra	send_two_byte_reply   /* send 3: block chksum fail */
transfer_done:
	sub.w	r6, r6
	mov.w	r6, @serial_incoming_dataptr

	mov.b	@rom_receive_buffer+5, r6l
	mov.b	@rom_receive_buffer+6, r6h
	sub.b	r6h, r6l
	bne	3f

	/* Do we want to checksum here? */
	
	mov.w	#5, r6		      /* fast tones up */
	push	r6
	mov.w	#0x4004, r6
	jsr	@play_system_sound
	adds	#2, r7
	mov.b	#0, r6l               /* send 0: okay */
	bra	send_two_byte_reply
	
3:	mov.b	#3, r6l
	bra	send_two_byte_reply   /* send 3: block chksum fail */
	
2:	cmp.b	#0xa5, r6l
	bne	wait_for_message
	/* opcode = 0xa5 (unlock firmware) */
	/* Check for LEGO(R) signature? */
	mov.b	#1, r6l
	mov.b	r6l, @rom_update_state
	bra	end_switch

1:	dec	r6l
	bne	1f

	/* rom_update_state == 5: send reply */
	mov.b	@rom_send_packet_length, r6l
	mov.b	#0, r6h
	push	r6
	mov.w	#rom_send_buffer, r6
	push	r6
	sub.w	r6,r6
	push	r6
	mov.w	#0x1775, r6      /* short packet */
	jsr	@send_packet
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	cmp.b	#0x4c, r6l
wait_for_message_1:
	bne	wait_for_message
	bra	end_switch

1:	dec	r6l
	bne	1f

	/* rom_update_state == 6: send reply and check firmware */
	mov.b	@rom_send_packet_length, r6l
	mov.b	#0, r6h
	push	r6
	mov.w	#rom_send_buffer, r6
	push	r6
	sub.w	r6,r6
	push	r6
	mov.w	#0x1775, r6      /* short packet */
	jsr	@send_packet
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	cmp.b	#0x4c, r6l
	beq	end_switch
	
	mov.b	#1, r6l
	mov.b	r6l, @rom_update_state
	bra	end_switch

1:	dec	r6l
	bne	end_switch

	/* rom_update_state == 7: beep twice */
	mov.w	#1, r6		      /* beep twice */
	push	r6
	mov.w	#0x4004, r6
	jsr	@play_system_sound
	adds	#2, r7
wait_for_message:
	mov.b	#2, r6l
	mov.b	r6l, @rom_update_state

end_switch:
	jsr	@lcd_refresh
	mov.w	@serial_incoming_dataptr, r5
	mov.w	@last_serial_dataptr, r6
	cmp.w	r5,r6
	beq	1f
	mov.w	r5, @last_serial_dataptr
	mov.w	#0x301c, r6
	jsr	@lcd_show		/* show short range light */
	mov.w	#100, r6
	mov.w	r6, @lcd_clear_timer
	add.b	#0x80, r5h
	bcc     1f
	sub.b	r6h, r6h
	mov.b	r5h, r6l
	mov.b   #10, r5h
	divxu.b r5h, r6
	mov.b	r6h, r5h
	mov.b	#10, r6h
	divxu.b r6h, r5
	mov.b	r6l, r5h
	mov.w	#0x3002, r6
	push	r6
	push	r5
	subs	#1, r6
	jsr	@lcd_number
	adds	#2, r7
	adds	#2, r7
1:	mov.w	@lcd_clear_timer, r6
	bne	1f
	mov.w	#100, r6
	mov.w	r6, @lcd_clear_timer
	jsr	@lcd_clear
1:
	mov.b	@rom_key_state,r6l
	cmp.b	#50, r6l		/* wait for first release */
	bne	1f
	mov.b	@_PORT4:8, r6l
	and.b	#2, r6l		       /* get on/off key state */
	bne	2f
	/* button still pressed */
	mov.b	r6l, @rom_onoff_key_debounce
	bra	3f
2:	mov.b	@rom_onoff_key_debounce, r6l
	inc	r6l
	mov.b	r6l, @rom_onoff_key_debounce
	cmp.b	#3, r6l
	bls     3f
	mov.b	#51, r6l
	mov.b	r6l, @rom_key_state
	bra	3f
1:	
	cmp.b	#51, r6l		/* wait for on/off press */
	bne	1f
	mov.b	@_PORT4:8, r6l
	and.b	#2, r6l		       /* get on/off key state */
	bne	1f
	/* on/off pressed, go to sleep */
	mov.b	r6l, @rom_onoff_key_debounce
	mov.b   r6l, @rom_ready_to_sleep  /* not yet ready */
	
	sub.w	r6,r6			/* blip*/
	push	r6
	mov.w	#0x4004, r6
	jsr	@play_system_sound
	adds	#2, r7
	
	mov.b   #8, r6l
	mov.b	r6l, @rom_main_loop_state /* go to sleep */
	mov.b	#52, r6l
	mov.b	r6l, @rom_key_state
	bra	3f
1:	
	cmp.b	#52, r6l		/* wait for on/off press */
	bne	3f
	
	mov.b	@_PORT4:8, r6l
	and.b	#2, r6l		       /* get on/off key state */
	bne	2f
	/* on/off still pressed */
	mov.b	r6l, @rom_onoff_key_debounce
	bra	3f
2:	mov.b	@rom_onoff_key_debounce, r6l
	inc	r6l
	mov.b	r6l, @rom_onoff_key_debounce
	cmp.b	#15, r6l
	bls     3f
	mov.b	#1, r6l
	mov.b   r6l, @rom_ready_to_sleep  /* ready to sleep */
	mov.b	#53, r6l
	mov.b	r6l, @rom_key_state
3:

	mov.w	@power_off_timer, r6
	bne	1f

	mov.w	#100, r6
	mov.w	r6, @power_off_timer

	sub.w	r6,r6			/* blip*/
	push	r6
	mov.w	#0x4004, r6
	jsr	@play_system_sound
	adds	#2, r7
	
	mov.b   #1, r6l
	mov.b   r6l, @rom_ready_to_sleep  /* ready to sleep */
	mov.b   #8, r6l
	mov.b	r6l, @rom_main_loop_state /* go to sleep */

1:	pop	r5
	sub.w	r6,r6
	rts

	

	. = 0xd18
shutdown_handlers:
	jsr	@shutdown_sensors
	jsr	@shutdown_motors
	jsr	@lcd_clear
	jsr	@lcd_refresh
	jsr	@reset_buttons_and_irq
	jsr	@shutdown_serial
	jsr	@set_port_6_bit_3_input
	rts

init_builtin_programs:
	mov.b	#1, r6l
	mov.b	r6l, @rom_program_number
	rts
	
run_builtin_programs:
	mov.w	#man_stand, r6    /* show standing man */
	jsr	@lcd_show
	sub.w	r6,r6
	push	r6
	mov.b	@rom_program_number, r6l
	push	r6
	mov.w	#digit, r6
	jsr	@lcd_number
	adds	#2,r7
	adds	#2,r7
	rts
		
	. = 0x148a
return_zero:
	sub.w	r6,r6
	rts

	. = 0x1498
init_sensors:
	mov.b	@PORT6_DDR_SHADOW, r6l
	or.b	#7, r6l
	mov.b	r6l, @PORT6_DDR_SHADOW
	mov.b	r6l, @_PORT6_DDR:8
	sub.b	r6l, r6l
	rts
	
	sensorstruct_type  = 0
	sensorstruct_mode  = 1
	sensorstruct_raw   = 2
	sensorstruct_value = 4
	sensorstruct_bool  = 6

	. = 0x14c0
read_sensors:
	/* params: 0x1000+INDEX, sensorstruct* */
	add.b	#(256 - 0x10), r6h
	bne	1f
	cmp.b	#2, r6l
	bls	2f
1:	sub.b	r6l,r6l
	rts
2:	/* calculate sensor address (0=AD_C, 1=AD_B, 2=AD_A)*/
	push	r2
	mov.w	r6, r2		/* r2 = sensor index */
	not	r6l
	add.b	r6l, r6l
	add.b	#_AD_D, r6l
	mov.b	#0xff, r6h
	mov.w	@r6, r6
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	/* Now r6 = raw value */
	
	push	r5
	push	r4
	push	r3
	mov.w	@(10, r7), r5

	sub.w	r3, r3
	mov.b	@(sensorstruct_mode,r5),r3l
	mov.w   r3, r4
	and.b	#0x1f,r3l
	bne	1f
	mov.w   #0x1cc, r3
	
1:	cmp.w	r3, r6
	bhs	1f
	mov.b	#1, r3l
	bra	bool_end
	
1:	push	r1
	mov	r6, r1
	add.b	#252, r1h
	add	r3, r1
	bhs	1f
	mov.b	#0, r3l
	bra	2f

1:	mov.w	@(sensorstruct_raw, r5), r1
	sub.w	r6, r1
	bcs	1f
	cmp.w	r3, r1
	bcs	3f
	mov.b	#1, r3l
	bra	2f
	
1:	add.w	r3, r1
	bcs	3f
	mov.b	#0, r3l
	bra	2f
	
3:	mov.b	@(sensorstruct_bool,r5), r3l
2:	pop	r1

bool_end:	
	mov.w	r6, @(sensorstruct_raw, r5)
	and.b	#0xe0, r4l
	shlr	r4l
	shlr	r4l
	shlr	r4l
	shlr	r4l
	mov.w	@(1f,r4),r4
	jmp	@r4
1:	.word	getsensor_raw
	.word	getsensor_bool
	.word	getsensor_edgecount
	.word	getsensor_pulsecount
	.word	getsensor_percentage
	.word	getsensor_celsius
	.word	getsensor_fahrenheit
	.word	getsensor_angle

getsensor_edgecount:
getsensor_pulsecount:
	mov.b	@(edge_inited_flag, r2), r3h
	bne	1f
	/* if edge_inited_flag[index] == 0 */
	mov.b	#1, r3h
	mov.b	r3h, @(edge_inited_flag, r2)
	mov.b	r3h, @(edge_debounce_cnt, r2)
	mov.b	r3l, @(sensorstruct_bool, r5)
	bra	getsensor_end_switch_0
	/* else */
1:	mov.b	@(edge_debounce_cnt, r2), r3h
	dec	r3h
	bra	2f
	mov.b	#1, r3h
	mov.b	@(sensorstruct_bool, r5), r4h
	cmp.b	r4h, r3l
	beq	2f
	mov.b	r3l, @(sensorstruct_bool, r5)
	mov.b	@(sensorstruct_mode,r5), r4l
	and.b	#0xe0, r4l
	cmp.b	#0x60, r4l /* pulse count */
	bne	3f
	mov.b	@(edge_counter, r2), r4h
	xor.b	#1, r4h
	mov.b	r4h, @(edge_counter, r2)
	beq	4f
3:	mov.w	@(sensorstruct_value, r5), r4
	adds	#1, r4
	mov.w	r4, @(sensorstruct_value,r5)
4:	mov.b	#100, r3h
2:	mov.b	r3h, @(edge_debounce_cnt, r2)
	bra	getsensor_end_switch

getsensor_celsius:	
getsensor_fahrenheit:	
getsensor_percentage:
	mov.b	r3l, @(sensorstruct_bool, r5)
	mov.w	#0x3ff, r3
	sub.w	r6, r3
	mov.b	#0x19, r5l
	mov.b	r6h, r4h
	mulxu.b	r5l, r6      /* r6 = r5l * r6l */
	mulxu.b	r4h, r5      /* r5 = r5l * r6h */
	add.b	r5l, r6h     /* r6 += r5 << 8 */
	mov.b	#0x9c, r5l
	divxu.b r5l, r6
	cmp.b	#100, r6l
	bls	1f
	mov.b	#100, r6l
	mov.b	#0, r6h
	mov.w	r6, @(sensorstruct_value, r5)
	bra	getsensor_end_switch

getsensor_bool:
	mov.b	#0, r3h
	mov.w	r3, r6   /* value = bool */
	/* fall through */
getsensor_raw:	
	mov.b	r3l, @(sensorstruct_bool, r5)
	mov.w	r6, @(sensorstruct_value, r5)
getsensor_end_switch_0:	
	bra	getsensor_end_switch
	
getsensor_angle:
	add.b	#(256-0xbf), r6l
	subx	#0, r6h
	blo	1f
	mov.b	#0, r6h
1:	mov.b	@(angle_state, r2), r6l
	xor.b	r6h, r6l
	beq	getsensor_end_switch
	mov.b	r6h, @(angle_state, r2)
	cmp.b	#3, r6l
	beq	getsensor_end_switch	/* two bits changed */
	mov.w	@(sensorstruct_value, r5), r4
	xor.b	r6h, r6l
	shlr	r6l
	xor.b	r6h, r6l
	and.b	#1, r6l
	beq	1f
	/* increase value */
	adds	#1, r4
	bra	2f
1:	subs	#1, r4
2:	mov.w	r4, @(sensorstruct_value, r5)

getsensor_end_switch:
	pop	r3
	pop	r4
	pop	r5
	pop	r2
	sub.w	r6, r6
	rts
	

	
	. = 0x1946
set_sensor_active:
	add.b	#(256 - 0x10), r6h
	bne	1f
	cmp.b	#2, r6l
	bhi	1f
	mov.b	@(sensor_xlat:16, r6), r6l
	push	r6
	mov.w	#0x700a, r6
	jsr	@control_output
	adds	#2, r7
1:
	sub.b	r6l,r6l
	rts
	
	. = 0x19c4
set_sensor_passive:
	add.b	#(256 - 0x10), r6h
	bne	1f
	cmp.b	#2, r6l
	bhi	1f
	mov.b	@(sensor_xlat:16, r6), r6l
	push	r6
	mov.w	#0x700a, r6
	jsr	@clear_sensor_or_timer_data
	adds	#2, r7
1:
	sub.b	r6l,r6l
	rts

sensor_xlat:
	.byte	4, 2, 1
	
	. = 0x1a22
shutdown_sensors:
	mov.b	@PORT6_DDR_SHADOW, r6l
	and.b	#~7, r6l
	mov.b	r6l, @PORT6_DDR_SHADOW
	mov.b	r6l, @_PORT6_DDR:8
	sub.b	r6l, r6l
	rts
	. = 0x1a4a
init_motors:
	sub.b	r6l, r6l
	rts
	. = 0x1a4e
control_motors:
	
	. = 0x1ab4
shutdown_motors:
	sub.b	r6l, r6l
	rts

irq0_handler:
	rts
	. = 0x1aba
init_buttons:
	mov.w	#irq0_handler, r6
	mov.w	r6, @irq0_vector
	mov.b	@PORT7_SHADOW:16, r6l
	and.b	#0x3f, r6l
	mov.b	r6l, @PORT7_SHADOW:16
	mov.b	r6l, @_PORT7:8
	mov.b	@PORT4_DDR_SHADOW:16, r6l
	bclr	#2, r6l
	mov.b	r6l, @PORT4_DDR_SHADOW:16
	mov.b	r6l, @_PORT4_DDR:8
	bset	#0, @_ISCR:8
	bset	#0, @_IER:8		/* irq 0 edge sense and enabled */
	mov.b	@PORT6_DDR_SHADOW:16, r6l
	or.b	#0x60, r6l
	mov.b	r6l, @PORT6_DDR_SHADOW:16
	mov.b	r6l, @_PORT6_DDR:8
	bset	#5, @_PORT6:8
	bset	#6, @_PORT6:8

	/* clear LCD area */	
	push	r5
	sub.b	r5l, r5l
	sub.w	r6, r6
1:	mov.b	r5l, @(i2c_area, r6)
	inc	r6l
	cmp.b	#0xf, r6l
	blo	1b
	mov.b	r5l, @dot_counter
	mov.b	r5l, @circle_counter
	pop	r5
	sub.b	r6l, r6l
	rts
	

	. = 0x1b32
play_button_sound:
	add.b	#(256 - 0x30), r6h
	bne	1f
	add.b	#(256 - 0x1e), r6l
	bne	1f
	mov.w	#7, r6
	push	r6
	mov.b	#0, r6l
	push	r6
	mov.b	#1, r6l
	push	r6
	mov.w	#0x700b, r6
	jsr	@control_output
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
1:	sub.b	r6l, r6l
	rts
	

.include "foss-rom-lcd.s"

printf:
	nop
	rts

/*	. = 0x2880*/
debug_rxi_handler:
	push	r6
	push	r5
	
	mov.b	#0, r5h
	mov.b	@serial_expect_complement, r5l
	push	r5

	mov.b	@serial_last_received_byte, r5l
	push	r5

	mov.b	r6l, r5l
	push	r5
	
	mov.b	@serial_receive_state, r5l
	push	r5

	mov.w	#rxi_str, r5
	push	r5

	bsr	printf

	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	
	pop	r5
	pop	r6
	rts

rxi_str:
	.string "Receive state %02x, byte %02x, last %02x, complements: %d"

	. = 0x294a
irq1_handler:
	/* Called on ON-OFF Button press */
	push	r6
        /* enable on/off button input in port4 */
	mov.b	@(PORT4_DDR_SHADOW:16), r6l
	bclr	#1, r6l
	mov.b	r6l, @(PORT4_DDR_SHADOW:16)
	mov.b	r6l, @(_PORT4_DDR:8)
	/* disable IRQ1 */
	bclr	#1, @(_IER:8)
	/* clear pending IRQs by clearing edge detect flag */
	bclr	#1, @(_ISCR:8)
	pop	r6
	rts

	. = 0x2964
init_power:
	mov	#irq1_handler, r6
	mov	r6, @irq1_vector
	bsr	irq1_handler      /* clear IRQ1, enable PORT 4 */

	/* External RAM power save is controlled by PORT5 bit 2 (SCK pin) */
	mov	@PORT5_DDR_SHADOW:16,r6l
	bset	#2, r6l
	mov.b	r6l, @PORT5_DDR_SHADOW:16
	mov.b	r6l, @_PORT5_DDR:8      /* Set DDR to output */
	bclr	#2, @_PORT5:8           /* clear bit to disable power save */

	/* Enable software standby mode (sleep instruction) and set settling
         * time to 131072 states (8.192ms)
         */
	mov	@_SYSCR:8,r6l
	or.b	#0xc0, r6l
	mov.b	r6l, @_SYSCR:8      /* Set DDR to output */
	rts

	. = 0x299a
play_system_sound:
	add.b	#(256 - 0x40), r6h
	bne	1f
	add.b	#(256 - 0x03), r6l
	bne	2f
	mov.w	#0x700d, r6
	bra	3f

2:	dec	r6l
	bne	1f
	mov.w	#0x700b, r6
3:
	push	r5
	sub.w	r5, r5
	push	r5
	mov.w	@(6, r7), r5
	push	r5
	mov.w	#1, r5
	push	r5
	jsr	@control_output
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	pop	r5

1:	sub.b	r6l, r6l
	rts

	. = 0x29f2
get_various:
	push	r5
	mov.w	@(4, r7), r5
	add.b	#(256 - 0x40), r6h
	bne	3f
	cmp.b	#0, r6l
	bne	2f
	
	mov.b	@_PORT4:8, r6l
	and.b	#2, r6l
	bra	1f
	
2:	dec.b	r6l
	bne	3f

	mov.w	@(0xff00+_AD_D), r6
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	shlr	r6h
	rotxr	r6l
	bra	1f

3:	sub.w	r6, r6
1:	mov.w	r6, @r5
	pop	r5
	sub.b	r6l,r6l
	rts

	. = 0x2a62
rom_power_off:
	mov.b	@PORT4_DDR_SHADOW, r6l
	bclr	#1, r6l
	mov.b	r6l, @PORT4_DDR_SHADOW
	mov.b	r6l, @_PORT4_DDR:8	/* enable IRQ1 input */
	bset	#1, @_ISCR:8		/* IRQ1 to edge sensed */
	bset	#1, @_IER:8		/* enable IRQ1 */
	bset	#2, @_PORT5:8		/* put ram to power save mode */
	sleep
	bclr	#2, @_PORT5:8
	sub.b	r6l, r6l
	rts
	
tei_handler:
	/* shutdown transmit and irqs, clear status flags */
	mov.b	@_S_CR:8, r6l
	and.b	#0x5b, r6l	/* disable TXI, TEI and clear TE flag */
	mov.b	r6l, @_S_CR:8
	mov.b	@_S_SR:8, r6l
	and.b	#0x7b, r6l	/* clear TDRE and TEND flags */
	mov.b	r6l, @_S_SR:8
	mov.b	#0x4f, r6l
	mov.b	r6l, @serial_xmit_flag
	rts
	
txi_handler:
	mov.w	@serial_out_remaining, r6
	bne	1f
	/* no byte remaining, disable TXI */
	mov.b	@_S_CR:8, r6l
	bclr	#7, @_S_CR:8	/* disable TXI */
	rts

1:
	subs	#1, r6
	mov.w	r6, @serial_out_remaining
	mov.w	@serial_out_packettype, r6
	add.b	#(256-0x17), r6h
	bne	1f
	add.b	#(256-0x75), r6l
	bne	2f
	
	/* packettype = 0x1775:  short message */
	mov.b	@serial_send_index, r6l
	mov.b	@(serial_outgoing_packet,r6), r6h
	inc	r6l
	mov.b	r6l, @serial_send_index
	mov.b	r6h, @_S_TDR:8

1:	bclr	#7, @_S_SR:8	/* clear transmit data register empty */
	rts

2:	dec	r6l
	bne	1b

	/* packettype = 0x1776:  large message */
	mov.b	@serial_out_headerbyte, r6l
	beq	1f

	/* We need to send header */
	dec	r6l
	mov.b	r6l, @serial_out_headerbyte
	bne	2f
	mov.b	@serial_out_chksum, r6l
	mov.b	r6l, r6h
	not.b	r6h
	mov.b	r6h, @serial_out_chksum
	bra	3f
2:	dec	r6l
	bne	2f
tx_send_inv_chksum:
	mov.b	@serial_out_chksum, r6l
	not.b	r6l
	bra	3f
2:	mov.b	@(serial_out_packet_header-1,r6), r6l
3:	mov.b	r6l, @_S_TDR:8
	bclr	#7, @_S_SR:8	/* clear transmit data register empty */
	rts

	/* long packet content */
1:
	mov.b	@serial_use_complement, r6l,
	beq	1f

	/* use complements */
	mov.w	@serial_out_remaining, r6
	beq	tx_send_inv_chksum	/* last byte: inverted chksum */
	add.b	#0xff, r6l
	addx.b	#0xff, r6h
	bne	2f
tx_send_chksum:
	mov.b	@serial_out_chksum, r6l
	bra	3b
	
	/* send data bytes */
2:	push	r5
	mov.w	@serial_out_packetptr, r5
	mov.b	@serial_send_index, r6h
	add.b	r6h, r5l
	addx.b	#0, r5h
	mov.b	@r5, r6l
	mov.b	@serial_out_complement, r5l
	xor.b	#1, r5l
	mov.b	r5l, @serial_out_complement
	bne	tx_send_byte_update_chksum
	
	not.b	r6l		/* invert byte */
	inc	r6h		/* increment send index */
	mov.b	r6h, @serial_send_index
	bra	tx_send_byte

	/* no complements */
1:
	mov.w	@serial_out_remaining, r6
	beq	tx_send_chksum	/* last byte: chksum */

	/* send byte from data */
	push	r5
	mov.w	@serial_out_packetptr, r5
	mov.b	@serial_send_index, r6h
	add.b	r6h, r5l
	addx.b	#0, r5h
	mov.b	@r5, r6l
	inc	r6h
	mov.b	r6h, @serial_send_index
tx_send_byte_update_chksum:
	mov.b	@serial_out_chksum, r5l
	add.b	r6l, r5l
	mov.b	r5l, @serial_out_chksum
tx_send_byte:
	mov.b	r6l, @_S_TDR:8
	pop	r5
	bclr	#7, @_S_SR:8	/* clear transmit data register empty */
	rts

serial_out_packet_header:
	.byte	0x00, 0xff, 0x55
	.align 1
			
rxi_handler:
	mov.b	@serial_xmit_flag, r6l
	add.b	#(256 - 0x4f), r6l
	beq	1f
	
	/* xmit_flag == 0x4f:  we are transmitting */
	mov.b	r6l, @serial_unpack_index
	mov.b	r6l, @serial_receive_index
	bclr	#6, @_S_SR:8		/* clear RDRF */
	rts
	
1:	push	r5
	mov.w	@serial_timer_struct, r5
	mov.w	@r5, r6
	bne	1f

	/* serial receive reset counter == 0: reset state */
	mov.b	r6l, @serial_expect_complement
	mov.b	#2, r6l
	mov.b	r6l, @serial_receive_state
	mov.b	#0xff, r6l
	mov.b	r6l, @serial_last_received_byte
1:
	/* restart reset counter */
	mov.b	#30, r6l
	mov.w	r6, @r5
	mov.b	@_S_RDR:8, r6l

	/* do we expect a complement? */
	mov.b	@serial_use_complement, r5l
	beq	1f
	mov.b	@serial_expect_complement, r5l
	bne	2f

	/* Now check state and branch */
1:	mov.b	@serial_receive_state, r5l
	sub.b	r5h, r5h
	add.b	#(256-2), r5l
	cmp.b	#6-2, r5l
	bhi	rxi_reset_state
	add	r5, r5
	mov.w	@(1f, r5), r5
	jmp	@r5

	/* use and expect complement */

2:	sub.b	r5l, r5l
	mov.b	r5l, @serial_expect_complement

	mov.b	@serial_last_received_byte, r6h
	not.b	r6l
	cmp.b	r6l, r6h
	bne	rxi_reset_state		/* invalid complement: reset state */
	
	/* received valid complement */
	mov.b	@serial_receive_state, r6l
	cmp.b	#99, r6l
	bne	rxi_cleanup
	
	/* this was the last byte of a valid packet */
	mov.b	#1, r6l
	mov.b	r6l, @serial_valid_incoming_data
	mov.w	@serial_handler_run_flag_addr, r6
	bset	#7, @r6

rxi_reset_state:
	mov.b	#2, r6l
	mov.b	r6l, @serial_receive_state
	mov.b	#0xff, r6l
	mov.b	r6l, @serial_last_received_byte
rxi_cleanup:
	pop	r5
	bclr	#6, @_S_SR:8		/* clear RDRF */
	rts
	
1:	.word	rxi_state2
	.word	rxi_state3
	.word	rxi_state4
	.word	rxi_state5
	.word	rxi_state6

rxi_state2:
	mov.b	@serial_has_packet_header, r5l
	beq	1f

	/* check packet header */
	cmp.b	#0, r6l
	beq	rxi_cleanup
	cmp.b	#0xff, r6l
	beq	rxi_cleanup
	cmp.b	#0x55, r6l
	beq	rxi_cleanup

	/* not a packet header -> it is probably an opcode */
1:	mov.b	#0, r5l
	mov.b	r5l, @serial_unpack_index
	mov.b	#1, r5l
	mov.b	r5l, @serial_receive_index
	mov.b	r6l, @serial_incoming_packet

	/* opcode & 7 is number of parameters */
	mov.b	r6l, r6h
	and.b	#7, r6h
	cmp.b	#6, r6h
	blo	1f
	add.b	#(256-6), r6h
1:	mov.b	r6h, @serial_receive_length_remaining
	mov.b	r6l, @serial_last_received_byte
	mov.b	r6l, @serial_receive_chksum

	mov.b	#1, r5l
	mov.b	r5l, @serial_expect_complement

	cmp.b	#0, r6h
	bne	2f

	/* opcode has no parameters --> state = 4 */
	mov.b	#4, r5l
	mov.b	r5l, @serial_receive_state
	bra	3f
2:
	and.b	#~8, r6l
	cmp.b	#0x45, r6l
	bne	2f
	
	/* opcode = 0x45 transfer data --> state = 6 */
	mov.b	#6, r5l
	mov.b	r5l, @serial_receive_state
	mov.b	#0, r5l
	mov.b	r5l, @serial_receive_data_chksum
	mov.b	#4, r5l
	mov.b	r5l, @serial_receive_length_remaining
	bra	3f

	/* normal opcode with data --> state = 3*/
2:	mov.b	#3, r5l
	mov.b	r5l, @serial_receive_state

3:	pop	r5
	bclr	#6, @_S_SR:8		/* clear RDRF */
	rts

rxi_state3:
	/* receive message operands */
	mov.b	@serial_receive_index, r5l
	sub.b	r5h, r5h
	mov.b	r6l, @(serial_incoming_packet, r5)
	inc	r5l
	cmp.b	#10, r5l
	bne	1f
	mov.b	#0, r5l
1:	mov.b	r5l, @serial_receive_index
	mov.b	r6l, @serial_last_received_byte
	mov.b	@serial_receive_chksum, r5l
	add.b	r6l, r5l
	mov.b	r5l, @serial_receive_chksum

	mov.b	#1, r5l
	mov.b	r5l, @serial_expect_complement

	/* decrease and check receive length */
	mov.b	@serial_receive_length_remaining, r5l
	dec	r5l
	mov.b	r5l, @serial_receive_length_remaining
	bne	6f

	/* no bytes remaining --> state = 4 */
	mov.b	#4, r5l
	mov.b	r5l, @serial_receive_state
	
6:	pop	r5
	bclr	#6, @_S_SR:8		/* clear RDRF */
	rts
	
rxi_state4:
	/* receive message checksum */
	mov.b	@serial_receive_index, r5l
	sub.b	r5h, r5h
	mov.b	r6l, @(serial_incoming_packet, r5)
	inc	r5l
	cmp.b	#10, r5l
	bne	1f
	mov.b	#0, r5l
	
1:	mov.b	r5l, @serial_receive_index
	mov.b	r6l, @serial_last_received_byte

	mov.b	#1, r5l
	mov.b	r5l, @serial_expect_complement

	mov.b	@serial_receive_chksum, r5l
	cmp.b	r5l, r6l
	bne	1f

	mov.b	@serial_use_complement, r5l
	beq	2f

	mov.b	#99, r6l		/* mark end of packet */
	mov.b	r6l, @serial_receive_state
	pop	r5
	bclr	#6, @_S_SR:8		/* clear RDRF */
	rts
	

	/* valid packet */
2:	mov.b	#1, r6l
	mov.b	r6l, @serial_valid_incoming_data
	mov.w	@serial_handler_run_flag_addr, r6
	bset	#7, @r6

1:	mov.b	#2, r6l
	mov.b	r6l, @serial_receive_state
	mov.b	#0xff, r6l
	mov.b	r6l, @serial_last_received_byte

	pop	r5
	bclr	#6, @_S_SR:8		/* clear RDRF */
	rts

	
rxi_state5:
	/* receive ROM download message checksum */
	mov.b	@serial_receive_index, r5l
	sub.b	r5h, r5h
	mov.b	r6l, @(serial_incoming_packet, r5)
	inc	r5l
	cmp.b	#10, r5l
	bne	1f
	mov.b	#0, r5l
1:	mov.b	r5l, @serial_receive_index
	mov.b	r6l, @serial_last_received_byte

	mov.b	@serial_receive_chksum, r5l
	add.b	r6l, r5l
	mov.b	r5l, @serial_receive_chksum

	mov.b	@serial_receive_data_chksum, r6l
	mov.b	@serial_receive_index, r5l
	sub.b	r5h, r5h
	mov.b	r6l, @(serial_incoming_packet, r5)
	inc	r5l
	cmp.b	#10, r5l
	bne	1f
	mov.b	#0, r5l
1:	mov.b	r5l, @serial_receive_index

	mov.b	#1, r5l
	mov.b	r5l, @serial_expect_complement
	mov.b	#4, r5l
	mov.b	r5l, @serial_receive_state
	pop	r5
	bclr	#6, @_S_SR:8		/* clear RDRF */
	rts
	
rxi_state6:
	/* receive ROM download message body */
	mov.b	r6l, @serial_last_received_byte
	mov.b	@serial_receive_chksum, r5l
	add.b	r6l, r5l
	mov.b	r5l, @serial_receive_chksum

	/* check if we are still in header */	
	mov.w	@serial_receive_length_remaining, r5
	beq	1f

	/* store byte into incoming packet area */
	mov.b	@serial_receive_index, r5l
	sub.b	r5h, r5h
	mov.b	r6l, @(serial_incoming_packet, r5)
	inc	r5l
	cmp.b	#10, r5l
	bne	2f
	mov.b	#0, r5l
2:	mov.b	r5l, @serial_receive_index

	
	mov.b	@serial_receive_length_remaining, r5l
	dec	r5l
	mov.b	r5l, @serial_receive_length_remaining
	bne	6f
	
	/* last byte of header.  evaluate it. */	
	mov.b	@(serial_incoming_packet+2), r5h
	mov.b	@(serial_incoming_packet+1), r5l
	mov.w	@serial_transfer_sequence_number, r6
	cmp.w	r5, r6
	bne	3f
	mov.w	@serial_saved_dataptr, r5
	mov.w	r5, @serial_incoming_dataptr
	bra	4f
3:	mov.w	r5, @serial_transfer_sequence_number
	mov.w	@serial_incoming_dataptr, r5
	mov.w	r5, @serial_saved_dataptr
4:
	mov.b	@(serial_incoming_packet+4), r5h
	mov.b	@(serial_incoming_packet+3), r5l
	mov.w	r5, @serial_receive_data_length
	bra	6f


	/* Not in header */
1:	mov.w	@serial_incoming_dataptr, r5
	beq	2f
	mov.b	r6l, @r5
	adds	#1, r5
	mov.w	r5, @serial_incoming_dataptr

  	mov.b	@serial_receive_data_chksum, r5l
	add.b	r6l, r5l
	mov.b	r5l, @serial_receive_data_chksum

2:	mov.w	@serial_receive_data_length, r5
	subs	#1, r5
	mov.w	r5, @serial_receive_data_length
	bne	6f

	/* finished data block */
	mov.b	#5, r5l
	mov.b	r5l, @serial_receive_state
	
6:
	mov.b	#1, r5l
	mov.b	r5l, @serial_expect_complement

	pop	r5
	bclr	#6, @_S_SR:8		/* clear RDRF */
	rts
	
	
			
eri_handler:
	/* receive error */
	mov.b	@serial_receive_state, r6l
	add.b	#(256-5), r6l
	cmp.b	#1, r6l
	bls	1f

	/* receive state is not 5 or 6. start over */
	mov.b	#2, r6l
	mov.b	r6l, @serial_receive_state
1:
	/* reset serial receive reset counter */
	mov.w	@serial_timer_struct, r6
	push	r5
	mov.w	#30, r5
	mov.w	r5, @r6
	pop	r5
	
	mov.b	@_S_SR:8, r6l
	and.b	#~0x38, r6l	/* clear errors */
	mov.b	r6l, @_S_SR:8
	rts
	
	. = 0x30d0
init_serial:
	push	r0

	mov.b	@(7,r7), r0l
	mov.b	r0l, @serial_has_packet_header
	mov.b	@(9,r7), r0l
	mov.b	r0l, @serial_use_complement
	sub.w	r0, r0
	mov.b	r0l, @serial_expect_complement
	mov.w	r6, @serial_handler_run_flag_addr
	mov.b	r0l, @serial_valid_incoming_data
	subs	#1, r0
	mov.b	r0l, @serial_last_received_byte
	mov.w	r0, @serial_transfer_sequence_number
	mov.b	#2, r6l
	mov.b	r6l, @serial_receive_state
	mov.w	@(4,r7), r6
	mov.w	r6, @serial_timer_struct
	sub.b	r6l,r6l
	mov.b	r6l, @_S_CR:8
	mov.b	#0x30, r6l	/* parenb, parodd, baud rate 2400*/
	mov.b	r6l, @_S_MR:8
	mov.b	#0xcf, r6l
	mov.b	r6l, @_S_BRR:8
	mov.b	@_S_SR:8, r6l	/* clear error flags */
	and.b	#0x84, r6l
	mov.b	r6l, @_S_SR:8
	mov.b	@PORT4_DDR_SHADOW, r6l	/* enable transmitter range pin */
	bset	#0, r6l
	mov.b	r6l, @PORT4_DDR_SHADOW
	mov.b	r6l, @_PORT4_DDR:8
	bset	#0, @_PORT4:8		/* and set to short range */
	mov.b	#9, r6l			/* set IR carrier frequency */
	mov.b	r6l, @_T1_CR:8
	mov.b	#0x13, r6l
	mov.b	r6l, @_T1_CSR:8
	mov.b	#26, r6l
	mov.b	r6l, @_T1_CORA:8
	mov.w	#tei_handler, r6
	mov.w	r6, @tei_vector
	mov.w	#txi_handler, r6
	mov.w	r6, @txi_vector
	mov.w	#rxi_handler, r6
	mov.w	r6, @rxi_vector
	mov.w	#eri_handler, r6
	mov.w	r6, @eri_vector

	push	r1
	push	r2
	mov.w	#serial_outgoing_packet, r0
	mov.w	#serial_outgoing_packet+64, r1
	jsr	@memclr
	mov.w	#serial_incoming_packet, r0
	mov.w	#serial_incoming_packet+16, r1
	jsr	@memclr
	pop	r2
	pop	r1
	sub.w	r6,r6
	mov.w	r6,  @serial_out_remaining
	mov.w	r6,  @serial_incoming_dataptr
	mov.b	r6l, @serial_receive_index
	mov.b	r6l, @serial_unpack_index
	mov.b	r6l, @serial_send_index
	mov.b	#0x4f, r6h
	mov.b	r6h, @serial_xmit_flag
	mov.b	@_S_CR:8, r6l
	or.b	#0x50, r6l		/* enable receive interrupts */
	mov.b	r6l, @_S_CR:8
	sub.b	r6l, r6l
	pop	r0
	rts
	
	. = 0x3250
set_range_long:
	cmp.b	#0x17, r6h
	bne	1f
	cmp.b	#0x70, r6l
	bne	1f
	bclr	#0, @_PORT4:8
1:	sub.b	r6l, r6l
	rts

	. = 0x3266
set_range_short:
	cmp.b	#0x17, r6h
	bne	1f
	cmp.b	#0x70, r6l
	bne	1f
	bset	#0, @_PORT4:8
1:	sub.b	r6l, r6l
	rts

	. = 0x327c
play_tone_or_set_data_pointer:
	add.b	#(256 - 0x17), r6h
	bne	1f
	add.b	#(256 - 0x71), r6l
	bne	2f

	/* r6 = 0x1771: set incoming data pointer */
	mov.w	@(2, r7), r6
	mov.w	r6, @serial_incoming_dataptr
	mov.w	#0xffff, r6
	mov.w	r6, @serial_transfer_sequence_number
1:	sub.b	r6l,r6l
	rts

2:	dec	r6l
	bne	2f

	/* r6 = 0x1772 : play system sound */
	mov.w	#7, r6
	push	r6
	mov.w	@(6, r7), r6
	push	r6
	mov.w	#1, r6
	push	r6
	mov.w	#0x700b, r6
	jsr	@control_output
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	sub.b	r6l,r6l
	rts
	
2:	dec	r6l
	bne	1b

	/* r6 = 0x1773 : play tone */
	push	r5
	mov.w	@(4, r7), r5
	beq	3f
	push	r3
	push	r4
	sub.w	r3, r3
	mov.w	r5, r4
	mov.w	#0x1200, r6
	mov.w	#0x007a, r5
	jsr	@@(softirq_vectors+12)
	pop	r4
	pop	r3
	cmp.b	#0, r5l
	beq	2f
	/* div == 400 or too high */
	shlr	r5l
	rotxr	r6h
	shlr	r5l
	bne	3f	/* too high */
	rotxr	r6h
	mov.b	#6, r5l
	bra	4f

3:	/* freq out of range */
	mov.b	#7, r5l
	sub.w	r6, r6
	bra	4f

2:	/* div <= 100 */
	mov.b	#7, r5l
	cmp.b	#0x40, r6h
	bhs	4f

	/* div <= 0x40 */
	mov.b	#4, r5l
	cmp.b	#0x20, r6h
	bhs	5f

	/* div <= 0x20 */
	mov.b	#5, r5l
	cmp.b	#0x08, r6h
	bhs	6f

	mov.b	#2, r5l
	add	r6, r6
	add	r6, r6
6:	add	r6, r6
5:	add	r6, r6
	add	r6, r6
4:	/* play sound */
	mov.b	r6h, r6l
	sub.b	r5h, r5h
	push	r5
	mov.w	@(8, r7), r5
	push	r5
	sub.b	r6h, r6h
	push	r6
	mov.w	#0x700b, r6
	jsr	@control_output
	adds	#2, r7
	adds	#2, r7
	adds	#2, r7
	pop	r5
	sub.b	r6l,r6l
	rts
	
	. = 0x339a
reset_internal_minute_timer:
	mov.w	#0x700e, r6
	jmp	@clear_sensor_or_timer_data

	. = 0x33b0
receive_data:
	push	r5
	push	r4
	push	r3
	mov.b	@serial_receive_index, r3l
	mov.b	@serial_unpack_index, r4l
	sub.b	r4h, r4h
	mov.b	@(9, r7),r5h	/* maxlen */
	sub.b	r5l, r5l	/* index */
	bra	3f
2:	mov.b	@(serial_incoming_packet, r4), r3h
	adds	#1, r4
	mov.b	r3h, @r6
	adds	#1, r6
	cmp.b	#10, r4l
	bne	1f
	mov.b	#0, r4l
1:	inc	r5l

3:	cmp.b	r5l, r5h
	bls	2f
	cmp.b	r4l, r3l
	bne	2b

2:	mov.b	r4l, @serial_unpack_index
	mov.b	r4h, @serial_valid_incoming_data
	mov.w	@(10, r7), r4
	mov.b	@r4, r6l
	add.b	r5l, r6l
	mov.b	r6l, @r4
	pop	r3
	pop	r4
	pop	r5
	sub.b	r6l,r6l
	rts

	. = 0x3426
check_for_data:
	push	r5
	mov.b	@serial_valid_incoming_data, r5l
	mov.b	r5l, @r6
	mov.w	@(4, r7), r6
	mov.w	@serial_incoming_dataptr, r5
	mov.w	r5, @r6
	sub.b	r6l, r6l
	pop	r5
	rts

	. = 0x343e
send_packet:
	push	r5
	mov.b	@serial_xmit_flag, r5l
	cmp.b	#0x4f, r5l
	beq	1f
	mov.b	#0x4c, r6l
2:	pop	r5
	rts
	
1:	add.b	#(256 - 0x17), r6h
	bne	2b
	add.b	#(256 - 0x75), r6l
	bne	2f

	/* code = 0x1775:  short packet */
	mov.b	@serial_has_packet_header, r5l
	beq	3f

	mov.b	#0x55, r5l
	mov.b	r5l, @(serial_outgoing_packet, r6)
	adds	#1, r6
	mov.b	#0xff, r5l
	mov.b	r5l, @(serial_outgoing_packet, r6)
	adds	#1, r6
	mov.b	#0x00, r5l
	mov.b	r5l, @(serial_outgoing_packet, r6)
	adds	#1, r6
	
3:
	push	r3
	push	r4
	mov.w	@(10,r7), r4	/* address */
	mov.w	@(12,r7), r3	/* length */

	mov.b	@serial_use_complement, r5l
	beq	3f

	/* use complements */
	sub.b	r5h, r5h
5:	mov.b	@r4+, r5l
	add.b	r5l, r5h
	mov.b	r5l, @(serial_outgoing_packet, r6)
	adds	#1, r6
	not.b	r5l
	mov.b	r5l, @(serial_outgoing_packet, r6)
	adds	#1, r6
	dec	r3l
	bne	5b

	mov.b	r5h, @(serial_outgoing_packet, r6)
	adds	#1, r6
	not.b	r5h
	mov.b	r5h, @(serial_outgoing_packet, r6)
	adds	#1, r6
	bra	4f
	
3:	/* no complements */
		
	sub.b	r5h, r5h
5:	mov.b	@r4+, r5l
	add.b	r5l, r5h
	mov.b	r5l, @(serial_outgoing_packet, r6)
	adds	#1, r6
	dec	r3l
	bne	5b

	mov.b	r5h, @(serial_outgoing_packet, r6)
	adds	#1, r6
4:
	mov.w	r6, @serial_out_remaining

	pop	r4
	pop	r3
	mov.w	#0x1775, r6
	bra	6f
	
2:	dec	r6l
	bne	1f

	/* code = 0x1776:  long packet */
	mov.b	@serial_has_packet_header, r5l
	beq	3f
	mov.b	#5, r6l
3:	mov.b	r6l, @serial_out_headerbyte
	
	mov.w	@(8,r7), r5	/* length */
	adds	#1, r5
	mov.b	@serial_use_complement, r6h
	beq	3f
	add.w	r5, r5
3:	add.b	r6l, r5l
	addx.b	#0, r5h
	mov.w	r5, @serial_out_remaining
	mov.w	@(6,r7), r6	/* addr */
	mov.w	r6, @serial_out_packetptr
	mov.b	@(5,r7), r6l
	mov.b	r6l, @serial_out_chksum
	sub.b	r6l, r6l
	mov.b	r6l, @serial_out_complement
	mov.w	#0x1776, r6

6:	mov.w	r6, @serial_out_packettype	
	sub.b	r5l, r5l
	mov.b	r5l, @serial_send_index
	mov.b	#0x13, r5l
	mov.b	r5l, @serial_xmit_flag
	mov.b	@_S_SR:8, r5l
	and.b	#0x7b, r5l	/* clear TDRE, TE flag */
	mov.b	r5l, @_S_SR:8
	mov.b	@_S_CR:8, r5l
	or.b	#0xa4, r5l	/* enbale TEI, TXI, transmit */
	mov.b	r5l, @_S_CR:8

	sub.b	r6l,r6l

1:	pop	r5
	rts

			
	. = 0x3636
shutdown_serial:
	mov.b	@PORT4_DDR_SHADOW, r6l
	bset	#0, r6l
	mov.b	r6l, @PORT4_DDR_SHADOW
	mov.b	r6l, @_PORT4_DDR:8
	bclr	#0, @_PORT4:8		/* clear range bit (set far) */
	mov.b	@PORT6_DDR_SHADOW, r6l
	bset	#7, r6l
	mov.b	r6l, @PORT6_DDR_SHADOW
	mov.b	r6l, @_PORT6_DDR:8
	bclr	#7, @_PORT6:8		/* clear IR carrier */
	sub.b	r6l, r6l
	mov.b	r6l, @_T1_CR:8
	mov.b	r6l, @_T1_CSR:8
	mov.b	@_S_CR:8, r6l
	and.b	#0x0b, r6l		/* disable serial IRQs, transmit, receive */
	mov.b	r6l, @_S_CR:8
	mov.b	@PORT5_DDR_SHADOW, r6l
	or.b	#3, r6l
	mov.b	r6l, @PORT5_DDR_SHADOW
	mov.b	r6l, @_PORT5_DDR:8
	bclr	#0, @_PORT5:8		/* clear transmit/receive data */
	bclr	#1, @_PORT5:8
	sub.b	r6l, r6l
	rts

	. = 0x3692
set_port_6_bit_3_output:
	mov.b	@PORT6_DDR_SHADOW, r6l
	bset	#3, r6l
	mov.b	r6l, @PORT6_DDR_SHADOW
	mov.b	r6l, @_PORT6_DDR:8
	bset	#3, @_PORT6:8
	sub.b	r6l, r6l
	rts

	. = 0x36a6
	sub.b	r6l, r6l
	rts
	
	. = 0x36aa
set_port_6_bit_3_input:
	mov.b	@PORT6_DDR_SHADOW, r6l
	bclr	#3, r6l
	mov.b	r6l, @PORT6_DDR_SHADOW
	mov.b	r6l, @_PORT6_DDR:8
	sub.b	r6l, r6l
	rts
	
ocia_handler:
	push	r5
	bclr	#3, @_T_CSR:8         /* clear output compare flag */
	mov.b	@MOTOR_OUT_SHADOW:16, r6l
	mov.b	r6l, @MOTOR_OUT:16
	mov.b	#0, r5h
	mov.b	@time_ctr_0:16, r5l
	mov.b	#3, r6l
	divxu.b r6l, r5
	cmp.b	#0, r6h
	bne	1f
	/* turn analog sensors passive (for polling them) */
	mov.b	@_PORT6:8, r6l
	and.b	#~0x07, r6l
	mov.b	r6l, @_PORT6:8
1:
	mov.w	#motor_dir, r5
	mov.b   @r5+, r6h

	/* Rotate motor_0 wave right and clear bits r0h accordingly */
	mov.b	@r5, r6l
	rotr	r6l
	mov.b	r6l, @r5
	bcs	1f
	and.b	#0x3f, r6h

	/* Rotate motor_1 wave right and clear bits in r0h accordingly */
1:	adds	#1, r5
	mov.b	@r5, r6l
	rotr	r6l
	mov.b	r6l, @r5
	bcs	1f
	and.b	#~0x0c, r6h

	/* Rotate motor_2 wave right and set bits 1,0 in r0h accordingly */
1:	adds	#1, r5
	mov.b	@r5, r6l
	rotr	r6l
	mov.b	r6l, @r5
	bcs	1f
	and.b	#~0x03, r6h

	/* store modulated motor output */
1:	adds	#1, r5
	mov.b	r6h, @r5

	mov.w	@timer_struct:16, r5
	/* update serial receive reset counter */
	mov.w	@r5, r6
	beq	1f
	subs	#1, r6
	mov.b	r6l,@r5
1:
	/* update motor timers */
	mov.w	@(timer_motor0, r5), r6
	beq	1f
	subs	#1, r6
	mov.w	r6, @(timer_motor0, r5)
	bne	1f
	mov.w	@run_handler_flags, r6
	adds	#1, r6
	bset	#7, @r6
	
1:	mov.w	@(timer_motor1, r5), r6
	beq	1f
	subs	#1, r6
	mov.w	r6, @(timer_motor1, r5)
	bne	1f
	mov.w	@run_handler_flags, r6
	adds	#1, r6
	bset	#7, @r6

1:	mov.w	@(timer_motor2, r5), r6
	beq	1f
	subs	#1, r6
	mov.w	r6, @(timer_motor2, r5)
	bne	1f
	mov.w	@run_handler_flags, r6
	/* activate handler 1 */
	adds	#1, r6
	bset	#7, @r6

	/* Check for sound status (runs every 10 ms) */
1:	mov.b	#0, r5h
	mov.b	@time_ctr_0:16, r5l
	mov.b	#10, r6l
	divxu.b	r6l, r5
	cmp.b	#0, r5h
	beq	1f
	jmp	end_of_ten_ms_part

	/* check if sound remaining */
1:	mov.b	@sound_duration, r6l
	beq     1f
	dec	r6l
	mov.b	r6l, @sound_duration
	jmp	end_of_sound_part

1:	mov.b	@sound_tune_active, r6l
	beq	3f

	mov.w	@sound_pitch_ptr, r6
	mov.b	@r6+, r5l
	cmp.b	#1, r5l
	bne	continue_tune
	mov.b	#0, r5l
	mov.b	r5l, @sound_tune_active

3:
	/* sound is not active any more, check if there are more
	 * sounds in the queue
	 */
	mov.b	@sound_queue_tail, r6h
	mov.b	@sound_queue_head, r6l
	inc.b	r6l
	cmp.b	#10, r6l
	bne	7f
	mov.b	#0, r6l
7:	cmp.b	r6l, r6h
	bne	3f

	/* sound queue empty  ---> stop sound */
	sub.w	r5, r5
	mov.b	r5l, @sound_playing 
	bra	play_sound
3:	
	mov.b	r6l, @sound_queue_head
	mov.b	#0, r6h
	mov.b	@(sound_queue_type, r6), r5l
	bne	3f

	/* play silence */
	mov.b	@(sound_queue_value, r6), r5h
	/*  r5l is already zero */
	bra	play_sound
	
3:	cmp.b	#1, r5l
	beq	3f

	/* play tone */
	mov.b	r5l, @_T0_CORA:8
	mov.b	@(sound_queue_value, r6), r5h
	mov.b	@(sound_queue_freq, r6), r5l
	bra	play_sound
3:	
	/* play tune */
	mov.b	#1, r5l
	mov.b	r5l, @sound_tune_active
	mov.b	@(sound_queue_value, r6), r6l
	add.w	r6, r6
	mov.w	@(sound_tunes_dur_ptr, r6), r5
	mov.w	r5, @sound_dur_ptr
	mov.w	@(sound_tunes_freqmul_ptr, r6), r5
	mov.w	r5, @sound_freqmul_ptr
	mov.w	@(sound_tunes_pitch_ptr, r6), r6
	mov.b	@r6+, r5l
	
continue_tune:	
	mov.w	r6, @sound_pitch_ptr
	mov.b	r5l, @_T0_CORA:8
	
	mov.w	@sound_dur_ptr, r6
	mov.b	@r6+, r5h
	mov.w	r6, @sound_dur_ptr
	mov.w	@sound_freqmul_ptr, r6
	mov.b	@r6+, r5l
	mov.w	r6, @sound_freqmul_ptr

play_sound:
	mov.b	r5h, @sound_duration
	shlr	r5l
	bst	#0, @_STCR:8
	mov.b	@_T0_CR:8, r6l
	and.b	#~0x3, r6l
	or.b	r5l, r6l
	mov.b	r6l, @_T0_CR:8
	
end_of_sound_part:
	mov.b	@time_ctr_3, r6l
	inc	r6l
	cmp.b	#13, r6l
	bne	2f
	/* activate handler 2 and 3 */
	mov.w	@run_handler_flags, r6
	adds	#2, r6
	bset	#7, @r6
	adds	#1, r6
	bset	#7, @r6
	mov.b	#0, r6l
2:	mov.b	r6l, @time_ctr_3

	mov.w	@time_ctr_2, r6
	adds	#1, r6
	mov.w	#6001, r5  /* should we really emulate the ROM bugs???? */
	cmp.w	r5,r6
	bne	2f

	/* decrease power off minutes */
	mov.w	@timer_struct, r5
	mov.w	@(timer_power_off, r5), r6
	beq	3f
	subs	#1, r6
	mov.w	r6, @(timer_power_off, r5)

	/* increase system time minutes */
3:	mov.w	@(timer_minute_counter, r5), r6
	adds	#1, r6
	mov.w   r6, @(timer_minute_counter, r5)
	cmp.b	#(1440 & 255), r6l
	subx.b	#(1440 >> 8), r6h
	blo	3f
	sub.w	r6,r6
	mov.w   r6, @(timer_minute_counter, r5)
3:	sub.w	r6, r6
2:	mov.w	r6, @time_ctr_2
	
end_of_ten_ms_part:
	mov.b	@time_ctr_1, r6l
	inc	r6l
	cmp.b	#100, r6l
	bne	2f
	/* increase firmware timer every 100 ms 
         * (and keep it in range 0 .. 32767) */
	mov.w	@timer_struct, r6
	/* timer 0 */
	adds	#2, r6
	mov.w	@r6, r5
	adds	#1, r5
	bclr	#7, r5h
	mov.w	r5, @r6

	/* timer 1 */
	adds	#2, r6
	mov.w	@r6, r5
	adds	#1, r5
	bclr	#7, r5h
	mov.w	r5, @r6
	
	/* timer 2 */
	adds	#2, r6
	mov.w	@r6, r5
	adds	#1, r5
	bclr	#7, r5h
	mov.w	r5, @r6
	
	/* timer 3 */
	adds	#2, r6
	mov.w	@r6, r5
	adds	#1, r5
	bclr	#7, r5h
	mov.w	r5, @r6
	mov.b	#0, r6l

2:	mov.b	r6l, @time_ctr_1

	
	mov.b	#0, r5h
	mov.b	@time_ctr_0:16, r5l
	mov.b	#10, r6l
	divxu.b	r6l, r5
	add.b	#timer_wakeup_delays, r5h
	mov.w	@timer_struct, r6
	add.b	r5h, r6l
	addx	#0, r6h
	mov.b	@r6, r5l
	beq	1f
	dec	r5l
	mov.b	r5l, @r6
	
1:	mov.b	#0, r5h
	mov.b	@time_ctr_0:16, r5l
	mov.b	#3, r6l
	divxu.b	r6l, r5
	cmp.b	#0, r5h
	bne	1f

	/* start polling */
	mov.b	@_AD_CSR:8,r6l
	or.b	#0x60, r6l
	mov.b	r6l, @_AD_CSR:8
1:
	mov.b	@time_ctr_0:16, r6l
	inc	r6l
	cmp.b	#31, r6l  
	blo	1f
	mov.b	#1, r6l     /* hmmm, why 1..30 instead of 0..29 ? */
1:	mov.b	r6l, @time_ctr_0:16
	pop	r5
	rts


	/* All tunes are in format:
	 * pitch:
         * .word pitch0, pitch1, .., 1  (= freqmul / freq of tone)
	 * dur:
	 * .byte dur0,   dur1, ...      (in 10ms)
	 * freqmul:
	 * .byte freqmul0, freqmul1,...   
         *   freqmul:  (2: 2MHz, 3: 8MHz, 4: 250kHz, 5: 500 kHz, 
	 *              6 : 15625 Hz , 7: 62500 Hz)
         */
sound_tune_0_pitch: /* blib */
	.byte	125, 0, 1
sound_tune_0_dur:
	.byte   1,   3

sound_tune_1_pitch: /* beep beep */
	.byte	250, 0, 250, 0, 1
sound_tune_1_dur:
	.byte	15,  5,  15, 5

sound_tune_2_pitch: /* downward tone */
	.byte   179, 192, 208, 227, 250, 139, 156, 179, 208, 250, 1
sound_tune_3_5_pitch: /* upward tone */
	.byte   250, 208, 179, 156, 139, 250, 227, 208, 192, 179, 1

sound_tune_2_3_dur:    /* slow */
	.byte   6,     6,   6,   6,   6,   6,   6,   6,   6,   6
sound_tune_5_dur:    /* fast */
	.byte   3,     3,   3,   3,   3,   3,   3,   3,   3,   3

sound_tune_4_6_pitch: /* long low buzz */
	.byte   78, 1
sound_tune_4_6_dur:
	.byte   100
sound_tune_freqmul:
	.byte   5, 5, 5, 5, 5, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6
	.align 1

sound_tunes_pitch_ptr:
	.word   sound_tune_0_pitch
	.word	sound_tune_1_pitch,   sound_tune_2_pitch
	.word   sound_tune_3_5_pitch, sound_tune_4_6_pitch
	.word   sound_tune_3_5_pitch, sound_tune_4_6_pitch
sound_tunes_dur_ptr:
	.word   sound_tune_0_dur
	.word	sound_tune_1_dur,   sound_tune_2_3_dur
	.word   sound_tune_2_3_dur, sound_tune_4_6_dur
	.word   sound_tune_5_dur,   sound_tune_4_6_dur
sound_tunes_freqmul_ptr:
	.word   sound_tune_freqmul
	.word   sound_tune_freqmul, sound_tune_freqmul
	.word   sound_tune_freqmul + 5, sound_tune_freqmul + 15
	.word   sound_tune_freqmul + 5, sound_tune_freqmul + 15

		
adi_handler:
	mov.b	@_AD_CSR:8, r6l
	and.b	#0x1f, r6l
	mov.b	r6l, @_AD_CSR:8
	mov.b	@active_sensors, r6h
	mov.b	@_PORT6:8, r6l
	or.b	r6h, r6l,
	mov.b	r6l, @_PORT6:8
	mov.w	@run_handler_flags, r6
	bset	#7, @r6
	rts


	. = 0x3b9a
init_timer:
	/* args:  ee74, ee64 */
	mov.w	r6, @timer_struct
	mov.w	@(2:16,r7), r6
	mov.w	r6, @run_handler_flags
	push	r0
	push	r1
	push	r2
	mov.w	#0xfd80, r0
	mov.w	#0xfd88, r1
	jsr	@memclr
	mov.w	#1,r0
	mov.b	r0h, @motor_dir
	mov.b	r0l, @time_ctr_0
	mov.b	r0l, @time_ctr_3
	mov.b	r0h, @time_ctr_1
	mov.b	r0h, @time_ctr_2
	mov.b	r0h, @active_sensors
	mov.b	r0l, @sound_queue_tail
	mov.b	r0h, @sound_queue_head
	mov.w	#sound_queue_type, r0
	mov.w	#sound_queue_freq+10, r1
	jsr	@memclr
	sub.w	r2,r2
	mov.b	r2h, @sound_tune_active
	mov.b	r2h, @sound_playing
	mov.w	#ocia_handler, r6
	mov.w	r6, @ocia_vector
	mov.w	#adi_handler, r6
	mov.w	r6, @adi_vector
	
	mov.b	#0xb, r6l
	mov.b	r6l, @_T0_CR:8
	mov.b	#0x3, r6l
	mov.b	r6l, @_T0_CSR:8
	bset	#0, @_STCR:8
	mov.b	#0xff, r6l
	mov.b	r6l, @_PORT1_DDR:8
	mov.b	r6l, @PORT1_DDR_SHADOW:16
	mov.b	r6l, @_PORT2_DDR:8
	mov.b	r6l, @PORT2_DDR_SHADOW:16
	mov.b	r2l, @MOTOR_OUT:16
	bclr	#7, @_AD_CR:8      /* No external A/D trigger */
	mov.b	#0x13, r6l         /* ADCSR_SCAN | channel 3 */
	mov.b	r6l, @_AD_CSR:8
	bclr	#5, @_WSCR:8
	bclr	#0, @_T_CR:8
	bset	#1, @_T_CR:8
	mov.b	#1, r6l
	mov.b	r6l, @_T_CSR:8
	bclr	#4, @_T_OCR:8
	mov.w	#500, r6
	mov.w	r6, @(0xff00+_T_OCRA):16
	mov.w	r2, @(0xff00+_T_CNT):16
	bclr	#3, @_T_CSR:8
	bset	#3, @_T_IER:8
	andc	#0x7f, ccr
	mov.b	@active_sensors, r6h
	mov.b	@_PORT6:8, r6l
	or.b	r6h, r6l
	mov.b	r6l, @_PORT6:8
	pop	r2
	pop	r1
	pop	r0
	rts

	. = 0x3ccc
get_sound_playing_flag:
	push	r5
	mov.w	@(4,r7), r5
	mov.b	@sound_playing, r6l
	mov.b	r6l, @r5
	pop	r5
	sub.b	r6l,r6l
	rts

	. = 0x3ce6  /* rom internal */
control_motor_2:
	add.b	#(256-0x70), r6h
	bne	1f
	add.b	#(256-0x01), r6l
	cmp.b	#3, r6l
	bhs.b	1f

	push	r5
	mov.b	@(control_motor_bit, r6), r6h
	mov.w	@(4,r7), r5
	mov.b	@(1,r5), r6l
	mov.b	@r5, r5l
	sub.b	r5h, r5h
	and.b	#7, r5l
	mov.b	@(control_motor_waveforms, r5), r5l
	mov.b	@motor_dir, r5h

	bclr	r6h, r5h
	btst	#1, r6l
	beq	3f
	bset	r6h, r5h
3:	inc.b	r6h
	bclr	r6h, r5h
	btst	#0, r6l
	beq	3f
	bset	r6h, r5h
3:	
2:
	mov.w	r5, @motor_dir
	pop	r5
	
1:	sub.b	r6l,r6l
	rts
	
control_motor_bit:
	.byte	6, 2, 0
control_motor_waveforms:
	.byte	0x01, 0x11, 0x49, 0x55, 0x6d, 0x77, 0x7f, 0xff

	. = 0x3de0
control_output:
	add.b	#(256-0x70), r6h
	bne	1f
	add.b	#(256-0x0a), r6l
	bne	2f

	/* r6 = 0x700a:  set active sensors */
	mov.b	@(3, r7), r6h
	mov.b	@active_sensors, r6l
	or.b	r6h, r6l
	mov.b	r6l, @active_sensors
	bra	1f
	
2:	dec.b	r6l
	bne	2f

	/* r6 = 0x700b: enqueue sound */
	push	r5
	mov.b	@sound_queue_head, r6h
	mov.b	@sound_queue_tail, r6l
	mov.b	r6l, r5l
	inc.b	r6l
	cmp.b	#10, r6l
	bne	3f
	mov.b	#0, r6l
3:	cmp.b	r6l, r6h
	beq	3f
	sub.b	r5h, r5h
	mov.b	@(5, r7), r6h
	mov.b	r6h, @(sound_queue_type,r5)
	mov.b	@(7, r7), r6h
	mov.b	r6h, @(sound_queue_value,r5)
	mov.b	@(9, r7), r6h
	mov.b	r6h, @(sound_queue_freq,r5)
	mov.b	r6l, @sound_queue_tail
	mov.b	#1, r6h
	mov.b	r6h, @sound_playing
3:	pop	r5
1:	bra	1f
	
	
2:	add.b	#(256-2), r6l
	bne	1f

	/* r6 = 0x700d: put undelayed sound */
	push	r5
	mov.b	@sound_queue_head, r6l
	inc.b	r6l
	cmp.b	#10, r6l
	bne	3f
	mov.b	#0, r6l
3:	/* r6h is still zero */
	mov.b	r6h, @sound_duration
	mov.b	@(5, r7), r5l
	mov.b	r5l, @(sound_queue_type,r6)
	mov.b	@(7, r7), r5l
	mov.b	r5l, @(sound_queue_value,r6)
	inc.b	r6l
	cmp.b	#10, r6l
	bne	3f
	mov.b	#0, r6l
3:	mov.b	r6l, @sound_queue_tail
	mov.b	#1, r6h
	mov.b	r6h, @sound_playing
	pop	r5
	
1:	sub.b	r6l, r6l
	rts


	. = 0x3e9e
clear_sensor_or_timer_data:
	add.b	#(256-0x70), r6h
	bne	1f
	add.b	#(256-0x0a), r6l
	bne	2f

	/* r6 = 0x700a:  clear active sensors */
	mov.b	@(3, r7), r6l
	mov.b	@active_sensors, r6h
	not.b	r6l
	and.b	r6l, r6h
	mov.b	r6h, @active_sensors
	bra	1f

2:	add.b	#(256-4), r6l
	bne	1f
	/* r6 = 0x700e:  clear clock seconds */
	mov.w	r6, @time_ctr_2

1:	sub.b	r6l, r6l
	rts

	. = 0x3ed4
shutdown_timer:
	mov.b	@PORT6_DDR_SHADOW, r6l
	bclr	#4, r6l
	mov.b	r6l, @PORT6_DDR_SHADOW
	mov.b	r6l, @_PORT6_DDR:8
	sub.b	r6l, r6l
	mov.b	r6l, @motor_dir		/* turn off motors */
	mov.b	r6l, @MOTOR_OUT
	mov.b	r6l, @_T0_CR:8		/* turn off sound  */
	mov.b	r6l, @_T0_CSR:8
	bclr	#0, @_STCR:8
	mov.b	@_AD_CSR:8, r6h		/* turn off A/D converter */
	and.b	#0x8c, r6h
	mov.b	r6h, @_AD_CSR:8
	bclr	#3, @_T_IER:8		/* turn off OCIA interrupt */
	rts
	
	
	

Do_you_byte:
	.string "Do you byte, when I knock?"
Just_a_bit:
	.string "Just a bit off the block!"

	
	. = 0x3ffd
	.byte	0
	
	rom_main_loop_state = 0xee5e
	rom_message_start = 0xee60
	firmware_entry = 0xee62
	rom_handler_struct = 0xee64
	rom_timer_struct = 0xee74
	power_off_timer = 0xee80
	firmware_holdoff_timer = 0xee82
	lcd_clear_timer = 0xee84
	rom_ready_to_sleep = 0xeeb4
	rom_send_buffer = 0xeeb6
	rom_receive_buffer = 0xeef6
	rom_update_state = 0xef06
	rom_key_state = 0xef07
	last_serial_dataptr = 0xef08
	rom_transfer_data_index = 0xef0a
	last_received_opcode = 0xef0c
	rom_send_packet_length = 0xef0d
	rom_onoff_key_debounce = 0xef0e
	rom_program_number = 0xef10
	angle_state       = 0xef35
	edge_inited_flag  = 0xef35
	edge_debounce_cnt = 0xef38
	edge_counter      = 0xef3b
	i2c_area       = 0xef3e
	lcd_display    = 0xef43
	dot_counter    = 0xef4e
	circle_counter = 0xef4f
	
serial_has_packet_header	= 0xef50
serial_use_complement		= 0xef51
serial_outgoing_packet		= 0xef52
serial_send_index		= 0xef92
serial_xmit_flag		= 0xef93
serial_out_packettype		= 0xef94
serial_out_remaining		= 0xef96
serial_out_packetptr		= 0xef98
serial_out_headerbyte		= 0xef9a
serial_out_opcode		= 0xef9b
serial_out_chksum		= 0xef9b
serial_out_complement		= 0xef9c
serial_incoming_packet		= 0xef9d
serial_incoming_dataptr		= 0xefae
serial_receive_index		= 0xefb0
serial_unpack_index		= 0xefb1
serial_receive_state		= 0xefb2
serial_expect_complement	= 0xefb3
serial_timer_struct		= 0xefb4
serial_valid_incoming_data	= 0xefb6
serial_handler_run_flag_addr	= 0xefb8
serial_last_received_byte	= 0xefba
serial_transfer_sequence_number	= 0xefbc
serial_receive_chksum		= 0xefbe
serial_receive_data_chksum	= 0xefbf
serial_receive_length_remaining	= 0xefc0	
serial_receive_data_length	= 0xefc2
serial_saved_dataptr		= 0xefc4

	run_sensor_handler      = 0x00
	run_motor_handler       = 0x01
	run_button_handler      = 0x02
	run_power_handler       = 0x03
	
	timer_struct   = 0xefc6
	timer_serial_receive_reset = 0x00
	timer_firmware_timer_0     = 0x02
	timer_firmware_timer_1     = 0x04
	timer_firmware_timer_2     = 0x06
	timer_firmware_timer_3     = 0x08
	timer_minute_counter       = 0x0a
	timer_power_off            = 0x0c
	timer_wakeup_delays        = 0x0e
	timer_motor0               = 0x22
	timer_motor1               = 0x24
	timer_motor2               = 0x26
	
	run_handler_flags = 0xefc8

	motor_dir    = 0xefca
	motor_wave_0 = 0xefcb
	motor_wave_1 = 0xefcc
	motor_wave_2 = 0xefcd
	MOTOR_OUT_SHADOW = 0xefce
	time_ctr_0 = 0xefcf 
	time_ctr_1 = 0xefd0
	time_ctr_2 = 0xefd2 
	time_ctr_3    = 0xefd4
	active_sensors = 0xefd5

	sound_queue_tail  = 0xefd6
	sound_queue_head  = 0xefd7
	sound_queue_type  = 0xefd8
	sound_queue_value = 0xefe2
	sound_queue_freq  = 0xefec

	sound_duration    = 0xeff6
	sound_pitch_ptr   = 0xeff8
	sound_dur_ptr     = 0xeffa
	sound_freqmul_ptr = 0xeffc
	sound_tune_active = 0xeffe
	sound_playing     = 0xefff


	MOTOR_OUT        = 0xf000
	PORT1_DDR_SHADOW = 0xfd80
	PORT2_DDR_SHADOW = 0xfd81
	PORT3_DDR_SHADOW = 0xfd82
	PORT4_DDR_SHADOW = 0xfd83
	PORT5_DDR_SHADOW = 0xfd84
	PORT6_DDR_SHADOW = 0xfd85
	PORT7_SHADOW     = 0xfd86

	high_firmware_checksum     = 0xfd88
	high_firmware_checksum_inv = 0xfd8a
	high_firmware_entry        = 0xfd8c

nmi_vector  = 0xfd92
irq0_vector = 0xfd94
irq1_vector = 0xfd96
irq2_vector = 0xfd98
icia_vector = 0xfd9a
icib_vector = 0xfd9c
icic_vector = 0xfd9e
icid_vector = 0xfda0
ocia_vector = 0xfda2
ocib_vector = 0xfda4
fovi_vector = 0xfda6
cmi0a_vector = 0xfda8
cmi0b_vector = 0xfdaa
ovi0_vector = 0xfdac
cmi1a_vector = 0xfdae
cmi1b_vector = 0xfdb0
ovi1_vector = 0xfdb2
eri_vector = 0xfdb4
rxi_vector = 0xfdb6
txi_vector = 0xfdb8
tei_vector = 0xfdba
adi_vector = 0xfdbc
wovf_vector = 0xfdbe



    _T_IER     = 0x90 
    _T_CSR     = 0x91 
    _T_CNT     = 0x92 
    _T_OCRA    = 0x94 
    _T_OCRB    = 0x94 
    _T_CR      = 0x96 
    _T_OCR     = 0x97 
    _T_ICRA    = 0x98 
    _T_ICRB    = 0x9a 
    _T_ICRC    = 0x9c 
    _T_ICRD    = 0x9e 
    _WDT_CSR   = 0xa8 
    _WDT_CNT   = 0xa9 
    _PORT1_PCR = 0xac 
    _PORT2_PCR = 0xad 
    _PORT3_PCR = 0xae 
    _PORT1_DDR = 0xb0 
    _PORT2_DDR = 0xb1 
    _PORT1     = 0xb2 
    _PORT2     = 0xb3 
    _PORT3_DDR = 0xb4 
    _PORT4_DDR = 0xb5 
    _PORT3     = 0xb6 
    _PORT4     = 0xb7 
    _PORT5_DDR = 0xb8 
    _PORT6_DDR = 0xb9 
    _PORT5     = 0xba 
    _PORT6     = 0xbb 
    _PORT7     = 0xbe 
    _WSCR      = 0xc2
    _STCR      = 0xc3 
    _SYSCR     = 0xc4 
    _ISCR      = 0xc6
    _IER       = 0xc7
    _T0_CR     = 0xc8 
    _T0_CSR    = 0xc9 
    _T0_CORA   = 0xca 
    _T0_CORB   = 0xcb 
    _T0_CNT    = 0xcc 
    _T1_CR     = 0xd0 
    _T1_CSR    = 0xd1 
    _T1_CORA   = 0xd2 
    _T1_CORB   = 0xd3 
    _T1_CNT    = 0xd4 
    _S_MR      = 0xd8 
    _S_BRR     = 0xd9 
    _S_CR      = 0xda 
    _S_TDR     = 0xdb 
    _S_SR      = 0xdc 
    _S_RDR     = 0xdd 
    _AD_A      = 0xe0 
    _AD_A_H    = 0xe0 
    _AD_A_L    = 0xe1 
    _AD_B      = 0xe2 
    _AD_B_H    = 0xe2 
    _AD_B_L    = 0xe3 
    _AD_C      = 0xe4 
    _AD_C_H    = 0xe4 
    _AD_C_L    = 0xe5 
    _AD_D      = 0xe6 
    _AD_D_H    = 0xe6 
    _AD_D_L    = 0xe7 
    _AD_CSR    = 0xe8 
    _AD_CR     = 0xe9 




  sign   = 0x3001		/*!< signed no leading zeros */
  e0     = 0x3002		/*!< whole */
  e_1    = 0x3003		/*!< 10ths */
  e_2    = 0x3004		/*!< 100ths */
  e_3    = 0x3005		/*!< 1000ths problematic with negatives */
  man_stand = 0x3006
  man_run   = 0x3007

  s1_select = 0x3008
  s1_active = 0x3009

  s2_select = 0x300A
  s2_active = 0x300B

  s3_select = 0x300C
  s3_active = 0x300D

  a_select  = 0x300E
  a_left    = 0x300F
  a_right   = 0x3010

  b_select  = 0x3011
  b_left    = 0x3012
  b_right   = 0x3013

  c_select  = 0x3014
  c_left    = 0x3015
  c_right   = 0x3016

  digit     = 0x3017		/*!< single digit on the right */

  circle    = 0x3018		/*!< 0..3 quarters: add one. 4 quarters: reset */

  dot       = 0x3019		/*!< 0..4 dots: add a dot. 5 dots: reset
      	      	      	           dots are added on the right. */
  dot_inv   = 0x301A		/*!< 0 dots: show 5. 1..4 dots: subtract one
      	      	      	           dots are removed on the left. */

  battery_x = 0x301B

  ir_half   = 0x301C		/*! the IR display values are mutually exclusive. */
  ir_full   = 0x301D   		/*! the IR display values are mutually exclusive. */

  unsign    = 0x301F		/*!< unsigned 0 displayed as 0000 */
  everything= 0x3020 
