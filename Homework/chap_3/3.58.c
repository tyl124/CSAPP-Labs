/* 3.58 */

long decode2(long x, long y, long z);

decode2:
	subq	%rdx, %rsi		# y = y - z
	imulq	%rsi, %rdi		# x = x * y
	movq	%rsi, %rax		# temp = y
	salq	$63,  %rax		# temp = temp << 63
	sarq	$63,  %rax		# temp = temp >> 63
	xorq	%rdi, %rax		# temp = temp ^ x
	ret

long decode2(long x, long y, long z){
	long temp = y - z;
	temp = (temp << 63 >> 63) ^ (x * temp);

	return temp; 
}
