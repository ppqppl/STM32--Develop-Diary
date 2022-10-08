 AREA MYDATA, DATA

 AREA MYCODE, CODE
	ENTRY
	EXPORT __main

__main
	MOV R0, #10
	MOV R1, #11
	MOV R2, #12
	MOV R3, #13
	;LDR R0, =func01

	BL	func01
	;LDR R1, =func02
	BL	func02

	BL 	func03
	LDR LR, =func01
	LDR PC, =func03
	B .

func01
	MOV R5, #05
	BX LR

func02
	MOV R6, #06
	BX LR

func03
	MOV R7, #07
	MOV R8, #08	
	BX LR
	
	END