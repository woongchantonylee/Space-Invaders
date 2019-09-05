; Print.s
; Student names: Woongchan Lee and Ben Pretzer
; Last modification date: 4/22/2019
; Runs on LM4F120 or TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB
big EQU 10000

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
	PUSH {R0,R1,R4,LR}	; save registers
	CMP R0,#10
	BLO Base			; base case if R0 < 10
	MOV R4,#10
	MOV R1,R0
	UDIV R0,R4
	BL LCD_OutDec		; OutDec(num/10)
	MOV R0,R1
	UDIV R1,R0,R4		; mod function
	MUL R1,R4
	SUB R0,R1			; R0 = R0 % 10
	ADD R0,#0x30
	BL ST7735_OutChar	; ST7735_OutChar(num % 10)
	B Done
Base
	ADD R0,#0x30		; base case
	BL ST7735_OutChar
Done
	POP {R0,R1,R4,LR}
    BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
	PUSH {R4,LR}
	LDR R1,=big
	CMP R0,R1
	BHS asterisk		; if input is too large
	
	MOV R3,#1000
	MOV R1,R0
	UDIV R1,R3
	ADD R1,#0x30
	MOV R4,R0
	MOV R0,R1
	BL ST7735_OutChar	; print "#"
	MOV R0,R4
	
	MOV R1,#0x2E		; contains '.'
	MOV R4,R0
	MOV R0,R1
	BL ST7735_OutChar	; print "."
	MOV R0,R4
	
	MOV R3,#100
	MOV R1,R0
	UDIV R1,R3
	MOV R3,#10
	UDIV R2,R1,R3		; mod function
	MUL R2,R3
	SUB R1,R2			; R1 = R1 % 10
	ADD R1,#0x30
	MOV R4,R0
	MOV R0,R1
	BL ST7735_OutChar	; print "#"
	MOV R0,R4
	
	MOV R3,#10
	MOV R1,R0
	UDIV R1,R3
	UDIV R2,R1,R3		; mod function
	MUL R2,R3
	SUB R1,R2			; R1 = R1 % 10
	ADD R1,#0x30
	MOV R4,R0
	MOV R0,R1
	BL ST7735_OutChar	; print "#"
	MOV R0,R4
	
	MOV R3,#10
	MOV R1,R0
	UDIV R2,R1,R3		; mod function
	MUL R2,R3
	SUB R1,R2			; R1 = R1 % 10
	ADD R1,#0x30
	MOV R4,R0
	MOV R0,R1
	BL ST7735_OutChar	; print "#"
	MOV R0,R4

	MOV R1,#0x20		; contains ' '
	MOV R0,R1
	BL ST7735_OutChar	; print " "
	B done
	
asterisk
	MOV R0,#0x2A		; contains '*'
	BL ST7735_OutChar	; print "*"
	MOV R0,#0x2E		; contains '.'
	BL ST7735_OutChar	; print "."
	MOV R0,#0x2A		; contains '*'
	BL ST7735_OutChar	; print "*"
	MOV R0,#0x2A		; contains '*'
	BL ST7735_OutChar	; print "*"
	MOV R0,#0x2A		; contains '*'
	BL ST7735_OutChar	; print "*"
	MOV R0,#0x20		; contains ' '
	BL ST7735_OutChar	; print " "

done
	POP {R4,LR}
	BX LR
;* * * * * * * * End of LCD_OutFix * * * * * * * *

    ALIGN                           ; make sure the end of this section is aligned
    END                             ; end of file
