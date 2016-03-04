# CS 61C Summer 2015 Project 2-2 
# string.s

#==============================================================================
#                              Project 2-2 Part 1
#                               String README
#==============================================================================
# In this file you will be implementing some utilities for manipulating strings.
# The functions you need to implement are:
#  - strlen()
#  - strncpy()
#  - copy_of_str()
# Test cases are in linker-tests/test_string.s
#==============================================================================

.data
newline:	.asciiz "\n"
tab:	.asciiz "\t"

.text
#------------------------------------------------------------------------------
# function strlen()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string input
#
# Returns: the length of the string
#------------------------------------------------------------------------------
strlen:
	# YOUR CODE HERE
	li	$t0, 0		# Counter initialized
strlen_loop:	
	lb	$t1, 0($a0)	# Load character
	beqz	$t1, strlen_end	# Check if character is null
	addi	$a0, $a0, 1	# Increment string pointer
	addi 	$t0, $t0, 1	# Increment counter
	j	strlen_loop	# Repeat loop (this is non-null case)
strlen_end:	
	addu	$v0, $t0, $0	# Load counter into return register
	jr 	$ra		# Return control

#------------------------------------------------------------------------------
# function strncpy()
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = pointer to destination array
#  $a1 = source string
#  $a2 = number of characters to copy
#
# Returns: the destination array
#------------------------------------------------------------------------------
strncpy:
	# YOUR CODE HERE
	addu	$t0, $0, $0		# Initialize counter
	addu	$v0, $a0, $0		# Store return address
strncpy_loop:
	beq	$t0, $a2, strncpy_check
	lb	$t1, 0($a1)		# Load character from source
	sb	$t1, 0($a0)		# Store said character
	addi	$a0, $a0, 1		# Increment destination string 
	addi	$a1, $a1, 1		# Increment source string
	addi	$t0, $t0, 1		# Increment counter
	j	strncpy_loop
strncpy_check:
	addiu	$a0, $a0, 1		# Increment destination array 
	sb	$0, 0($a0)		# Null-terminate string
strncpy_done:
	jr	$ra			# Return control

#------------------------------------------------------------------------------
# function copy_of_str()
#------------------------------------------------------------------------------
# Creates a copy of a string. You will need to use sbrk (syscall 9) to allocate
# space for the string. strlen() and strncpy() will be helpful for this function.
# In MARS, to malloc memory use the sbrk syscall (syscall 9). See help for details.
#
# Arguments:
#   $a0 = string to copy
#
# Returns: pointer to the copy of the string
#------------------------------------------------------------------------------
copy_of_str:			
	# YOUR CODE HERE
	addu	$sp, $sp, -12	# Begin copy_of_str
	sw	$ra, 0($sp)	# Store $ra on stack
	sw	$s0, 4($sp)	# Store $s0 on stack
	sw	$s1, 8($sp)	# Store $s1 on stack
	addu	$s0, $a0, $0	# Store string address into $s0
	jal	strlen		# Calculate strlen of string
	addu	$s1, $v0, $0	# Store strlen into $s1
	li	$v0, 9		# Load malloc syscall (9)
	addu	$a0, $s1, $0	# Store strlen into argument register (number of bytes needed)
	syscall			# Malloc
	addu	$a0, $v0, $0	# Load malloc pointer into argument register
	addu	$a1, $s0, $0	# Load string into argument register
	addu	$a2, $s1, $0	# Load strlen into argument register
	jal	strncpy		# $v0 contains address to new string
	lw	$ra, 0($sp)	# Reload saved registers
	lw	$s0, 4($sp)	# ""
	lw	$s1, 8($sp)	# ""
	addiu	$sp, $sp, 12 	# Restore stack
	jr $ra			# Restore control

###############################################################################
#                 DO NOT MODIFY ANYTHING BELOW THIS POINT                       
###############################################################################

#------------------------------------------------------------------------------
# function streq() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Arguments:
#  $a0 = string 1
#  $a1 = string 2
#
# Returns: 0 if string 1 and string 2 are equal, -1 if they are not equal
#------------------------------------------------------------------------------
streq:
	beq $a0, $0, streq_false	# Begin streq()
	beq $a1, $0, streq_false
streq_loop:
	lb $t0, 0($a0)
	lb $t1, 0($a1)
	addiu $a0, $a0, 1
	addiu $a1, $a1, 1
	bne $t0, $t1, streq_false
	beq $t0, $0, streq_true
	j streq_loop
streq_true:
	li $v0, 0
	jr $ra
streq_false:
	li $v0, -1
	jr $ra			# End streq()

#------------------------------------------------------------------------------
# function dec_to_str() - DO NOT MODIFY THIS FUNCTION
#------------------------------------------------------------------------------
# Convert a number to its unsigned decimal integer string representation, eg.
# 35 => "35", 1024 => "1024". 
#
# Arguments:
#  $a0 = int to write
#  $a1 = character buffer to write into
#
# Returns: the number of digits written
#------------------------------------------------------------------------------
dec_to_str:
	li $t0, 10			# Begin dec_to_str()
	li $v0, 0
dec_to_str_largest_divisor:
	div $a0, $t0
	mflo $t1		# Quotient
	beq $t1, $0, dec_to_str_next
	mul $t0, $t0, 10
	j dec_to_str_largest_divisor
dec_to_str_next:
	mfhi $t2		# Remainder
dec_to_str_write:
	div $t0, $t0, 10	# Largest divisible amount
	div $t2, $t0
	mflo $t3		# extract digit to write
	addiu $t3, $t3, 48	# convert num -> ASCII
	sb $t3, 0($a1)
	addiu $a1, $a1, 1
	addiu $v0, $v0, 1
	mfhi $t2		# setup for next round
	bne $t2, $0, dec_to_str_write
	jr $ra			# End dec_to_str()
