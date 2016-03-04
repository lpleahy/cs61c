Main:	addiu $s0, $0, 5			# $s0 = 5
		addi  $s1, $s0, 1			# $s1 = 6
		add   $s2, $s0, $s1			# $s2 = 11
		addu  $s0, $s0, $s2     	# $s0 = 16
		and   $s2, $s0, $s1			# $s2 = 0
		addiu $s1, $0, 16 			# $s1 = 16
		and   $s2, $s0, $s1			# $s2 = 16
		addiu $s1, $0, 5			# $s1 = 5
		andi  $s2, $s1, 4			# $s2 = 4
		beq   $s1, $s2, Main 		# No jump
		addiu $s0, $0, 5
		beqal $s0, $s1, Loop  		# Jump to Loop, $ra = jump instr. (next line)
		j 	  Other	  				# Jump Other
Loop:	subu  $s0, $s1, $s2			# $s0 = 1
		sub   $s1, $s2, $s0			# $s1 = 3
		addiu $s2, $0, $0
		bne	  $s0, $s1, Loop    	# Jump Loop once then they are equal 
		j     $ra					# Jump to beqal line

Other:	addiu $s0, $0, 4
		addiu $s1, $0, 9
		addiu $s2, $0, 67306520577	
		sw	  $s0, 0($0)			# Stored 4 at 0 (word)
		sh    $s2, 4($0)			# Stored 1 at 4 (half word)
		sb    $s1, 6($0)			# Stored 9 at 6 (byte)
		sb    $s1, 8($0)			# Stored 9 at 8 (byte)
		multu 
		divu  $1, $s0				# 2 with remainder 1
		lw	  $s2, 0($0)			# $s2 = 4
		lhu	  $s2, 4($0)			# $s2 = 1
		lhu	  $s2, 6($0)			# $s2 = 589833
		lb    $s2, 6($0)			# $s2 = 9
		lui   $s2, 1				# $s2 = 65536
