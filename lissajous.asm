section .data
step    dq 0.001
max_t   dq 100.0
margin  dd 10
two     dd 2

section .text
bits 64
global generateLissajousCurves

generateLissajousCurves:
        push rbp
        mov rbp, rsp
        sub rsp, 1216

        mov dword [rbp-4], edi
        mov dword [rbp-8], esi
        movsd qword [rbp-16], xmm0
        mov dword [rbp-20], edx
        mov dword [rbp-24], ecx
        mov qword [rbp-32], r8

        mov eax, edx
        add eax, 3
        and eax, -4
        mov dword [rbp-36], eax

        imul eax, ecx
        mov dword [rbp-40], eax

        mov rdi, 0
        mov esi, dword [rbp-40]
        mov rdx, 3
        mov r10, 0x22
        mov r8, -1
        mov r9, 0
        mov rax, 9
        syscall
        mov qword [rbp-48], rax

        mov rdi, rax
        mov ecx, dword [rbp-40]
        mov al, 255
        rep stosb

        fild dword [rbp-20]
        fidiv dword [two]
        fst qword [rbp-56]
        fisub dword [margin]
        fstp qword [rbp-64]

        fild dword [rbp-24]
        fidiv dword [two]
        fst qword [rbp-72]
        fisub dword [margin]
        fstp qword [rbp-80]

        fldz
        fstp qword [rbp-88]

.calc_loop:
        fld qword [rbp-88]
        fimul dword [rbp-4]
        fadd qword [rbp-16]
        fsin
        fmul qword [rbp-64]
        fadd qword [rbp-56]
        fistp dword [rbp-92]

        fld qword [rbp-88]
        fimul dword [rbp-8]
        fsin
        fmul qword [rbp-80]
        fadd qword [rbp-72]
        fistp dword [rbp-96]

        mov eax, dword [rbp-92]
        cmp eax, 0
        jl .skip_pixel
        cmp eax, dword [rbp-20]
        jge .skip_pixel

        mov r8d, dword [rbp-96]
        cmp r8d, 0
        jl .skip_pixel
        cmp r8d, dword [rbp-24]
        jge .skip_pixel

        mov eax, r8d
        imul eax, dword [rbp-36]
        add eax, dword [rbp-92]
        mov rdi, qword [rbp-48]
        add rdi, rax

        mov byte [rdi], 0

.skip_pixel:
        fld qword [rbp-88]
        fadd qword [step]
        fst qword [rbp-88]

        fcomp qword [max_t]
        fstsw ax
        sahf
        jb .calc_loop

        lea rdi, [rbp-1200]
        mov ecx, 1078
        xor al, al
        rep stosb

        lea rdi, [rbp-1200]
        mov word [rdi], 0x4D42
        mov eax, dword [rbp-40]
        add eax, 1078
        mov dword [rdi+2], eax
        mov dword [rdi+10], 1078
        mov dword [rdi+14], 40
        mov eax, dword [rbp-20]
        mov dword [rdi+18], eax
        mov eax, dword [rbp-24]
        mov dword [rdi+22], eax
        mov word [rdi+26], 1
        mov word [rdi+28], 8
        mov eax, dword [rbp-40]
        mov dword [rdi+34], eax
        mov dword [rdi+38], 2835
        mov dword [rdi+42], 2835
        mov dword [rdi+46], 256
        mov dword [rdi+50], 256
        mov dword [rdi+1074], 0x00FFFFFF

        mov rax, 2
        mov rdi, qword [rbp-32]
        mov rsi, 577
        mov rdx, 420
        syscall
        cmp rax, 0
        jl .cleanup
        mov dword [rbp-1204], eax

        mov rax, 1
        mov edi, dword [rbp-1204]
        lea rsi, [rbp-1200]
        mov rdx, 1078
        syscall

        mov rax, 1
        mov edi, dword [rbp-1204]
        mov rsi, qword [rbp-48]
        mov edx, dword [rbp-40]
        syscall

        mov rax, 3
        mov edi, dword [rbp-1204]
        syscall

.cleanup:
        mov rax, 11
        mov rdi, qword [rbp-48]
        mov esi, dword [rbp-40]
        syscall

        mov rsp, rbp
        pop rbp
        ret