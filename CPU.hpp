#include "RAM.hpp"

template<UInt8 XLEN = 64>
class CPU {
    public:
    typedef typename std::conditional<XLEN == 64, Int64, Int32>::type IntType;
    typedef typename std::conditional<XLEN == 64, UInt64, UInt32>::type UIntType;
    typedef typename std::conditional<XLEN == 64, Float64, Float32>::type FloatType;

    struct {
        UIntType i[32]; // pc = i[0]
        FloatType f[32];
        UInt16 csr[128];
    } registers;


};
