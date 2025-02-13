long_number_size: equ 128
                section         .text

                global          _start
_start:
                sub             rsp, 3 * long_number_size * 8
                lea             rdi, [rsp + long_number_size * 8]
                mov             rcx, long_number_size
                call            read_long
                mov             rdi, rsp
                call            read_long
                lea             rsi, [rsp + long_number_size * 8]
                call            mul_long_long

                mov             rdi, rsi
                mov             rcx, long_number_size * 2
                call            write_long

                mov             al, 0x0a
                call            write_char

                jmp             exit


; multiplies long number by a long number
;    rdi -- address of multiplier #1 (long number)
;    rsi -- address of multiplier #2 (long number)
;    rcx -- length of long number in qwords
; result:
;    product is written to rsi (2x long number)
mul_long_long:
                push            rsi
                push            rbx
                push            r14

                lea             rsi, [rsi + rcx * 8 - 8]  ; shift rsi for reverse reading by multiplier #2

                push            rdi                       ; clearing [rsi + 8 ... rsi + rcx)
                lea             rdi, [rsi + 8]            ; to
                call            set_zero                  ; write
                pop             rdi                       ; result

                mov             rbp, rcx                  ; used as rcx for loop
                lea             r14, [rcx * 8 - 8]        ; r14 -- shift for mul_long_short_shifted_res_addr
                lea             r14, [r14 + rcx * 8]      ; r14 = (2 * rcx - 1) * 8
.loop:
                mov             rbx, qword [rsi]          ; short multiplier
                mov             qword [rsi], 0            ; clearing to write result

                call            mul_long_short_shifted_res_addr

                sub             r14, 8
                sub             rsi, 8
                dec             rbp
                jnz             .loop

                pop             r14
                pop             rbx
                pop             rsi
                ret

; multiplies long number by a short
;    rdi -- address of multiplier #1 (long number)
;    rbx -- multiplier #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
;    r14 -- shift the product relative to rdi
; result:
;    product is written to rdi + r14
mul_long_short_shifted_res_addr:
                push            rax
                push            rdi
                push            rsi
                push            r8

                xor             rsi, rsi             ; stores the current additional term
                mov             r8, rcx              ; used as rcx for loop
.loop:
                mov             rax, [rdi]
                mul             rbx                  ; rdx : rax = rax * rbx
                add             rax, rsi
                adc             rdx, 0

                add             rdi, r14
                call            add_long_short
                sub             rdi, r14

                add             rdi, 8
                mov             rsi, rdx
                dec             r8
                jnz             .loop

                lea             rdi, [rdi + r14];
                add             [rdi], rsi
                call            push_carry
                sub             rdi, r14

                pop             r8
                pop             rsi
                pop             rdi
                pop             rax
                ret

; adds carry to long number
;    rdi -- address of long number (starts addition with rdi + 8)
;    CF -- actual carry flag
; result:
;    sum is written to rdi
push_carry:
                push            rdi
.loop:
                lea             rdi, [rdi + 8]
                adc             qword [rdi], 0
                jc              .loop

                pop             rdi
                ret

; adds 64-bit number to long number
;    rdi -- address of summand #1 (long number)
;    rax -- summand #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    sum is written to rdi
add_long_short:
                push            rdi
                push            rcx
                push            rdx

                xor             rdx,rdx
.loop:
                add             [rdi], rax
                adc             rdx, 0
                mov             rax, rdx
                xor             rdx, rdx
                add             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rdx
                pop             rcx
                pop             rdi
                ret

; multiplies long number by a short
;    rdi -- address of multiplier #1 (long number)
;    rbx -- multiplier #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    product is written to rdi
mul_long_short:
                push            rax
                push            rdi
                push            rcx

                xor             rsi, rsi
.loop:
                mov             rax, [rdi]
                mul             rbx
                add             rax, rsi
                adc             rdx, 0
                mov             [rdi], rax
                add             rdi, 8
                mov             rsi, rdx
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rdi
                pop             rax
                ret

; divides long number by a short
;    rdi -- address of dividend (long number)
;    rbx -- divisor (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    quotient is written to rdi
;    rdx -- remainder
div_long_short:
                push            rdi
                push            rax
                push            rcx

                lea             rdi, [rdi + 8 * rcx - 8]
                xor             rdx, rdx

.loop:
                mov             rax, [rdi]
                div             rbx
                mov             [rdi], rax
                sub             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rax
                pop             rdi
                ret

; assigns a zero to long number
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
set_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep stosq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; checks if a long number is a zero
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
; result:
;    ZF=1 if zero
is_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep scasq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; read long number from stdin
;    rdi -- location for output (long number)
;    rcx -- length of long number in qwords
read_long:
                push            rcx
                push            rdi

                call            set_zero
.loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              .done
                cmp             rax, '0'
                jb              .invalid_char
                cmp             rax, '9'
                ja              .invalid_char

                sub             rax, '0'
                mov             rbx, 10
                call            mul_long_short
                call            add_long_short
                jmp             .loop

.done:
                pop             rdi
                pop             rcx
                ret

.invalid_char:
                mov             rsi, invalid_char_msg
                mov             rdx, invalid_char_msg_size
                call            print_string
                call            write_char
                mov             al, 0x0a
                call            write_char

.skip_loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              exit
                jmp             .skip_loop

; write long number to stdout
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
write_long:
                push            rax
                push            rcx

                mov             rax, 20
                mul             rcx
                mov             rbp, rsp
                sub             rsp, rax

                mov             rsi, rbp

.loop:
                mov             rbx, 10
                call            div_long_short
                add             rdx, '0'
                dec             rsi
                mov             [rsi], dl
                call            is_zero
                jnz             .loop

                mov             rdx, rbp
                sub             rdx, rsi
                call            print_string

                mov             rsp, rbp
                pop             rcx
                pop             rax
                ret

; read one char from stdin
; result:
;    rax == -1 if error occurs
;    rax \in [0; 255] if OK
read_char:
                push            rcx
                push            rdi

                sub             rsp, 1
                xor             rax, rax
                xor             rdi, rdi
                mov             rsi, rsp
                mov             rdx, 1
                syscall

                cmp             rax, 1
                jne             .error
                xor             rax, rax
                mov             al, [rsp]
                add             rsp, 1

                pop             rdi
                pop             rcx
                ret
.error:
                mov             rax, -1
                add             rsp, 1
                pop             rdi
                pop             rcx
                ret

; write one char to stdout, errors are ignored
;    al -- char
write_char:
                sub             rsp, 1
                mov             [rsp], al

                mov             rax, 1
                mov             rdi, 1
                mov             rsi, rsp
                mov             rdx, 1
                syscall
                add             rsp, 1
                ret

exit:
                mov             rax, 60
                xor             rdi, rdi
                syscall

; print string to stdout
;    rsi -- string
;    rdx -- size
print_string:
                push            rax

                mov             rax, 1
                mov             rdi, 1
                syscall

                pop             rax
                ret


                section         .rodata
invalid_char_msg:
                db              "Invalid character: "
invalid_char_msg_size: \
                equ             $ - invalid_char_msg
