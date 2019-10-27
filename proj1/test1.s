# Short test case for your project.
#
# Testing LUI, SW, LW, SLL, and SRL for correctness


		.text	
_start:
		addiu	$t0, $0, 1
		addiu	$t1, $0, 2
		addiu	$t2, $0, 3
		lui	$t0, 0x0040
		addiu	$t0, $t0, 0x1000 # Ox00401000 is outside of code
		sw	$t1, 0($t0)
		lw	$t2, 0($t0)
		sll	$t2, $t2, 4
		srl	$t2, $t2, 4
		addiu	$0, $0, 0	# Terminate