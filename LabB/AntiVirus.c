#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <byteswap.h>


typedef struct virus {
unsigned short SigSize;
char virusName[16];
unsigned char* sig;
} virus;

typedef struct link link;
struct link {
link *nextVirus;
virus *vir;
};

struct fun_desc {
    char *name;
    link * (*fun)(link*, const char*);
}; 

virus* readVirus(FILE* file);
void printVirus(virus* virus, FILE* output);
void list_print(link *virus_list, FILE* output);
link* list_append(link* virus_list, virus* data);
link* creatList(virus* virus);
void list_free(link *virus_list);
void printMenu(struct fun_desc menu[]);
link * loadSignatures(link *virus_list, const char *name);
link * printSignatures(link *virus_list, const char *fileName);
link * detectViruses(link *virus_list, const char *fileName);
link * fixFile(link *virus_list, const char *fileName);
link * quit(link *virus_list, const char *fileName);
void detect_virus(char *buffer, unsigned int size, link *virus_list);
void neutralize_virus(const char *fileName, int signatureOffset);

int main(int argc, char const *argv[])
{

    struct fun_desc menu[] = {{"Load signatures", loadSignatures},
                            {"Print signatures", printSignatures},
                            {"Detect viruses", detectViruses},
                            {"Fix file", fixFile},
                            {"Quit", quit}};

    link *virus_list = NULL;
    char *line = malloc(sizeof(int));
    printf("Please choose a function:\n");
    printMenu(menu);
    while (1)
    {
        fgets(line, 10, stdin);
        int number = atoi(line);

        if (number < 0 || number > 5) // bound checking.
        {
            printf("Not within bounds\n");
            break;
        }
        if (number == 5)
        {
            free(line);
        }
        
        virus_list = menu[number-1].fun(virus_list, argv[1]);

        printf("Please choose a function:\n");
        printMenu(menu);
    }
    free(line);
}

/**
 * This function reads a virus from a file.
 * note that I assume I allready check the 4-magic-bytes.
*/
virus* readVirus(FILE* file){
    virus *myVirus = (virus*)malloc(sizeof(virus));
    int succeed = fread(myVirus, 1 , 18, file);
    if(succeed == 0){
        free(myVirus);
        return NULL;
    }
    myVirus->sig = (unsigned char*)malloc(myVirus->SigSize);
    succeed = fread(myVirus->sig, 1, myVirus->SigSize, file);
    if(succeed == 0){
        free(myVirus->sig);
        free(myVirus);
        return NULL;
    }

    return myVirus;
    
}
/**
 * This function receives a virus and a pointer to an output file.
 * The function prints the virus to the given output. 
 * It prints the virus name (in ASCII), the virus signature length (in decimal), and the virus signature (in hexadecimal representation). 
*/
void printVirus(virus* virus, FILE* output){
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output, "signature:");
    for (size_t i = 0; i < virus->SigSize; i++)
    {
        if (i % 20 == 0)
        {
            fprintf(output, "\n");
            fprintf(output, "%02X", virus->sig[i]);
        }
        else{
            fprintf(output, " %02X", virus->sig[i]);
        }
        

    }
    fprintf(output, "\n\n");
}

/**
 * DESTRUCTOR FOR VIRUS TYPE.
*/
void freeVirus(virus* virus){
    free(virus->sig);
    free(virus);
}
/**
 * Print the data of every link in list to the given stream. Each item followed by a newline character. 
*/
void list_print(link *virus_list, FILE* output){
    if (virus_list != NULL)
    {
        printVirus(virus_list->vir, output);
        list_print(virus_list->nextVirus, output);
    }
}
/**
 * Add a new link with the given data to the beginning of the and return a pointer to the list, 
 * or if the list is null - create a new list and return a pointer to it.
*/
link* list_append(link* virus_list, virus* data){
    link *newLink = (link *)malloc(sizeof(link));
    newLink->vir = data;
    if (virus_list == NULL)
    {
        newLink->nextVirus = NULL;
    }
    else{
        newLink->nextVirus = virus_list;
    }
    return newLink;
}

/**
 * Creat new virus list.
*/
link* creatList(virus* virus){
    return list_append(NULL, virus);
}

/**
 * Free the memory allocated by the list
*/
void list_free(link *virus_list){
    if(virus_list != NULL){
        freeVirus(virus_list->vir);
        list_free(virus_list->nextVirus);
        free(virus_list);
    }
}
/**
 * print the menu.
*/
void printMenu(struct fun_desc menu[]){
  for (size_t i = 0; i < 5; i++)
  {
    printf("%d.%s\n", (i+1), menu[i].name);
  }
}

/**
 * read signature from file, store them in linked-list and retureit
 * */
link * loadSignatures(link *virus_list, const char *name){
    printf("Please enter the signature file name:\n");
    char *fileName = (char*)(malloc(100*sizeof(char)));
    fgets(fileName, 100, stdin);   // get file name from the user.

    if (fileName[strlen(fileName)-1] == '\n'){ // correcting the string, fgets return a string ended with \n, we remove it.
        fileName[strlen(fileName)-1] = '\0';
    }
    FILE *input = fopen(fileName, "r"); // OPEN THE FILE.
    free(fileName);;

    if(input == NULL){
        printf("Error!\n");
        quit(virus_list, fileName);

    }

    char* c = (char *)malloc(5);
    fread(c, 4, 1, input);
    c[4] = '\0';
    if(c != NULL && strcmp(c, "VIRL") != 0 && strcmp(c, "VISL") != 0){
        fprintf(stderr, "Error!\n");
        free(c);      
        exit(0);
    }
    free(c);       

    virus *myVirus = readVirus(input);
    list_free(virus_list);
    virus_list = NULL;
    while (myVirus != NULL)
    {
        if (virus_list == NULL)
            virus_list = creatList(myVirus);
        else{
            virus_list = list_append(virus_list, myVirus);
        }
        myVirus = readVirus(input);
    }
    fclose(input);
    return virus_list;
}

/**
 * Check if the list is valid, i.e virus_list != null,
 * is so print it, else do nothing.
*/
link * printSignatures(link *virus_list, const char *fileName){
    if(virus_list){
        list_print(virus_list, stdout);
    }
    return virus_list;
}

/**
 * This function check if there is a viruses in a given file, if there is it print them.
*/
link * detectViruses(link *virus_list, const char *fileName){
    FILE *input = fopen(fileName, "r"); // OPEN THE FILE.
    char *buffer = (char*)malloc(10000);
    size_t bufferSize = fread(buffer, 1, 10000, input);
    detect_virus(buffer, bufferSize, virus_list);

    free(buffer);
    fclose(input);

    return virus_list;
}
/**
 * This function fix given file. 
*/
link * fixFile(link *virus_list, const char *fileName){
    FILE *input = fopen(fileName, "r"); // OPEN THE FILE.
    char *buffer = (char*)malloc(10000);
    size_t bufferSize = fread(buffer, 1, 10000, input);
    
    for (size_t i = 0; i < bufferSize; i++)
    {
        link* currentVirus = virus_list;
        while (currentVirus)
        {
            unsigned short virusSignatureSize = currentVirus->vir->SigSize;
            unsigned char* virusSignatue = currentVirus->vir->sig;

            if((i + virusSignatureSize < bufferSize)){  // we in range.
                if(memcmp(buffer + i, virusSignatue, virusSignatureSize) == 0){
                    fclose(input);
                    neutralize_virus(fileName, i);
                    input = fopen(fileName, "r");
                }
            }
            currentVirus = currentVirus->nextVirus;
        }
    }


    free(buffer);
    fclose(input);
    return virus_list;
}
/**
 * delete alocated memory and quit program.
*/
link * quit(link *virus_list, const char *fileName){
    if(virus_list){
        list_free(virus_list);
    }
    printf("Quiting..\n");
    exit(0);
}
/**
 * compare every byte sequence to all virus from virus list.
*/
void detect_virus(char *buffer, unsigned int size, link *virus_list){
    for (size_t i = 0; i < size; i++)
    {
        link* currentVirus = virus_list;
        while (currentVirus)
        {
            unsigned short virusSignatureSize = currentVirus->vir->SigSize;
            unsigned char* virusSignatue = currentVirus->vir->sig;
            char *virusName = currentVirus->vir->virusName;

            if((i + virusSignatureSize < size)){  // we in range.
                if(memcmp(buffer + i, virusSignatue, virusSignatureSize) == 0){
                    printf("The starting byte location: %d(0x%x)\nThe virus name: %s\nThe size of the virus signature: %d\n", i, i, virusName, virusSignatureSize);
                }
            }
            currentVirus = currentVirus->nextVirus;
        }

    }
    
}
/**
 * This function overwrite one byte at the signatureOffset location.
*/
void neutralize_virus(const char *fileName, int signatureOffset){
    FILE *file = fopen(fileName, "r+"); // open the file in read/write mode.
    fseek(file, signatureOffset, SEEK_SET);  // update the offset to the correct location.
    char c = 0xC3;
    fwrite(&c, 1, 1, file);
    fclose(file);
}