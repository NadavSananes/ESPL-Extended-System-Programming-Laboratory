section .rodata
    str1: db "Command line arguments: ", 10, 0
    str2: db 10,0
    str3: db "-i", 0
    str4: db "-o", 0
    WRITE EQU 4
    READ EQU 3
    STDIN EQU 0
    STDOUT EQU 1
    STDERR EQU 2
    EXIT EQU 1
    OPEN EQU 5
    CLOSE EQU 6

section .data
    infile: dd STDIN
    outfile: dd STDOUT

segment .text
    global main
    extern strlen
    extern strncmp

main:
    push    ebp         ; Save caller state              
    mov     ebp, esp    

    mov edi, [ebp+8]    ;int argc.
    mov esi, [ebp + 12] ; start argv0


    mov     eax, WRITE     
    mov     ebx, STDERR 
    mov     ecx, str1   ;The string to print.
    mov     edx, 26     ;The size of the print.
    int     0x80        ;Run the function.

    
    loop:
        cmp     edi, 0      ;compare argc to 0.
        jz      end_of_loop ;"jump if zero", we finish the loop.
        
        sub     edi, 1      ; argc = argc - 1.
    
        push    dword [esi]
        call    strlen      ;call strlen function.

        ;print the argument.
        mov     edx, eax    ; strlen store the return value in eax. we move it to edx.
        mov     eax, WRITE  ; write fucntion.
        mov     ebx, STDERR
        mov     ecx, [esi]  ; The string to print.
        int     0x80        ; Run the function.

        ;print new line.
        mov     eax, WRITE  ;Write fucntion is 4.    
        mov     ebx, STDERR
        mov     ecx, str2   ;The string to print.
        mov     edx, 2     ;The size of the print.
        int     0x80        ;Run the function.

        ;check for input/output redirecting.
        push    dword 2     ; 2 length comapring.
        push    dword str3  ; str3 = "-i"
        push    dword [esi]
        call    strncmp
        mov     edx, eax
        cmp     edx, 0
        jnz     check_output
        ;open the file, and redirect infile.
        mov     eax, OPEN
        mov     ebx, [esi]
        add     ebx, 2
        mov     ecx, 0 | 64      ;read and create.
        mov     edx, 777
        int     0x80        ;Run the function.
        mov     edx, eax    ; save return value
        mov     [infile], edx


    check_output:
        push    dword 2
        push    dword str4
        push    dword [esi]
        call    strncmp
        mov     edx, eax
        cmp     edx, 0
        jnz     no_output
        ;open the file, and redirect outfile.
        mov     eax, OPEN
        mov     ebx, [esi]
        add     ebx, 2
        mov     ecx, 1 | 64      ;write and create.
        mov     edx, 777
        int     0x80        ;Run the function.
        mov     edx, eax    ; save return value
        mov     [outfile], edx

    no_output:
        add     esi, 4      ;updating the argv pointer to point the next argument.
        add     esp, 4      ;clean up stack- line 31.
        jmp     loop

    end_of_loop:
        call    encode
        mov     esp, ebp
        pop     ebp
        mov     ebx, 0      ;return value- good.
        mov     eax, EXIT
        int     0x80        ;Run the function.

encode:
        push    ebp         ; Save caller state              
        mov     ebp, esp 
        sub     esp, 4      ; make room on the stack for variable.

    encode_loop:

        mov     eax, READ  
        mov     ebx, [infile]
        mov     ecx, esi    ;the buffer.
        mov     edx, 1      ;size if byts to get.
        int     0x80        ;Run the function.

        ;check if we finish read, return value = 0, and finish loop.
        mov     edx, eax
        cmp     edx, 0
        jz      encode_end_of_loop

        ;check if chae is in range and needed to encode.
        cmp     byte [esi], 'A'    ; compare the char and 'A'.
        jl      print       ;jump to print if the char is lesser that 'A'.
        cmp     byte [esi], 'y'    ; compare the char and 'z'.
        jg      print       ;jump to print if the char is greater that 'z'.

        inc     byte [esi]

    print:  
        mov     eax, WRITE    
        mov     ebx, [outfile]
        mov     ecx, esi    ;the buffer.
        mov     edx, 1     ;The size of the print.
        int     0x80        ;Run the function.

        jmp encode_loop


    encode_end_of_loop:
                add     esp, 4      ;clean the room on stack we created.
                mov     esp, ebp
                pop     ebp
                ret
