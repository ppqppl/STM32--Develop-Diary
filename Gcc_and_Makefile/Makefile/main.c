#include <stdio.h>
#include <math.h>
#include "sub1.h"
#include "add1.h"

int main(){

    int a,b;
    scanf("%d %d",&a,&b);
    float c = x2x(a,b);
    printf("函数 x2x 结果 c 是：%.5f\n",c);
    int d = add1(a,b);
    printf("函数 add1 结果 d 是：%d",d);
    return 0;
}
