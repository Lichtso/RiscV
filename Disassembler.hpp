#pragma once
#include "Instruction.hpp"

class Disassembler {
	public:
	enum {
		FlagNop = 1<<0,
		FlagMove = 1<<1,
		FlagJump = 1<<2,
		FlagCSR = 1<<3,
		FlagABI = 1<<4,
		FlagDec = 1<<5,
		FlagLowerCase = 1<<6,
		FlagAll = 0xFF
	} flags = FlagAll;

	char buffer[64];
	std::map<AddressType, std::string> textSection;
	std::map<AddressType, std::string> jumpMarks;

	void addJumpMark(AddressType address) {
		auto iter = jumpMarks.find(address);
		if(iter == jumpMarks.end()) {
			char str[32];
			sprintf(str, "mark_%ld", jumpMarks.size());
			auto res = jumpMarks.insert(std::pair<AddressType, std::string>(address, str));
			iter = res.first;
		}
		sprintf(buffer, "%s %s", buffer, iter->second.c_str());
	}

	void addInstruction(const Instruction& instruction, AddressType address);
	void addSection(const UInt8* base, AddressType address, AddressType size);
	void serializeTextSection(std::ostream& stream);
	void writeToFile(const std::string& path);
};
