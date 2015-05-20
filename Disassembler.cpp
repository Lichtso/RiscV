#include "Disassembler.hpp"
#include <elfio/elfio.hpp>

const UInt8 ELF_machine = 243;

const char* typesOfOpcode03[] = {
	"LB", "LH", "LW", "LD",
	"LBU", "LHU", "LWU", NULL
};

const char* typesOfOpcode0F[] = {
	"W", "R", "O", "I"
};

const char* typesOfOpcode23[] = {
	"SB", "SH", "SW", "SD",
	NULL, NULL, NULL, NULL
};

const char* typesOfOpcode2F[] = {
	NULL, NULL, ".W", ".D"
};

const char* typesOfOpcode2F_2[] = {
	".RL", ".AQ"
};

const char* typesOfOpcode27[] = {
	"FSW", "FSD"
};

const char* typesOfOpcode33[] = {
	"MUL", "MULH", "MULHSU", "MULHU",
	"DIV", "DIVU", "REM", "REMU"
};

const char* typesOfOpcode3X_0[] = {
	"ADD", "SUB"
};

const char* typesOfOpcode3X_5[] = {
	"SRL", "SRA"
};

const char* typesOfOpcode4X[] = {
	"FMADD", "FMSUB", "FNMSUB", "FNMADD"
};

const char* typesOfFloat[] = {
	".S", ".D"
};

const char* floatStatusFlags[] = {
	"NX", "UF", "OF", "DZ", "NV"
};

const char* roundingModes[] = {
	".RNE", ".RTZ", ".RDN", ".RUP", ".RMM"
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
	".W", ".WU", ".L", ".LU"
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
	NULL, "FSFLAGS", "FSRM", "FSCSR"
};

const char* intRegisterABINames[] = {
	"zero", "ra", "fp", "s1", "s2", "s3", "s4", "s5",
	"s6", "s7", "s8", "s9", "s10", "s11", "sp", "tp",
	"v0", "v1", "a0", "a1", "a2", "a3", "a4", "a5",
	"a6", "a7", "t0", "t1", "t2", "t3", "t4", "gp"
};



void printSeperator(Disassembler& self) {
	strcat(self.buffer, ",");
}

void printUInt32(Disassembler& self, UInt32 value) {
	if(self.flags&Disassembler::FlagDec)
		sprintf(self.buffer, "%s %d", self.buffer, value);
	else
		sprintf(self.buffer, "%s 0x%x", self.buffer, value);
}

void printInt32(Disassembler& self, Int32 value) {
	if(self.flags&Disassembler::FlagDec)
		sprintf(self.buffer, "%s %d", self.buffer, value);
	else
		sprintf(self.buffer, "%s %s0x%x", self.buffer, value<0?"-":"", value<0?-(unsigned)value:value);
}

void printIntRegister(Disassembler& self, UInt8 index) {
	if(self.flags&Disassembler::FlagABI)
		sprintf(self.buffer, "%s %s", self.buffer, intRegisterABINames[index]);
	else
		sprintf(self.buffer, "%s x%d", self.buffer, index);
}

void printFloatRegister(Disassembler& self, UInt8 index) {
	if(self.flags&Disassembler::FlagABI) {
		if(index < 16)
			sprintf(self.buffer, "%s fs%d", self.buffer, index);
		else if(index < 18)
			sprintf(self.buffer, "%s fv%d", self.buffer, index-16);
		else if(index < 26)
			sprintf(self.buffer, "%s fa%d", self.buffer, index-18);
		else
			sprintf(self.buffer, "%s ft%d", self.buffer, index-26);
	}else
		sprintf(self.buffer, "%s f%d", self.buffer, index);
}

void printRoundingMode(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[0] <= 4)
		strcat(self.buffer, roundingModes[instruction.funct[0]]);
}

void printAtomicMode(Disassembler& self, const Instruction& instruction) {
	for(UInt8 i = 0; i < 2; ++i)
		if((instruction.funct[0]>>i)&1)
			strcat(self.buffer, typesOfOpcode2F_2[0]);
}

void print_x_x(Disassembler& self, const Instruction& instruction, UInt8 index = 0) {
	printIntRegister(self, instruction.reg[index]);
	printSeperator(self);
	printIntRegister(self, instruction.reg[index+1]);
}

void print_x_x_i(Disassembler& self, const Instruction& instruction, UInt8 index = 0) {
	printIntRegister(self, instruction.reg[index]);
	printSeperator(self);
	printIntRegister(self, instruction.reg[index+1]);
	printSeperator(self);
	printUInt32(self, instruction.imm);
}

void print_x_x_x(Disassembler& self, const Instruction& instruction, UInt8 index = 0) {
	printIntRegister(self, instruction.reg[index]);
	printSeperator(self);
	printIntRegister(self, instruction.reg[index+1]);
	printSeperator(self);
	printIntRegister(self, instruction.reg[index+2]);
}



void disassembleOpcode03(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, typesOfOpcode03[instruction.funct[0]]);
	print_x_x_i(self, instruction);
}

void disassembleOpcode07(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, (instruction.funct[0] == 2) ? "FLW" : "FLD");
	print_x_x_i(self, instruction);
}

void disassembleOpcode0F(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[0] == 0) {
		strcpy(self.buffer, "FENCE");
		if(instruction.imm < 0xFF) {
			strcat(self.buffer, " ");
			for(UInt8 i = 0; i < 4; ++i)
				if((instruction.imm>>(i+4))&1)
					strcat(self.buffer, typesOfOpcode0F[i]);
			strcat(self.buffer, ", ");
			for(UInt8 i = 0; i < 4; ++i)
				if((instruction.imm>>i)&1)
					strcat(self.buffer, typesOfOpcode0F[i]);
		}
	}else
		strcpy(self.buffer, "FENCE.I");
}

void disassembleOpcode13(Disassembler& self, const Instruction& instruction) {
	UInt32 imm = instruction.imm;
	switch(instruction.funct[0]) {
		case 0:
		if(imm == 0) {
			if(self.flags&Disassembler::FlagNop && instruction.reg[0] == 0) {
				strcpy(self.buffer, "NOP");
				return;
			}else if(self.flags&Disassembler::FlagMove) {
				strcpy(self.buffer, "MV");
				print_x_x(self, instruction);
				return;
			}
		}
		strcpy(self.buffer, "ADDI");
		break;
		case 1:
		strcpy(self.buffer, "SLLI");
		break;
		case 2:
		strcpy(self.buffer, "SLTI");
		break;
		case 3:
		strcpy(self.buffer, "SLTIU");
		break;
		case 4:
		strcpy(self.buffer, "XORI");
		break;
		case 5:
		if((imm&(1<<10)) == 0)
			strcpy(self.buffer, "SRLI");
		else{
			strcpy(self.buffer, "SRAI");
			imm &= TrailingBitMask(5);
		}
		break;
		case 6:
		strcpy(self.buffer, "ORI");
		break;
		case 7:
		strcpy(self.buffer, "ANDI");
		break;
	}
	print_x_x_i(self, instruction);
}

void disassembleOpcode17(Disassembler& self, const Instruction& instruction, AddressType address) {
	strcpy(self.buffer, "AUIPC");
	printIntRegister(self, instruction.reg[0]);
	printSeperator(self);
	self.addJumpMark(address+static_cast<Int32>(instruction.imm));
}

void disassembleOpcode1B(Disassembler& self, const Instruction& instruction) {
	UInt32 imm = instruction.imm;
	switch(instruction.funct[0]) {
		case 0:
		strcpy(self.buffer, "ADDIW");
		break;
		case 1:
		strcpy(self.buffer, "SLLIW");
		break;
		case 5:
		if((imm&(1<<10)) == 0)
			strcpy(self.buffer, "SRLIW");
		else{
			strcpy(self.buffer, "SRAIW");
			imm &= TrailingBitMask(5);
		}
		break;
		default:
		throw Exception(Exception::Code::IllegalInstruction);
	}
	print_x_x_i(self, instruction);
}

void disassembleOpcode23(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, typesOfOpcode23[instruction.funct[0]]);
	print_x_x_i(self, instruction, 1);
}

void disassembleOpcode27(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, typesOfOpcode27[instruction.funct[0]]);
	print_x_x_i(self, instruction, 1);
}

void disassembleOpcode2F(Disassembler& self, const Instruction& instruction) {
	switch(instruction.funct[1]&(TrailingBitMask(5)<<2)) {
		case 0x00:
		strcpy(self.buffer, "AMOADD");
		break;
		case 0x04:
		strcpy(self.buffer, "AMOSWAP");
		break;
		case 0x08:
		strcpy(self.buffer, "LR");
		strcat(self.buffer, typesOfOpcode2F[instruction.funct[0]]);
		printAtomicMode(self, instruction);
		print_x_x(self, instruction);
		return;
		case 0x0C:
		strcpy(self.buffer, "SC");
		break;
		case 0x10:
		strcpy(self.buffer, "AMOXOR");
		break;
		case 0x20:
		strcpy(self.buffer, "AMOOR");
		break;
		case 0x30:
		strcpy(self.buffer, "AMOAND");
		break;
		case 0x40:
		strcpy(self.buffer, "AMOMIN");
		break;
		case 0x50:
		strcpy(self.buffer, "AMOMAX");
		break;
		case 0x60:
		strcpy(self.buffer, "AMOMINU");
		break;
		case 0x70:
		strcpy(self.buffer, "AMOMAXU");
		break;
	}
	strcat(self.buffer, typesOfOpcode2F[instruction.funct[0]]);
	printAtomicMode(self, instruction);
	print_x_x_x(self, instruction);
}

void disassembleOpcode33(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[1] == 1)
		strcpy(self.buffer, typesOfOpcode33[instruction.funct[0]]);
	else
		switch(instruction.funct[0]) {
			case 0:
			strcpy(self.buffer, typesOfOpcode3X_0[instruction.funct[1]]);
			break;
			case 1:
			strcpy(self.buffer, "SLL");
			break;
			case 2:
			strcpy(self.buffer, "SLT");
			break;
			case 3:
			strcpy(self.buffer, "SLTU");
			break;
			case 4:
			strcpy(self.buffer, "XOR");
			break;
			case 5:
			strcpy(self.buffer, typesOfOpcode3X_5[instruction.funct[1]]);
			break;
			case 6:
			strcpy(self.buffer, "OR");
			break;
			case 7:
			strcpy(self.buffer, "AND");
			break;
		}
	print_x_x_x(self, instruction);
}

void disassembleOpcode37(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, "LUI");
	printIntRegister(self, instruction.reg[0]);
	printSeperator(self);
	printUInt32(self, instruction.imm);
}

void disassembleOpcode3B(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[1] == 1) {
		strcpy(self.buffer, typesOfOpcode33[instruction.funct[0]]);
		strcat(self.buffer, "W");
	}else
		switch(instruction.funct[0]) {
			case 0:
			strcpy(self.buffer, typesOfOpcode3X_0[instruction.funct[1]]);
			break;
			case 1:
			strcpy(self.buffer, "SLL");
			break;
			case 5:
			strcpy(self.buffer, typesOfOpcode3X_5[instruction.funct[1]]);
			break;
			default:
			throw Exception(Exception::Code::IllegalInstruction);
		}
	strcat(self.buffer, "W");
	print_x_x_x(self, instruction);
}

void disassembleOpcode4X(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, typesOfOpcode4X[(instruction.opcode>>2)&TrailingBitMask(2)]);
	strcat(self.buffer, typesOfFloat[instruction.funct[1]]);
	printRoundingMode(self, instruction);
	printFloatRegister(self, instruction.reg[0]);
	printSeperator(self);
	printFloatRegister(self, instruction.reg[1]);
	printSeperator(self);
	printFloatRegister(self, instruction.reg[2]);
	printSeperator(self);
	printFloatRegister(self, instruction.reg[3]);
}

void disassembleOpcode53(Disassembler& self, const Instruction& instruction) {
	const char* type = typesOfFloat[instruction.funct[1]&TrailingBitMask(1)];
	switch(instruction.funct[1]&~TrailingBitMask(1)) {
		case 0x00:
		strcpy(self.buffer, "FADD");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		break;
		case 0x04:
		strcpy(self.buffer, "FSUB");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		break;
		case 0x08:
		strcpy(self.buffer, "FMUL");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		break;
		case 0x0C:
		strcpy(self.buffer, "FDIV");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		break;
		case 0x20:
		strcpy(self.buffer, "FCVT");
		strcat(self.buffer, type);
		strcat(self.buffer, typesOfFloat[instruction.reg[2]]);
		printFloatRegister(self, instruction.reg[0]);
		printSeperator(self);
		printFloatRegister(self, instruction.reg[1]);
		return;
		case 0x2C:
		strcpy(self.buffer, "FSQRT");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		printFloatRegister(self, instruction.reg[0]);
		printSeperator(self);
		printFloatRegister(self, instruction.reg[1]);
		return;
		case 0x10:
		strcpy(self.buffer, typesOfOpcode53_10[instruction.funct[0]]);
		strcat(self.buffer, type);
		break;
		case 0x14:
		strcpy(self.buffer, typesOfOpcode53_14[instruction.funct[0]]);
		strcat(self.buffer, type);
		break;
		case 0x50:
		strcpy(self.buffer, typesOfOpcode53_50[instruction.funct[0]]);
		strcat(self.buffer, type);
		break;
		case 0x60:
		strcpy(self.buffer, "FCVT");
		strcat(self.buffer, typesOfOpcode53_CVT[instruction.reg[2]]);
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		printIntRegister(self, instruction.reg[0]);
		printSeperator(self);
		printFloatRegister(self, instruction.reg[1]);
		return;
		case 0x68:
		strcpy(self.buffer, "FCVT");
		strcat(self.buffer, type);
		strcat(self.buffer, typesOfOpcode53_CVT[instruction.reg[2]]);
		printRoundingMode(self, instruction);
		printFloatRegister(self, instruction.reg[0]);
		printSeperator(self);
		printIntRegister(self, instruction.reg[1]);
		return;
		case 0x70:
		strcat(self.buffer, typesOfOpcode53_70[instruction.funct[0]]);
		strcat(self.buffer, type);
		printIntRegister(self, instruction.reg[0]);
		printSeperator(self);
		printFloatRegister(self, instruction.reg[1]);
		return;
		case 0x78:
		strcpy(self.buffer, "FMV");
		strcat(self.buffer, type);
		strcat(self.buffer, ".X");
		printFloatRegister(self, instruction.reg[0]);
		printSeperator(self);
		printIntRegister(self, instruction.reg[1]);
		return;
	}
	printFloatRegister(self, instruction.reg[0]);
	printSeperator(self);
	printFloatRegister(self, instruction.reg[1]);
	printSeperator(self);
	printFloatRegister(self, instruction.reg[2]);
}

void disassembleOpcode63(Disassembler& self, const Instruction& instruction, AddressType address) {
	strcpy(self.buffer, typesOfOpcode63[instruction.funct[0]]);
	print_x_x(self, instruction, 1);
	printSeperator(self);
	self.addJumpMark(address+static_cast<Int32>(instruction.imm)*2);
}

void disassembleOpcode67(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, "JALR");
	printIntRegister(self, instruction.reg[0]);
	printSeperator(self);
	printIntRegister(self, instruction.reg[1]);
	printSeperator(self);
	printInt32(self, static_cast<Int32>(instruction.imm));
}

void disassembleOpcode6F(Disassembler& self, const Instruction& instruction, AddressType address) {
	if(self.flags&Disassembler::FlagJump && instruction.reg[0] == 0)
		strcpy(self.buffer, "J");
	else{
		strcpy(self.buffer, "JAL");
		printIntRegister(self, instruction.reg[0]);
		printSeperator(self);
	}
	self.addJumpMark(address+static_cast<Int32>(instruction.imm)*2);
}

void disassembleOpcode73(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[0] == 0) {
		switch(instruction.imm) {
			case 0x0000:
			strcpy(self.buffer, "ECALL");
			break;
			case 0x0001:
			strcpy(self.buffer, "EBREAK");
			break;
			case 0x0100:
			strcpy(self.buffer, "ERET");
			break;
			case 0x0101:
			strcpy(self.buffer, "SFENCE.VM");
			printIntRegister(self, instruction.reg[1]);
			return;
			case 0x0102:
			strcpy(self.buffer, "WFI");
			break;
			case 0x0205:
			strcpy(self.buffer, "HRTS");
			break;
			case 0x0305:
			strcpy(self.buffer, "MRTS");
			break;
			case 0x0306:
			strcpy(self.buffer, "MRTH");
			break;
			default:
			throw Exception(Exception::Code::IllegalInstruction);
		}
	}else{
		if(self.flags&Disassembler::FlagCSR) {
			switch(instruction.funct[0]) {
				case 1:
					if(instruction.imm == 0 || instruction.imm > 3)
						break;
					strcpy(self.buffer, typesOfOpcode73_2[instruction.imm]);
					print_x_x(self, instruction);
				return;
				case 2: {
					const char* type = NULL;
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
						strcpy(self.buffer, type);
						printIntRegister(self, instruction.reg[0]);
						return;
					}
				} break;
				break;
				case 5:
					if(instruction.imm == 0 || instruction.imm > 2)
						break;
					strcpy(self.buffer, typesOfOpcode73_2[instruction.imm]);
					strcat(self.buffer, "I");
					print_x_x(self, instruction);
				break;
			}
		}
		strcpy(self.buffer, typesOfOpcode73[instruction.funct[0]]);
		printIntRegister(self, instruction.reg[0]);
		if(instruction.funct[0] <= 4) {
			printSeperator(self);
			printUInt32(self, instruction.imm);
			printSeperator(self);
			printIntRegister(self, instruction.reg[1]);
		}else{
			printSeperator(self);
			printUInt32(self, instruction.imm);
			printSeperator(self);
			printUInt32(self, instruction.reg[1]);
		}
	}

	// TODO: floatStatusFlags[]
}



void Disassembler::addInstruction(const Instruction& instruction, AddressType address) {
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
		disassembleOpcode17(*this, instruction, address);
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
		case 0x47:
		case 0x4B:
		case 0x4F:
		disassembleOpcode4X(*this, instruction);
		break;
		case 0x53:
		disassembleOpcode53(*this, instruction);
		break;
		case 0x63:
		disassembleOpcode63(*this, instruction, address);
		break;
		case 0x67:
		disassembleOpcode67(*this, instruction);
		break;
		case 0x6F:
		disassembleOpcode6F(*this, instruction, address);
		break;
		case 0x73:
		disassembleOpcode73(*this, instruction);
		break;
	}
	if(flags&Disassembler::FlagLowerCase)
		std::transform(buffer, buffer+strlen(buffer), buffer, ::tolower);
	textSection.insert(std::pair<AddressType, std::string>(address, buffer));
}

void Disassembler::addFunction(const UInt8* base, const std::string& name, AddressType address, AddressType size) {
	if(!base) return;
	Instruction instruction;
	jumpMarks.insert(std::pair<AddressType, std::string>(address, name));
	for(AddressType i = 0; i < size; i += sizeof(InstructionType)) {
		try {
			instruction.decode32(*reinterpret_cast<const InstructionType*>(base+i));
		}catch(Exception e) {
			instruction.decode32(0x00000013);
		}
		addInstruction(instruction, address+i);
	}
}

void Disassembler::serializeTextSection(std::ostream& stream) {
	for(auto& i : textSection) {
		auto jm = jumpMarks.find(i.first);
		if(jm != jumpMarks.end())
			stream << jm->second << ":\n";
		stream << "\t" << std::hex << i.first << " : " << i.second << "\n";
	}
}

bool Disassembler::writeToFile(const std::string& path) {
	std::ofstream file(path);
	if(!file.is_open())
		return false;

	file << ".text\n";
	serializeTextSection(file);
	file.close();
	return true;
}

const UInt8* segmentsTranslate(ELFIO::elfio& reader, AddressType address) {
	ELFIO::Elf_Half seg_num = reader.segments.size();
	for(size_t i = 0; i < seg_num; ++i) {
		const ELFIO::segment* pseg = reader.segments[i];
		if(address < pseg->get_virtual_address()) continue;
		AddressType ptr = address-pseg->get_virtual_address();
		if(ptr < pseg->get_memory_size())
			return reinterpret_cast<const UInt8*>(pseg->get_data()+ptr);
	}
	return NULL;
}

bool Disassembler::readFromFile(const std::string& path) {
	ELFIO::elfio reader;
	if(!reader.load(path))
        return false;

	if(reader.get_encoding() != ELFDATA2LSB || reader.get_machine() != ELF_machine)
		return false;

	std::cout << "ELF file class    : ";
	if(reader.get_class() == ELFCLASS32)
	    std::cout << "ELF32" << std::endl;
	else
	    std::cout << "ELF64" << std::endl;

	ELFIO::Elf_Half seg_num = reader.segments.size();
	std::cout << "Number of segments: " << seg_num << std::endl;
	for(size_t i = 0; i < seg_num; ++i) {
		const ELFIO::segment* pseg = reader.segments[i];
		std::cout << "  [" << i << "] 0x" << std::hex
				<< pseg->get_flags()
				<< " 0x"
				<< pseg->get_virtual_address()
				<< " 0x"
				<< pseg->get_file_size()
				<< " 0x"
				<< pseg->get_memory_size()
				<< std::endl;
	}

	std::string name;
	ELFIO::Elf64_Addr value;
	ELFIO::Elf_Xword size;
	unsigned char bind;
	unsigned char type;
	ELFIO::Elf_Half section_index;
	unsigned char other;

	ELFIO::Elf_Half textSecIndex = 0, sec_num = reader.sections.size();
	std::cout << "Number of sections: " << sec_num << std::endl;
	for(size_t i = 0; i < sec_num; ++i) {
		ELFIO::section* psec = reader.sections[i];
	    std::cout << "  [" << i << "] "
	              << psec->get_name()
	              << "\t"
				  << psec->get_type()
				  << "\t"
	              << psec->get_size()
	              << std::endl;

		if(reader.sections[i]->get_name() == ".text")
			textSecIndex = i;

	    if(psec->get_type() == SHT_SYMTAB) {
	        const ELFIO::symbol_section_accessor symbols(reader, psec);
			auto sym_num = symbols.get_symbols_num();
			for(unsigned int j = 0; j < sym_num; ++j) {
	            symbols.get_symbol(j, name, value, size, bind, type, section_index, other);
				if(size == 0 || name.size() == 0 || section_index != textSecIndex) continue;

				addFunction(segmentsTranslate(reader, value), name, value, size);
	        }
	    }
	}

	return true;
}



bool Assembler::writeToFile(const std::string& path) {
	return false;
}

bool Assembler::readFromFile(const std::string& path) {
	std::ifstream file(path);
	if(!file.is_open())
		return false;

	for(std::string line; getline(file, line); ) {
		trim(line);

		auto colon = line.find(':');
		if(colon != std::string::npos) {
			std::string jumpMark = line.substr(0, colon);

		}
	}
	file.close();

	return true;
}
