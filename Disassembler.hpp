#pragma once
#include "Instruction.hpp"

class Disassembler {
	public:
	char buffer[64];
	AddressType currentPosition;
	std::vector<std::string> textSection;
	std::map<AddressType, UInt64> jumpMarks;

	UInt64 addJumpMark(AddressType address) {
		auto iter = jumpMarks.find(address);
		if(iter == jumpMarks.end()) {
			auto res = jumpMarks.insert(std::pair<AddressType, UInt64>(address, jumpMarks.size()));
			iter = res.first;
		}
		return iter->second;
	}

	void disassembleInstruction(const Instruction& instruction);
};
