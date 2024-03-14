#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

const char *byte_registers[] = {
    "al", "cl", "dl", "bl",
    "ah", "ch", "dh", "bh"
};

const char *word_registers[] = {
    "ax", "cx", "dx", "bx",
    "sp", "bp", "si", "di"
};

struct instructionBytes {
    uint8_t op, d, w;
    uint8_t mod, reg, rm;

    const char *src;
    const char *dst;
    //...
};

typedef struct instructionBytes *InstructionBytes;

void print_decoded_instruction(InstructionBytes ib) {
    // for now, everything is mov
    printf("mov %s, %s\n", ib->dst, ib->src);
}

void decode_first_byte(uint8_t first_byte, InstructionBytes ib) {
    ib->op = (first_byte & 0xFF) >> 2;
    ib->d  = (first_byte & 0x02) >> 1;
    ib->w  = (first_byte & 0x01) >> 0;
}

void decode_second_byte(uint8_t second_byte, InstructionBytes ib) {
    ib->mod = (second_byte & 0xC0) >> 6;
    ib->reg = (second_byte & 0x38) >> 3;
    ib->rm  = (second_byte & 0x07) >> 0;

    // for now, only mod = 0x3 is supported
    ib->src = ib->d ?
	    ib->w ? word_registers[ib->rm]
		  : byte_registers[ib->rm]
	    : ib->w ? word_registers[ib->reg]
		    : byte_registers[ib->reg];

    ib->dst = ib->d ?
	    ib->w ? word_registers[ib->reg]
		  : byte_registers[ib->reg]
	    : ib->w ? word_registers[ib->rm]
		    : byte_registers[ib->rm];
}

void decode_instruction_byte(uint8_t inst_byte, InstructionBytes ib,
			     int read_count) {
    if (read_count % 2 != 0) {
	//printf("%x ", inst_byte);
	decode_first_byte(inst_byte, ib);
    } else {
	//printf("%x\n", inst_byte);
	decode_second_byte(inst_byte, ib);
    }
}

void decode(const char *filename)
{
    uint8_t inst_byte;
    size_t size, count;
    int read_count;
    InstructionBytes ib;
    FILE *instructions;

    inst_byte = 0x00;
    count = size = sizeof(inst_byte);
    read_count = 0;
    ib = malloc(sizeof(*ib));
    instructions = fopen(filename, "r");

    printf("bits 16\n\n");

    while (fread(&inst_byte, size, count, instructions) == size) {
	++read_count;
	decode_instruction_byte(inst_byte, ib, read_count);

	if (read_count % 2 == 0)
	    print_decoded_instruction(ib);
    }

    fclose(instructions);
    free(ib);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
	return 1;
    }

    decode(argv[1]);

    return 0;
}
