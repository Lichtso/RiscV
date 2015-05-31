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
		FlagArithmeticPseudo = 1<<0,
		FlagLogicPseudo = 1<<1,
		FlagFloatPseudo = 1<<2,
		FlagJumpPseudo = 1<<3,
		FlagCSRPseudo = 1<<4,
		FlagRegisterABI = 1<<5,
		FlagDecimal = 1<<6,
		FlagLowerCase = 1<<7,
		FlagAddresses = 1<<8,
		FlagDataSection = 1<<9,
		FlagAll = (1<<10)-1
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
