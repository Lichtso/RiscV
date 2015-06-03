#pragma once
#include "Float.hpp"

class Exception {
    public:
    enum Code {
		InstructionAddressMisaligned = 0,
		InstructionAccessFault = 1,
		IllegalInstruction = 2,
		Breakpoint = 3,
		LoadAddressMisaligned = 4,
		LoadAccessFault = 5,
		StoreAddressMisaligned = 6,
		StoreAccessFault = 7,
		EnvironmentCallFromU = 8,
		EnvironmentCallFromS = 9,
		EnvironmentCallFromH = 10,
		EnvironmentCallFromM = 11,
        SoftwareInterrupt = 0,
        TimerInterrupt = 1
	} cause;
    bool interrupt;

    Exception(Code _cause, bool _interrupt = false)
        :cause(_cause), interrupt(_interrupt)
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
