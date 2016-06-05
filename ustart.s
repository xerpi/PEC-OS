	.data
foo:	.word 0xAABB
bar:	.word 0xCCDD

	.text
	.global _start
_start:
	movi r0, 0
	bz r0, _start
