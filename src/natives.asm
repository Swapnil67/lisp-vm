print_i64:
    ;; Extracting input from the BM's stack
    mov rsi, [stack_top]
    sub rsi, BM_WORD_SIZE
    mov rax, [rsi]
    mov [stack_top], rsi
    ;; rax contains the value we need to print
    mov rdi,  0		; counter of chars
    ;; Adding a newline
    dec rsp
    inc rdi
    mov BYTE [rsp], 10
.loop:
    xor rdx, rdx
    mov rbx, 10
    div rbx
    add rdx, '0'
    dec rsp
    inc rdi
    mov [rsp], dl
    cmp rax, 0
    jne .loop
    ;; rsp - points at the beginning of the buffer
    ;; rdi - contains the size of the buffer
    mov rbx, rdi
    ;; write(STDOUT, buf, buf_size)
    mov rax, SYS_WRITE
    mov rdi, STDOUT		; stream
    mov rsi, rsp		; buffer
    mov rdx, rbx		; count
    syscall
    add rsp, rbx
    ret


print_f64:
	ret
