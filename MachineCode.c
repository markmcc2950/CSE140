/*
	CODE DONE ENTIRELY BY MARK MCCULLOUGH
	ALL FUNCTIONS (binary, add, jump, shift) DONE BY MARK MCCULLOUGH
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_SIZE 32
char* registers[INPUT_SIZE];

/*
	Used for converting numbers to binary
	- int b: Number to be converted to binary
	- int d: Number of digits to print out
		Example: If printing value of rs register 16, b = 16, d = 5
*/
int binary(int b, int d) {
	if (d == 0) { return 1; }
	if (b == 0) {																// Base case: If converting zero to binary, just print out zeroes for however many digits are needed
		for (int i = 0; i < d; i++) {
			printf("0");
		}
		printf(" ");
		return 1;																// Don't bother with rest of function, end it here if it's zero
	}

	int value = b;																// Value passed to binary function
	int binMod;																	// Stores the value of the modulus value to store binary value
	int list[INPUT_SIZE];														// Allocate list of size 32
	int start = 0;																// Finds the first value '1' stored in the list to return proper binary order

	for (int i = 0; i < INPUT_SIZE; i++) {										// Get the 1's and 0's of our final binary value, store them in a list
		binMod = value % 2;
		list[i] = binMod;
		value = value / 2;
		if (value == 0 && i != INPUT_SIZE) {
			list[i + 1] = 0;
			break;
		}
	}
	
	for (int i = d - 1; i >= 0; i--) {											// This prints the binary value we need in the proper order.
		printf("%d", list[i]);
	}
	printf(" ");
}

int add(char* a, int f) {
	char* reg[3];
	int regNum[3] = { 0, 0, 0 };
	char* token = strtok(a, ", ");

	for (int i = 0; i < 3; i++) {
		if (i != 2) {
			token = strtok(NULL, ", ");
		}
		else {
			token = strtok(NULL, " \n");
		}
		reg[i] = token;
		for (int j = 0; j < INPUT_SIZE; j++) {
			if (strstr(reg[i], registers[j])) {
				regNum[i] = j;
				break;
			}
		}		
	}	
	token = strtok(a, ", ");													// Token becomes just the operation of the argument passed to this function

	// Print the output
	printf("\nOperation: %s\nRs: %s (R%d)\nRt: %s (R%d)\nRd: %s (R%d)\nShamt: 0\nFunct: %d\n", token, reg[1], regNum[1], reg[2], regNum[2], reg[0], regNum[0], f);

	printf("Machine Code: "); binary(0, 6); binary(regNum[1], 5); binary(regNum[2], 5); binary(regNum[0], 5); binary(0, 5); binary(f, 6);
	printf("\n");
}

int shift(char* s, int f) {
	char* reg[3];
	int shamt;
	int regNum[2] = { 0, 0 };
	char* token = strtok(s, ", ");

	for (int i = 0; i < 3; i++) {
		if (i != 2) {
			token = strtok(NULL, ", ");
			reg[i] = token;
			for (int j = 0; j < INPUT_SIZE; j++) {
				if (strstr(reg[i], registers[j])) {
					regNum[i] = j;
					break;
				}
			}
		}
		else {
			token = strtok(NULL, " \n");
			reg[i] = token;
		}		
	}
	token = strtok(s, ", ");													// Token becomes just the operation of the argument passed to this function
	shamt = atoi(reg[2]);														// Convert char to int for shamt

	// Print the output
	printf("\nOperation: %s\nRs: %d (R%d)\nRt: %s (R%d)\nRd: %s (R%d)\nShamt: %d\nFunct: %d\n", token, 0, 0, reg[1], regNum[1], reg[0], regNum[0], shamt, f);


	printf("Machine Code: "); binary(0, 6); binary(0, 5); binary(regNum[1], 5); binary(regNum[0], 5); binary(shamt, 5); binary(f, 6);
	printf("\n");
}

int jump(char* jp, int f) {
	char* reg;
	int regNum = 0;
	char* token = strtok(jp, " ");

	token = strtok(NULL, "\n ");
	reg = token;
	for (int j = 0; j < INPUT_SIZE; j++) {
		if (strstr(reg, registers[j])) {
			regNum = j;			
			break;
		}
	}

	token = strtok(jp, " ");													// Token becomes just the operation of the argument passed to this function
	// Print the output
	printf("\nOperation: %s\nRs: %s (R%d)\nRt: %d (R%d)\nRd: %d (R%d)\nShamt: %d\nFunct: %d\n", token, reg, regNum, 0, 0, 0, 0, 0, f);
	printf("Machine Code: "); binary(0, 6); binary(regNum, 5); binary(0, 5); binary(0, 5); binary(0, 5); binary(f, 6);
	printf("\n");
}

int main(void) {
	char input[INPUT_SIZE];
	char inputCpy[INPUT_SIZE];
	char* token;
	//printf("Input your machine code command:\n");
	fgets(input, INPUT_SIZE, stdin);
	if (input != NULL || input != " ") {
		// Allocate the register names for each register number
		registers[0] = "zero"; registers[1] = "at";
		registers[2] = "v0"; registers[3] = "v1";
		registers[4] = "a0"; registers[5] = "a1";  registers[6] = "a2"; registers[7] = "a3";
		registers[8] = "t0"; registers[9] = "t1"; registers[10] = "t2"; registers[11] = "t3";  registers[12] = "t4"; registers[13] = "t5"; registers[14] = "t6"; registers[15] = "t7"; registers[24] = "t8"; registers[25] = "t9";
		registers[16] = "s0"; registers[17] = "s1";  registers[18] = "s2"; registers[19] = "s3"; registers[20] = "s4"; registers[21] = "s5"; registers[22] = "s6"; registers[23] = "s7";
		registers[26] = "k0"; registers[27] = "k1"; registers[28] = "gp"; registers[29] = "sp";  registers[30] = "fp"; registers[31] = "ra";

		// Get the operation inputted by the user
		strcpy(inputCpy, input);
		token = strtok(inputCpy, ", ");
		if (strstr("add", token)) { add(input, 32); }			// add		(add $d, $s, $t)
		else if (strstr("addu", token)) { add(input, 33); }			// addu		(addu $d, $s, $t)
		else if (strstr("and", token)) { add(input, 36); }			// and		(and $d, $s, $t)
		else if (strstr("nor", token)) { add(input, 39); }			// nor		(nor $d, $s, $t)
		else if (strstr("or", token)) { add(input, 37); }			// or		(or $d, $s, $t)
		else if (strstr("slt", token)) { add(input, 42); }			// slt		(slt $d, $s, $t)
		else if (strstr("sltu", token)) { add(input, 43); }			// sltu		(sltu $d, $s, $t)
		else if (strstr("sub", token)) { add(input, 34); }			// sub		(sub $d, $s, $t)
		else if (strstr("subu", token)) { add(input, 35); }			// subu		(subu $d, $s, $t)

		//	Non-arithmetic functions (Jump and two shift functions)
		else if (strstr("jr", token)) { jump(input, 8); }			// jr		(jr $s)	
		else if (strstr("sll", token)) { shift(input, 0); }			// sll		(sll $d, $t, h)			RD = RT shifted left by some amount h
		else if (strstr("srl", token)) { shift(input, 2); }			// srl		(srl, $d, $t, h)		RD = RT shifted right by some amount h

		// Invalid input
		else { printf("FAILURE: Invalid input %s \n", input); }
	}
	// Empty input
	else { printf("Error: Empty input\n"); }

// End program
return 0;
}


/*
	Use strcpy(destination, source)
	char input[size]
	Use gets(input)
	Use char* [variableName] = strtok(0, "character")
		- Takes from position 0 until "character", save to [variableName]
		- Can use multiple times in a row, makes all previously grabbed values null from that char

*/
