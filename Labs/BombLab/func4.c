void func4(int eax, int ecx, int edx, int esi, int edi){
	ecx = (edx - esi) >> 31;
	eax = ((edx - esi) + (edx -esi) >> 31) >> 1;
	ecx = rax + rsi*1;
	if(ecx <= edi){
		eax = 0;
		if(ecx >= edi) // ecx == edi
			return;
		else{
				// ecx < edi
				esi = ecx + 1;
				func4();
				eax = 2*rax + 1;
			}
	}
	else{
		edx = rcx - 1;
		func4();
		eax = eax * 2;
		return;
	}
	return;
}






}
