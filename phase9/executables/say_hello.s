# say_hello.s
#
# In order to MsgSnd(&my_msg)/MsgRcv(&my_msg), 1st calc DRAM addr of my_msg.
#
# As program runs, subtract 4096 from ESP to get where the DRAM starts.
# $my_msg is given location 2G+x (a virtual address given by gcc link386).
# Subtract 2G from $my_msg we get x.
# Add x to the actual beginning of DRAM, that's where my_msg really is.

.text                       # code segment
.global _start              # _start is main()

_start:

   popl %eax
   #movl  $5, 4(%eax)		# StdoutPID is 5 (this must match your own)
   				# set msg.recipient = 5 (4 bytes offset, .data)
   int   $54			# call MsgSnd(&my_msg)

   popl  %eax			# pop a copy of my_msg address
   #movl  $4, %eax
   int   $55			# call MsgRcv(&my_msg)

   popl  %ebx			# pop another copy of my_msg address
   movl  8(%ebx), %eax		# get my_msg.OS_clock (offset 8, .data) as exit code (eax)
   
   #phase 9
   movl $_start, %eax;

   int   $58			# call Exit(my_msg.OS_clock)

.data       # data segment always follows code segment in RAM
my_msg:     # my_msg (this must match your own msg_t)
	
	.long	0	#sender
	.long 	5	#recipient
	.long	0	#OS_clock
	
	.ascii "MY OS Team MIOS, How're you doing?\n"
   	.rept 73               # repeat: 101-28 = 73 times
      		.ascii "\0"         # filled with 93 NUL
   	.endr                  # end of repeat

	.long 	0	#code[0]
	.long 	0	#code[0]
	.long 	0	#code[0]




