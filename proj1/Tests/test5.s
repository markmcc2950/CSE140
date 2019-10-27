# Short test case for your project.
#
# Testing SUBU, AND, ANDI, including negative integer


		.text	
_start:
		addiu	$t0, $0, -1
		addiu	$t1, $0, 2
		subu	$t2, $t1, $t0
		and	$t3, $t2, $t1
		andi	$t4, $t0, 1
		
		