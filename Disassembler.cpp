#include "Disassembler.hpp"

const char* typesOfOpcode03[] = {
	"LB", "LH", "LW", "LD",
	"LBU", "LHU", "LWU", NULL
};

const char* typesOfOpcode23[] = {
	"SB", "SH", "SW", "SD",
	NULL, NULL, NULL, NULL
};

const char* typesOfOpcode33[] = {
	"MUL", "MULH", "MULHSU", "MULHU",
	"DIV", "DIVU", "REM", "REMU"
};

const char* typesOfOpcode3B[] = {
	"MULW", "DIVW", NULL, NULL,
	NULL, "DIVUW", "REMW", "REMUW"
};

const char* typesOfOpcode3X_0[] = {
	"ADD", "SUB"
};

const char* typesOfOpcode3X_5[] = {
	"SRL", "SRA"
};

const char* typesOfOpcode53_10[] = {
	"FSGNJ", "FSGNJN", "FSGNJX"
};

const char* typesOfOpcode53_14[] = {
	"FMIN", "FMAX"
};

const char* typesOfOpcode53_50[] = {
	"FLE", "FLT", "FEQ"
};

const char* typesOfOpcode53_CVT[] = {
	"W", "WU", "L", "LU"
};

const char* typesOfOpcode53_70[] = {
	"FMV.X", "FCLASS"
};

const char* typesOfOpcode63[] = {
	"BEQ", "BNE", NULL, NULL,
	"BLT", "BGE", "BLTU", "BGEU"
};

const char* typesOfOpcode73[] = {
	NULL, "CSRRW", "CSRRS", "CSRRC",
	NULL, "CSRRWI", "CSRRSI", "CSRRCI"
};

const char* typesOfOpcode73_2[] = {
	NULL, "FLAGS", "RM", "CSR"
};



void printSignedHexInt32(char* buffer, Int32 value) {
	sprintf(buffer, "%s0x%lx", value<0?"-":"", value<0?-(unsigned)value:value);
}

void printSignedHexInt64(char* buffer, Int64 value) {
	sprintf(buffer, "%s0x%llx", value<0?"-":"", value<0?-(unsigned)value:value);
}



void disassembleOpcode03(Disassembler& self, const Instruction& instruction) {
	sprintf(self.buffer, "%s x%d, x%d, 0x%lx", typesOfOpcode03[instruction.funct[0]], instruction.reg[0], instruction.reg[1], instruction.imm);
}

void disassembleOpcode07(Disassembler& self, const Instruction& instruction) {
	const char* type = (instruction.funct[0] == 2) ? "FLW" : "FLD";
	sprintf(self.buffer, "%s x%d, x%d, 0x%lx", type, instruction.reg[0], instruction.reg[1], instruction.imm);
}

void disassembleOpcode0F(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[0] == 0)
		sprintf(self.buffer, "FENCE 0x%lx", instruction.imm);
	else
		sprintf(self.buffer, "FENCE.I");

	// TODO: What about pred, succ in imm
}

void disassembleOpcode13(Disassembler& self, const Instruction& instruction) {
	const char* type;
	UInt32 imm = instruction.imm;
	switch(instruction.funct[0]) {
		case 0:
		if(imm == 0) {
			if(self.flags&Disassembler::FlagNop && instruction.reg[0] == 0) {
				type = "NOP";
				break;
			}else if(self.flags&Disassembler::FlagMove) {
				sprintf(self.buffer, "MV x%d, x%d", instruction.reg[0], instruction.reg[1]);
				return;
			}
		}
		type = "ADDI";
		break;
		case 1:
		type = "SLLI";
		break;
		case 2:
		type = "SLTI";
		break;
		case 3:
		type = "SLTIU";
		break;
		case 4:
		type = "XORI";
		break;
		case 5:
		if((imm&(1<<10)) == 0)
			type = "SRLI";
		else{
			type = "SRAI";
			imm &= TrailingBitMask(5);
		}
		break;
		case 6:
		type = "ORI";
		break;
		case 7:
		type = "ANDI";
		break;
	}
	sprintf(self.buffer, "%s x%d, x%d, 0x%lx", type, instruction.reg[0], instruction.reg[1], imm);
}

void disassembleOpcode17(Disassembler& self, const Instruction& instruction) {
	sprintf(self.buffer, "AUIPC x%d, ", instruction.reg[0]);
	self.addJumpMark(self.currentPosition+static_cast<Int32>(instruction.imm));
}

void disassembleOpcode1B(Disassembler& self, const Instruction& instruction) {
	const char* type;
	UInt32 imm = instruction.imm;
	switch(instruction.funct[0]) {
		case 0:
		type = "ADDIW";
		break;
		case 1:
		type = "SLLIW";
		break;
		case 5:
		if((imm&(1<<10)) == 0)
			type = "SRLIW";
		else{
			type = "SRAIW";
			imm &= TrailingBitMask(5);
		}
		break;
		default:
		throw Exception(Exception::Code::IllegalInstruction);
	}
	sprintf(self.buffer, "%s x%d, x%d, 0x%lx", type, instruction.reg[0], instruction.reg[1], imm);
}

void disassembleOpcode23(Disassembler& self, const Instruction& instruction) {
	sprintf(self.buffer, "%s x%d, x%d, 0x%lx", typesOfOpcode23[instruction.funct[0]], instruction.reg[1], instruction.reg[2], instruction.imm);
}

void disassembleOpcode27(Disassembler& self, const Instruction& instruction) {
	const char* type = (instruction.funct[0] == 2) ? "W" : "D";
	sprintf(self.buffer, "FS%s x%d, x%d, 0x%lx", type, instruction.reg[1], instruction.reg[2], instruction.imm);
}

void disassembleOpcode2F(Disassembler& self, const Instruction& instruction) {
	const char *typeA, *typeB = (instruction.funct[0] == 2) ? "W" : "D";
	switch(instruction.funct[1]&(TrailingBitMask(5)<<2)) {
		case 0x00:
		typeA = "AMOADD";
		break;
		case 0x04:
		typeA = "AMOSWAP";
		break;
		case 0x08:
		sprintf(self.buffer, "LR.%s x%d, x%d", typeB, instruction.reg[0], instruction.reg[1]);
		return;
		case 0x0C:
		typeA = "SC";
		break;
		case 0x10:
		typeA = "AMOXOR";
		break;
		case 0x20:
		typeA = "AMOOR";
		break;
		case 0x30:
		typeA = "AMOAND";
		break;
		case 0x40:
		typeA = "AMOMIN";
		break;
		case 0x50:
		typeA = "AMOMAX";
		break;
		case 0x60:
		typeA = "AMOMINU";
		break;
		case 0x70:
		typeA = "AMOMAXU";
		break;
	}
	sprintf(self.buffer, "%s.%s x%d, x%d, x%d", typeA, typeB, instruction.reg[0], instruction.reg[1], instruction.reg[2]);

	// TODO: What about aq, rl in funct7
}

void disassembleOpcode33(Disassembler& self, const Instruction& instruction) {
	const char* type;
	if(instruction.funct[1] == 1)
		type = typesOfOpcode33[instruction.funct[0]];
	else
		switch(instruction.funct[0]) {
			case 0:
			type = typesOfOpcode3X_0[instruction.funct[1]];
			break;
			case 1:
			type = "SLL";
			break;
			case 2:
			type = "SLT";
			break;
			case 3:
			type = "SLTU";
			break;
			case 4:
			type = "XOR";
			break;
			case 5:
			type = typesOfOpcode3X_0[instruction.funct[1]];
			break;
			case 6:
			type = "OR";
			break;
			case 7:
			type = "AND";
			break;
		}
	sprintf(self.buffer, "%s x%d, x%d, x%d", type, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
}

void disassembleOpcode37(Disassembler& self, const Instruction& instruction) {
	sprintf(self.buffer, "LUI x%d, 0x%lx, ", instruction.reg[0], instruction.imm);
}

void disassembleOpcode3B(Disassembler& self, const Instruction& instruction) {
	const char* type;
	if(instruction.funct[1] == 1)
		type = typesOfOpcode3B[instruction.funct[0]];
	else
		switch(instruction.funct[0]) {
			case 0:
			type = typesOfOpcode3X_0[instruction.funct[1]];
			break;
			case 1:
			type = "SLL";
			break;
			case 5:
			type = typesOfOpcode3X_0[instruction.funct[1]];
			break;
			default:
			throw Exception(Exception::Code::IllegalInstruction);
		}
	sprintf(self.buffer, "%sW x%d, x%d, x%d", type, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
}

void disassembleOpcode43(Disassembler& self, const Instruction& instruction) {
	const char* type = (instruction.funct[1] == 0) ? "S" : "D";
	sprintf(self.buffer, "FMADD.%s x%d, x%d, x%d, x%d", type, instruction.reg[0], instruction.reg[1], instruction.reg[2], instruction.reg[3]);
}

void disassembleOpcode47(Disassembler& self, const Instruction& instruction) {
	const char* type = (instruction.funct[1] == 0) ? "S" : "D";
	sprintf(self.buffer, "FMSUB.%s x%d, x%d, x%d, x%d", type, instruction.reg[0], instruction.reg[1], instruction.reg[2], instruction.reg[3]);
}

void disassembleOpcode4B(Disassembler& self, const Instruction& instruction) {
	const char* type = (instruction.funct[1] == 0) ? "S" : "D";
	sprintf(self.buffer, "FNMSUB.%s x%d, x%d, x%d, x%d", type, instruction.reg[0], instruction.reg[1], instruction.reg[2], instruction.reg[3]);
}

void disassembleOpcode4F(Disassembler& self, const Instruction& instruction) {
	const char* type = (instruction.funct[1] == 0) ? "S" : "D";
	sprintf(self.buffer, "FNMADD.%s x%d, x%d, x%d, x%d", type, instruction.reg[0], instruction.reg[1], instruction.reg[2], instruction.reg[3]);
}

void disassembleOpcode53(Disassembler& self, const Instruction& instruction) {
	const char *typeA = ((instruction.funct[1]&TrailingBitMask(1)) == 0) ? "S" : "D";
	switch(instruction.funct[1]&~TrailingBitMask(1)) {
		case 0x00:
		sprintf(self.buffer, "FADD.%s x%d, x%d, x%d", typeA, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
		break;
		case 0x04:
		sprintf(self.buffer, "FSUB.%s x%d, x%d, x%d", typeA, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
		break;
		case 0x08:
		sprintf(self.buffer, "FMUL.%s x%d, x%d, x%d", typeA, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
		break;
		case 0x0C:
		sprintf(self.buffer, "FDIV.%s x%d, x%d, x%d", typeA, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
		break;
		case 0x20:
		sprintf(self.buffer, "FCVT.%s x%d, x%d", typeA, instruction.reg[0], instruction.reg[1]);
		break;
		case 0x2C:
		sprintf(self.buffer, "FSQRT.%s x%d, x%d", typeA, instruction.reg[0], instruction.reg[1]);
		break;
		case 0x10:
		sprintf(self.buffer, "%s.%s x%d, x%d, x%d", typesOfOpcode53_10[instruction.funct[0]], typeA, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
		break;
		case 0x14:
		sprintf(self.buffer, "%s.%s x%d, x%d, x%d", typesOfOpcode53_14[instruction.funct[0]], typeA, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
		break;
		case 0x50:
		sprintf(self.buffer, "%s.%s x%d, x%d, x%d", typesOfOpcode53_50[instruction.funct[0]], typeA, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
		break;
		case 0x60:
		sprintf(self.buffer, "FCVT.%s.%s x%d, x%d", typesOfOpcode53_CVT[instruction.reg[2]], typeA, instruction.reg[0], instruction.reg[1]);
		break;
		case 0x68:
		sprintf(self.buffer, "FCVT.%s.%s x%d, x%d", typeA, typesOfOpcode53_CVT[instruction.reg[2]], instruction.reg[0], instruction.reg[1]);
		break;
		case 0x70:
		sprintf(self.buffer, "%s.%s x%d, x%d", typesOfOpcode53_70[instruction.funct[0]], typeA, instruction.reg[0], instruction.reg[1]);
		break;
		case 0x78:
		sprintf(self.buffer, "FMV.%s x%d, x%d", typeA, instruction.reg[0], instruction.reg[1]);
		break;
	}

	// TODO: What about rm in funct3
}

void disassembleOpcode63(Disassembler& self, const Instruction& instruction) {
	sprintf(self.buffer, "%s x%d, x%d, ", typesOfOpcode63[instruction.funct[0]], instruction.reg[1], instruction.reg[2]);
	self.addJumpMark(self.currentPosition+static_cast<Int32>(instruction.imm)*2);
}

void disassembleOpcode67(Disassembler& self, const Instruction& instruction) {
	printSignedHexInt32(self.buffer, static_cast<Int32>(instruction.imm));
	sprintf(self.buffer, "JALR x%d, x%d, %s", instruction.reg[0], instruction.reg[1], self.buffer);
}

void disassembleOpcode6F(Disassembler& self, const Instruction& instruction) {
	if(self.flags&Disassembler::FlagJump && instruction.reg[0] == 0)
		sprintf(self.buffer, "J ");
	else
		sprintf(self.buffer, "JAL x%d, ", instruction.reg[0]);
	self.addJumpMark(self.currentPosition+static_cast<Int32>(instruction.imm)*2);
}

void disassembleOpcode73(Disassembler& self, const Instruction& instruction) {
	const char* type;
	if(instruction.funct[0] == 0) {
		switch(instruction.imm) {
			case 0x0000:
			type = "ECALL";
			break;
			case 0x0001:
			type = "EBREAK";
			break;
			case 0x0100:
			type = "ERET";
			break;
			case 0x0101:
			sprintf(self.buffer, "SFENCE.VM x%d", instruction.reg[1]);
			return;
			case 0x0102:
			type = "WFI";
			break;
			case 0x0205:
			type = "HRTS";
			break;
			case 0x0305:
			type = "MRTS";
			break;
			case 0x0306:
			type = "MRTH";
			break;
			default:
			throw Exception(Exception::Code::IllegalInstruction);
		}
		strcpy(self.buffer, type);
	}else{
		if(self.flags&Disassembler::FlagCSR) {
			switch(instruction.funct[0]) {
				case 1:
					if(instruction.imm == 0 || instruction.imm > 3)
						break;
					sprintf(self.buffer, "FS%s x%d, x%d", typesOfOpcode73_2[instruction.imm], instruction.reg[0], instruction.reg[1]);
				return;
				case 2: {
					type = NULL;
					switch(instruction.imm) {
						case 0x0001:
						type = "FRFLAGS";
						break;
						case 0x0002:
						type = "FRRM";
						break;
						case 0x0003:
						type = "FRCSR";
						break;
						case 0x0C00:
						type = "RDCYCLE";
						break;
						case 0x0C80:
						type = "RDCYCLEH";
						break;
						case 0x0C01:
						type = "RDTIME";
						break;
						case 0x0C81:
						type = "RDTIMEH";
						break;
						case 0x0C02:
						type = "RDINSTRET";
						break;
						case 0x0C82:
						type = "RDINSTRETH";
						break;
					}
					if(type) {
						sprintf(self.buffer, "%s x%d", type, instruction.reg[0]);
						return;
					}
				} break;
				break;
				case 5:
					if(instruction.imm == 0 || instruction.imm > 2)
						break;
					sprintf(self.buffer, "FS%sI x%d, x%d", typesOfOpcode73_2[instruction.imm], instruction.reg[0], instruction.reg[1]);
				break;
			}
		}
		type = typesOfOpcode73[instruction.funct[0]];
		if((instruction.funct[0]&TrailingBitMask(3)) == 0)
			sprintf(self.buffer, "%s x%d, 0x%lx, x%d", type, instruction.reg[0], instruction.imm, instruction.reg[1]);
		else
			sprintf(self.buffer, "%s x%d, 0x%lx, 0x%hhx", type, instruction.reg[0], instruction.imm, instruction.reg[1]);
	}

	// TODO: What about SCALL, SBREAK
}



void Disassembler::disassembleInstruction(const Instruction& instruction) {
	switch(instruction.opcode) {
		case 0x03:
		disassembleOpcode03(*this, instruction);
		break;
		case 0x07:
		disassembleOpcode07(*this, instruction);
		break;
		case 0x0F:
		disassembleOpcode0F(*this, instruction);
		break;
		case 0x13:
		disassembleOpcode13(*this, instruction);
		break;
		case 0x17:
		disassembleOpcode17(*this, instruction);
		break;
		case 0x1B:
		disassembleOpcode1B(*this, instruction);
		break;
		case 0x23:
		disassembleOpcode23(*this, instruction);
		break;
		case 0x27:
		disassembleOpcode27(*this, instruction);
		break;
		case 0x2F:
		disassembleOpcode2F(*this, instruction);
		break;
		case 0x33:
		disassembleOpcode33(*this, instruction);
		break;
		case 0x37:
		disassembleOpcode37(*this, instruction);
		break;
		case 0x3B:
		disassembleOpcode3B(*this, instruction);
		break;
		case 0x43:
		disassembleOpcode43(*this, instruction);
		break;
		case 0x47:
		disassembleOpcode47(*this, instruction);
		break;
		case 0x4B:
		disassembleOpcode4B(*this, instruction);
		break;
		case 0x4F:
		disassembleOpcode4F(*this, instruction);
		break;
		case 0x53:
		disassembleOpcode53(*this, instruction);
		break;
		case 0x63:
		disassembleOpcode63(*this, instruction);
		break;
		case 0x67:
		disassembleOpcode67(*this, instruction);
		break;
		case 0x6F:
		disassembleOpcode6F(*this, instruction);
		break;
		case 0x73:
		disassembleOpcode73(*this, instruction);
		break;
	}
	textSection.push_back(buffer);
}
