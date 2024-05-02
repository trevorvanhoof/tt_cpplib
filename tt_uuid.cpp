#include "tt_uuid.h"
#include "windont.h"
#include <guiddef.h>

static_assert(offsetof(TT::UUID, Data1) == offsetof(GUID, Data1));
static_assert(offsetof(TT::UUID, Data2) == offsetof(GUID, Data2));
static_assert(offsetof(TT::UUID, Data3) == offsetof(GUID, Data3));
static_assert(offsetof(TT::UUID, Data4) == offsetof(GUID, Data4));
static_assert(sizeof(TT::UUID) == sizeof(GUID));

namespace TT {
    UUID::UUID() { CoCreateGuid((GUID*)this); }

    UUID::UUID(const char* str) {
        UuidFromStringA((unsigned char*)str, (GUID*)this);
    }

    UUID::operator std::string() const {
        static char buf[39];
        UuidToStringA((GUID*)this, (unsigned char**)&buf);
        return buf;
    }
}
