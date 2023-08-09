section .rodata
hexaFormat: db "%02hhx", 0
hexaFormat2: db "%llx",10, 0
I_flag_str: db "-I", 0
R_flag_str: db "-R", 0
newLine: db 10, 0
EXIT EQU 1



section .data
buffer_size: dd 600    ; Size of the buffer
x_struct: db 5
x_num: db 0xaa, 1,2,0x44,0x4f
y_struct: db 6
y_num: db 0xaa, 1,2,3,0x44,0x4f

state: dw 0xACE1
mask:  dw 0x001D


section .bss
buffer resd 600     ; Allocate 600 bytes for the buffer

segment .text
global main
global print_multi
global getmulti
global getMaxMin
global add_multi
global rand_num
global PR_multi
extern fgets
extern stdin
extern strlen
extern sscanf
extern malloc
extern memcpy
extern strcmp
extern printf

main:
push    ebp         ; Save caller state              
mov     ebp, esp    

mov edi, [ebp + 8]    ;int argc.
mov esi, [ebp + 12] ; argv

;check if this is the first case. no arguments.
cmp     edi, 1
jz      first_case_no_argument

;check if this is the third case, argument = "-R"
add     esi, 4
push    dword [esi]
push    dword R_flag_str
call    strcmp
add     esp, 8

cmp     eax, 0
jz      third_case_PRNG

;check if this the second case, argument ="-I"
push    dword [esi]
push    dword I_flag_str
call    strcmp
add     esp, 8

cmp     eax, 0
jz      second_case_user_input

jmp     done_main

first_case_no_argument:
push    x_struct
call    print_multi
add     esp, 4

push    y_struct
call    print_multi
add     esp, 4

push    x_struct
push    y_struct
call    add_multi
add     esp, 8

push    eax
call    print_multi
add     esp, 4
jmp     done_main

second_case_user_input:
call    getmulti
push    eax
call    getmulti
mov     ebx, eax

call    print_multi
push    ebx
call    print_multi


call    add_multi
push    eax
call    print_multi
add     esp, 12

jmp     done_main

third_case_PRNG:
call    PR_multi
mov     ebx, eax
push    dword ebx
call    print_multi

call    PR_multi
mov     ecx, eax
push    dword ecx
call    print_multi

call    add_multi
add     esp, 8

push    eax
call    print_multi
add     esp, 4

jmp     done_main

done_main:
mov     esp, ebp
pop     ebp
mov     ebx, 0      ;return value- good.
mov     eax, EXIT
int     0x80        ;Run the function.

print_multi:
push    ebp         ; Save caller state              
mov     ebp, esp    

mov     edi, [ebp + 8]  ; first argument- struct multi p*

movzx   esi, byte [edi]      ; esi =  p->char size
add     edi, esi          ; edi =  p->char num[]

check_leading_zero:
cmp     [edi], byte 0
jnz     loop
sub     esi, 1       ; updating the counter(size) and the num[]
sub     edi, 1
jmp     check_leading_zero

loop:

cmp     esi, 0
jz      end_of_loop


push    dword [edi]
push    dword hexaFormat
call    printf

add     esp, 8  ; clear the stack. 

sub     esi, 1       ; updating the counter(size) and the num[]
sub     edi, 1

jmp     loop

end_of_loop:

;print new line.
push    dword newLine
call    printf

add     esp, 4          ;clear stack.

mov     esp, ebp
pop     ebp
mov     eax, 0
ret


rand_num:

;start of function.
push    ebp             ; Save caller state              
mov     ebp, esp  

mov     ax, [state]     ;state
mov     bx, [mask]      ;mask


and     ax, bx        ;bit = (state & 0x001d)
add     ax, 1
xor     ax, 0          ;xor to check the parity flag.

jpo     odd_parity

even_parity:
mov     bx, 0
jmp     countinue

odd_parity:
mov     bx, 1

;now the bit is 0 or 1 according to the parity check.

countinue:
mov     ax, [state]
shr     ax, 1      ;shift right the state.
shl     bx, 15     ;shift left the bit 15 time and now it is in ths msb.
or      ax, bx     ; bit[0]#state[15,..,1]

; inc     eax
; cmp     edx, edi
; jnz     rand_loop

mov     word [state] ,ax
movzx   eax, ax

;end of fucntion.
mov     esp, ebp
pop     ebp
ret

PR_multi:
;start of function.
push    ebp             ; Save caller state              
mov     ebp, esp 

do_it_again:
call    rand_num
mov     cl, ah          ;store the lowest 8 bits of eax at bl.
mov     bl, al
cmp     cl, 0
jz      do_it_again



movzx   edi, cl         ;correct eax.


; call malloc
push    edi
call    malloc
add     esp, 4

mov     esi, eax        ;save the pointer to memory.
mov     edx, 0          ;counter.
mov     [esi + edx], byte bl
add     edx, 1


PR_multi_loop:
;if we finish the loop.
cmp     edi, edx
jz      PR_multi_end
;put the first byte.
call    rand_num
mov     [esi + edx], byte ah
add     edx, 1
;if we finish the loop.
cmp     edi, edx
jz      PR_multi_end
;put the second byte
mov     [esi + edx], byte al
add     edx, 1


jmp     PR_multi_loop



PR_multi_end:

;end of fucntion.
mov     esp, ebp
pop     ebp
mov     eax, esi
ret

getmulti:
push    ebp         ; Save caller state              
mov     ebp, esp   

; read from stdin.
push    dword [stdin]
push    dword buffer_size ; number of bytes to read
push    buffer            ; buffer to read into
call    fgets
add     esp, 12

;find the string length / 2 => the number of hex number. .
push    buffer         
call    strlen
add     esp, 4
; sub     eax, 1
shr     eax, 1
add     eax, 1
mov     edi, eax

push    edi
call    malloc
add     esp, 4

sub     edi, 1
mov     esi, eax
mov     eax, edi
mov     [esi], byte al

;convert string to hexdecimal using sscanf.
add     esi, 1
push    dword esi
push    dword hexaFormat2
push    dword buffer
call    sscanf
add     esp, 12
sub     esi, 1



mov     esp, ebp
pop     ebp
mov     eax, esi
ret


getMaxMin:
;start of function.
push    ebp         ; Save caller state              
mov     ebp, esp   

;we assume one number is in eax, and the second is in ebx.

movzx     ecx, byte [eax]  ;load from our pointer the first byte of each number. (first byte hold the size)
movzx     edx, byte [ebx]  

cmp     ecx, edx          ; compare the two bytes.
jge     do_not_switch   ; if eax >= ebx (jump greater equal)

;switch
mov     ecx, eax        ;temp
mov     eax, ebx        
mov     ebx, ecx

do_not_switch:
;end of fucntion.
mov     esp, ebp
pop     ebp
ret


add_multi:
;start of function.
push    ebp         ; Save caller state              
mov     ebp, esp   

mov     eax, [ebp + 8]  ;first argument- struc multi *p
mov     ebx, [ebp + 12] ;second argument- struc multi *q


call    getMaxMin       ;now the bigger number is in eax.

mov     edi, eax        ;now the bigger number is in edi.

movzx   edx, byte [edi] ;store the biggest number size in edx.


;allocate memory at the size of the biggest number size + 1.
add     edx, 2
push    edx
call    malloc
mov     esi, eax

movzx   edx, byte [edi] ;store the biggest number size in edx. (it is ruined by malloc call)

;store the size in our new allocated memory at the start.
mov     [esi], byte dl

;put 0 at the number last position, if there is a carry we increse it, else when we print_multi we ignore leading zeros.
mov     [esi + edx], byte 0x0

add     esi, 1          ;point the the number itself!
add     edi, 1

push    edx
push    edi
push    esi
call    memcpy

;now esi-the bigger number, edi-the smaller number.
mov     edi, ebx
add     edi, 1


;now we start the loop and add the two numbers, esi and edi.
;esi = p->num, ebx = q->num, edx = smallerNumberLength, ebx = biggerNumberLength.

mov     ecx, 0              ;counter = 0
movzx   ebx, byte [edi-1]   ;ebx = length of the smallset number
movzx   edx, byte [esi-1]   ;edx = length of the biggest number

small_length_loop:
cmp     ecx, ebx            ;check if we finisg the loop.
jz      done

;add the bytes at [ecx]--> p[i] = p[i] + q[i]
clc                         ;clear carry from previus add.
mov     eax, 0
mov     byte al, byte [edi + ecx]
adc     byte al, byte [esi + ecx] 
mov     byte [esi + ecx], byte al

mov     eax, ecx            ;save the counter for multi carry.

jc      if_was_carry        ;if there were a carry- jump.

no_carry:                   ;no carry or we take care of the cary, inc the counter and continue the loop.
add     ecx, 1
jnc     small_length_loop

if_was_carry:               ;if was carry, clear carry flag, add to the next byte p[i+1], and check if there was another carry.
add     eax, 1
clc
adc     byte [esi + eax], byte 0x1

jnc     no_carry            ;we finish taking care of the carry.

jmp     if_was_carry        ;there is another carry.

done:
;store return value.
sub     esi, 1

;increase the size of the number so if there was a carry we notice.
movzx   edx, byte [esi]       ;edx = size of the number.
add     edx, 1                ;edx++
mov     [esi], byte dl        ;esi[0] = edx

mov     eax, esi

;end of fucntion.
mov     esp, ebp
pop     ebp
ret

