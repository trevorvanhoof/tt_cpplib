#pragma once

#include <string>
#include "tt_math.h"

#pragma comment(lib, "Rpcrt4.lib")

namespace TT {
    class UUID {
    public:
        unsigned long  Data1;
        unsigned short Data2;
        unsigned short Data3;
        unsigned char  Data4[ 8 ];

        UUID();
        static UUID generate();
        UUID(const std::string_view str);
        operator std::string() const;

        // The compare operator is required by std::unordered_map
        bool operator==(const UUID& rhs) const {
            return std::memcmp(this, &rhs, sizeof(UUID)) == 0;
        }
    };
}

// Specialize std::hash
namespace std {
    template<> struct hash<TT::UUID> {
        size_t operator()(const TT::UUID& guid) const noexcept {
            const std::uint64_t* p = reinterpret_cast<const std::uint64_t*>(&guid);
            static std::hash<std::uint64_t> hash;
            return TT::hashCombine(hash(p[0]), hash(p[1]));
        }
    };
}
