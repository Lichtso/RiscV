#pragma once
#include "Base.hpp"

class Exception {
    public:
    enum Code {
		InstructionAddressMisaligned,
		InstructionAccessFault,
		IllegalInstruction,
		Breakpoint,
		LoadAddressMisaligned,
		LoadAccessFault,
		StoreAddressMisaligned,
		StoreAccessFault,
		EnvironmentCallFromU,
		EnvironmentCallFromS,
		EnvironmentCallFromH,
		EnvironmentCallFromM
	} cause;

    Exception(Code _cause)
        :cause(_cause)
        { }
};

class MemoryAccessException : public Exception {
    public:
    AddressType address;

    MemoryAccessException(Code _cause, AddressType _address)
        :Exception(_cause),
        address(_address)
        { }
};
