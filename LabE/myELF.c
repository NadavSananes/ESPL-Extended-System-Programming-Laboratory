#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <elf.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h> 


// struct to keep the fd and the pointer to mapped ELF.
typedef struct{
    int fd;
    char* map_start;
    int size;
    char file_name[128];
} ELFInfo;

// Global variables.
ELFInfo* firstELF = NULL;
ELFInfo* secondELF = NULL;
char debugMode = 0;

struct fun_desc {
    char *name;
    void (*fun)();
}; 

void printMenu();
void toggleDebugMode();
void quit();
void closeAndfreeELFInfo(ELFInfo* f);
void examineELFFile();
void printInformation(Elf32_Ehdr *elfFileMaped);
bool isValidELF(Elf32_Ehdr *elfFileMaped);
ELFInfo* initialELFInfo(int fd, char* map_start, int size, char* name);
void printSectionNames();
void printFileSectionNames(ELFInfo* file);
char* findSectionType(Elf32_Word type);
void printSymbols();
void filePrintSymbols(ELFInfo* file);
void CheckMerge();
Elf32_Shdr* findSymbolTable(ELFInfo* file);
Elf32_Sym* findSymbol(char* toFindName, ELFInfo* file);
void loopOverAndCheck(ELFInfo* one, ELFInfo* two);
void mergeELFFiles();
Elf32_Shdr* findSection(char* name, ELFInfo* file);
char* getSectionName(Elf32_Shdr* section, ELFInfo* file);
void concatAndAppendMergableSections(int new_file_fd, char* section_name, Elf32_Shdr* currentSection);
int getFileSize(int fd);


struct fun_desc menu[] = {{"Toggle Debug Mode", toggleDebugMode},
                            {"Examine ELF File", examineELFFile},
                            {"Print Section Names", printSectionNames},
                            {"Print Symbols", printSymbols},
                            {"Check Files for Merge", CheckMerge},
                            {"Merge ELF Files", mergeELFFiles},
                            {"Quit", quit},
                            {NULL, NULL}};



int main(int argc, char const *argv[])
{   

    // infinite loop.
    while (1)
    {
        // debugPrint(current_state);
        printMenu();  // print menu.
        char line[4];
        if(fgets(line, 4, stdin) != 0){ // get user choise.
            int index = atoi(line);
            if(index > 6 || index < 0){ // check if input is in range.
                printf("Error! out of bounce.\n");
            }
            else{
                menu[index].fun();
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
}

/**
 * tooggle debug mode (on/off).
*/
void toggleDebugMode(){
    if(debugMode == 1){
        debugMode = 0;
        printf("Debug flag now off\n");
    }
    else{
        debugMode = 1;
        printf("Debug flag now on\n");
    }
}

/**
 * quit gracefully.
*/
void quit(){
    fprintf(stderr, "quitting\n");
    closeAndfreeELFInfo(firstELF);
    closeAndfreeELFInfo(secondELF);
    exit(0);
}

/**
 * close the f ELFInfo struct, close the file, unmap the file, and free memory allocation.
*/
void closeAndfreeELFInfo(ELFInfo* f){
    if(f != NULL){
        munmap(f->map_start, f->size);
        close(f->fd);
        free(f);
    }
}


/**
 * recive an file_name from the user, open the file, mapped the file to memory, check if is it ELF file,
 * print the required information and save it in a global variable.
*/
void examineELFFile(){

    char file_name[128];
    printf("Please enter file name:\n");
    if(fgets(file_name, 128, stdin) != 0){

        if(file_name[strlen(file_name) - 1] == '\n')
            file_name[strlen(file_name) - 1] = '\0';

        if(debugMode == 1)
            fprintf(stderr, "Debug: examine file %s\n", file_name);

        int fd;
        if((fd = open(file_name, O_RDONLY)) >= 0){
            struct stat buf;
            if(fstat(fd, &buf) == 0){
                char* elfFileMaped = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);    // load the elf file to memory.
                if(elfFileMaped != MAP_FAILED){
                    ELFInfo* elfFileInfo = initialELFInfo(fd, elfFileMaped, buf.st_size, file_name);
                    if(isValidELF((Elf32_Ehdr *)elfFileInfo->map_start)){
                        if(firstELF == NULL || secondELF == NULL){
                            printInformation((Elf32_Ehdr *)elfFileInfo->map_start);
                            if(firstELF == NULL)
                                firstELF = elfFileInfo;
                            else
                                secondELF = elfFileInfo;
                        }
                        else{
                            closeAndfreeELFInfo(elfFileInfo);
                            printf("Error! can not process more that two ELF file!\n");
                        }
                    }
                    else{
                        closeAndfreeELFInfo(elfFileInfo);
                        printf("Error! Not an ELF file!\n");
                    }
                    
                }
                else{
                    perror("Error");
                }

            }
            else{
                perror("Error");
            }
        }
        else{
            perror("Error");
        }

    }
    else{
        perror("Error");
    }
}

/**
 * print the needed information from the elf file.
*/
void printInformation(Elf32_Ehdr *elfFileMaped){

    printf("Bytes 1,2,3 of the magic number: %c%c%c\n", elfFileMaped->e_ident[1], elfFileMaped->e_ident[2], elfFileMaped->e_ident[3]);   // 1. magic bytes 1, 2 and 3.
    
    if(elfFileMaped->e_ident[1])    // 2. The data encoding scheme of the object file. 
        printf("Data encoding scheme: little endian\n");
    else    
        printf("Data encoding scheme: big endian\n");

    printf("Entry point: %p\n", (void*)elfFileMaped->e_entry);   // 3. Entry point (hexadecimal address). 

    printf("Section header table offset: %d\n", elfFileMaped->e_shoff);    // 4. The file offset in which the section header table resides. 

    printf("Number of section headers: %d\n", elfFileMaped->e_shnum);    // 5. The number of section header entries. 

    printf("Size of section headers entry : %d\n", elfFileMaped->e_shentsize); // 6. The size of each section header entry. 

    printf("Program header table offset: %d\n", elfFileMaped->e_phoff); // 7. The file offset in which the program header table resides. 

    printf("Number of program headers: %d\n", elfFileMaped->e_phnum); // 8. The number of program header entries. 

    printf("Program header entry size: %d\n", elfFileMaped->e_phentsize); // 9. The size of each program header entry. 

}

/**
 * check if mapped file is an ELF file.
*/
bool isValidELF(Elf32_Ehdr *elfFileMaped){
    return elfFileMaped->e_ident[1] == 'E' && elfFileMaped->e_ident[2] == 'L' && elfFileMaped->e_ident[3] == 'F';
}

/**
 * allocate memory and initial ELFInfo struct.
*/
ELFInfo* initialELFInfo(int fd, char* map_start, int size, char* name){
    ELFInfo *e = malloc(sizeof(ELFInfo));
    e->fd = fd;
    e->map_start = map_start;
    e->size = size;
    strcpy(e->file_name, name);

    return e;
}

/**
 * check if there is file we have examine and print their section names.
*/
void printSectionNames(){
    if(!firstELF && !secondELF){
        printf("Error! There is not a files currently mapped\n");
    }
    else{
        if(firstELF)
            printFileSectionNames(firstELF);

        if(secondELF)
            printFileSectionNames(secondELF);
    }
}

/**
 * print a given ELFInfo file his section names.
*/
void printFileSectionNames(ELFInfo* file){ 

    Elf32_Ehdr* fileHeader = (Elf32_Ehdr*)file->map_start;
    int numberOfSectionHeaders = fileHeader->e_shnum;

    if(fileHeader->e_shoff != 0){   // the file has section header table.
        // the file section header.
        Elf32_Shdr* fileSectionHeader = (Elf32_Shdr*)(file->map_start + fileHeader->e_shoff);
        // the string table section header.
        Elf32_Shdr* stringTableSectionHeader = (Elf32_Shdr*)(fileSectionHeader + fileHeader->e_shstrndx); 
        // the string table itself.
        char* stringTable = (char*)(file->map_start + stringTableSectionHeader->sh_offset);

        printf("File %s\n", file->file_name);

        for (size_t i = 0; i < numberOfSectionHeaders; i++)
        {
            Elf32_Shdr* currentFileSectionHeader = (Elf32_Shdr*)(fileSectionHeader + i);
            printf("%02d %-20s %08x %06x %06x %s\n", i, (char*)(stringTable + currentFileSectionHeader->sh_name), currentFileSectionHeader->sh_addr, currentFileSectionHeader->sh_offset, currentFileSectionHeader->sh_size, findSectionType(currentFileSectionHeader->sh_type));
        }

        if(debugMode){  // debug printing.
            printf("Deubg:\nThere are %d section headers, starting at offset 0x%x\n",numberOfSectionHeaders,  fileHeader->e_shoff);
            printf("The index of the string table at the section headers table is: %d, starting at offset %p\n", fileHeader->e_shstrndx, stringTableSectionHeader);
            printf("THe string table start at offset 0x%x\n", stringTableSectionHeader->sh_offset);
        }
    }
}

/**
 * return the real value of section type in string.
*/
char* findSectionType(Elf32_Word type){
    switch (type)
    {
    case 0:
        return "NULL";
    case 1:
        return "PROGBITS";
    case 2:
        return "SYMTAB";
    case 3:
        return "STRTAB";
    case 4:
        return "RELA";
    case 5:
        return "HASH";
    case 6:
        return "DYNAMIC";
    case 7:
        return "NOTE";
    case 8:
        return "NOBITS";
    case 9:
        return "REL";
    case 10:
        return "SHLI";
    case 11:
        return "DYNSYM";
    case 0x70000000:
        return "LOPROC";
    case 0x7fffffff:
        return "HIPROC";
    case 0x80000000:
        return "LOUSER";
    case 0xffffffff:
        return "HIUSER";
    case 0x6FFFFFFF:
        return "GNU_versym";
    case 0x6FFFFFFE:
        return "GNU_verneed";
    default:
        return "Undefined";
    }
}

/**
 * check if there is file we have examine and print their symbols.
*/
void printSymbols(){
    if(!firstELF && !secondELF){
        printf("Error! There is not a files currently mapped\n");
    }
    else{
        if(firstELF)
            filePrintSymbols(firstELF);

        if(secondELF)
            filePrintSymbols(secondELF);
    }
}

/**
 * print a given ELFInfo file his symbols.
*/
void filePrintSymbols(ELFInfo* file){
    Elf32_Ehdr* fileHeader = (Elf32_Ehdr*)file->map_start;
    int numberOfSectionHeaders = fileHeader->e_shnum;

    if(fileHeader->e_shoff != 0){   // the file has section header table.
        // the file section header.
        Elf32_Shdr* fileSectionHeader = (Elf32_Shdr*)(file->map_start + fileHeader->e_shoff);
        // the string table section header.
        Elf32_Shdr* stringTableSectionHeader = (Elf32_Shdr*)(fileSectionHeader + fileHeader->e_shstrndx); 
        // the string table itself.
        char* stringTable = (char*)(file->map_start + stringTableSectionHeader->sh_offset);
        char* symbolsStringTable = stringTable;
        printf("File %s\n", file->file_name);

        for (size_t i = 0; i < numberOfSectionHeaders; i++)
        {
            Elf32_Shdr* currentFileSectionHeader = (Elf32_Shdr*)(fileSectionHeader + i);
            
            if(currentFileSectionHeader->sh_type == SHT_SYMTAB || currentFileSectionHeader->sh_type == SHT_DYNSYM){
                // printf("%s\n", (char*)(stringTable + (fileSectionHeader + currentFileSectionHeader->sh_link)->sh_name));                
                symbolsStringTable = (char*)(file->map_start + (fileSectionHeader + currentFileSectionHeader->sh_link)->sh_offset);
                
                Elf32_Sym *firstSymbol = (Elf32_Sym*)(file->map_start + currentFileSectionHeader->sh_offset);
                int numberOfSymbols = currentFileSectionHeader->sh_size / sizeof(Elf32_Sym);

                if(debugMode){
                    printf("Symbol table %s\n", (char*)(stringTable + currentFileSectionHeader->sh_name));
                    printf("The size of the symbol table is: %x\n", currentFileSectionHeader->sh_size);
                    printf("The number of symbols is: %d\n", numberOfSymbols);
                }
                for (size_t j = 0; j < numberOfSymbols; j++)
                {
                    Elf32_Sym *currentSymbol = (Elf32_Sym*)(firstSymbol + j);
                    printf("%02d %08x %02u %-20s %s\n", j, currentSymbol->st_value, currentSymbol->st_shndx, currentSymbol->st_shndx > numberOfSectionHeaders ? "" : (char*)(stringTable + ((Elf32_Shdr*)(fileSectionHeader + currentSymbol->st_shndx))->sh_name), (char*)(symbolsStringTable + currentSymbol->st_name));
                }
                
            }

        }

    }
}

/**
 * check if there is duplicated define symbols or undefined symbol and print the errors.
*/
void CheckMerge(){
    // check if we have two ELF file opened, mapped, and different.
    if(firstELF && secondELF && strcmp(firstELF->file_name, secondELF->file_name) != 0){
        loopOverAndCheck(firstELF, secondELF);
        loopOverAndCheck(secondELF, firstELF);
    }
}

/**
 * return for a given file its symtab.
*/
Elf32_Shdr* findSymbolTable(ELFInfo* file){

    Elf32_Ehdr* fileHeader = (Elf32_Ehdr*)file->map_start;
    int numberOfSectionHeaders = fileHeader->e_shnum;

    if(fileHeader->e_shoff != 0){   // the file has section header table.
        // the file section header.
        Elf32_Shdr* fileSectionHeader = (Elf32_Shdr*)(file->map_start + fileHeader->e_shoff);

        for (size_t i = 0; i < numberOfSectionHeaders; i++)
        {
            Elf32_Shdr* currentFileSectionHeader = (Elf32_Shdr*)(fileSectionHeader + i);
            
            if(currentFileSectionHeader->sh_type == SHT_SYMTAB)
                return currentFileSectionHeader;
        }

    }
    return NULL;
}


/**
 * loop over the one elf file symbols and check for the errors.
*/
void loopOverAndCheck(ELFInfo* one, ELFInfo* two){

    // first file information we need.
    Elf32_Ehdr* fileHeader1 = (Elf32_Ehdr*)one->map_start;
    Elf32_Shdr* symtab1 = findSymbolTable(one);
    int NumberOfSymbols1 = symtab1->sh_size / sizeof(Elf32_Sym);
    Elf32_Sym *firstSymbol1 = (Elf32_Sym*)(one->map_start + symtab1->sh_offset);
    Elf32_Shdr* fileSectionHeader1 = (Elf32_Shdr*)(one->map_start + fileHeader1->e_shoff);
    char* symbolsStringTable1 = (char*)(one->map_start + (fileSectionHeader1 + symtab1->sh_link)->sh_offset);

    for (size_t i = 1; i < NumberOfSymbols1; i++)
    {
        Elf32_Sym* sym1 = (Elf32_Sym*)(firstSymbol1 + i);
        if(strcmp((char*)(symbolsStringTable1 + sym1->st_name), "") != 0){
            Elf32_Sym* sym2 = findSymbol((char*)(symbolsStringTable1 + sym1->st_name), two);
            if((sym1->st_shndx == SHN_UNDEF && !sym2) ||        // sym1 undifne, and did not found sym2.
            (sym1->st_shndx == SHN_UNDEF && sym2 && sym2->st_shndx == SHN_UNDEF)){  // both sym1 and sym2 are undefined.
                printf("Symbol sym undefined\n");
            }
            else if(sym1->st_shndx != SHN_UNDEF && sym2 && sym2->st_shndx != SHN_UNDEF){    // sym1 define, and sym2 define.
                printf("Symbol sym multiply defined\n");
            }
        }
    }
    
}

/**
 * find a given symbol by his name.
*/
Elf32_Sym* findSymbol(char* toFindName, ELFInfo* file){

    Elf32_Ehdr* fileHeader = (Elf32_Ehdr*)file->map_start;
    Elf32_Shdr* symtab = findSymbolTable(file);
    int NumberOfSymbols = symtab->sh_size / sizeof(Elf32_Sym);
    Elf32_Sym *firstSymbol = (Elf32_Sym*)(file->map_start + symtab->sh_offset);
    Elf32_Shdr* fileSectionHeader = (Elf32_Shdr*)(file->map_start + fileHeader->e_shoff);
    char* symbolsStringTable = (char*)(file->map_start + (fileSectionHeader + symtab->sh_link)->sh_offset);

    for (size_t i = 1; i < NumberOfSymbols; i++)
    {
        Elf32_Sym* currentSymbol = (Elf32_Sym*)(firstSymbol + i);

        if(strcmp(toFindName, (char*)(symbolsStringTable + currentSymbol->st_name)) == 0)
            return currentSymbol;
    }
    return NULL;
}

/**
 * Merge two ELF files.
*/
void mergeELFFiles(){

    // create new file by name out.ro.
    int new_file_fd = open("out.ro", O_RDWR | O_CREAT, 0644);

    // coping the elf header from the first file.
    Elf32_Ehdr* new_file_header = malloc(sizeof(Elf32_Ehdr));
    new_file_header = (Elf32_Ehdr*)memcpy(new_file_header, firstELF->map_start, sizeof(Elf32_Ehdr));

    // append to the empty file the elf header. (we will change it later on).
    write(new_file_fd, new_file_header, sizeof(Elf32_Ehdr));

    // coping the elf section header table from the first file.
    int numberOfSections = ((Elf32_Ehdr*)(firstELF->map_start))->e_shnum;
    Elf32_Shdr* new_file_section_header = malloc(sizeof(Elf32_Shdr) * numberOfSections);
    new_file_section_header = (Elf32_Shdr*)memcpy(new_file_section_header, firstELF->map_start + ((Elf32_Ehdr*)(firstELF->map_start))->e_shoff, sizeof(Elf32_Shdr) * numberOfSections);
    
    // for each section
    int section_table_size = 0;
    for (size_t i = 0; i < numberOfSections; i++)
    {
        Elf32_Shdr* currentSection = (Elf32_Shdr*)(new_file_section_header + i);
        char* section_name = getSectionName(currentSection, firstELF);

        // Mergable sections
        if(strcmp(section_name, ".text") == 0 || strcmp(section_name, ".data") == 0 || strcmp(section_name, ".rodata") == 0)
        {
            concatAndAppendMergableSections(new_file_fd, section_name, currentSection);
        }
        // used without changes because of the restricting assumptions.
        else
        {

            int newOffset = getFileSize(new_file_fd);
            write(new_file_fd, (char*)(firstELF->map_start + currentSection->sh_offset), currentSection->sh_size);   
            currentSection->sh_offset = newOffset;   
        }
        section_table_size += currentSection->sh_size;
    }

    int section_table_offset = getFileSize(new_file_fd);

    write(new_file_fd, new_file_section_header, section_table_size);

    char* elfFileMaped = mmap(NULL, getFileSize(new_file_fd), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, new_file_fd, 0);    // load the elf file to memory.
    if(elfFileMaped == MAP_FAILED){
        perror("Error");
    }
    else{
        ((Elf32_Ehdr*)elfFileMaped)->e_shoff = section_table_offset;
        munmap(elfFileMaped, getFileSize(new_file_fd));
    }
    free(new_file_header);
    free(new_file_section_header);
    close(new_file_fd);
}


/**
 * find a section in a elf file by name.
*/
Elf32_Shdr* findSection(char* name, ELFInfo* file){
    int numberOfSections = ((Elf32_Ehdr*)(file->map_start))->e_shnum;
    Elf32_Shdr* fileSectionTable = (Elf32_Shdr*)(file->map_start + ((Elf32_Ehdr*)(file->map_start))->e_shoff);

    for (size_t i = 0; i < numberOfSections; i++)
    {
        Elf32_Shdr* currentSection = (Elf32_Shdr*)(fileSectionTable + i);
        if(strcmp(name, getSectionName(currentSection, file)) == 0)
            return currentSection;
    }
    return NULL;
}

/**
 * return a give section in a given file name.
*/
char* getSectionName(Elf32_Shdr* section, ELFInfo* file){
    Elf32_Shdr* fileSectionTable = (Elf32_Shdr*)(file->map_start + ((Elf32_Ehdr*)(file->map_start))->e_shoff);
    char* stringTable = (char*)(file->map_start + ((Elf32_Shdr*)(fileSectionTable + ((Elf32_Ehdr*)(file->map_start))->e_shstrndx))->sh_offset);
    return (char*)(stringTable + section->sh_name);
}

void concatAndAppendMergableSections(int new_file_fd, char* section_name, Elf32_Shdr* currentSection){

    Elf32_Shdr* parallelSection = findSection(section_name, secondELF);

    // get the new offset
    int newOffset = getFileSize(new_file_fd);
    if(parallelSection){
        // get the new size of the section.
        int newSize = currentSection->sh_size + parallelSection->sh_size;

        // append to the file the concated information.
        write(new_file_fd, (char*)(firstELF->map_start + currentSection->sh_offset), currentSection->sh_size);
        write(new_file_fd, (char*)(secondELF->map_start + parallelSection->sh_offset), parallelSection->sh_size);

        // update the section for the size and new offset.
        currentSection->sh_size = newSize;
    }
    else{   // there is no paralles section, we append the current section only.
        write(new_file_fd, (char*)(firstELF->map_start + currentSection->sh_offset), currentSection->sh_size);   
    }
    currentSection->sh_offset = newOffset;
}

int getFileSize(int fd){
    struct stat buf;
    fstat(fd, &buf);
    return buf.st_size;
}


