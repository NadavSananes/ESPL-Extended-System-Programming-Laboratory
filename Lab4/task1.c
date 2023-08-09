#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>


/**
 * state struct.
*/
typedef struct {
  char debug_mode;
  char file_name[128];
  int unit_size;
  unsigned char mem_buf[10000];
  size_t mem_count;
  char display_mode;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} state;

void printMenu();
state* createState();
void debugPrint(state* s);
void toggleDebugMode(state* s);
void setFileName(state* s);
void setUnitSize(state* s);
void loadIntoMemory(state* s);
void toggleDisplayMode(state* s);
void memoryDisplay(state* s);
void saveIntoFile(state* s);
void memoryModify(state* s);
void quit(state* s);

struct fun_desc {
    char *name;
    void (*fun)(state* s); // Getting the state as a pointer allows the functions to change it.    
}; 

struct fun_desc menu[] = {{"Toggle Debug Mode", &toggleDebugMode}, //0
                          {"Set File Name", &setFileName}, //1
                          {"Set Unit Size", &setUnitSize}, //2
                          {"Load Into Memory", &loadIntoMemory}, //3
                          {"Toggle Display Mode", &toggleDisplayMode}, //4
                          {"Memory Display", &memoryDisplay}, //5
                          {"Save Into File", &saveIntoFile}, //6
                          {"Memory Modify", &memoryModify}, //7
                          {"Quit", &quit}, //8
                          {NULL, NULL}  //9
                          };

static char* hex_formats[] = {"No such unit", "%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
static char* dec_formats[] = {"No such unit", "%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};



int main(int argc, char const *argv[])
{   
    state* current_state = createState();

    // infinite loop.
    while (1)
    {
        debugPrint(current_state);
        printMenu();  // print menu.
        char *line = malloc(sizeof(int)); 
        if(fgets(line,4, stdin) != 0){ // get user choise.
            int index = atoi(line);
            free(line);
            if(index > 8 || index < 0){ // check if input is in range.
                printf("Error! out of bounce.\n");
            }
            else{
                menu[index].fun(current_state);
            }

        }

    }
    

    return 0;
}

/**
 * print the menu to the user.
*/
void printMenu(){
    int i = 0;
    while (1)
    {
        if(menu[i].name == NULL)
            break;
        else{
            printf("%d-%s\n", i, menu[i].name);
        }
        i++;
    }
    
    // printf("0-Toggle Debug Mode\n1-Set File Name\n2-Set Unit Size\n3-Load Into Memory\n4-Toggle Display Mode\n5-Memory Display\n6-Save Into File\n7-Memory Modify\n8-Quit\n");
}
/**
 * if debug mode is ok, print the variables: unit_size, file_name, and mem_count.
*/
void debugPrint(state* s){
    if(s->debug_mode == 1){
        printf("unit_size: %d\nfile_name: %s\nmem_count:%d\n", s->unit_size, s->file_name, s->mem_count);
    }
}
/**
 * creat new state, initilaize all state variable.
*/
state* createState(){
    state* s = (state*)malloc(sizeof(state));
    s->debug_mode = 0;
    s->display_mode = 0;
    s->file_name[0] = 0; // first char is '\0'.
    // s->mem_count[0] = 0;
    s->unit_size = 1;   // default unit size.
    return s;
}
/**
 * print given length unit size bytes, from our memory buffer.
*/
void printFromBuffer(state* s, int length){
    if (s->display_mode == 1)
    {
        printf("Hexadecimal:\n");
        for (size_t i = 0; i < length; i++)
        {
            printf(hex_formats[s->unit_size], *((int*)(s->mem_buf + i * s->unit_size)));
        }
        
    }
    else{
        printf("Decimal:\n");
        for (size_t i = 0; i < length; i++)
        {
            printf(dec_formats[s->unit_size], *((int*)(s->mem_buf + i * s->unit_size)));
        }
    }
    
}
/**
 * print given length unit size bytes, from memory start at addres location.
*/
void printFromMemory(state* s, int length, int location){

    if (s->display_mode == 1)
    {
        printf("Hexadecimal:\n");
        for (size_t i = 0; i < length; i++)
        {
            printf(hex_formats[s->unit_size], *((int*)(location + i * s->unit_size)));
        }
    }
    else{
        printf("Decimal:\n");
        for (size_t i = 0; i < length; i++)
        {
            printf(dec_formats[s->unit_size], *((int*)(location + i * s->unit_size)));
        }
    }
}
/**
 * tooggle debug mode (on/off).
*/
void toggleDebugMode(state* s){
    if(s->debug_mode == 1){
        s->debug_mode = 0;
        printf("Debug flag now off\n");
    }
    else{
        s->debug_mode = 1;
        printf("Debug flag now on\n");
    }
}
/**
 * set file name to given file name from the user.
*/
void setFileName(state* s){
    printf("Please enter file name:\n");
    if(fgets(s->file_name, 100, stdin) != 0){

        if(s->file_name[strlen(s->file_name) - 1] == '\n')
            s->file_name[strlen(s->file_name) - 1] = '\0';

        if(s->debug_mode == 1)
            fprintf(stderr, "Debug: file name set to %s\n", s->file_name);
    }

}
/**
 * recive input from the user and set unit size.
*/
void setUnitSize(state* s){
    printf("Please enter a number:\n");
    char *line = malloc(sizeof(int)); 
        if(fgets(line, 4, stdin) != 0){ // get user choise.
            int num = atoi(line);
            if(num == 1 || num ==2 || num == 4){
                s->unit_size = num;
                fprintf(stderr, "Debug: set size to %d\n", num);
            }
            else{
                fprintf(stderr, "Error: number is not 1, 2 or 4, not: %d\n", num);
            }
        }
    free(line);
}
/**
 * load length bytes, from location memory address to buffer.
*/
void loadIntoMemory(state* s){
    if(strcmp(s->file_name, "") == 0){
        fprintf(stderr, "Error: not file name\n");
    }
    else{
        FILE* file = fopen(s->file_name, "r+");
        if(file == NULL)
        {
            fprintf(stderr, "Error: can not open file\n");
        }
        else{
            printf("Please enter location and length:\n");
            int location, length;

            char *line = malloc(100); 
            if(fgets(line, 100, stdin) != 0){ // get user choise.

                if(line[strlen(line) - 1] == '\n')
                    line[strlen(line) - 1] = '\0';

                sscanf(line, "%X %d", &location, &length);
                // printf("%x %d\n", location, length);
            }
            free(line);

            if(s->debug_mode == 1){
                fprintf(stderr, "file name: %s\nlocation: %X\nlength: %d\n", s->file_name, location, length);
            }

            if(fseek(file, location, SEEK_SET) == -1){
                fprintf(stderr, "Error: can not preform lseek\n");
            }
            else{
                if((s->mem_count = fread(s->mem_buf, s->unit_size, length, file)) <= 0){
                    fprintf(stderr, "Error: can not read to buffer\n");
                }
                else{
                    fprintf(stderr, "Loaded %d units into memory\n", length);
                }
            }
            fclose(file);

        }
        
    }
}
/**
 * toggle display mode, off/on.
*/
void toggleDisplayMode(state* s){
    if(s->display_mode == 0){
        s->display_mode = 1;
        fprintf(stderr, "Display flag now on, hexadecimal representation\n");
    }
    else{
        s->display_mode = 0;
        fprintf(stderr, "Display flag now off, decimal representation\n");
    }
}
/**
 * disply to stdout length*unit_size bytes from buffer/memory location.
*/
void memoryDisplay(state* s){
    printf("Please enter location and length:\n");
    int location, length;

    char *line = malloc(100); 
    if(fgets(line, 100, stdin) != 0){ // get user choise.
        if(line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        sscanf(line, "%X %d", &location, &length);
    }
    free(line);

    if(location == 0){
        printFromBuffer(s, length);
    }
    else{
        printFromMemory(s, length, location);
    }
}
/**
 * save length * unit_size byts from our buffer/sourse_adress to our file at offset of target location.
*/
void saveIntoFile(state* s){
    int source_address, target_location, length;
        char *line = malloc(100); 
        printf("Please enter <source_address> <target_location> <length>\n");
        if(fgets(line, 100, stdin) != 0){ // get user choise.

            if(line[strlen(line) - 1] == '\n')
                line[strlen(line) - 1] = '\0';

            sscanf(line, "%X %X %d", &source_address, &target_location, &length);
        }
        free(line);

        FILE* file = fopen(s->file_name, "r+");
        if(file == NULL){
            fprintf(stderr, "Error: can not open the file\n");
        }
        else{
            fseek(file, target_location, SEEK_SET);
            char* buffer;
            if(source_address == 0){
                buffer = (char*)s->mem_buf;
            }
            else{
                buffer = (char*)source_address;
            }

            fwrite(buffer, s->unit_size, length, file);
            fclose(file);   
        }


}
/**
 * modify our memory at the buffer by 1 unit at location with val.
*/
void memoryModify(state* s){
    int location, val;
    char *line = malloc(100); 
    printf("Please enter <location> <val>>\n");
    if(fgets(line, 100, stdin) != 0){ // get user choise.

        if(line[strlen(line) - 1] == '\n')
            line[strlen(line) - 1] = '\0';

        sscanf(line, "%X %X", &location, &val);
    }
    free(line);

    if(s->debug_mode == 1){
        fprintf(stderr, "Debug: location: %X, val: %X\n", location, val);
    }
    memcpy((void *)(s->mem_buf + location), (void*)&val, s->unit_size); // copy unit bytes from &val to our buffer at location.
    
}
/**
 * quit gracefully.
*/
void quit(state* s){
    fprintf(stderr, "quitting\n");
    free(s);
    exit(0);
}