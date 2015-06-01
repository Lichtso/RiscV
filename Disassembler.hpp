/*
	WARNING:

	The assembler / disassembler was made for testing purpose only.
	They are highly instable and might produce wrong outputs.
	Do not use them in production or any other projects!
*/

#pragma once
#include "Instruction.hpp"

class Disassembler {
	public:
	enum {
		FlagArithmeticPseudo = 1U<<0,
		FlagLogicPseudo = 1U<<1,
		FlagFloatPseudo = 1U<<2,
		FlagJumpPseudo = 1U<<3,
		FlagCSRPseudo = 1U<<4,
		FlagRegisterABI = 1U<<5,
		FlagDecimal = 1U<<6,
		FlagLowerCase = 1U<<7,
		FlagAddresses = 1U<<8,
		FlagDataSection = 1U<<9,
		FlagAll = (1U<<10)-1
	} flags = FlagAll;

	char buffer[64];
	std::map<AddressType, std::string> textSection;
	std::map<AddressType, std::string> symbols;
	std::map<AddressType, std::string> jumpMarks;
	std::stringstream extension;

	void addJumpMark(AddressType address) {
		auto jumpMarkIter = jumpMarks.find(address);
		if(jumpMarkIter == jumpMarks.end()) {
			char str[128];
			auto symbolIter = symbols.upper_bound(address);
			if(symbolIter == symbols.begin()) {
				sprintf(str, "null[%llx]", address);
			}else{
				--symbolIter;
				sprintf(str, "%s[%llx]", symbolIter->second.c_str(), address-symbolIter->first);
			}
			jumpMarkIter = jumpMarks.insert(std::pair<AddressType, std::string>(address, str)).first;
		}
		sprintf(buffer, "%s %s", buffer, jumpMarkIter->second.c_str());
	}
	void addToTextSection(AddressType address);
	void addInstruction(AddressType address, const Instruction& instruction);
	void addFunction(const UInt8* base, const std::string& name, AddressType address, AddressType size);
	bool writeToFile(const std::string& path);
	bool readFromFile(const std::string& path);
};

class Assembler {
	public:
	AddressType addresses[3];
	std::map<AddressType, std::unique_ptr<UInt8>> sections[3];
	std::map<AddressType, std::string> jumpMarks;

	void writeInSection(UInt8 index, UInt8 length, const void* data);
	void addInstruction(std::string command);
	bool writeToFile(const std::string& path);
	bool readFromFile(const std::string& path);
};
