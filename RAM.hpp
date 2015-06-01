#pragma once
#include "Disassembler.hpp"

class RAM {
    public:
    UInt8 size;
    std::unique_ptr<UInt8> data;

    void setSize(UInt8 _size) {
        size = _size;
        data.reset((size) ? new UInt8[1ULL<<size] : NULL);
    }

    RAM() :size(0) { }

    void dump(std::ostream& out) {
        if(size < 6) return;
        AddressType upTo = 1ULL<<size;
        out << std::setfill('0') << std::hex;
        out << std::setw(16) << *reinterpret_cast<UInt64*>(data.get());
        for(AddressType i = sizeof(UInt64); i < upTo; i += sizeof(UInt64)) {
            if(i%(8*sizeof(UInt64)) == 0) {
                out << std::endl;
                if(i%4096 == 0)
                    out << std::endl;
            }else
                out << " ";
            out << std::setw(16) << *reinterpret_cast<UInt64*>(data.get()+i);
        }
        out << std::endl;
    }

    template<typename type, bool aligned>
    void get(AddressType address, type* value) {
        if(aligned)
            *value = *reinterpret_cast<type*>(data.get()+address);
        else
            memcpy(value, data.get()+address, sizeof(type));
    }

    template<typename type, bool aligned>
    void set(AddressType address, type* value) {
        if(aligned)
            *reinterpret_cast<type*>(data.get()+address) = *value;
        else
            memcpy(data.get()+address, value, sizeof(type));
    }
};

extern RAM ram;
