# Project 2: MIPS Simulator
#### The goal of this project is to write a simulator program that simulates the MIPS microarchitecture (single cycle execution). This simulator should compile and run bellow instructions: add, addi, jal, j, jr, lw, sw, slti, beq
---
## Usage
\
$ gcc mips_sim.c -o mips_sim
$ ./mips_sim ./runme.hex 0 		// debug mode
$ ./mips_sim ./runme.hex 1 		// run mode
\
---
## Attribute
### Control attribute
'''
c
unsigned char regDst[2];
unsigned char jump;
unsigned char branch;
unsigned char memRead;
unsigned char memToReg[2];
unsigned char aluOp[2];
unsigned char memWrite;
unsigned char aluSrc;
unsigned char regWrite;
'''
1. ***regDst*** indicates that this instruction uses rd-register or not.
2. ***jump*** indicates that this instruction is J-instruction or not.
