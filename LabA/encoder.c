#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int addAndWrapAround(int c, int toAdd);
int subAndWrapAround(int c, int toSub);

int main(int argc, char const *argv[])
{
    bool debugMode = false;

    bool add = false;   // indicate if the falg is add
    bool sub = false;   // indicate if the flag is subtraction
    char *keyValue;     // save the keyword
    FILE *infile = stdin;
    FILE *outfile = stdout;

    for (size_t i = 1; i < argc; i++)
    {
        if (debugMode)   // if debug mode is on, print to stderr.
        {
            fprintf(stderr, argv[i]);
            fprintf(stderr, "\n");
        }

        // look for debug flags.
        if (strcmp(argv[i], "-D") == 0)
        {
            debugMode = false;
        }
        if (strcmp(argv[i], "+D") == 0)
        {
            debugMode = true;
        }

        //look for add/sub flags.
        if (argv[i][0] == '+' && argv[i][1] == 'e')
        {
            add = true;
            keyValue = argv[i];
        }
        if (argv[i][0] == '-' && argv[i][1] == 'e')
        {
            sub = true;
            keyValue = argv[i];
        }

        //look for input/output files flags.
        if (argv[i][0] == '-' && argv[i][1] == 'i')
        {
            char *fname = &argv[i][2];
            infile = fopen(fname, "r");
        }
        if (argv[i][0] == '-' && argv[i][1] == 'o')
        {
            char *fname = &argv[i][2];
            outfile = fopen(fname, "w");
        }
    }

    int i = 2;

    if (infile == NULL || outfile == NULL){ // if was not able to open one of the file- print error to stderr and exit the program.
        fprintf(stderr, "Error!");
        exit(0);
    }
    
    char c = fgetc(infile);
    while (c != EOF)
    {
        if (add) // if add flag is on.
        {
            c = addAndWrapAround(c, keyValue[i] - '0');
        }
        else if(sub){  // if the sub flag is on.
            c = subAndWrapAround(c, keyValue[i] - '0');
        }
        fputc(c, outfile);
        c = fgetc(infile);
        i++;
        if(keyValue[i] == '\0')
            i = 2;
        
    }
    
    
    return 0;
}

int addAndWrapAround(int c, int toAdd){
    
    if (c >= 97 && c <= 122 && c + toAdd > 122) // c is a cher beetwin a-z
    {
        return 97 + (c + toAdd) % 123;
    }
    if (c >= 65 && c <= 90 && c + toAdd > 90) // c is a cher beetwin A-Z
    {
        return 65 + (c + toAdd) % 91;
    }
    if (c >= 48 && c <= 57 && c + toAdd > 57) // c is a cher beetwin 0-9
    {
        return 48 + (c + toAdd) % 58;
    }
    if((c >= 97 && c <= 122) || (c >= 65 && c <= 90) || (c >= 48 && c <= 57))
        return c + toAdd;
    return c;
}

int subAndWrapAround(int c, int toSub){
    if (c >= 97 && c <= 122 && c - toSub < 97) // c is a cher beetwin a-z
    {
        return 123 - (97 - (c - toSub));
    }
    if (c >= 65 && c <= 90 && c - toSub < 65) // c is a cher beetwin A-Z
    {
        return 91 - (65 - (c - toSub));
    }
    if (c >= 48 && c <= 57 && c - toSub < 48) // c is a cher beetwin 0-9
    {
        return 58 - (48 - (c - toSub));
    }
    if((c >= 97 && c <= 122) || (c >= 65 && c <= 90) || (c >= 48 && c <= 57))
        return c - toSub;
    return c;
}
