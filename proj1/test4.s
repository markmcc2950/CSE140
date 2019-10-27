# Short test case for your project.
#
# Testing OR, ORI, SLT, including negative integer


		.text	
_start:
		addiu	$t0, $0, 1
		addiu	$t1, $0, 2
		addiu	$t2, $0, -1
		or	$t3, $t1, $t2
		ori	$t4, $t1, 1
		slt	$t5, $t1, $t0
		slt	$t6, $t0, $t1