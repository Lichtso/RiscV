#pragma once
#include "Instruction.hpp"

class RAM {
    public:
    typedef UInt64 PointerType;

    UInt8 size;
    std::unique_ptr<UInt8> data;

    void setSize(UInt8 _size) {
        size = _size;
        data.reset((size) ? new UInt8[1<<size] : NULL);
    }

    RAM() :size(0) { }

    #define accessorOfType(type) \
    type getUnaligned##type(PointerType address) { \
        type value; \
        memcpy(&value, data.get()+address, sizeof(type)); \
        return value; \
    } \
    void setUnaligned##type(PointerType address, type value) { \
        memcpy(data.get()+address, &value, sizeof(type)); \
    } \
    type getAligned##type(PointerType address) { \
        return *reinterpret_cast<type*>(data.get()+address); \
    } \
    void setAligned##type(PointerType address, type value) { \
        *reinterpret_cast<type*>(data.get()+address) = value; \
    }

    accessorOfType(Float64)
    accessorOfType(UInt64)
    accessorOfType(Int64)
    accessorOfType(Float32)
    accessorOfType(UInt32)
    accessorOfType(Int32)
    accessorOfType(UInt16)
    accessorOfType(Int16)
    accessorOfType(UInt8)
    accessorOfType(Int8)
};

extern RAM ram;
