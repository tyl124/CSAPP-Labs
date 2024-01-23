/* 3.61 */

long cread(long *xp){
    return (xp ? *xp : 0);
}

/*
    改为编译时产生条件传送指令
    依赖于具体的gcc版本
*/

long cread_alt(long *xp){
    long rval = 0;
    if(xp != 0)
        rval = *xp;
    return rval;
}

long cread_alt2(long *xp){
    return (!xp ? 0 : *xp);
}
