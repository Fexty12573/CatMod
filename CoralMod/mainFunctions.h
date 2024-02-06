#pragma once
#include <Windows.h>
#include <vector>
#include <cstdlib>
#include <ctime>

namespace mem
{
	typedef std::vector<BYTE> vByte;
	typedef unsigned int uint;

	//Internal: specific for coral mod
	void writeMem(uintptr_t address, uintptr_t moduleBase, vByte insert_attack);
	void delMem(uintptr_t address, uintptr_t moduleBase, vByte original_asm);
	void blockDefHp(int random, uintptr_t defSAdd, uintptr_t defLAddr, uintptr_t hpAddr, vByte toAff, vByte toRec, vByte toAtkS, vByte toSta);
	void uninstallHeroics(uintptr_t defSAddr, uintptr_t defLAddr, uintptr_t hpAddr, vByte defS, vByte defL, vByte Hp);
	int randomNum(uint num);
	uintptr_t getPtrAddr(uintptr_t moduleBase, uintptr_t baseOffset, std::vector<uint> offsets);

	//Internal: general
	void Patch(uintptr_t moduleBase, uintptr_t address, vByte OpBytes);
	void Nop(uintptr_t moduleBase, uintptr_t address, uint size);
	void PatchA(void* address, vByte OpBytes);
	void NopA(void* address, uint size);
	
	//External
	uintptr_t getModuleBase(DWORD pID, const char* moduleName);
	uintptr_t getPointerAddrEx(HANDLE pHandle, uintptr_t baseOffset, vByte offsets);
	void nopEx(HANDLE pHandle, uintptr_t moduleBase, uintptr_t address, uint size);
	void writeMemEx(HANDLE pHandle, uintptr_t moduleBase, uintptr_t address, vByte input);
	void writeValEx(HANDLE pHandle, uintptr_t moduleBase, uintptr_t address, uint value);
}
