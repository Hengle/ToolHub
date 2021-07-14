#pragma vengine_package vengine_dll

#include "VGuid.h"
#include <objbase.h>
namespace vstd {
Guid::Guid() {
	static_assert(sizeof(data) == sizeof(_GUID), "GUID size incorrect!");
	HRESULT h = CoCreateGuid(reinterpret_cast<_GUID*>(data));
	if (h != S_OK) {
		VEngine_Log("GUID Generate Failed!\n"_sv);
		VENGINE_EXIT;
	}
}
std::ostream& operator<<(std::ostream& out, const Guid& obj) noexcept {
	out << obj.data[0] << ',' << obj.data[1];
	return out;
}

}// namespace vstd
