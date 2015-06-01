/*
	WARNING:

	The assembler / disassembler was made for testing purpose only.
	They are highly instable and might produce wrong outputs.
	Do not use them in production or any other projects!
*/

#include "AssemblerMnemonics.hpp"

const UInt8 ELF_machine = 243;



const char* getDisassemblerEntry(const std::map<UInt8, std::string>& map, UInt8 key) {
	auto iter = map.find(key);
	if(iter == map.end())
		throw Exception(Exception::Code::IllegalInstruction);
	return iter->second.c_str();
}

UInt8 getAssemblerEntry(const std::map<std::string, UInt8>& map, const std::string& key) {
	auto iter = map.find(key);
	if(iter == map.end())
		throw Exception(Exception::Code::IllegalInstruction);
	return iter->second;
}

void printSeperator(Disassembler& self) {
	strcat(self.buffer, ",");
}

void printUInt32(Disassembler& self, UInt32 value) {
	if(self.flags&Disassembler::FlagDecimal)
		sprintf(self.buffer, "%s %d", self.buffer, value);
	else
		sprintf(self.buffer, "%s 0x%x", self.buffer, value);
}

void printInt32(Disassembler& self, Int32 value) {
	if(self.flags&Disassembler::FlagDecimal)
		sprintf(self.buffer, "%s %d", self.buffer, value);
	else
		sprintf(self.buffer, "%s %s0x%x", self.buffer, value<0?"-":"", value<0?-(unsigned)value:value);
}

void printIntRegister(Disassembler& self, UInt8 index) {
	if(self.flags&Disassembler::FlagRegisterABI)
		sprintf(self.buffer, "%s %s", self.buffer, getDisassemblerEntry(disassembler_IntRegABINames, index));
	else
		sprintf(self.buffer, "%s x%d", self.buffer, index);
}

void printFloatRegister(Disassembler& self, UInt8 index) {
	if(self.flags&Disassembler::FlagRegisterABI) {
		sprintf(self.buffer, "%s %s", self.buffer, getDisassemblerEntry(disassembler_FloatRegABINames, index));
	}else
		sprintf(self.buffer, "%s f%d", self.buffer, index);
}

void printRoundingMode(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[1] <= 4) {
		strcat(self.buffer, ".");
		strcat(self.buffer, getDisassemblerEntry(disassembler_FloatRoundingModes, instruction.funct[0]));
	}
}

void printAtomicMode(Disassembler& self, const Instruction& instruction) {
	for(UInt8 i = 0; i < 2; ++i)
		if((instruction.funct[1]>>i)&1) {
			strcat(self.buffer, ".");
			strcat(self.buffer, getDisassemblerEntry(disassembler_2F_2, i));
		}
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
	strcpy(self.buffer, getDisassemblerEntry(disassembler_03, instruction.funct[0]));
	print_x_x_i(self, instruction);
}

void disassembleOpcode07(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, getDisassemblerEntry(disassembler_07, instruction.funct[0]));
	print_x_x_i(self, instruction);
}

void disassembleOpcode0F(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[0] > 1)
		throw Exception(Exception::Code::IllegalInstruction);
	strcpy(self.buffer, "FENCE");
	if(instruction.funct[0] == 1)
		strcat(self.buffer, ".I");
	else if(instruction.imm < 0xFF) {
		strcat(self.buffer, " ");
		for(UInt8 i = 0; i < 4; ++i)
			if((instruction.imm>>(i+4))&1)
				strcat(self.buffer, getDisassemblerEntry(disassembler_0F, i));
		strcat(self.buffer, ", ");
		for(UInt8 i = 0; i < 4; ++i)
			if((instruction.imm>>i)&1)
				strcat(self.buffer, getDisassemblerEntry(disassembler_0F, i));
	}
}

void disassembleOpcode13(Disassembler& self, const Instruction& instruction) {
	UInt32 imm = instruction.imm;
	switch(instruction.funct[0]) {
		case 0:
		if(self.flags&Disassembler::FlagArithmeticPseudo && instruction.imm == 0) {
			if(instruction.reg[0] == 0) {
				strcpy(self.buffer, "NOP");
				return;
			}else{
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
		if(self.flags&Disassembler::FlagLogicPseudo && instruction.imm == 1) {
			strcpy(self.buffer, "SEQZ");
			print_x_x(self, instruction);
			return;
		}
		strcpy(self.buffer, "SLTIU");
		break;
		case 4:
		if(self.flags&Disassembler::FlagLogicPseudo && instruction.imm == -1) {
			strcpy(self.buffer, "NOT");
			print_x_x(self, instruction);
			return;
		}
		strcpy(self.buffer, "XORI");
		break;
		case 5:
		if((imm&(1ULL<<10)) == 0)
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
		default:
		throw Exception(Exception::Code::IllegalInstruction);
	}
	print_x_x_i(self, instruction);
}

void disassembleOpcode17(Disassembler& self, const Instruction& instruction, AddressType address) {
	strcpy(self.buffer, "AUIPC");
	printIntRegister(self, instruction.reg[0]);
	printSeperator(self);
	printInt32(self, static_cast<Int32>(instruction.imm));
}

void disassembleOpcode1B(Disassembler& self, const Instruction& instruction) {
	UInt32 imm = instruction.imm;
	switch(instruction.funct[0]) {
		case 0:
		if(self.flags&Disassembler::FlagArithmeticPseudo && imm == 0) {
			strcpy(self.buffer, "SEXT.W");
			print_x_x(self, instruction);
			return;
		}
		strcpy(self.buffer, "ADDIW");
		break;
		case 1:
		strcpy(self.buffer, "SLLIW");
		break;
		case 5:
		if((imm&(1ULL<<10)) == 0)
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
	strcpy(self.buffer, getDisassemblerEntry(disassembler_23, instruction.funct[0]));
	print_x_x_i(self, instruction, 1);
}

void disassembleOpcode27(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, getDisassemblerEntry(disassembler_27, instruction.funct[0]));
	print_x_x_i(self, instruction, 1);
}

void disassembleOpcode2F(Disassembler& self, const Instruction& instruction) {
	switch(instruction.funct[0]&(TrailingBitMask(5)<<2)) {
		case 0x00:
		strcpy(self.buffer, "AMOADD");
		break;
		case 0x04:
		strcpy(self.buffer, "AMOSWAP");
		break;
		case 0x08:
		strcpy(self.buffer, "LR");
		strcat(self.buffer, ".");
		strcat(self.buffer, getDisassemblerEntry(disassembler_2F, instruction.funct[1]));
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
		default:
		throw Exception(Exception::Code::IllegalInstruction);
	}
	strcat(self.buffer, ".");
	strcat(self.buffer, getDisassemblerEntry(disassembler_2F, instruction.funct[1]));
	printAtomicMode(self, instruction);
	print_x_x_x(self, instruction);
}

void disassembleOpcode33(Disassembler& self, const Instruction& instruction) {
	if(instruction.funct[0] == 1)
		strcpy(self.buffer, getDisassemblerEntry(disassembler_33, instruction.funct[1]));
	else
		switch(instruction.funct[1]) {
			case 0:
			strcpy(self.buffer, getDisassemblerEntry(disassembler_3X_0, instruction.funct[0]));
			break;
			case 1:
			strcpy(self.buffer, "SLL");
			break;
			case 2:
			strcpy(self.buffer, "SLT");
			break;
			case 3:
			if(self.flags&Disassembler::FlagLogicPseudo && instruction.reg[1] == 0) {
				strcpy(self.buffer, "SNEZ");
				printIntRegister(self, instruction.reg[0]);
				printSeperator(self);
				printIntRegister(self, instruction.reg[2]);
				return;
			}
			strcpy(self.buffer, "SLTU");
			break;
			case 4:
			strcpy(self.buffer, "XOR");
			break;
			case 5:
			strcpy(self.buffer, getDisassemblerEntry(disassembler_3X_5, instruction.funct[0]));
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
	if(instruction.funct[0] == 1) {
		strcpy(self.buffer, getDisassemblerEntry(disassembler_33, instruction.funct[1]));
	}else
		switch(instruction.funct[1]) {
			case 0:
			strcpy(self.buffer, getDisassemblerEntry(disassembler_3X_0, instruction.funct[0]));
			break;
			case 1:
			strcpy(self.buffer, "SLL");
			break;
			case 5:
			strcpy(self.buffer, getDisassemblerEntry(disassembler_3X_5, instruction.funct[0]));
			break;
			default:
			throw Exception(Exception::Code::IllegalInstruction);
		}
	strcat(self.buffer, "W");
	print_x_x_x(self, instruction);
}

void disassembleOpcode4X(Disassembler& self, const Instruction& instruction) {
	strcpy(self.buffer, getDisassemblerEntry(disassembler_4X, (instruction.opcode>>2)&TrailingBitMask(2)));
	strcat(self.buffer, ".");
	strcat(self.buffer, getDisassemblerEntry(disassembler_Float, instruction.funct[0]));
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
	const char* type = getDisassemblerEntry(disassembler_Float, instruction.funct[0]&TrailingBitMask(1));
	switch(instruction.funct[0]&~TrailingBitMask(1)) {
		case 0x00:
		strcpy(self.buffer, "FADD.");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		break;
		case 0x04:
		strcpy(self.buffer, "FSUB.");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		break;
		case 0x08:
		strcpy(self.buffer, "FMUL.");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		break;
		case 0x0C:
		strcpy(self.buffer, "FDIV.");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		break;
		case 0x20:
		strcpy(self.buffer, "FCVT.");
		strcat(self.buffer, type);
		strcat(self.buffer, getDisassemblerEntry(disassembler_Float, instruction.reg[2]));
		printFloatRegister(self, instruction.reg[0]);
		printSeperator(self);
		printFloatRegister(self, instruction.reg[1]);
		return;
		case 0x2C:
		strcpy(self.buffer, "FSQRT.");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		printFloatRegister(self, instruction.reg[0]);
		printSeperator(self);
		printFloatRegister(self, instruction.reg[1]);
		return;
		case 0x10:
		if(self.flags&Disassembler::FlagFloatPseudo && instruction.reg[1] == instruction.reg[2]) {
			strcpy(self.buffer, getDisassemblerEntry(disassembler_53_10_2, instruction.funct[1]));
			strcat(self.buffer, ".");
			strcat(self.buffer, type);
			printFloatRegister(self, instruction.reg[0]);
			printSeperator(self);
			printFloatRegister(self, instruction.reg[1]);
			return;
		}
		strcpy(self.buffer, getDisassemblerEntry(disassembler_53_10, instruction.funct[1]));
		strcat(self.buffer, ".");
		strcat(self.buffer, type);
		break;
		case 0x14:
		strcpy(self.buffer, getDisassemblerEntry(disassembler_53_14, instruction.funct[1]));
		strcat(self.buffer, ".");
		strcat(self.buffer, type);
		break;
		case 0x50:
		strcpy(self.buffer, getDisassemblerEntry(disassembler_53_50, instruction.funct[1]));
		strcat(self.buffer, ".");
		strcat(self.buffer, type);
		break;
		case 0x60:
		strcpy(self.buffer, "FCVT.");
		strcat(self.buffer, getDisassemblerEntry(disassembler_53_CVT, instruction.reg[2]));
		strcat(self.buffer, ".");
		strcat(self.buffer, type);
		printRoundingMode(self, instruction);
		printIntRegister(self, instruction.reg[0]);
		printSeperator(self);
		printFloatRegister(self, instruction.reg[1]);
		return;
		case 0x68:
		strcpy(self.buffer, "FCVT.");
		strcat(self.buffer, type);
		strcat(self.buffer, ".");
		strcat(self.buffer, getDisassemblerEntry(disassembler_53_CVT, instruction.reg[2]));
		printRoundingMode(self, instruction);
		printFloatRegister(self, instruction.reg[0]);
		printSeperator(self);
		printIntRegister(self, instruction.reg[1]);
		return;
		case 0x70:
		strcat(self.buffer, getDisassemblerEntry(disassembler_53_70, instruction.funct[1]));
		if(instruction.funct[0] == 0)
			strcat(self.buffer, ".X");
		strcat(self.buffer, ".");
		strcat(self.buffer, type);
		printIntRegister(self, instruction.reg[0]);
		printSeperator(self);
		printFloatRegister(self, instruction.reg[1]);
		return;
		case 0x78:
		strcpy(self.buffer, "FMV.");
		strcat(self.buffer, type);
		strcat(self.buffer, ".X");
		printFloatRegister(self, instruction.reg[0]);
		printSeperator(self);
		printIntRegister(self, instruction.reg[1]);
		return;
		default:
		throw Exception(Exception::Code::IllegalInstruction);
	}
	printFloatRegister(self, instruction.reg[0]);
	printSeperator(self);
	printFloatRegister(self, instruction.reg[1]);
	printSeperator(self);
	printFloatRegister(self, instruction.reg[2]);
}

void disassembleOpcode63(Disassembler& self, const Instruction& instruction, AddressType address) {
	strcpy(self.buffer, getDisassemblerEntry(disassembler_63, instruction.funct[0]));
	print_x_x(self, instruction, 1);
	printSeperator(self);
	self.addJumpMark(address+static_cast<Int32>(instruction.imm));
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
	if(self.flags&Disassembler::FlagJumpPseudo && instruction.reg[0] == 0)
		strcpy(self.buffer, "J");
	else{
		strcpy(self.buffer, "JAL");
		printIntRegister(self, instruction.reg[0]);
		printSeperator(self);
	}
	self.addJumpMark(address+static_cast<Int32>(instruction.imm));
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
		if(self.flags&Disassembler::FlagCSRPseudo) {
			switch(instruction.funct[0]) {
				case 1:
					if(instruction.imm == 0 || instruction.imm > 3)
						break;
					strcpy(self.buffer, getDisassemblerEntry(disassembler_73_2, instruction.imm));
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
					strcpy(self.buffer, getDisassemblerEntry(disassembler_73_2, instruction.imm));
					strcat(self.buffer, "I");
					print_x_x(self, instruction);
				break;
			}
		}
		strcpy(self.buffer, getDisassemblerEntry(disassembler_73, instruction.funct[0]));
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
}



void Disassembler::addToTextSection(AddressType address) {
	if(flags&Disassembler::FlagLowerCase)
		std::transform(buffer, buffer+strlen(buffer), buffer, ::tolower);
	textSection.insert(std::pair<AddressType, std::string>(address, buffer));
}

void Disassembler::addInstruction(AddressType address, const Instruction& instruction) {
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
	addToTextSection(address);
}

void Disassembler::addFunction(const UInt8* base, const std::string& name, AddressType address, AddressType size) {
	if(!base) return;

	Instruction instruction;
	for(AddressType i = 0; i < size; i += sizeof(UInt32)) {
		UInt32 data = *reinterpret_cast<const UInt32*>(base+i);
		try {
			instruction.decode32(data);
			addInstruction(address+i, instruction);
		}catch(Exception e) {
			sprintf(buffer, ".word 0x%08x", data);
			addToTextSection(address+i);
		}
	}
}

bool Disassembler::writeToFile(const std::string& path) {
	std::ofstream file(path);
	if(!file.is_open() || textSection.size() == 0)
		return false;

	file << ".text\n";
	if(textSection.begin()->first > 0) {
		sprintf(buffer, ".skip 0x%llx", textSection.begin()->first);
		file << buffer << std::endl;
	}

	for(auto& i : textSection) {
		auto jm = jumpMarks.find(i.first);
		if(jm != jumpMarks.end())
			file << jm->second << ":\n";
		if(flags&Disassembler::FlagAddresses)
			file << std::setw(16) << std::setfill('0') << std::hex << i.first;
		file << "\t" << i.second << "\n";
	}

	file << extension.str();
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

	/*std::cout << "ELF file class    : ";
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
	}*/

	std::string name;
	ELFIO::Elf64_Addr address;
	ELFIO::Elf_Xword size;
	UInt8 bind, type, other;
	ELFIO::Elf_Half section_index, textSecIndex = 0, sec_num = reader.sections.size();

	//std::cout << "Number of sections: " << sec_num << std::endl;
	for(unsigned int i = 0; i < sec_num; ++i) {
		ELFIO::section* psec = reader.sections[i];
	    /*std::cout << "  [" << i << "] "
	              << psec->get_name()
	              << "\t"
				  << psec->get_type()
				  << "\t"
	              << psec->get_size()
	              << std::endl;*/

		if(reader.sections[i]->get_name() == ".text")
			textSecIndex = i;
		else if(reader.sections[i]->get_name() == ".data") {
			if(flags&Disassembler::FlagDataSection) {
				extension << "\n.data\n";
				for(unsigned int j = 0; j < psec->get_size(); j += 8) {
					auto data = reinterpret_cast<const UInt64*>(psec->get_data()+j);
					if(j%64 == 0) {
						if(j > 0)
							extension << "\n";
						extension << ".dword ";
					}else
						extension << ", ";
					extension << "0x" << std::setw(16) << std::setfill('0') << std::hex << *data;
				}
			}
		}

	    if(psec->get_type() == SHT_SYMTAB) {
	        const ELFIO::symbol_section_accessor symbolAccessor(reader, psec);
			auto sym_num = symbolAccessor.get_symbols_num();

			for(unsigned int j = 0; j < sym_num; ++j) {
				symbolAccessor.get_symbol(j, name, address, size, bind, type, section_index, other);
				if(size == 0 || name.size() == 0 || section_index != textSecIndex) continue;

				std::pair<AddressType, std::string> pair(address, name);
				jumpMarks.insert(pair);
				auto result = symbols.insert(pair);
				//if(!result.second)
				//	printf("Address already bound %llx %s : %s\n", address, name.c_str(), result.first->second.c_str());
			}

			for(unsigned int j = 0; j < sym_num; ++j) {
				symbolAccessor.get_symbol(j, name, address, size, bind, type, section_index, other);
				if(size == 0 || name.size() == 0 || section_index != textSecIndex) continue;
				addFunction(segmentsTranslate(reader, address), name, address, size);
	        }
	    }
	}

	return true;
}



void Assembler::writeInSection(UInt8 index, UInt8 length, const void* data) {
	AddressType ptr = addresses[index]/4096*4096;
	auto iter = sections[index].find(ptr);
	if(iter == sections[index].end()) {
		std::unique_ptr<UInt8> page(new UInt8[4096]);
		memset(page.get(), 0, 4096);
		iter = sections[index].insert(std::pair<AddressType, std::unique_ptr<UInt8>>(ptr, std::move(page))).first;
	}
	memcpy(iter->second.get()+(addresses[index]-ptr), data, length);
	addresses[index] += length;
}

void Assembler::addInstruction(std::string command) {
	auto seperator = command.find(' ');
	std::string token;
	std::istringstream ss;
	std::vector<std::string> commandParts, arguments;

	if(seperator != std::string::npos) {
		token = command.substr(seperator+1);
		command = command.substr(0, seperator);
		if(token.size()) {
			ss.str(token);
			while(std::getline(ss, token, ','))
				arguments.push_back(std::trim(token));
		}
	}

	ss.clear();
	ss.str(command);
	while(std::getline(ss, token, '.'))
		commandParts.push_back(std::trim(token));

	Instruction instruction;
	instruction.opcode = getAssemblerEntry(assembler_instructions, commandParts[0]);

	// TODO
	//auto data = instruction.encode32();
	//writeInSection(assembler_dir_text, sizeof(data), &data);
}

bool Assembler::writeToFile(const std::string& path) {
	// TODO
	return false;
}

bool Assembler::readFromFile(const std::string& path) {
	std::ifstream file(path);
	if(!file.is_open())
		return false;

	assembler_dir_type currentSection;
	for(std::string line; getline(file, line); ) {
		line = std::trim(line);
		if(line.size() == 0) continue;

		auto seperator = line.rfind('#');
		if(seperator != std::string::npos) {
			line = line.substr(0, seperator);
			if(line.size() == 0) continue;
		}

		std::transform(line.begin(), line.end(), line.begin(), ::toupper);

		if(line[0] == '.') {
			std::vector<std::string> arguments;
			auto seperator = line.find(' ');
			if(seperator != std::string::npos) {
				std::string token, argumentStr = line.substr(seperator+1);
				line = line.substr(0, seperator);
				std::istringstream ss(argumentStr);
				while(std::getline(ss, token, ','))
					arguments.push_back(std::trim(token));
			}

			auto iter = assembler_cmds.find(line);
			if(iter == assembler_cmds.end()) {
				printf("Unknown assembler directive %s\n", line.c_str());
				continue;
			}
			switch(iter->second) {
				case assembler_dir_text:
				case assembler_dir_data:
					currentSection = iter->second;
				break;
				case assembler_dir_align: {
					addresses[currentSection] = (addresses[currentSection]/4+1)*4;
				} break;
				case assembler_dir_skip: {
					UInt64 data;
					sscanf(arguments[0].c_str(), "0X%llx", &data);
					addresses[currentSection] += data;
				} break;
				case assembler_dir_byte:
				for(auto& argument : arguments) {
					UInt8 data;
					sscanf(argument.c_str(), "0X%hhx", &data);
					writeInSection(currentSection, sizeof(data), &data);
				}
				break;
				case assembler_dir_half:
				for(auto& argument : arguments) {
					UInt16 data;
					sscanf(argument.c_str(), "0X%hx", &data);
					writeInSection(currentSection, sizeof(data), &data);
				}
				break;
				case assembler_dir_word:
				for(auto& argument : arguments) {
					UInt32 data;
					sscanf(argument.c_str(), "0X%x", &data);
					writeInSection(currentSection, sizeof(data), &data);
				}
				break;
				case assembler_dir_dword:
				for(auto& argument : arguments) {
					UInt64 data;
					sscanf(argument.c_str(), "0X%llx", &data);
					writeInSection(currentSection, sizeof(data), &data);
				}
				break;
			}
			continue;
		}

		seperator = line.rfind(':');
		if(seperator != std::string::npos) {
			std::string jumpMark = line.substr(0, seperator);
			jumpMarks.insert(std::pair<AddressType, std::string>(addresses[currentSection], std::trim(jumpMark)));
			line = line.substr(seperator+1);
		}

		if(line.size() == 0) continue;
		addInstruction(line);
	}
	file.close();

	return true;
}
