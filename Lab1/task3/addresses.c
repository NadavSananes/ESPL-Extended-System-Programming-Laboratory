#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int addr5;  //BSS
int addr6;  //BSS
// Every variable that stored outside the main function is a global variable and will be store at the data segment.
// Every variable that is not initialize or is equal to zero and it is global/static will be stored at BSS.
// code is in the text segment.
int foo() //TEXT
{
    return -1;
}
void point_at(void *p);//TEXT
void foo1();//TEXT
char g = 'g'; //DATA
void foo2();//TEXT

int main(int argc, char **argv)
{ 
    int addr2;
    int addr3;
    char *yos = "ree";
    int *addr4 = (int *)(malloc(50));
    
    printf("Print addresses:\n");
    printf("- &addr2: %p\n", &addr2);
    printf("- &addr3: %p\n", &addr3);
    printf("- foo: %p\n", &foo);
    printf("- &addr5: %p\n", &addr5);

    printf("- argc %p\n", &argc);
    printf("- argv %p\n", argv);
    printf("- &argv %p\n", &argv);
    
    printf("Print distances:\n");
    point_at(&addr5);

    printf("Print more addresses:\n");
    printf("- &addr6: %p\n", &addr6);
    printf("- yos: %p\n", yos);
    printf("- gg: %p\n", &g);
    printf("- addr4: %p\n", addr4);
    printf("- &addr4: %p\n", &addr4);

    printf("- &foo1: %p\n", &foo1);
    printf("- &foo1: %p\n", &foo2);
    
    printf("Print another distance:\n");
    printf("- &foo2 - &foo1: %ld\n", (long) (&foo2 - &foo1));

   
    printf("Arrays Mem Layout (T1b):\n");
    int iarray[3];
    float farray[3];
    double darray[3];
    char carray[3]; 
    /* task 1 b here */
    printf("Task 1B HERE:\n");
    printf("iarray: %p\n", iarray);
    printf("iarray+1: %p\n", iarray+1);
    printf("farray: %p\n", farray);
    printf("farray+1: %p\n", farray+1);
    printf("darray: %p\n", darray);
    printf("darray+1: %p\n", darray+1);
    printf("carray: %p\n", carray);
    printf("carray+1: %p\n", carray+1);
    //int jump by 4, float 4, double 8 and char 1
    printf("the diffrencess is at size:%d, %d, %d, %d\n", sizeof(int), sizeof(float), sizeof(double), sizeof(char));
    
    printf("Pointers and arrays (T1d): ");
    int iarray2[] = {1,2,3};
    char carray2[] = {'a','b','c'};
    int* iarray2Ptr;
    char* carray2Ptr; 
    /* task 1 d here */
    printf("Task 1D\n");
    iarray2Ptr = iarray2;
    carray2Ptr = carray2;
    for (size_t i = 0; i < sizeof(iarray2) / sizeof(int); i++)
    {
        printf("%d", *(iarray2Ptr + i));
        printf("\n");
    }

    for (size_t i = 0; i < sizeof(carray2) / sizeof(char); i++)
    {
        printf("%c", *(carray2Ptr + i));
        printf("\n");
    }

    void *p;
    printf("%p", p);
    printf("\n");
    
    printf("Command line arg addresses (T1e):\n");
    /* task 1 e here */
    printf("Task 1E\n");
    for (size_t i = 0; i < argc; i++)
    {
        printf("address:%d and content:%s \n", &argv[i], argv[i]);
    }
    
    
    return 0;
}

void point_at(void *p)
{
    int local;  // STACK
    static int addr0 = 2; //DATA
    static int addr1;  // BSS

    long dist1 = (size_t)&addr6 - (size_t)p; // &add6 - &add5 = 4 => the size int
    long dist2 = (size_t)&local - (size_t)p; // local is in stack and &add5 is in BSS
    long dist3 = (size_t)&foo - (size_t)p;  // &foo is in data and &add7 in BSS

    printf("- dist1: (size_t)&addr6 - (size_t)p: %ld\n", dist1);
    printf("- dist2: (size_t)&local - (size_t)p: %ld\n", dist2);
    printf("- dist3: (size_t)&foo - (size_t)p:  %ld\n", dist3);
    
    printf("Check long type mem size (T1a):\n");
    /* part of task 1 a here */
    printf("Task 1A\n");
    printf("size of long: %ld\n", sizeof(long));

    printf("- addr0: %p\n", &addr0);
    printf("- addr1: %p\n", &addr1);
}

void foo1()
{
    printf("foo1\n");
}

void foo2()
{
    printf("foo2\n");
}
