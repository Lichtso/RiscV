#pragma once
#include "Instruction.hpp"

class Disassembler {
	public:
	enum {
		FlagNop = 1<<0,
		FlagMove = 1<<1,
		FlagJump = 1<<2,
		FlagCSR = 1<<3
	} flags;

	char buffer[64];
	AddressType currentPosition;
	std::vector<std::string> textSection;
	std::map<AddressType, UInt64> jumpMarks;

	void addJumpMark(AddressType address) {
		auto iter = jumpMarks.find(address);
		if(iter == jumpMarks.end()) {
			auto res = jumpMarks.insert(std::pair<AddressType, UInt64>(address, jumpMarks.size()));
			iter = res.first;
		}
		sprintf(buffer, "%sJM_%llx", buffer, iter->second);
	}

	void disassembleInstruction(const Instruction& instruction);
};
