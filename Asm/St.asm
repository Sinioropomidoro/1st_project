section .data
    msg     db "Factorial of 8 = "      ; текст перед результатом
    msg_len equ $ - msg                 ; длина текста (вычисляется автоматически)
    nl      db 10                       ; символ перевода строки

section .bss
    buf     resb 20                     ; буфер под строку с числом

section .text
    global _start                       ; точка входа для линкера

_start:
    mov rax, 1                          ; номер системного вызова write
    mov rdi, 1                          ; дескриптор 1 = stdout
    mov rsi, msg                        ; адрес строки
    mov rdx, msg_len                    ; длина строки
    syscall                             ; выводим текст

    mov rcx, 8                          ; rcx = n (счётчик цикла)
    mov rax, 1                          ; rax = результат, начальное значение 1

.fact_loop:
    imul rax, rcx                       ; rax = rax * rcx
    dec rcx                             ; rcx = rcx - 1
    jnz .fact_loop                      ; если rcx != 0, повторяем цикл
                                        ; после цикла rax = 40320

    lea rsi, [buf + 20]                 ; rsi указывает на конец буфера
    mov rbx, 10                         ; делитель 10

.to_str:
    xor rdx, rdx                        ; обнуляем rdx перед делением
    div rbx                             ; rax = rax / 10, rdx = остаток (цифра)
    add dl, '0'                         ; цифра -> ASCII-символ
    dec rsi                             ; сдвигаем указатель назад
    mov [rsi], dl                       ; кладём символ в буфер
    test rax, rax                       ; частное == 0?
    jnz .to_str                         ; нет, берём следующую цифру

    lea rdx, [buf + 20]                 ; конец буфера
    sub rdx, rsi                        ; rdx = длина строки с числом
    mov rax, 1                          ; write
    mov rdi, 1                          ; stdout
    syscall                             ; выводим число (rsi уже указывает на начало)

    mov rax, 1                          ; write
    mov rdi, 1                          ; stdout
    mov rsi, nl                         ; адрес символа новой строки
    mov rdx, 1                          ; длина 1 байт
    syscall                             ; выводим перевод строки

    mov rax, 60                         ; номер системного вызова exit
    xor rdi, rdi                        ; код возврата 0
    syscall                             ; завершаем программу