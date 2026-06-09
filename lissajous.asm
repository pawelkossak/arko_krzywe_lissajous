section .data
header_start:
        db "BM" ; bitmapa
        dd 786486 ; rozmiar pliku - 54 naglowek + 3 bajty * 512^2
        dw 0, 0
        dd 54 ; offset (naglowka)
        dd 40 ; druga czesc naglowka
        dd 512 ; szerokosc
        dd 512 ; wysokosc
        dw 1 ; liczba plaszczyzn binarnych
        dw 24 ; glebia kolorow 2^3 (RGB)
        dd 0 ; brak kompresji
        dd 786432 ; 3 bajty * 512^2
        dd 2835, 2835
        dd 0, 0

amp     dq 250.0
center  dq 256.0
step    dq 0.001
max_t   dq 100.0

        section .bss
grid    resb 786432

        section .text
        bits 64
        global generateLissajousCurves

generateLissajousCurves:
        push    rbp
        mov     rbp, rsp
        sub     rsp, 48

        mov     dword [rbp - 12], edi
        mov     dword [rbp - 16], esi
        movsd   qword [rbp - 24], xmm0
        mov     qword [rbp - 32], rdx

        mov     rdi, grid
        mov     rcx, 786432
        mov     al, 255
        rep     stosb

        fldz
        fstp    qword [rbp - 8]

.calc_loop:
        fld     qword [rbp - 8]
        fimul   dword [rbp - 12]
        fadd    qword [rbp - 24]
        fsin
        fmul    qword [amp]
        fadd    qword [center]
        fistp   dword [rbp - 36]

        fld     qword [rbp - 8]
        fimul   dword [rbp - 16]
        fsin
        fmul    qword [amp]
        fadd    qword [center]
        fistp   dword [rbp - 40]

        mov     eax, dword [rbp - 40]
        shl     eax, 9
        add     eax, dword [rbp - 36]
        lea     rax, [rax + rax * 2]

        mov     rcx, grid
        mov     byte [rcx + rax], 0
        mov     byte [rcx + rax + 1], 0
        mov     byte [rcx + rax + 2], 0

        fld     qword [rbp - 8]
        fadd    qword [step]
        fst     qword [rbp - 8]

        fcomp   qword [max_t]
        fstsw   ax
        sahf
        jb      .calc_loop

        mov     rax, 2
        mov     rdi, qword [rbp - 32]
        mov     rsi, 577
        mov     rdx, 420
        syscall

        cmp     rax, 0
        jl      .done
        mov     r8, rax

        mov     rax, 1
        mov     rdi, r8
        mov     rsi, header_start
        mov     rdx, 54
        syscall

        mov     rax, 1
        mov     rdi, r8
        mov     rsi, grid
        mov     rdx, 786432
        syscall

        mov     rax, 3
        mov     rdi, r8
        syscall

.done:
        mov     rsp, rbp
        pop     rbp
        ret