#include <stdio.h>
#include <stdlib.h>

int myDigitCounter(char *s){
    int ans = 0;
    int i = 0;
    while (1)
    {
        if(s[i] == '\0')
            break;
        if(s[i] >= '0' && s[i] <= '9')
            ans++;
        i++;
        
    }
    return ans;
}
int main(int argc, char const *argv[])
{
    // printf("The number of digits is:%d\n", myDigitCounter(argv[1]));
    return 0;
}
