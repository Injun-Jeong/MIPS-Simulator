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

void isOpcode(){
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
	if (inst_format == 'j')
	{
		set_offset();
		if (regDst[1] == 1)
			rd = 31;
	}
	else
	{
		if (regDst[0] == 0)
		{ // I-format instruction
			rs = set_readReg1();
			rt = set_readReg2();
			set_offset();
		}
		else
		{ // R-format instruction
			set_funct();
			if (funct == 8)
			{ // jr instruction
				rs = 31;
				rt = 0;
				rd = 0;
			}
			else
			{
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






