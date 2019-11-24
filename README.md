# Project 2: MIPS Simulator
#### The goal of this project is to write a simulator program that simulates the MIPS microarchitecture (single cycle execution). This simulator should compile and run bellow instructions: add, addi, jal, j, jr, lw, sw, slti, beq
---
## Usage
```
$ gcc mips_sim.c -o mips_sim
$ ./mips_sim ./runme.hex 0 		// debug mode
$ ./mips_sim ./runme.hex 1 		// run mode
```
---
## Example
1. add_test.asm
```
$ cat add_test.asm

addi $s0, $zero, 10
addi $s1, $zero, 20
add $s2, $s0, $s1
addi $t1, $zero, 10
```
***add_test.hex*** is the representation of assambly code by **hexadecimal** according to ***add_test.asm***.
```
$ cat add_test.hex
2010000a
20110014
02119020
2009000a
```
**Execute** the mips_sim for ***add_test.hex*** in **run mode**.
```
$ ./mips_sim add_hex 1

Clock cycles = 4
PC	   = 16

R0   [r0] = 0
R1   [at] = 0
R2   [v0] = 0
R3   [v1] = 0
R4   [a0] = 0
R5   [a1] = 0
R6   [a2] = 0
R7   [a3] = 0
R8   [t0] = 0
R9   [t1] = 10
R10  [t2] = 0
R11  [t3] = 0
R12  [t4] = 0
R13  [t5] = 0
R14  [t6] = 0
R15  [t7] = 0
R16  [s0] = 10
R17  [s1] = 20
R18  [s2] = 30
R19  [s3] = 0
R20  [s4] = 0
R21  [s5] = 0
R22  [s6] = 0
R23  [s7] = 0
R24  [t8] = 0
R25  [t9] = 0
R26  [k0] = 0
R27  [k1] = 0
R28  [gp] = 0
R29  [sp] = 0
R30  [fp] = 0
R31  [ra] = 0
```

2. runme.asm
```
$ cat runme.asm

  main:
        addi $sp, $zero, 32764
        addi $a0, $zero, 10
        jal accm
        j   exit
  accm:
        addi  $sp, $sp, -8
        sw    $ra, 4($sp)  # save the return address
        sw    $a0, 0($sp)
        slti  $t0,$a0,1     # test for n < 1
        beq   $t0,$zero,L1  # if n >= 1, go to L1
        addi  $v0,$zero,0 # return 0
        addi  $sp,$sp,8   # pop 2 items off stack
        jr    $ra         # return to caller
  L1:
        addi $a0,$a0,-1
        jal accm         # call fact with (n –1)
        lw $a0, 0($sp) # return from jal: restore argument n lw $ra, 4($sp) # restore the return address
        lw $ra, 4($sp) # restore the return address
        addi $sp, $sp, 8 # adjust stack pointer to pop 2 items
        add $v0, $a0, $v0
        jr   $ra           # return to the caller
 
  exit:
        addi $t1, $zero, 10  #exit simulation if $t1=10
```
***runme.hex*** is the representation of assambly code by **hexadecimal** according to ***runme.asm***.
```
$ runme.hex

201d7ffc
2004000a
0c000004
08000013
23bdfff8
afbf0004
afa40000
28880001
11000003
20020000
23bd0008
03e00008
2084ffff
0c000004
8fa40000
8fbf0004
23bd0008
00821020
03e00008
2009000a
```
**Execute** the mips_sim for ***runme.hex*** in **run mode**.
```
$ ./mips_sim runme.hex 1

Clock cycles = 133
PC	   = 80

R0   [r0] = 0
R1   [at] = 0
R2   [v0] = 55
R3   [v1] = 0
R4   [a0] = 10
R5   [a1] = 0
R6   [a2] = 0
R7   [a3] = 0
R8   [t0] = 1
R9   [t1] = 10
R10  [t2] = 0
R11  [t3] = 0
R12  [t4] = 0
R13  [t5] = 0
R14  [t6] = 0
R15  [t7] = 0
R16  [s0] = 0
R17  [s1] = 0
R18  [s2] = 0
R19  [s3] = 0
R20  [s4] = 0
R21  [s5] = 0
R22  [s6] = 0
R23  [s7] = 0
R24  [t8] = 0
R25  [t9] = 0
R26  [k0] = 0
R27  [k1] = 0
R28  [gp] = 0
R29  [sp] = 32764
R30  [fp] = 0
R31  [ra] = 12
```
---
## Code
### Attribute
```c
unsigned char regDst[2];
unsigned char jump;
unsigned char branch;
unsigned char memRead;
unsigned char memToReg[2];
unsigned char aluOp[2];
unsigned char memWrite;
unsigned char aluSrc;
unsigned char regWrite;
```
1. ***regDst*** indicates that **this instruction uses rd-register or not**.
```
/* This instruction is jal instruction and uses the rd-register for $ra register. */
regDst[1] = 1;
regDst[0] = 0;

/* The format of this instruction is R-format, so this instruction uses the rd-register. */
regDst[1] = 0;
regDst[0] = 1;

/*
* This instruction is I-format instruction or j instruction.
* So this instruction does not use the rd-register.
*/
regDst[1] = 0;
regDst[0] = 0;
```
2. ***jump*** indicates that **this instruction is J-instruction or not**. The value of jump for all instruction has **0** except J-format instructions.
 
3. ***branch*** indicates that this istruction is beq or not. If the instruction is beq, then the value of branch is **1**.

4. ***memRead*** controls whether this instruction reads the data in memory or not. If the value of memRead is **1**, then this instruction reads the data according to memory address **at mem section**.

5. ***memToReg*** controls that what data will be stored into register **at wb section**.
```
/* This instruction is jal instruction. The PC + 4 is stored into $ra register. */
memToReg[1] = 1;
memToReg[0] = 0;

/* This case will store the result of ALU into the write register. */
memToReg[1] = 0;
memToReg[0] = 0;

/* This case will store the data of memory into the write register. */
memToReg[1] = 0;
memToReg[0] = 1;
```

6. ***aluOp*** with the data of instrucion indicates what for ALU to do.

7. ***memWrite*** controls whether this instruction writes the data of register into memory or not. If the value of memWrite is **1**, then this instruction writes the data according to memory address **at mem section**.

8. ***aluSrc*** indicates that what data will be used in ALU. If the value of aluSrc is **0**, then ALU uses the data of register. If the value of aluSRc is **1**, then ALU uses the data of instruction's offset.

9. ***regWrite*** controls whether some data will be written in register or not. If the value of regWrite is **0**, then nothing does **at wb section** except jal.
---
## Method
```c
void fetch();
void decode();
void exe();
void mem();
void wb();
void update_pc();
```
1. fetch(): Instruction fetch from memory
```c
void fetch(){

	/* Translating hex data into binary data. */
	HexToBin();

	/* Confirming the format of instruction */
	isOpcode();

	switch (inst_format){
		case 'R':
		case 'r':
			...;
		case 'I':
		case 'i':
			...;
		case 'J':
		case 'j':
			...;
	}
}

void isOpcode() {
	/* MSB 6-digits */
	op = 0;
	op += inst[31] << 5;
	op += inst[30] << 4;
	op += inst[29] << 3;
	op += inst[28] << 2;
	op += inst[27] << 1;
	op += inst[26];

	if (op == 0)
		inst_format = 'r';
	else if (op == 2 || op == 3)
		inst_format = 'j';
	else
		inst_format = 'i';
}
```
2. decode(): Instruction decode & register read
```c
void decode() {
	if (inst_format == 'j') {
		set_offset();
		if (regDst[1] == 1)
			rd = 31;
	}
	else {
		if (regDst[0] == 0) { // I-format instruction
			rs = set_readReg1();
			rt = set_readReg2();
			set_offset();
		}
		else { // R-format instruction
			set_funct();
			if (funct == 8) { // jr instruction
				rs = 31;
				rt = 0;
				rd = 0;
			}
			else {
				rs = set_readReg1();
				rt = set_readReg2();
				rd = set_writeReg();
			}
		}
	}
}
```
3. exe(): Execute operation or calculate address
```c
void exe() {
	set_aluControl();
	zero = 0;
	
	/* jr instuction does nothing */
	if (aluOp[1] = 1 && aluOp[0] == 0 && funct == 8) {;}
	
	/* the ALU control of and */
	else if (aluControl == 0) { ... }
	
	/* the ALU control of OR */
	else if (aluControl == 1) { ... }
	
	/* the ALU control of ADD */
	else if (aluControl == 2) { ... }

	/* the ALU control of SUBTRACT */
	else if (aluControl == 6) { ... }

	/* the ALU control of SET ON LESS THAN */
	else if (aluControl == 7) { ... }
}

/* Decide what for ALU to do */
void set_aluControl() {
	aluControl = 0;
	if (aluOp[1] == 0 && aluOp[0] == 0)
		aluControl = 2; // add
	else if (aluOp[0] == 1)
		aluControl = 6; // subtract
	else if (aluOp[1] == 1)
	{
		if (funct == 32)
			aluControl = 2; // add
		else if (funct == 34)
			aluControl = 6; // subtract
		else if (funct == 36)
			aluControl = 0; // and
		else if (funct == 37)
			aluControl = 1; // or
		else if (funct == 42)
			aluControl = 7; // set on less than
	}
}
```
4. mem(): Access memory operand
```c
void mem() {
	/* jr instruction does nothing */
	if (aluOp[1] = 1 && aluOp[0] == 0 && funct == 8) {;}
	else if (memWrite == 1) // sw instruction
		data_mem[aluResult] = regs[rd];
	else if (memRead == 1) // lw instruction
		readData = data_mem[aluResult];
}
```
5. wb(): Write resultback to register
```c
void wb() {
	/* jr instruction does noting */
	if (aluOp[1] = 1 && aluOp[0] == 0 && funct == 8) {;}
	else if (memToReg[1] == 1) // jal
		regs[31] = pc + 4;
	else
	{
		if (memToReg[0] == 1)
			writeData = readData;
		else
			writeData = aluResult;
		if (regWrite == 1)
			regs[rd] = writeData;
	}
}
```
6. update_pc(): Update the program counter
```c
void update_pc() {
	/* jr instrucion */
	if (the instruction is jr)
		pc <- $ra 

	/* J-format instrucion */
	else if (the value of jump is 1)
		pc <- offset x 4

	/* The other instruction or branch instruction */
	else
		pc = pc + 4
		if (the instruction is branch AND the constraint is true)
			offset <- offset x 4			
			pc <- pc + offset
}
```






