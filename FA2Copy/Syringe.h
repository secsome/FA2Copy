#pragma once
#define SYR_VER 2
#if SYR_VER == 2

#pragma pack(push, 16)
#pragma warning(push)
#pragma warning( disable : 4324)
__declspec(align(16)) struct hookdecl {
	unsigned int hookAddr;
	unsigned int hookSize;
	const char * hookName;
};

__declspec(align(16)) struct hostdecl {
	unsigned int hostChecksum;
	const char * hostName;
};
#pragma warning(pop)
#pragma pack(pop)

#pragma section(".syhks00", read, write)
#pragma section(".syexe00", read, write)
namespace SyringeData {
	namespace Hooks {

	};
	namespace Hosts {

	};
};

#define declhost(exename, checksum) \
namespace SyringeData { namespace Hosts { __declspec(allocate(".syexe00")) hostdecl _hst__ ## exename = { checksum, #exename }; }; };

#define declhook(hook, funcname, size) \
namespace SyringeData { namespace Hooks { __declspec(allocate(".syhks00")) hookdecl _hk__ ## hook ## funcname = { 0x ## hook, 0x ## size, #funcname }; }; };

#endif // SYR_VER == 2