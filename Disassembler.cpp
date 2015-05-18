#include "Disassembler.hpp"

const char* typesOfOpcode33[] = {
	"MUL", "MULH", "MULHSU", "MULHU",
	"DIV", "DIVU", "REM", "REMU"
};

const char* typesOfOpcode63[] = {
	"BEQ", "BNE", NULL, NULL,
	"BLT", "BGE", "BLTU", "BGEU"
};

void printSignedHexInt32(char* buffer, Int32 value) {
	sprintf(buffer, "%s0x%lx", value<0?"-":"", value<0?-(unsigned)value:value);
}

void printSignedHexInt64(char* buffer, Int64 value) {
	sprintf(buffer, "%s0x%llx", value<0?"-":"", value<0?-(unsigned)value:value);
}

void disassembleOpcode03(Disassembler& self, const Instruction& instruction) {
	Int32 test = -0x03;
	/* I-Type
	LB rd,rs1,imm
	LH rd,rs1,imm
	LW rd,rs1,imm
	LBU rd,rs1,imm
	LHU rd,rs1,imm
	LWU rd,rs1,imm
	LD rd,rs1,imm
	*/
}

void disassembleOpcode07(Disassembler& self, const Instruction& instruction) {
	/* I-Type
	FLW rd,rs1,imm
	FLD rd,rs1,imm
	*/
}

void disassembleOpcode0F(Disassembler& self, const Instruction& instruction) {
	/* I-Type
	FENCE
	FENCE.I
	*/
}

void disassembleOpcode13(Disassembler& self, const Instruction& instruction) {
	/* I-Type
	ADDI rd,rs1,imm
	SLTI rd,rs1,imm
	SLTIU rd,rs1,imm
	XORI rd,rs1,imm
	ORI rd,rs1,imm
	ANDI rd,rs1,imm
	SLLI rd,rs1,shamt
	SRLI rd,rs1,shamt
	SRAI rd,rs1,shamt
	*/
}

void disassembleOpcode17(Disassembler& self, const Instruction& instruction) {
	AddressType dst = self.addJumpMark(self.currentPosition+static_cast<Int32>(instruction.imm));

	char buffer[64];
	sprintf(buffer, "AUIPC x%d, jm_%llx", instruction.reg[0], dst);
	self.textSection.push_back(buffer);
}

void disassembleOpcode1B(Disassembler& self, const Instruction& instruction) {
	/* I-Type
	ADDIW rd,rs1,imm
	SLLIW rd,rs1,shamt
	SRLIW rd,rs1,shamt
	SRAIW rd,rs1,shamt
	*/
}

void disassembleOpcode23(Disassembler& self, const Instruction& instruction) {
	/* S-Type
	SB rs1,rs2,imm
	SH rs1,rs2,imm
	SW rs1,rs2,imm
	SD rs1,rs2,imm
	*/
}

void disassembleOpcode27(Disassembler& self, const Instruction& instruction) {
	/* S-Type
	FSW rs1,rs2,imm
	FSD rs1,rs2,imm
	*/
}

void disassembleOpcode2F(Disassembler& self, const Instruction& instruction) {
	/* U-Type
	LR.W rd,rs1
	SC.W rd,rs1,rs2
	AMOSWAP.W rd,rs1,rs2
	AMOADD.W rd,rs1,rs2
	AMOXOR.W rd,rs1,rs2
	AMOAND.W rd,rs1,rs2
	AMOOR.W rd,rs1,rs2
	AMOMIN.W rd,rs1,rs2
	AMOMAX.W rd,rs1,rs2
	AMOMINU.W rd,rs1,rs2
	AMOMAXU.W rd,rs1,rs2
	LR.D rd,rs1
	SC.D rd,rs1,rs2
	AMOSWAP.D rd,rs1,rs2
	AMOADD.D rd,rs1,rs2
	AMOXOR.D rd,rs1,rs2
	AMOAND.D rd,rs1,rs2
	AMOOR.D rd,rs1,rs2
	AMOMIN.D rd,rs1,rs2
	AMOMAX.D rd,rs1,rs2
	AMOMINU.D rd,rs1,rs2
	AMOMAXU.D rd,rs1,rs2
	*/
}

void disassembleOpcode33(Disassembler& self, const Instruction& instruction) {
	char buffer[64];
	const char* type;
	if(instruction.funct[1] == 1)
		type = typesOfOpcode33[instruction.funct[0]];
	else
		switch(instruction.funct[0]) {
			case 0:
			type = (instruction.funct[1] == 0) ? "ADD" : "SUB";
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
			type = (instruction.funct[1] == 0) ? "SRL" : "SRA";
			break;
			case 6:
			type = "OR";
			break;
			case 7:
			type = "AND";
			break;
		}
	sprintf(buffer, "%s x%d, x%d, x%d", type, instruction.reg[0], instruction.reg[1], instruction.reg[2]);
	self.textSection.push_back(buffer);
}

void disassembleOpcode37(Disassembler& self, const Instruction& instruction) {
	/* U-Type
	LUI rd,imm
	*/
}

void disassembleOpcode3B(Disassembler& self, const Instruction& instruction) {
	/* R-Type
	ADDW rd,rs1,rs2
	SUBW rd,rs1,rs2
	SLLW rd,rs1,rs2
	SRLW rd,rs1,rs2
	SRAW rd,rs1,rs2
	MULW rd,rs1,rs2
	DIVW rd,rs1,rs2
	DIVUW rd,rs1,rs2
	REMW rd,rs1,rs2
	REMUW rd,rs1,rs2
	*/
}

void disassembleOpcode43(Disassembler& self, const Instruction& instruction) {
	/* R4-Type
	FMADD.S rd,rs1,rs2,rs3
	FMADD.D rd,rs1,rs2,rs3
	*/
}

void disassembleOpcode47(Disassembler& self, const Instruction& instruction) {
	/* R4-Type
	FMSUB.S rd,rs1,rs2,rs3
	FMSUB.D rd,rs1,rs2,rs3
	*/
}

void disassembleOpcode4B(Disassembler& self, const Instruction& instruction) {
	/* R4-Type
	FNMSUB.S rd,rs1,rs2,rs3
	FNMSUB.D rd,rs1,rs2,rs3
	*/
}

void disassembleOpcode4F(Disassembler& self, const Instruction& instruction) {
	/* R4-Type
	FNMADD.S rd,rs1,rs2,rs3
	FNMADD.D rd,rs1,rs2,rs3
	*/
}

void disassembleOpcode53(Disassembler& self, const Instruction& instruction) {
	/* R-Type
	FADD.S rd,rs1,rs2
	FSUB.S rd,rs1,rs2
	FMUL.S rd,rs1,rs2
	FDIV.S rd,rs1,rs2
	FSQRT.S rd,rs1
	FSGNJ.S rd,rs1,rs2
	FSGNJN.S rd,rs1,rs2
	FSGNJX.S rd,rs1,rs2
	FMIN.S rd,rs1,rs2
	FMAX.S rd,rs1,rs2
	FCVT.W.S rd,rs1
	FCVT.WU.S rd,rs1
	FMV.X.S rd,rs1
	FEQ.S rd,rs1,rs2
	FLT.S rd,rs1,rs2
	FLE.S rd,rs1,rs2
	FCLASS.S rd,rs1
	FCVT.S.W rd,rs1
	FCVT.S.WU rd,rs1
	FMV.S.X rd,rs1
	FRCSR rd
	FRRM rd
	FRFLAGS rd
	FSCSR rd,rs1
	FSRM rd,rs1
	FSFLAGS rd,rs1
	FSRMI rd,imm
	FSFLAGSI rd,imm
	FCVT.L.S rd,rs1
	FCVT.LU.S rd,rs1
	FCVT.S.L rd,rs1
	FCVT.S.LU rd,rs1
	FADD.D rd,rs1,rs2
	FSUB.D rd,rs1,rs2
	FMUL.D rd,rs1,rs2
	FDIV.D rd,rs1,rs2
	FSQRT.D rd,rs1
	FSGNJ.D rd,rs1,rs
	FSGNJN.D rd,rs1,r
	FSGNJX.D rd,rs1,r
	FMIN.D rd,rs1,rs2
	FMAX.D rd,rs1,rs2
	FCVT.S.D rd,rs1
	FCVT.D.S rd,rs1
	FEQ.D rd,rs1,rs2
	FLT.D rd,rs1,rs2
	FLE.D rd,rs1,rs2
	FCLASS.D rd,rs1
	FCVT.W.D rd,rs1
	FCVT.WU.D rd,rs1
	FCVT.D.W rd,rs1
	FCVT.D.WU rd,rs1
	FCVT.L.D rd,rs1
	FCVT.LU.D rd,rs1
	FMV.X.D rd,rs1
	FCVT.D.L rd,rs1
	FCVT.D.LU rd,rs1
	FMV.D.X rd,rs1
	*/
}

void disassembleOpcode63(Disassembler& self, const Instruction& instruction) {
	AddressType dst = self.addJumpMark(self.currentPosition+static_cast<Int32>(instruction.imm)*2);

	char buffer[64];
	sprintf(buffer, "%s x%d, x%d, jm_%llx", typesOfOpcode63[instruction.funct[0]], instruction.reg[1], instruction.reg[2], dst);
	self.textSection.push_back(buffer);
}

void disassembleOpcode67(Disassembler& self, const Instruction& instruction) {
	char buffer[64];
	sprintf(buffer, "JALR x%d, x%d, %ld", instruction.reg[0], instruction.reg[1], static_cast<Int32>(instruction.imm));
	//printSignedHexInt32(buffer, static_cast<Int32>(instruction.imm));

	self.textSection.push_back(buffer);
}

void disassembleOpcode6F(Disassembler& self, const Instruction& instruction) {
	AddressType dst = self.addJumpMark(self.currentPosition+static_cast<Int32>(instruction.imm)*2);

	char buffer[64];
	if(instruction.reg[0])
		sprintf(buffer, "JAL x%d, jm_%llx", instruction.reg[0], dst);
	else
		sprintf(buffer, "J jm_%llx", dst);
	self.textSection.push_back(buffer);
}

void disassembleOpcode73(Disassembler& self, const Instruction& instruction) {
	/* I-Type
	SCALL
	SBREAK
	RDCYCLE rd
	RDCYCLEH rd
	RDTIME rd
	RDTIMEH rd
	RDINSTRET rd
	RDINSTRETH rd
	CSRRW rd,csr,rs1
	CSRRS rd,csr,rs1
	CSRRC rd,csr,rs1
	CSRRWI rd,csr,imm
	CSRRSI rd,csr,imm
	CSRRCI rd,csr,imm
	ECALL
	EBREAK
	ERET
	MRTS
	MRTH
	HRTS
	WFI
	SFENCE.VM rs1
	*/
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
}
