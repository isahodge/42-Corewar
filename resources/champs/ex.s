.name "zork"
.comment "just a basic living prog"

#	ld %-5,r1
#	aff r15
l2:	sti	r1,%:live,%0
	and	r1,%0,r1
live:	live	%1
	zjmp	%:live
