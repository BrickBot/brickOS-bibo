	. = 0x1b62
lcd_show:
	add.b	#256-(man_stand & 0xff), r6l   /* do not change!!!! */
	add.b	#256-(man_stand >> 8), r6h  /* MUST NOT BE subx (zero flag) */
	bne	1f

	cmp.b	#(man_run - man_stand), r6l
	bhi	2f

	/*  show_man */
	mov.b	@(lcd_man_info:16, r6), r6h
	mov.b	@(lcd_display:16), r6l
	and.b	#0xf0, r6l
	or.b	r6h, r6l
	mov.b	r6l, @(lcd_display:16)
1:	sub.w	r6, r6
	rts

2:	cmp.b	#(c_right - man_stand), r6l
	bhi	1f

	/* show_simple */
	push	r5
	mov.b	@((lcd_simple_bit - 2):16, r6), r5l
	add.w	r6, r6
	mov.w	@((lcd_simple_addr - 2*2):16, r6), r6
	bset	r5l, @r6
	pop	r5
2:	sub.w	r6,r6
	rts

1:	cmp.b	#(battery_x - man_stand),r6l
	blo	1f
	bne	2f

	/* battery_x */
	mov.w	#(lcd_display+4), r6
	bset	#0, @r6
	sub.w	r6,r6
	rts

2:	cmp.b	#(ir_full - man_stand), r6l
	bhi	2f
	/* ir_low / full */
	mov.w	#(lcd_display+6), r6
	bist    #3, @r6
	bset	#2, @r6
	sub.w	r6,r6
	rts

2:	cmp.b   #(everything - man_stand), r6l
	bne	2f
	/* everything */
	mov.w	#0x09ff, r6
	push	r5
3:	mov.w   #lcd_display+9,r5
	mov.b	r6l, @-r5
	dec	r6h
	bne	3b
	pop	r5
2:	mov.b	#0, r6l
	rts
	
1:	/* circle, dot, dot_inv */
	cmp.b	#(circle - man_stand),r6l
	blo	2b
	bhi	1f

	/* circle*/
	mov.b	@circle_counter, r6l
	inc	r6l
	cmp.b	#5, r6l
	beq	2f
	mov.b	r6l, @circle_counter
	push	r5
	mov.w	#(lcd_display + 2), r5
	mov.b	@(lcd_circle_order-1, r6), r6l
	bset	r6l, @r5
	pop	r5
	sub.w	r6,r6
	rts
1:	
	mov.b	@(lcd_display + 2), r6l
	and.b	#0xf0, r6l
	mov.b	r6l, @(lcd_display + 2)
	sub.w	r6,r6
	mov.b	r6l, @circle_counter
	rts
	
	/* dot or dot_inv*/
1:	push	r5
	mov.b	@dot_counter, r5l
	inc	r5l
	cmp.b	#6, r5l
	bne	1f
	mov.b	#0, r5l
	mov.b	r5l, @dot_counter

	cmp	#(dot_inv - man_run), r6l
	mov.w	#(lcd_display+6), r6
	beq	1f

	cmp.b	#1, r5l
	bhi	2f
	bclr	#4, @r6
	inc	r6l
	bclr	#1, @r6
	/* carry still 1 iff r5l changed from 5 -> 0 */
	bist    #4, @r6
	inc	r6l
	bclr	#1, @r6
	bclr	#4, @r6
	bra	3f
2:	mov.b	#0, r5h
	add.b	#1, r5l
	shlr	r5l
	mov.b	@(lcd_dot_order_addr-1, r5), r6l
	bist    #2, r5h    /* set bit 4, if r5 even, set bit 0 otherwise */
	bset    r5h, @r6
3:	pop	r5
	sub.w	r6,r6
	rts

1:	/* dot_inv */
	cmp.b	#1, r5l
	bne	2f
	bset	#4, @r6
	inc	r6l
	bset	#0, @r6
	bset	#4, @r6
	inc	r6l
	bset	#0, @r6
	bset	#4, @r6
	bra	3f

2:	mov	#0, r5h
	shlr	r5l        /* r6 + r5l / 2 = lcd_addr */
	bist    #2, r5h    /* bit 4, iff r5 % 2 == 0 */
4:	bclr    r5h, @r6
3:	pop	r5
	sub.w	r6,r6
	rts

lcd_man_info:
	.byte 0x6, 0xb

lcd_circle_order:
	.byte	0, 2, 3, 1
lcd_dot_order_addr:
	.byte	0x4a, 0x4b, 0x49
	
lcd_simple_bit:
	.byte	0
	.byte	1
	.byte   4
	.byte   0
	.byte   4
	.byte   4
	.byte   2
	.byte   6
	.byte   2
	.byte   6
	.byte   2
	.byte   2
	.byte   6
	.byte   6
	.byte   6
	
lcd_simple_addr:
	.word 0xef49
	.word 0xef49
	.word 0xef48
	.word 0xef48
	.word 0xef47
	.word 0xef45
	.word 0xef4a
	.word 0xef46
	.word 0xef46
	.word 0xef43
	.word 0xef44
	.word 0xef47
	.word 0xef44
	.word 0xef47
	.word 0xef45



	. = 0x1e4a
lcd_hide:
	add.b	#256-(man_stand & 0xff), r6l   /* do not change!!!! */
	add.b	#256-(man_stand >> 8), r6h  /* MUST NOT BE subx (zero flag) */
	bne	1f

	cmp.b	#(man_run - man_stand), r6l
	bhi	2f

	/*  hide man */
	mov.b	@(lcd_man_info:16, r6), r6h
	not.b	r6h
	mov.b	@(lcd_display:16), r6l
	and.b	r6h, r6l
	mov.b	r6l, @(lcd_display:16)
1:	sub.w	r6, r6
	rts

2:	cmp.b	#(c_right - man_stand), r6l
	bhi	1f

	/* hide simple */
	push	r5
	mov.b	@((lcd_simple_bit - 2):16, r6), r5l
	add.w	r6, r6
	mov.w	@((lcd_simple_addr - 2*2):16, r6), r6
	bclr	r5l, @r6
	pop	r5
2:	sub.w	r6,r6
	rts

1:	cmp.b	#(battery_x - man_stand),r6l
	blo	1f
	bne	2f

	/* battery_x */
	mov.w	#(lcd_display+4), r6
	bclr	#0, @r6
	sub.w	r6,r6
	rts

2:	cmp.b	#(ir_full - man_stand), r6l
	bhi	2f
	/* ir_low / full */
	mov.w	#(lcd_display+6), r6
	bclr    #3, @r6
	bclr	#2, @r6
2:	sub.w	r6,r6
	rts

1:	/* circle, dot, dot_inv */
	cmp.b	#(circle - man_stand),r6l
	blo	2b
	bhi	1f

	/* circle*/
	mov.b	@(lcd_display + 2), r6l
	and.b	#0xf0, r6l
	mov.b	r6l, @(lcd_display + 2)
	sub.w	r6,r6
	mov.b	r6l, @circle_counter
	rts
	
	/* dot or dot_inv*/
1:	mov.b	r6h, @dot_counter
	mov.w	#(lcd_display+6), r6
	bclr	#4, @r6
	inc	r6l
	bclr	#0, @r6
	bclr	#4, @r6
	inc	r6l
	bclr	#0, @r6
	bclr	#4, @r6
	sub.w	r6,r6
	rts

	. = 0x1fb6
read_buttons:
	add.b	#0, r6l
	addx	#256-0x30, r6h
	bne	1f
	push	r5
	mov.b	@(_PORT7:8),r5h
	bld	#6, r5h
	bist	#1, r6l
	bld	#7, r5h
	bist	#2, r6l
	bld	#2, @(_PORT4:8)
	bist	#0, r6l
	mov.w	@(4:16,r7), r5
	mov.w	r6, @r5
	pop	r5
1:	sub.w	r6,r6
	rts

	. = 0x1ff2
lcd_number:
	cmp.b	#0x30, r6h
	beq	1f
lcd_num_ret:	
	sub.w	r6, r6
	rts
1:
	cmp.b	#0x01, r6l
	beq	1f
	cmp.b	#0x1f, r6l
	beq	1f
	cmp.b	#0x17, r6l
	bne	lcd_num_ret
	jmp	lcd_num_digit

1:	
	push	r0
	push	r1
	push	r2
	push	r3
	push	r4
	push	r5
	mov.w	r6,r4
	mov.w	@(14:16,r7),r6	/* counter */
	mov.w	@(16:16,r7),r3  /* comma */
	cmp.b	#0x30, r3h
	bne	1f
	cmp.b	#0x06, r3l
	bcs	2f
1:	sub.w	r3,r3
2:	mov.b	r3l, r4h
	cmp.b	#0x01, r4l
	bne	1f
	mov.w	r6, r6
	bpl	1f
	not.b	r6l
	not.b	r6h
	adds	#1,r6
	mov.b	#0x00, r4l
1:	mov.w   #9999, r1
	cmp.w	r1, r6
	bls	1f
	mov.w	r1, r6
1:
	mov.b   @(lcd_display + 0), r0h
	mov.b   @(lcd_display + 1), r0l
	mov.b	@(lcd_display + 3), r1h
	mov.b	@(lcd_display + 4), r1l
	mov.b	@(lcd_display + 5), r2h
	mov.b	@(lcd_display + 6), r2l
	mov.b	@(lcd_display + 7), r3h
	mov.b	@(lcd_display + 8), r3l
	and.b	#0x4f, r0h
	and.b	#0xf4, r0l
	and.b	#0x44, r1h
	and.b	#0xf5, r1l
	and.b	#0x15, r2h
	and.b	#0x1f, r2l
	and.b	#0x15, r3h
	and.b	#0x15, r3l

	mov.b	r6h, r5l
	mov.b   #0, r5h
	mov.b	#10, r6h
	divxu.b	r6h, r5     /* divide 0:r6h / 10 -> r5h, r5l */
	mov.b	r5h, r6h
	mov.b	#10, r5h
	divxu.b r5h, r6     /* divide high:r6l / 10 -> r6h, r6l */
	mov.b   r5l, r5h
	mov.b   r6h, r5l
	mov.b   r5h, r6h
	mov.b   #0, r5h
	
	mov.b	@(lcd_num_dig2mask, r5), r5l
	bld	#2, r5l
	bst	  #4, r0h
	bld	#0, r5l
	bst	  #5, r0h
	bld	#5, r5l
	bst	  #7, r0h
	bld	#1, r5l
	bst	  #1, r1l
	bld	#6, r5l
	bst	  #3, r1l
	bld	#3, r5l
	bst	  #5, r2h
	bld	#4, r5l
	bst	  #7, r2h

	cmp.b	#2, r4h    /* check dot */
	bhi	1f         /* dot will follow, we must print this digit */
	mov.w	r6, r6
	bne	1f         /* more digits we have to print */
	cmp.b   #1, r4l    /* check for sign */
	bhi     1f         /* unsigned : print it */
	bst	#1, r0l    /* Now counter sign is in carry */
	jmp	lcd_num_finish

1:	mov.b	#10, r5l
	divxu.b r5l, r6
	mov.b	r6h, r5l
	mov.b	#0, r6h
	mov.b	@(lcd_num_dig2mask, r5), r5l
	bld	#2, r5l
	bst	  #0, r0l
	bld	#0, r5l
	bst	  #1, r0l
	bld	#5, r5l
	bst	  #3, r0l
	bld	#1, r5l
	bst	  #1, r2h
	bld	#6, r5l
	bst	  #3, r2h
	bld	#3, r5l
	bst	  #5, r2l
	bld	#4, r5l
	bst	  #7, r2l

	cmp.b	#3, r4h    /* check dot */
	bhi	1f
	bist	#6, r2h    /* r4h == 3 -> nocarry -> set dot */
	mov.b	r6l, r6l
	bne	1f         /* more digits we have to print */
	cmp.b   #1, r4l    /* check for sign */
	bhi     1f         /* unsigned : print it */
	bst	#1, r1h    /* Now counter sign is in carry */
	bra	lcd_num_finish
	
1:	mov.b	#10, r5l
	divxu.b r5l, r6
	mov.b	r6h, r5l
	mov.b	#0, r6h
	mov.b	@(lcd_num_dig2mask, r5), r5l
	bld	#2, r5l
	bst	  #0, r1h
	bld	#0, r5l
	bst	  #1, r1h
	bld	#5, r5l
	bst	  #3, r1h
	bld	#1, r5l
	bst	  #1, r3l
	bld	#6, r5l
	bst	  #3, r3l
	bld	#3, r5l
	bst	  #5, r3l
	bld	#4, r5l
	bst	  #7, r3l

	cmp.b	#4, r4h    /* check dot */
	bhi	1f
	bist	#6, r2l    /* r4h == 4 -> nocarry -> set dot */
	mov.b	r6l, r6l
	bne	1f         /* more digits we have to print */
	cmp.b   #1, r4l    /* check for sign */
	bhi     1f         /* unsigned : print it */
	bst	#5, r1h    /* Now counter sign is in carry */
	bra	lcd_num_finish
	
1:	mov.b	@(lcd_num_dig2mask, r6), r5l
	bld	#2, r5l
	bst	  #4, r1h
	bld	#0, r5l
	bst	  #5, r1h
	bld	#5, r5l
	bst	  #7, r1h
	bld	#1, r5l
	bst	  #1, r3h
	bld	#6, r5l
	bst	  #3, r3h
	bld	#3, r5l
	bst	  #5, r3h
	bld	#4, r5l
	bst	  #7, r3h


	cmp.b	#5, r4h    /* check dot */
	bne	1f
	bset	#6, r3l    /* r4h == 4 -> nocarry -> set dot */
1:	cmp.b   #1, r4l    /* check for sign */
	bst	#6, r3h    /* Now counter sign is in carry */

lcd_num_finish:
	mov.b   r0h, @(lcd_display + 0)
	mov.b   r0l, @(lcd_display + 1)
	mov.b	r1h, @(lcd_display + 3)
	mov.b	r1l, @(lcd_display + 4)
	mov.b	r2h, @(lcd_display + 5)
	mov.b	r2l, @(lcd_display + 6)
	mov.b	r3h, @(lcd_display + 7)
	mov.b	r3l, @(lcd_display + 8)
	pop	r5
	pop	r4
	pop	r3
	pop	r2
	pop	r1
	pop	r0
	sub.w	r6,r6
	rts

lcd_num_digit:
	mov.w	@(2:16,r7),r6	/* counter */
	cmp.b	#10, r6l
	bcc     1f
	mov	r6h, r6h
	bne	1f
	mov.b	@(lcd_num_dig2mask, r6), r6l

	push	r0
	mov.b	@(lcd_display+1), r0h
	mov.b	@(lcd_display+2), r0l
	mov.b	@(lcd_display+4), r6h
	and.b	#0x4f,r0h
	and.b	#0x5f,r0l
	and.b	#0x5f,r6h
	bld	#2, r6l
	bst	  #4, r0h
	bld	#0, r6l
	bst	  #5, r0h
	bld	#5, r6l
	bst	  #7, r0h
	bld	#1, r6l
	bst	  #5, r0l
	bld	#6, r6l
	bst	  #7, r0l
	bld	#3, r6l
	bst	  #5, r6h
	bld	#4, r6l
	bst	  #7, r6h
	mov.b	r0h, @(lcd_display+1)
	mov.b	r0l, @(lcd_display+2)
	mov.b	r6h, @(lcd_display+4)
	pop	r0

1:	sub.w	r6,r6
	rts
	
lcd_num_dig2mask:
	.byte	0x7e, 0x42, 0x37, 0x67, 0x4b
	.byte	0x6d, 0x7d, 0x46, 0x7f, 0x6f




	. = 0x27ac
lcd_clear:
	/* clear lcd area from 0xef43 to 0xef4c (10 bytes) */
	push	r5
	mov.w	#0xef4d, r5
	mov.w	#10, r6
1:	mov.b	r6h, @-r5
	dec	r6l
	bne	1b
	pop	r5
	rts

	. = 0x27c8
lcd_refresh:
	/* refresh lcd display:	 add header before lcd area and call i2c */
	push	r5
	mov.w	#lcd_display, r6
	mov.b	#0x00, r5l  /* LCD_LOAD_DATAPTR 0 */
	mov.b	r5l, @-r6
	mov.b	#0xe0, r5l  /* LCD_LOAD_SUBADDR 0 | I2C_MORE_DATA */
	mov.b	r5l, @-r6
	mov.b	#0xc8, r5l  /* LCD_MODE_SET | LCD_ENABLE | I2C_MORE_DATA */
	mov.b	r5l, @-r6
	mov.b	#0x7c, r5l  /* LCD_DEV_ID | I2C_WRITE */
	mov.b	r5l, @-r6

	mov.b	#13, r5h    /* number of bytes to transfer */
	bsr	write_lcd_output
	sub.b	r6l, r6l
	pop	r5
	rts

	. = 0x27f4
reset_buttons_and_irq:
	mov.b	@PORT4_DDR_SHADOW:16, r6l   /* make irq0 pin input. (PWR BTN)*/
	bclr    #2, r6l
	mov.b	r6l, @PORT4_DDR_SHADOW:16
	mov.b	r6l, @_PORT4_DDR:8

	bset	#0, @_ISCR:8
	bset	#0, @_IER:8
	
	mov.b	@PORT6_DDR_SHADOW:16, r6l
	or.b	#0x60, r6l
	mov.b	r6l, @_PORT6_DDR:8
	mov.b	r6l, @PORT6_DDR_SHADOW:16
	rts
	

write_lcd_output:
	/*  r6: pointer to output i2c data  */
	/* r5h: number of bytes to transfer */
	/* NOT COMPATIBLE with original function at 0x283c */

/* #define	DELAY   /* XXX Do we need a delay? - brickemu certainly does not. */
.macro DELAY
.endm

	mov.b	@PORT6_DDR_SHADOW:16, r5l
	or.b	#0x60, r5l
	mov.b	r5l, @_PORT6_DDR:8
	mov.b	r5l, @PORT6_DDR_SHADOW:16

	bset	#6, @_PORT6:8        /* I2C START */
	bset	#5, @_PORT6:8
	bclr	#6, @_PORT6:8
	DELAY
	bclr	#5, @_PORT6:8

2:	mov.b	@r6+, r5l
	shll	r5l                /* get first bit */
	or.b	#1, r5l            /* put marker bit for emptiness test */
1:	bst	#6, @_PORT6:8      /* set i2c data to bit */
	DELAY
	bset	#5, @_PORT6:8      /* set i2c clock */
	DELAY
	bclr	#5, @_PORT6:8      /* clear i2c clock */
	shll	r5l                /* get next bit */
	bne	1b                 /* repeat until marker bit removed */

	mov.b	@PORT6_DDR_SHADOW:16, r5l
	bclr	#6, r5l            /* set i2c to read */
	mov.b	r5l, @_PORT6_DDR:8
	bset	#5, @_PORT6:8      /* set i2c clock */
	DELAY
	/* we do not read acknowledge bit, though */
	bclr	#5, @_PORT6:8      /* clear i2c clock */
	bset	#6, r5l
	mov.b	r5l, @_PORT6_DDR:8
	dec	r5h
	bne	2b

	bclr	#6, @_PORT6:8        /* I2C END */
	DELAY
	bset	#5, @_PORT6:8
	DELAY
	bset	#6, @_PORT6:8
	rts
