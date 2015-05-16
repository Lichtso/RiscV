#include "Instruction.hpp"

class RAM {
    public:
    UInt64 size;
    UInt8* data;

    RAM(UInt64 _size) :size(_size), data(new UInt8[_size]) {

    }

    ~RAM() {
        delete[] data;
    }
};
