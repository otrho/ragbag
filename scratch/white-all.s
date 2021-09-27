
@{{BLOCK(white_all)

@=======================================================================
@
@	white_all, 64x64@4, 
@	+ palette 256 entries, not compressed
@	+ 64 tiles not compressed
@	Total size: 512 + 2048 = 2560
@
@	Time-stamp: 2021-09-27, 20:02:36
@	Exported by Cearn's GBA Image Transmogrifier, v0.8.16
@	( http://www.coranac.com/projects/#grit )
@
@=======================================================================

	.section .rodata
	.align	2
	.global white_allTiles		@ 2048 unsigned chars
	.hidden white_allTiles
white_allTiles:
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x11111111
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x11111111
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x11111111
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x11111111
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x11111111

	.word 0x11111111,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x11111111,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001
	.word 0x11111111,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x11111111,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001
	.word 0x10000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x11111111,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000

	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0x11000000,0x11100000,0x11100000
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000001,0x00000011,0x00000111,0x00000111
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x11111111
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x11111111
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x11111111
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x11111111

	.word 0x11100000,0x11000000,0x11000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x00000111,0x00000011,0x00000011,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001
	.word 0x11111111,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x11111111,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x11111111,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x11111111,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001

	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x10000000,0x11100000,0x11111111
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000011,0x00000111,0x00001111
	.word 0x11111111,0x00000001,0x01110001,0x00001001,0x00000001,0x11111001,0x10100001,0x00000001
	.word 0x11111111,0x10000000,0x10001100,0x10010000,0x10100000,0x10000110,0x10011010,0x10000000
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x11111111
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x11111111
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x11111111
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000001

	.word 0x11111111,0x11100000,0x10000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x00001111,0x00000111,0x00000011,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x00000001,0x10000001,0x10010001,0x00110001,0x01100001,0x11000001,0x00000001,0x11111111
	.word 0x10000001,0x10000001,0x10001000,0x10001000,0x10000110,0x10000001,0x10000000,0x11111111
	.word 0x11111111,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x11111111,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x11111111,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001

	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x11000000,0x11000000,0x11100000
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000011,0x00000011,0x00000111
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x11000000,0x11100000,0x11110000
	.word 0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000001,0x00000111,0x11111111
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x11111111
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x11111111
	.word 0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001

	.word 0x11100000,0x11100000,0x11000000,0x10000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x00000111,0x00000111,0x00000011,0x00000001,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x11110000,0x11100000,0x11000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x11111111,0x00000111,0x00000001,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000,0x10000000
	.word 0x11111111,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001,0x00000001
	.word 0x11111111,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
	.word 0x00000001,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000

	.section .rodata
	.align	2
	.global white_allPal		@ 512 unsigned chars
	.hidden white_allPal
white_allPal:
	.hword 0x0000,0x7FFF,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000
	.hword 0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000

@}}BLOCK(white_all)
