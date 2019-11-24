#include <stdio.h>
#include <string.h>

//some definitions
#define FALSE 0
#define TRUE 1
//clock cycles
long long cycles = 0;

// registers
int regs[32];

// program counter
unsigned long pc = 0;

// memory
#define INST_MEM_SIZE 32 * 1024
#define DATA_MEM_SIZE 32 * 1024
unsigned long inst_mem[INST_MEM_SIZE]; //instruction memory
unsigned long data_mem[DATA_MEM_SIZE]; //data memory
unsigned char inst[32];				   // current instuction

// control
unsigned char regDst[2];
unsigned char jump;
unsigned char branch;
unsigned char memRead;
unsigned char memToReg[2];
unsigned char aluOp[2];
unsigned char memWrite;
unsigned char aluSrc;
unsigned char regWrite;

// the information of instruction
unsigned char inst_format;
unsigned char op;
unsigned char funct;
unsigned long offset;

// alu control
unsigned int aluControl;

// alu
unsigned char zero;
unsigned int aluResult;

// data
unsigned int readData;
unsigned int writeData;

// using register number
unsigned char rs;
unsigned char rt;
unsigned char rd;

//misc. function
int init(char *filename);
void print_cycles();
void print_reg();
void print_pc();
void fetch();
void decode();
void exe();
void mem();
void wb();
void update_pc();
void HexToBin();
void isOpcode();
void set_aluControl();
unsigned char set_readReg1();
unsigned char set_readReg2();
unsigned char set_writeReg();
void set_funct();
void set_offset();
void sign_extension();

int main(int ac, char *av[])
{
	if (ac < 3)
	{
		printf("./mips_sim filename mode\n");
		return -1;
	}

	char done = FALSE;
	if (init(av[1]) != 0)
		return -1;

	while (!done)
	{
		/* Todo */
		fetch();	 //fetch an instruction from the instruction memory
		decode();	//decode the instruction and read data from register file
		exe();		 //perform an appropriate operation
		mem();		 //access the data memory
		wb();		 //write result of arithmetic operation or data read from the data memory to register file
		update_pc(); //update the pc to the address of the next instruction

		cycles++; //increase clock cycle

		//if debug mode, print clock cycle, pc, reg
		if (*av[2] == '0')
		{
			print_cycles(); //print clock cycles
			print_pc();		//print pc
			print_reg();	//print registers
		}
		// check the exit condition
		if (regs[9] == 10) //if value in $t1 is 10, finish the simulation
			done = TRUE;
	}

	if (*av[2] == '1')
	{
		print_cycles(); //print clock cycles
		print_pc();		//print pc
		print_reg();	//print registers
	}

	return 0;
}

/* initialize all datapat elements
//fill the instruction and data memory
//reset the registers
*/
int init(char *filename)
{
	FILE *fp = fopen(filename, "r");
	int i;
	long inst;

	if (fp == NULL)
	{
		fprintf(stderr, "Error opening file.\n");
		return -1;
	}

	/* fill instruction memory */
	i = 0;
	while (fscanf(fp, "%lx", &inst) == 1)
	{
		inst_mem[i++] = inst;
	}

	/*reset the registers*/
	for (i = 0; i < 32; i++)
	{
		regs[i] = 0;
	}

	/*reset pc*/
	pc = 0;
	/*reset clock cycles*/
	cycles = 0;
	return 0;
}

void print_cycles()
{
	printf("---------------------------------------------------\n");

	printf("Clock cycles = %lld\n", cycles);
}

void print_pc()
{
	printf("PC	   = %ld\n\n", pc);
}

void print_reg()
{
	printf("R0   [r0] = %d\n", regs[0]);
	printf("R1   [at] = %d\n", regs[1]);
	printf("R2   [v0] = %d\n", regs[2]);
	printf("R3   [v1] = %d\n", regs[3]);
	printf("R4   [a0] = %d\n", regs[4]);
	printf("R5   [a1] = %d\n", regs[5]);
	printf("R6   [a2] = %d\n", regs[6]);
	printf("R7   [a3] = %d\n", regs[7]);
	printf("R8   [t0] = %d\n", regs[8]);
	printf("R9   [t1] = %d\n", regs[9]);
	printf("R10  [t2] = %d\n", regs[10]);
	printf("R11  [t3] = %d\n", regs[11]);
	printf("R12  [t4] = %d\n", regs[12]);
	printf("R13  [t5] = %d\n", regs[13]);
	printf("R14  [t6] = %d\n", regs[14]);
	printf("R15  [t7] = %d\n", regs[15]);
	printf("R16  [s0] = %d\n", regs[16]);
	printf("R17  [s1] = %d\n", regs[17]);
	printf("R18  [s2] = %d\n", regs[18]);
	printf("R19  [s3] = %d\n", regs[19]);
	printf("R20  [s4] = %d\n", regs[20]);
	printf("R21  [s5] = %d\n", regs[21]);
	printf("R22  [s6] = %d\n", regs[22]);
	printf("R23  [s7] = %d\n", regs[23]);
	printf("R24  [t8] = %d\n", regs[24]);
	printf("R25  [t9] = %d\n", regs[25]);
	printf("R26  [k0] = %d\n", regs[26]);
	printf("R27  [k1] = %d\n", regs[27]);
	printf("R28  [gp] = %d\n", regs[28]);
	printf("R29  [sp] = %d\n", regs[29]);
	printf("R30  [fp] = %d\n", regs[30]);
	printf("R31  [ra] = %d\n", regs[31]);
	printf("\n");
}

void HexToBin()
{
	unsigned long hex;
	memset(inst, 0, 32);
	hex = inst_mem[pc / 4];
	for (int i = 0; i < 32; i++)
	{
		inst[i] = hex % 2;
		hex = hex / 2;
	}
}

void fetch()
{
	HexToBin();
	isOpcode();
	switch (inst_format)
	{
	case 'R':
	case 'r':
		regDst[1] = 0;
		regDst[0] = 1;
		jump = 0;
		aluSrc = 0;
		memToReg[1] = 0;
		memToReg[0] = 0;
		regWrite = 1;
		memRead = 0;
		memWrite = 0;
		branch = 0;
		aluOp[1] = 1;
		aluOp[0] = 0;
		break;
	case 'I':
	case 'i':
		if (op == 4)
		{ // beq instruction
			regDst[1] = 0;
			regDst[0] = 0;
			jump = 0;
			aluSrc = 0;
			memToReg[1] = 0;
			memToReg[0] = 0;
			regWrite = 0;
			memRead = 0;
			memWrite = 0;
			branch = 1;
			aluOp[1] = 0;
			aluOp[0] = 1;
		}
		else if (op == 8)
		{ // addi instruction
			regDst[1] = 0;
			regDst[0] = 0;
			jump = 0;
			aluSrc = 1;
			memToReg[1] = 0;
			memToReg[0] = 0;
			regWrite = 1;
			memRead = 0;
			memWrite = 0;
			branch = 0;
			aluOp[1] = 0;
			aluOp[0] = 0;
		}
		else if (op == 10)
		{ // slti instruction
			regDst[1] = 0;
			regDst[0] = 0;
			jump = 0;
			aluSrc = 1;
			memToReg[1] = 0;
			memToReg[0] = 0;
			regWrite = 1;
			memRead = 0;
			memWrite = 0;
			branch = 0;
			aluOp[1] = 0;
			aluOp[0] = 1;
		}
		else if (op == 35)
		{ // lw instruction
			regDst[1] = 0;
			regDst[0] = 0;
			jump = 0;
			aluSrc = 1;
			memToReg[1] = 0;
			memToReg[0] = 1;
			regWrite = 1;
			memRead = 1;
			memWrite = 0;
			branch = 0;
			aluOp[1] = 0;
			aluOp[0] = 0;
		}
		else if (op == 43)
		{ // sw instruction
			regDst[1] = 0;
			regDst[0] = 0;
			jump = 0;
			aluSrc = 1;
			memToReg[1] = 0;
			memToReg[0] = 0;
			regWrite = 0;
			memRead = 0;
			memWrite = 1;
			branch = 0;
			aluOp[1] = 0;
			aluOp[0] = 0;
		}
		break;
	case 'J':
	case 'j':
		if (op == 2)
		{ // j instruction
			regDst[1] = 0;
			regDst[0] = 0;
			jump = 1;
			aluSrc = 0;
			memToReg[1] = 0;
			memToReg[0] = 0;
			regWrite = 0;
			memRead = 0;
			memWrite = 0;
			branch = 0;
			aluOp[1] = 0;
			aluOp[0] = 0;
		}
		else if (op == 3)
		{				   // jal instruction
			regDst[1] = 1; // $ra
			regDst[0] = 0;
			jump = 1;
			aluSrc = 0;
			memToReg[1] = 1; // for $ra
			memToReg[0] = 0;
			regWrite = 1; // write the data into $ra register
			memRead = 0;
			memWrite = 0;
			branch = 0;
			aluOp[1] = 0;
			aluOp[0] = 0;
		}
		break;
	}
}

void isOpcode()
{
	/* msb 6-digits */
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

void decode()
{
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
			rd = set_readReg2();
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

void exe()
{
	set_aluControl();
	zero = 0;
	
	/* omit the ALU control of jr instuction */
	if (inst_format == 'r' && funct == 8) {;}
	
	/* omit the ALU control of and */
	else if (aluControl == 0) {;}
	
	/* omit the ALU control of OR */
	else if (aluControl == 1) {;}
	
	else if (aluControl == 2)	// add
	{
		if (aluSrc == 0)
			aluResult = regs[rs] + regs[rt];	// R-format
		else
			aluResult = regs[rs] + offset;		// I-format
	}
	else if (aluControl == 6)	// subtract
	{ 
		if (aluSrc == 0)
			aluResult = regs[rs] - regs[rd];
		else
			aluResult = regs[rs] - offset;
		if (aluResult == 0)
			zero = 1;
		if (op == 10)
		{
			if ((aluResult >> 31) == 1)
				aluResult = 1;
			else
				aluResult = 0;
		}
	}

	/* omit the ALU control of set on less than */
	else if (aluControl == 7) {;}
}

void mem()
{
	/* jr instruction does noting */
	if (inst_format == 'r' && funct == 8) {;}
	else if (memWrite == 1) // sw instruction
		data_mem[aluResult] = regs[rd];
	else if (memRead == 1) // lw instruction
		readData = data_mem[aluResult];
}

void wb()
{
	/* jr instruction does noting */
	if (inst_format == 'r' && funct == 8) {;}
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

void update_pc()
{
	if (inst_format == 'r' && funct == 8) // jr instruction
		pc = regs[31];
	else if (jump == 1)
	{
		pc = 0;
		/* omit [31-7] */
		pc += inst[6] << 6;
		pc += inst[5] << 5;
		pc += inst[4] << 4;
		pc += inst[3] << 3;
		pc += inst[2] << 2;
		pc += inst[1] << 1;
		pc += inst[0];
		pc = pc << 2;
	}
	else
	{
		pc = pc + 4;
		if ((branch == 1) && (zero == 1))
		{
			offset = offset << 2;
			pc = pc + offset;
		}
	}
}

void set_aluControl()
{
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

unsigned char set_readReg1()
{
	unsigned char readRegNum = 0;
	readRegNum += inst[25] << 4;
	readRegNum += inst[24] << 3;
	readRegNum += inst[23] << 2;
	readRegNum += inst[22] << 1;
	readRegNum += inst[21];
	return readRegNum;
}

unsigned char set_readReg2()
{
	unsigned char tempRegNum = 0;
	tempRegNum += inst[20] << 4;
	tempRegNum += inst[19] << 3;
	tempRegNum += inst[18] << 2;
	tempRegNum += inst[17] << 1;
	tempRegNum += inst[16];
	return tempRegNum;
}

unsigned char set_writeReg()
{
	unsigned char tempRegNum = 0;
	tempRegNum += inst[15] << 4;
	tempRegNum += inst[14] << 3;
	tempRegNum += inst[13] << 2;
	tempRegNum += inst[12] << 1;
	tempRegNum += inst[11];
	return tempRegNum;
}

void set_funct()
{
	funct = 0;
	funct += inst[5] << 5;
	funct += inst[4] << 4;
	funct += inst[3] << 3;
	funct += inst[2] << 2;
	funct += inst[1] << 1;
	funct += inst[0];
}

void set_offset()
{
	offset = 0;
	offset += inst[15] << 15;
	offset += inst[14] << 14;
	offset += inst[13] << 13;
	offset += inst[12] << 12;
	offset += inst[11] << 11;
	offset += inst[10] << 10;
	offset += inst[9] << 9;
	offset += inst[8] << 8;
	offset += inst[7] << 7;
	offset += inst[6] << 6;
	offset += inst[5] << 5;
	offset += inst[4] << 4;
	offset += inst[3] << 3;
	offset += inst[2] << 2;
	offset += inst[1] << 1;
	offset += inst[0];
	sign_extension();
}

void sign_extension()
{
	unsigned char msb = inst[15];
	if (msb == 1)
	{
		offset += 1 << 31;
		offset += 1 << 30;
		offset += 1 << 29;
		offset += 1 << 28;
		offset += 1 << 27;
		offset += 1 << 26;
		offset += 1 << 25;
		offset += 1 << 24;
		offset += 1 << 23;
		offset += 1 << 22;
		offset += 1 << 21;
		offset += 1 << 20;
		offset += 1 << 19;
		offset += 1 << 18;
		offset += 1 << 17;
		offset += 1 << 16;
	}
}