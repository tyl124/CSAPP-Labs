/* 3.62 */

typedef enum {MODE_A, MODE_B, MODE_C, MODE_D, MODE_E} mode_t;

long switch3(long *p1, long *p2, mode_t action){
    long result = 0;
    switch(action){
        case MODE_A:
            result = *p2;
            *p2 = *p1;
            break;

        case MODE_B:
            result = *p1;
            result += *p2;
            *p1 = result;
            break;

        case MODE_C:
            *p1 = 59;
            result = *p2;
            break;

        case MODE_D:
            result = *p2;
            *p1 = result;
            result = 27;
            break;

        case MODE_E:
            result = 27;
            break;

        default:
            result = 12;
            break;
    }

    return result;
}



// p1 in %rdi, p2 in %rsi, action in %edx
// result in %eax

.L8:                            # MODE_E
    movl    $27,    %eax
    ret
.L3:                            # MODE_A
    movq    (%rsi), %rax
    movq    (%rdi), %rdx
    movq    %rdx,   (%rsi)
    ret
.L5:                            # MODE_B
    movq    (%rdi), %rax
    addq    (%rsi), %rax
    movq    %rax,   (%rdi)
    ret
.L6:                            # MODE_C
    movq    $59,    (%rdi)
    movq    (%rsi), %rax
    ret
.L7:                            # MODE_D
    movq    (%rsi), %rax
    movq    %rax,   (%rdi)
    movl    $27,    %eax
    ret
.L9:                            # default
    movl    $12,    %eax
    ret
