#pragma once
#include "Disassembler.hpp"

class RAM {
    public:
    UInt8 size;
    std::unique_ptr<UInt8> data;

    void setSize(UInt8 _size) {
        size = _size;
        data.reset((size) ? new UInt8[1<<size] : NULL);
    }

    RAM() :size(0) { }

    template<typename type, bool unaligned>
    void get(AddressType address, type* value) {
        if(unaligned)
            memcpy(value, data.get()+address, sizeof(type));
        else
            *value = *reinterpret_cast<type*>(data.get()+address);
    }

    template<typename type, bool unaligned>
    void set(AddressType address, type* value) {
        if(unaligned)
            memcpy(data.get()+address, value, sizeof(type));
        else
            *reinterpret_cast<type*>(data.get()+address) = *value;
    }
};

extern RAM ram;
