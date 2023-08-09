#include <elf.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>


void load_phdr(Elf32_Phdr *phdr, int fd);
int checkProtectionFlags(Elf32_Word flag);
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg);
void printProgramHeaderInformation(Elf32_Phdr *elfph, int i);
char* checkType(Elf32_Word p_type);
char* checkFlag(Elf32_Word flag);
Elf32_Word checkMappingFlags(Elf32_Word flag);
extern int startup(int argc,char** argv,int(*func)(int,char**));



int main(int argc, char const *argv[])
{
    if(argc < 2){
        printf("Missing ELF name!\n");
    }
    else{
        int fd = open(argv[1], O_RDWR);
        if(fd > 0){
            struct stat buf;
            if(fstat(fd, &buf) == 0){
                void* elfFileMaped = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);    // load the exe file to memory.
                printf("Type         Offset   VirtAddr   PhysAddr   FileSiz MemSiz  Flg  Align    Protection-flags Mapping-flags\n");
                foreach_phdr(elfFileMaped, load_phdr, fd);              // load the program header of the exe file to memory.
                char** newArgv = (char**)(argv + 1);
                startup(argc - 1,newArgv, (void*)(((Elf32_Ehdr*)elfFileMaped)->e_entry));   // run the exe file.
            }
            else{
                perror("Error: ");
            }
        }
        else{
            perror("Error: ");
        }
        close(fd);
    }
    return 0;
}

/**
 * Load program header to memory.
*/
void load_phdr(Elf32_Phdr *phdr, int fd){
    if(phdr->p_type == 1){      // check if p_load flag is on and the size in > 0.

        // According to reading material.
        void* vaddr = (void*)(phdr->p_vaddr & 0xfffff000);
        int offset = phdr->p_offset & 0xfffff000;
        int padding = phdr->p_vaddr & 0xfff;
        void* elfFileMaped = mmap(vaddr, phdr->p_memsz + padding, checkProtectionFlags(phdr->p_flags), MAP_FIXED | MAP_PRIVATE, fd, offset); 
        if(elfFileMaped == MAP_FAILED){
            perror("Error");
            _exit(1);
        }
        else{
            printProgramHeaderInformation(phdr, 0);
        }
    }
}

/**
 *  print program header.
*/
void printProgramHeaderInformation(Elf32_Phdr *elfph, int i){
    printf("%s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %s 0x%05x  0x%x              0x%x\n",checkType(elfph->p_type), elfph->p_offset, elfph->p_vaddr, elfph->p_paddr, elfph->p_filesz, elfph->p_memsz, checkFlag(elfph->p_flags), elfph->p_align, checkProtectionFlags(elfph->p_flags), checkMappingFlags(elfph->p_flags));
}

/**
 * interator of all the program headers at the file and apply func on them.
*/
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *,int), int arg){
    Elf32_Ehdr* elfh = (Elf32_Ehdr*)map_start;
    int numberOfProgramHeader = elfh->e_phnum; 
    int sizeOfProgramHeaderEntry = elfh->e_phentsize; 
    int programHeaderOffset = elfh->e_phoff;    

    char* ph_start_address = (char*)(map_start + programHeaderOffset);
    
    for (size_t i = 0; i < numberOfProgramHeader; i++)
    {
        Elf32_Phdr* ph = (Elf32_Phdr*)(ph_start_address + i * sizeOfProgramHeaderEntry);
        func(ph, arg);
    }
    return 0;
}

/**
 * Check and return the protection flags.
*/
int checkProtectionFlags(Elf32_Word flag){
    switch (flag)
    {
    case 0x1:       // executable
        return PROT_EXEC;
    case 0x2:       // writable
        return PROT_WRITE;
    case 0x3:       // writable | executable
        return PROT_WRITE | PROT_EXEC;
    case 0x4:       // readable
        return PROT_READ;
    case 0x5:       // readable | executable
        return PROT_READ | PROT_EXEC;
    case 0x6:       // readable | writable
        return PROT_READ | PROT_WRITE;
    case 0x7:       // readable | writable | executable
        return PROT_READ | PROT_WRITE | PROT_EXEC; 
    default:
        return PROT_NONE;
    }
}

/**
 * This function return the p_type value in string.
*/
char* checkType(Elf32_Word p_type){
    switch (p_type)
    {
    case 0:
        return "NULL        ";
    case 1:
        return "LOAD        ";
    case 2:
        return "DYNAMIC     ";
    case 3:
        return "INTERP      ";
    case 4:
        return "NOTE        ";
    case 5:
        return "SHLIB       ";
    case 6:
        return "PHDR        ";
    case 0x70000000:
        return "LOPROC      ";
    case 0x7fffffff:
        return "HIPROC      ";
    case 0x6474e550:
        return "GNU_EH_FRAME";
    case 0x6474e551:
        return "GNU_STACK   ";
    case 0x6474e552:
        return "GNU_RELRO   ";
    default:
        return "Not defined";
    }
}

/**
 * This function return the flag value in string.
*/
char* checkFlag(Elf32_Word flag){
    switch (flag)
    {
    case 0x1:       // executable
        return "E   ";
    case 0x2:       // writable
        return "W   ";
    case 0x3:       // writable | executable
        return "W E ";
    case 0x4:       // readable
        return "R   ";
    case 0x5:       // readable | executable
        return "R E ";
    case 0x6:       // readable | writable
        return "RW  ";
    case 0x7:       // readable | writable | executable
        return "RW E"; 
    default:
        return "Not defined";
    }
}

/**
 * Check and return the Mapping flags. 
 * if program is writable and readable - we use the shared flag so we can change it.
 * if it is just readable - we use private.
*/
Elf32_Word checkMappingFlags(Elf32_Word flag){  
    switch (flag)
    {
    case 0x1:       // executable
        return MAP_EXECUTABLE;
    case 0x2:       // writable
        return MAP_SHARED;
    case 0x3:       // writable | executable
        return MAP_SHARED;
    case 0x4:       // readable
        return MAP_PRIVATE;
    case 0x5:       // readable | executable
        return MAP_PRIVATE;
    case 0x6:       // readable | writable
        return MAP_SHARED;
    case 0x7:       // readable | writable | executable
        return MAP_SHARED; 
    default:
        return 0;
        break;
    }
}