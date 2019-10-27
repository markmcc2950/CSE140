#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "computer.h"
#undef mips			/* gcc already has a def for mips */

unsigned int endianSwap(unsigned int);

void PrintInfo (int changedReg, int changedMem);
unsigned int Fetch (int);
void Decode (unsigned int, DecodedInstr*, RegVals*);
int Execute (DecodedInstr*, RegVals*);
int Mem(DecodedInstr*, int, int *);
void RegWrite(DecodedInstr*, int, int *);
void UpdatePC(DecodedInstr*, int);
void PrintInstruction (DecodedInstr*);

/*Globally accessible Computer variable*/
Computer mips;
RegVals rVals;

/*
 *  Return an initialized computer with the stack pointer set to the
 *  address of the end of data memory, the remaining registers initialized
 *  to zero, and the instructions read from the given file.
 *  The other arguments govern how the program interacts with the user.
 */
void InitComputer (FILE* filein, int printingRegisters, int printingMemory,
  int debugging, int interactive) {
    int k;
    unsigned int instr;

    /* Initialize registers and memory */

    for (k=0; k<32; k++) {
        mips.registers[k] = 0;
    }
    
    /* stack pointer - Initialize to highest address of data segment */
    mips.registers[29] = 0x00400000 + (MAXNUMINSTRS+MAXNUMDATA)*4;

    for (k=0; k<MAXNUMINSTRS+MAXNUMDATA; k++) {
        mips.memory[k] = 0;
    }

    k = 0;
    while (fread(&instr, 4, 1, filein)) {
	/*swap to big endian, convert to host byte order. Ignore this.*/
        mips.memory[k] = ntohl(endianSwap(instr));
        k++;
        if (k>MAXNUMINSTRS) {
            fprintf (stderr, "Program too big.\n");
            exit (1);
        }
    }

    mips.printingRegisters = printingRegisters;
    mips.printingMemory = printingMemory;
    mips.interactive = interactive;
    mips.debugging = debugging;
}

unsigned int endianSwap(unsigned int i) {
    return (i>>24)|(i>>8&0x0000ff00)|(i<<8&0x00ff0000)|(i<<24);
}

/*
 *  Run the simulation.
 */
void Simulate () {
    char s[40];  /* used for handling interactive input */
    unsigned int instr;
    int changedReg=-1, changedMem=-1, val;
    DecodedInstr d;
    
    /* Initialize the PC to the start of the code section */
    mips.pc = 0x00400000;
    while (1) {
        if (mips.interactive) {
            printf ("> ");
            fgets (s,sizeof(s),stdin);
            if (s[0] == 'q') {
                return;
            }
        }

        /* Fetch instr at mips.pc, returning it in instr */
        instr = Fetch (mips.pc);

        printf ("Executing instruction at %8.8x: %8.8x\n", mips.pc, instr);

        /* 
	 * Decode instr, putting decoded instr in d
	 * Note that we reuse the d struct for each instruction.
	 */
        Decode (instr, &d, &rVals);

        /*Print decoded instruction*/
        PrintInstruction(&d);

        /* 
	 * Perform computation needed to execute d, returning computed value 
	 * in val 
	 */
        val = Execute(&d, &rVals);

	UpdatePC(&d,val);

        /* 
	 * Perform memory load or store. Place the
	 * address of any updated memory in *changedMem, 
	 * otherwise put -1 in *changedMem. 
	 * Return any memory value that is read, otherwise return -1.
         */
        val = Mem(&d, val, &changedMem);

        /* 
	 * Write back to register. If the instruction modified a register--
	 * (including jal, which modifies $ra) --
         * put the index of the modified register in *changedReg,
         * otherwise put -1 in *changedReg.
         */
        RegWrite(&d, val, &changedReg);

        PrintInfo (changedReg, changedMem);
    }
}

/*
 *  Print relevant information about the state of the computer.
 *  changedReg is the index of the register changed by the instruction
 *  being simulated, otherwise -1.
 *  changedMem is the address of the memory location changed by the
 *  simulated instruction, otherwise -1.
 *  Previously initialized flags indicate whether to print all the
 *  registers or just the one that changed, and whether to print
 *  all the nonzero memory or just the memory location that changed.
 */
void PrintInfo ( int changedReg, int changedMem) {
    int k, addr;
    printf ("New pc = %8.8x\n", mips.pc);
    if (!mips.printingRegisters && changedReg == -1) {
        printf ("No register was updated.\n");
    } else if (!mips.printingRegisters) {
        printf ("Updated r%2.2d to %8.8x\n",
        changedReg, mips.registers[changedReg]);
    } else {
        for (k=0; k<32; k++) {
            printf ("r%2.2d: %8.8x  ", k, mips.registers[k]);
            if ((k+1)%4 == 0) {
                printf ("\n");
            }
        }
    }
    if (!mips.printingMemory && changedMem == -1) {
        printf ("No memory location was updated.\n");
    } else if (!mips.printingMemory) {
        printf ("Updated memory at address %8.8x to %8.8x\n",
        changedMem, Fetch (changedMem));
    } else {
        printf ("Nonzero memory\n");
        printf ("ADDR	  CONTENTS\n");
        for (addr = 0x00400000+4*MAXNUMINSTRS;
             addr < 0x00400000+4*(MAXNUMINSTRS+MAXNUMDATA);
             addr = addr+4) {
            if (Fetch (addr) != 0) {
                printf ("%8.8x  %8.8x\n", addr, Fetch (addr));
            }
        }
    }
}
/*
 *  Return the contents of memory at the given address. Simulates
 *  instruction fetch. 
 */
unsigned int Fetch ( int addr) {
    return mips.memory[(addr-0x00400000)/4];
}

// Kills program
void die(){
	exit(0);
}

// Base Hex: 0x00000000 or 0xffffffff
void R_Decode(unsigned int instr, DecodedInstr* d, RegVals* rVals) {
	// Funct is last 6 binary digits:  11 1111 = 0x0000003f
	d->regs.r.funct = instr & 0x0000003f;
	// Move instruction over by 6 bits to check next component... repeat for other components
	instr = instr >> 6; 
	// Shamt is next 5 binary digits: 1 1111 = 0x0000001f
	d->regs.r.shamt = instr & 0x000001f;
	instr = instr >> 5;
	// RD is next 5 binary digits: 1 1111 = 0x00001f
	d->regs.r.rd = instr & 0x00001f;
	instr = instr >> 5;
	// RT is next 5 binary digits: 1 1111 = 0x001f
	d->regs.r.rt = instr & 0x001f;
	instr = instr >> 5;
	// RS is next 5 binary digits: 1 1111 = 0x01f
	d->regs.r.rs = instr & 0x01f;
	// Then fill Registers
	rVals->R_rs = mips.registers[d->regs.r.rs];
	rVals->R_rt = mips.registers[d->regs.r.rt];
	rVals->R_rd = mips.registers[d->regs.r.rd];
	if ( d->regs.r.rd == 0 && d->regs.r.funct != 8 ) die(); // Unsupported Write
}

void sign_extend(DecodedInstr* d){
	// Recover the sign bit 1000 0000 0000 0000 
	// If negative
	if ((d->regs.i.addr_or_immed & 0x8000) > 0 ) {
		// Sign extension for negatives... 
		// Immediate is 16-bit, so value is always 0x0000XXXX
		// To sign extend, use OR bitwise operation with 0xffff0000
		// First 4 bytes will always become 1's, the rest will depend on value of immediate
		d->regs.i.addr_or_immed = d->regs.i.addr_or_immed | 0xffff0000;
	}
	// If positive, leave as is because front value are already 0's
}


void I_Decode(unsigned int instr, DecodedInstr* d, RegVals* rVals){
	// Immediate is last 16 binary digits: 1111 1111 1111 1111 = 0x0000ffff
	d->regs.i.addr_or_immed = instr & 0x0000ffff;
	sign_extend(d);
	//printf("Val = %d\n", d->regs.i.addr_or_immed);
	instr = instr >> 16;
	// RT is next 5 binary digits: 0000 0000 0001 1111 = 0x001f
	d->regs.i.rt = instr & 0x001f;
	instr = instr >> 5;
	// RS is next 5 binary digits: -000 0001 1111 = 0x01f
	d->regs.i.rs = instr & 0x01f;
	// Then fill registers
	rVals->R_rs = mips.registers[d->regs.i.rs];
	rVals->R_rt = mips.registers[d->regs.i.rt];
	if (d->op != 4 && d-> op != 5) {
		if ( d->regs.i.rt == 0 ) die(); // Unsupported Write
	}
}
void J_Decode(unsigned int instr, DecodedInstr* d, RegVals* rVals){
	// Target is last 26 binary digits: 0x03ffffff
	// Remember to shift it over 2 digits
	d->regs.j.target = (instr & 0x03ffffff) << 2;
	// Then copy remaining 4 digits from mips.pc
	d->regs.j.target = (mips.pc & 0xf0000000) + d->regs.j.target;
	// No register fill needed for J-format
}

/* Decode instr, returning decoded instruction. */
void Decode ( unsigned int instr, DecodedInstr* d, RegVals* rVals) {
	//printf("Reached Decode\n");
	// Grab first 6 bits of instruction
	d->op = instr >> 26;
	//printf("%d\n", d->op);
	// Determine Format from Opcode
	switch(d->op) {
		case 0:
			d->type = R;
			R_Decode(instr, d, rVals);
			break;
		case 9:
		case 12:
		case 13:
		case 15:
		case 4:
		case 5:
		case 35:
		case 43:
			d->type = I;
			I_Decode(instr, d, rVals);
			break;
		case 2:
		case 3:
			d->type = J;
			J_Decode(instr, d, rVals);
			break;
		// If none of the given MIPS commands, invalid instruction
		default: die();
	}
}

/*
 *  Print the disassembled version of the given instruction
 *  followed by a newline.
 */
void PrintInstruction(DecodedInstr* d) {										// Me
	int opcode = d->op;
	char* titledReg[32];
	titledReg[0] = "0"; titledReg[1] = "1";
	titledReg[2] = "2"; titledReg[3] = "3";
	titledReg[4] = "4"; titledReg[5] = "5";  titledReg[6] = "6"; titledReg[7] = "7";
	titledReg[8] = "8"; titledReg[9] = "9"; titledReg[10] = "10"; titledReg[11] = "11";  titledReg[12] = "12"; titledReg[13] = "13"; titledReg[14] = "14"; titledReg[15] = "15"; titledReg[24] = "24"; titledReg[25] = "25";
	titledReg[16] = "16"; titledReg[17] = "17";  titledReg[18] = "18"; titledReg[19] = "19"; titledReg[20] = "20"; titledReg[21] = "21"; titledReg[22] = "22"; titledReg[23] = "23";
	titledReg[26] = "26"; titledReg[27] = "27"; titledReg[28] = "28"; titledReg[29] = "29";  titledReg[30] = "30"; titledReg[31] = "31";

	// R-Format
	if (opcode == 0) {
		/*
			All these values should be saved as a numerical value converted from binary.
			R-Format Instructions
				addu
				subu
				sll
				srl
				and
				or
				slt
				jr
		*/
		char* rs = titledReg[d->regs.r.rs];
		char* rt = titledReg[d->regs.r.rt];
		char* rd = titledReg[d->regs.r.rd];
		unsigned int shamt = d->regs.r.shamt;
		unsigned int funct = d->regs.r.funct;

		if (funct == 33) { printf("addu\t$%s, $%s, $%s\n", rd, rs, rt); }
		else if (funct == 35) { printf("subu\t$%s, $%s, $%s\n", rd, rs, rt); }
		else if (funct == 0) { printf("sll\t$%s, $%s, %d\n", rd, rt, shamt); }
		else if (funct == 2) { printf("srl\t$%s, $%s, %d\n", rd, rt, shamt); }
		else if (funct == 36) { printf("and\t$%s, $%s, $%s\n", rd, rs, rt); }
		else if (funct == 37) { printf("or\t$%s, $%s, $%s\n", rd, rs, rt); }
		else if (funct == 42) { printf("slt\t$%s, $%s, $%s\n", rd, rs, rt); }
		else if (funct == 8) { printf("jr\t$%s\n", rs); }
	}
	// I-Format
	else if (opcode == 9 || opcode == 12 || opcode == 4 || opcode == 5 || opcode == 15 || opcode == 35 || opcode == 13 || opcode == 43) {
		/*
			I-Format Instructions
				addiu	rt, rs, imm		9
				andi	rt, rs, imm		c
				beq		rs, rt, offset	4
				bne		rs, rt, offset	5
				lui		rt, imm			f
				lw		rt, imm(rs)		23
				ori		rt, rs, imm		d
				sw		2b
		*/
		char* rs = titledReg[d->regs.i.rs];
		char* rt = titledReg[d->regs.i.rt];
		unsigned int imm = d->regs.i.addr_or_immed;
		if (opcode == 9) { printf("addiu\t$%s, $%s, %d\n", rt, rs, imm); }
		else if (opcode == 12) { printf("andi\t$%s, $%s, %x\n", rt, rs, imm); }
		else if (opcode == 4) { printf("beq\t$%s, $%s, 0x%8.8x\n", rs, rt, mips.pc + imm*4 + 4); }
		else if (opcode == 5) { printf("bne\t$%s, $%s, 0x%8.8x\n", rs, rt, mips.pc + imm*4 + 4); }
		else if (opcode == 15) { printf("lui\t$%s, %x\n", rt, imm); }
		else if (opcode == 35) { printf("lw\t$%s, %d($%s)\n", rt, imm, rs); }
		else if (opcode == 13) { printf("ori\t$%s, $%s, %x\n", rt, rs, imm); }
		else if (opcode == 43) { printf("sw\t$%s, %d($%s)\n", rt, imm, rs); }
	}
	// J-Format
	else if (opcode == 2 || opcode == 3) {
		/*
			J-Format Instructions
				j		imm			2
				jal		imm			3
		*/
		unsigned int target = d->regs.j.target;
		if (opcode == 2) { printf("j\t0x%8.8x\n", target); }
		else { printf("jal\t0x%8.8x\n", target); }
	}
}

/* Perform computation needed to execute d, returning computed value */
int Execute ( DecodedInstr* d, RegVals* rVals) {
	//printf("Reached Execute\n");
    	int result = 0;
	if (d->op == 0) {
		unsigned int funct = d->regs.r.funct;
		unsigned int rs = rVals->R_rs;
		unsigned int rt = rVals->R_rt;
		unsigned int shamt = d->regs.r.shamt;
		// ADDU: RS + RT
		if (funct == 33) result = rs + rt;
		// SUBU: RS - RT
		else if (funct == 35) result = rs - rt;
		// SLL: RT << shamt
		else if (funct == 0) result = rt << shamt;
		// SRL: RT >> shamt
		else if (funct == 2) result = rt >> shamt;
		// AND: RS & RT
		else if (funct == 36) result = rs & rt;
		// OR: RS | RT
		else if (funct == 37) result = rs | rt;
		// SLT: RS < RT
		else if (funct == 42) result = rs < rt;
		// JR: RS
		else if (funct == 8) result = rs;
		// Anything else is invalid
		else die();
	}
	else if (d->type == I) {
		unsigned int rs = rVals->R_rs;
		unsigned int rt = rVals->R_rt;
		unsigned int immed = d->regs.i.addr_or_immed;
		// ADDIU: RS + immed
		if (d->op == 9) result = rs + immed;
		// ANDI: rs & immed
		else if (d->op == 12) result = rs & immed;
		// ORI: rs | immed
		else if (d->op == 13) result = rs | immed;
		// LUI: rs << 16
		else if (d->op == 15) result = immed << 16;
		// BEQ: rs == rt
		else if (d->op == 4) {
			if (rs == rt) result = 1;
		}
		// BNE: rs != rt
		else if (d->op == 5) {
			if (rs != rt) result = 1;
		}
		// LW/SW: rs + immed
		else if (d->op == 35 || d->op == 43) result = rs + immed;
		else die();
	}
	else if (d->type == J) {
		unsigned int target = d->regs.j.target;
		// J: Target
		if (d->op == 2) result = target;
		// JAL: +4, Target
		else if (d->op == 3) result = target;
		else die();
	}
	// Anything else is invalid
	else die();
  return result;
}

/* 
 * Update the program counter based on the current instruction. For
 * instructions other than branches and jumps, for example, the PC
 * increments by 4 (which we have provided).
 */
void UpdatePC(DecodedInstr* d, int val) {                                        // Me
	mips.pc += 4;
	//printf("Reached Update\n");
	/*
	Branch - Add PC
	Jump - Change PC
	*/

	int opcode = d->op;
	//printf("val = %d", val);
	if (d->regs.r.funct == 8 && opcode == 0) {     // jr ra
		mips.pc = val;
	}
	else if (val == 1) {
		if (opcode == 4 || opcode == 5) {                                            // BEQ or BNE
			unsigned int branchAmt = d->regs.i.addr_or_immed;
			mips.pc += (4 * branchAmt);
		}
	}
	else if (opcode == 2 || opcode == 3) {
		mips.pc = val;
	}
}


/*
 * Perform memory load or store. Place the address of any updated memory 
 * in *changedMem, otherwise put -1 in *changedMem. Return any memory value 
 * that is read, otherwise return -1. 
 *
 * Remember that we're mapping MIPS addresses to indices in the mips.memory 
 * array. mips.memory[0] corresponds with address 0x00400000, mips.memory[1] 
 * with address 0x00400004, and so forth.
 *
 */
void memory_error(int val) {
	printf("Memory Access Exception at 0x%8.8x: address 0x%8.8x\n", mips.pc - 4, val);
	die();
}

int Mem( DecodedInstr* d, int val, int *changedMem) {
	//printf("Reached Mem\n");
	// Default: Only SW changes memory
	// Val represents address we are passing in
	// If we do not do LW or SW, we return -1
	*changedMem = -1;
	// Load word 
	if (d->op == 35) {
		// Check if address is valid
		if (val < 0x00401000 || val > 0x00404000) memory_error(val);
		// Check if address is word-aligned
		// Last two bits of address XX00 must be zero if word aligned
		// Use val & 0011 == 0 to check (if it is not equal to 0, then last two bits must not be 0's) 
		if ((val & 0x03) != 0) memory_error(val);
		val = Fetch(val);
	}
	// Store Word
	else if (d->op == 43){
		// Like Fetch, but setting memory instead of grabbing value
		// Grab value from RT and set to memory at IMM(RS) = VAL
		int sw_rt = mips.registers[d->regs.i.rt];
		// Same memory tests as LW:
		if (val < 0x00401000 || val > 0x00404000) memory_error(val);
		if ((val & 0x03) != 0) memory_error(val);
		// For SW, rt contains address for jump. Need to change to index value for memory.
		mips.memory[(val - 0x00400000) / 4] = sw_rt;
		*changedMem = val;
	}
	return val;
}

/* 
 * Write back to register. If the instruction modified a register--
 * (including jal, which modifies $ra) --
 * put the index of the modified register in *changedReg,
 * otherwise put -1 in *changedReg.
 */
void RegWrite( DecodedInstr* d, int val, int *changedReg) {
	//printf("Reached Write\n");
	int opcode = d->op;
	// R-Format
	if (opcode == 0) {
		int rd = d->regs.r.rd;
		unsigned int funct = d->regs.r.funct;

		if (funct == 33 || funct == 35 || funct == 0 || funct == 2 || funct == 36 || funct == 37 || funct == 42) { 
			mips.registers[rd] = val;
			*changedReg = rd; }
		else if (funct == 8) {													// jr ra only
			*changedReg = -1;
		}
	}
	// I-Format
	else if (opcode == 9 || opcode == 12 || opcode == 4 || opcode == 5 || opcode == 15 || opcode == 35 || opcode == 13) {
		int rt = d->regs.i.rt;
		if (opcode == 9 || opcode == 12 || opcode == 15 || opcode == 35 || opcode == 13 ) {
			mips.registers[rt] = val;
			*changedReg = rt;
		}
		else if (opcode == 4 || opcode == 5) {
			*changedReg = -1;
		}
	}
	// J-Format
	else if (opcode == 3) {
		int target = d->regs.j.target;
		mips.registers[31] = target - 4;
		*changedReg = 31;
	}
	else {
		*changedReg = -1;
	}
}
