#pragma once
#include "Base.hpp"

class Exception {
	public:

	enum Type {
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
	} type;

	Exception(Type _type) :type(_type) {

	}
};
