   AREA EXAMPLE,CODE,READONLY
	ENTRY
	EXPORT __main
		
__main
	LDR SP,=0X40000000
        
        ;ldr r4,=0x40000000
        ;SWAP
        ;ldr r5,[r4]
        ;ldr r6,[r4,#4]
        
        ;str r5,[r4,#4]
        ;str r6,[r4]
        
        ;ldr r5,[r4]
        ;ldr r6,[r4,#4]
        
        ;R0,R2 INNITAIL

	MOV R2,R0

LOOP_1    CMP R1,R0
        BGE LOOP_END
        ADD R1,R1,#1
        SUB    R2,R2,#1
        MOV R3,#0
        LDR R4,=0X40000000
        B    LOOP_2
        
LOOP_2    CMP    R3,R2
        BGE LOOP_1
        ADD R3,R3,#1
        LDR R5,[R4]
        LDR R6,[R4,#4]
        ADD R4,R4,#4
        CMP    R5,R6
        BLS LOOP_2
        SUB R4,R4,#4
        STR R5,[R4,#4]
        STR R6,[R4]
        ADD R4,R4,#4
        B LOOP_2
        
LOOP_END
        MOV R2,R0,LSR#1
        MOV R3,#4
        MUL    R1,R2,R3
        LDR    R1,[SP,R1]
        
        END