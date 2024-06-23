#include "tt_uuid.h"
#include "tt_messages.h"
#include "windont.h"
#include <guiddef.h>

static_assert(offsetof(TT::UUID, Data1) == offsetof(GUID, Data1));
static_assert(offsetof(TT::UUID, Data2) == offsetof(GUID, Data2));
static_assert(offsetof(TT::UUID, Data3) == offsetof(GUID, Data3));
static_assert(offsetof(TT::UUID, Data4) == offsetof(GUID, Data4));
static_assert(sizeof(TT::UUID) == sizeof(GUID));

namespace TT {
    UUID::UUID() : Data1(0), Data2(0), Data3(0), Data4 {0,0,0,0,0,0,0,0} {}
    
    UUID UUID::generate() {
        UUID r;
        TT::assert(CoCreateGuid((GUID*)&r) == RPC_S_OK);
        return r;
    }

    UUID::UUID(const std::string_view str) {
        TT::assert(UuidFromStringA((unsigned char*)str.data(), (GUID*)this) == RPC_S_OK);
    }

    UUID::operator std::string() const {
        std::string buf;
        RPC_CSTR szUuid = NULL;
        if (UuidToStringA((GUID*)this, &szUuid) == RPC_S_OK) {
            buf = (char*)szUuid;
            RpcStringFreeA(&szUuid);
        }
        return buf;
    }
}
