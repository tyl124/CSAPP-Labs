/* c description of code in outdump.s from line 552 to 569 */
void func6(){
	esi = 0;
	ecx = *(rsp + rsi);	// 存储栈内数据，从栈顶开始
	while(rsi != 0x18)	// 0x18 = 16 + 8 = 24 = 4 * 6 
	{
		if(ecx > 1){
			eax = 1;
			edx = 0x6032d0;
			while(eax != ecx){
				rdx = *(rdx + 8);
				eax++;
			}
		}
		else{
			edx = 0x6032d0;
		}
		*(0x20 + rsp + rsi * 2) = rdx;
		rsi += 4;
		ecx = *(rsp + rsi);
	}
	return;
}
