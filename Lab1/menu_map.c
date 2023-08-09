#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char xprt(char c);
char my_get(char c);
char cprt(char c);
char encrypt(char c);
char decrypt(char c);

char* map(char *array, int array_length, char (*f) (char)){
  char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
  for (size_t i = 0; i < array_length; i++)
  {
    mapped_array[i] = f(array[i]);
  }
  
  return mapped_array;
}
 
/* xprt prints the value of c in a hexadecimal representation followed by a new line, and returns c unchanged. */ 
char xprt(char c)
{
  printf("%x\n", c);
  return c;
}

/* Ignores c, reads and returns a character from stdin using fgetc. */
char my_get(char c)
{
  return fgetc(stdin);
}

/* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns the value of c unchanged. */
char cprt(char c){
  if(c >= 0x20 && c<= 0x7E)
    printf("%c\n", c);
  else{
    printf("%c\n", '.');
  }
  return c;
}

/* Gets a char c and returns its encrypted form by adding 1 to its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char encrypt(char c){
  if(c >= 0x20 && c <= 0x7E)
    return c+1;

  return c;
}

/* Gets a char c and returns its decrypted form by reducing 1 from its value. If c is not between 0x20 and 0x7E it is returned unchanged */
char decrypt(char c){
  if(c >= 0x20 && c<= 0x7E)
    return c-1;

  return c;
}

struct fun_desc {
    char *name;
    char (*fun)(char);
}; 

struct fun_desc menu[] = {{"Get String", &my_get},
                            {"Print String", &cprt},
                            {"Encrypt", &encrypt},
                            {"Decrypt", &decrypt},
                            {"Print Hex", &xprt}};

void printMenu(){
  for (size_t i = 0; i < 5; i++)
  {
    printf("%d.%s\n", i, menu[i].name);
  }
  
}

int main(int argc, char const *argv[])
{ 
    char *carray = (char*)(malloc(5*sizeof(char)));
    char *line = malloc(sizeof(int));
    printf("Please choose a function (ctrl^D for exit):\n");
    printMenu();

    while(fgets(line,100, stdin) != 0){
      if(atoi(line) > 4 || atoi(line) < 0){
        printf("not Within bounds\nDONE.\n");
        break;
      }
      else{
        printf("Within bounds\n");
        char *carray2 = carray;
        carray = map(carray, 5, menu[atoi(line)].fun);
        free(carray2);
        printf("DONE.\n");
      }
      printf("Please choose a function (ctrl^D for exit):\n");
      printMenu();
    }
    free(line);
    free(carray);

}
