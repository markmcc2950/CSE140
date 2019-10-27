# Short test case for your project.
#
# Testing invalid memory access for termination


		.text	
_start:
		addiu	$t0, $0, 1
		addiu	$t1, $0, 2
		addiu	$t2, $0, 3
		lui	$t0, 0x0040	
		sw	$t1, 0($t0)	# Will terminate early due to invalid memory access at 0x00400000
		lw	$t2, 0($t0)
		addiu	$0, $0, 0	# Terminate
