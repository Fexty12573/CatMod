#include "pch.h"
#include "mainFunctions.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <vector>

using namespace mem;

//Internal ---------------------------------------------------------------------------------------------------

void mem::writeMem(uintptr_t address, uintptr_t moduleBase, vByte insert_attack)
{
    DWORD oldProtect;
    VirtualProtect((LPVOID)(moduleBase + address), insert_attack.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)(moduleBase + address), &insert_attack[0], insert_attack.size()); //writes memory to gaurantee attack buff
    VirtualProtect((LPVOID)(moduleBase + address), insert_attack.size(), oldProtect, &oldProtect);
}

void mem::delMem(uintptr_t address, uintptr_t moduleBase, vByte original_asm) //restores original code at moduleBaseAddr+address
{
    DWORD oldProtect;
    VirtualProtect((LPVOID)(moduleBase + address), original_asm.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)(moduleBase + address), &original_asm[0], original_asm.size());
    VirtualProtect((LPVOID)(moduleBase + address), original_asm.size(), oldProtect, &oldProtect);
}

void mem::blockDefHp(int random, uintptr_t defSAddr, uintptr_t defLAddr, uintptr_t hpAddr, vByte toAff, vByte toRec, vByte toAtkS, vByte toSta) //disables defS, defL, Hp for heroics mode
{ //stupid function that needs way too many arguments
    DWORD oldProtect1;
    DWORD oldProtect2;
    DWORD oldProtect3;
    VirtualProtect((LPVOID)(defSAddr), toSta.size(), PAGE_EXECUTE_READWRITE, &oldProtect1);
    VirtualProtect((LPVOID)(defLAddr), toSta.size(), PAGE_EXECUTE_READWRITE, &oldProtect2);
    VirtualProtect((LPVOID)(hpAddr), toSta.size(), PAGE_EXECUTE_READWRITE, &oldProtect3);
    switch (random) //unnecessarily complicated way to decide alternate buff
    {
    case 1:
    {
        memcpy((LPVOID)(defSAddr), &toSta[0], toSta.size());
        memcpy((LPVOID)(defLAddr), &toSta[0], toSta.size());
        memcpy((LPVOID)(hpAddr), &toSta[0], toSta.size());
        break;
    }
    case 2:
    {
        memcpy((LPVOID)(defSAddr), &toAff[0], toAff.size());
        memcpy((LPVOID)(defLAddr), &toAff[0], toAff.size());
        memcpy((LPVOID)(hpAddr), &toAff[0], toAff.size());
        break;
    }
    case 3:
    {
        memcpy((LPVOID)(defSAddr), &toRec[0], toRec.size());
        memcpy((LPVOID)(defLAddr), &toRec[0], toRec.size());
        memcpy((LPVOID)(hpAddr), &toRec[0], toRec.size());
        break;
    }
    case 4:
    {
        memcpy((LPVOID)(defSAddr), &toAtkS[0], toAtkS.size());
        memcpy((LPVOID)(defLAddr), &toAtkS[0], toAtkS.size());
        memcpy((LPVOID)(hpAddr), &toAtkS[0], toAtkS.size());
        break;
    }
    }
    VirtualProtect((LPVOID)(defSAddr), toSta.size(), oldProtect1, &oldProtect1);
    VirtualProtect((LPVOID)(defLAddr), toSta.size(), oldProtect2, &oldProtect2);
    VirtualProtect((LPVOID)(hpAddr), toSta.size(), oldProtect3, &oldProtect3);
}

void mem::uninstallHeroics(uintptr_t defSAddr, uintptr_t defLAddr, uintptr_t hpAddr, vByte defS, vByte defL, vByte Hp)
{
    DWORD oldProtect1;
    DWORD oldProtect2;
    DWORD oldProtect3;
    VirtualProtect((LPVOID)(defSAddr), defS.size(), PAGE_EXECUTE_READWRITE, &oldProtect1); //remove memory protection for all 3 locations
    VirtualProtect((LPVOID)(defLAddr), defS.size(), PAGE_EXECUTE_READWRITE, &oldProtect2);
    VirtualProtect((LPVOID)(hpAddr), defS.size(), PAGE_EXECUTE_READWRITE, &oldProtect3);
    memcpy((void*)(defSAddr), &defS[0], defS.size()); //write asm variables
    memcpy((void*)(defLAddr), &defL[0], defL.size());
    memcpy((void*)(hpAddr), &Hp[0], Hp.size());
    VirtualProtect((LPVOID)(defSAddr), defS.size(), oldProtect1, &oldProtect1); //restore protection
    VirtualProtect((LPVOID)(defLAddr), defS.size(), oldProtect2, &oldProtect2);
    VirtualProtect((LPVOID)(hpAddr), defS.size(), oldProtect3, &oldProtect3);
}

uintptr_t mem::getPtrAddr(uintptr_t moduleBase, uintptr_t baseOffset, std::vector<uint> offsets) //calculates attack address
{
     
    uintptr_t pointerAddr = moduleBase + baseOffset;
    for (uint i = 0; i < offsets.size(); ++i) { //loop however many times specified in passed vector
        pointerAddr = *(uintptr_t*)pointerAddr; //cast to pointer and dereference to get value (address)
        pointerAddr += offsets[i]; //add offset
    }
    return pointerAddr;
}

int mem::randomNum(uint num) //generates a random number between 1 and 4
{
    srand((unsigned)time(0));
    int randomNumber;
    for (int index = 0; index < 10; index++) {
        randomNumber = (rand() % num) + 1; //change 4 to however many desired options
    }
    return randomNumber;
}
//Internal: General ------------------------------------------------------------------------------------------

void mem::Patch(uintptr_t moduleBase, uintptr_t address, vByte OpBytes)
{
    DWORD oldProtect;
    VirtualProtect((LPVOID)(moduleBase + address), OpBytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((LPVOID)(moduleBase + address), &OpBytes[0], OpBytes.size()); //writes memory to gaurantee attack buff
    VirtualProtect((LPVOID)(moduleBase + address), OpBytes.size(), oldProtect, &oldProtect);
}

void mem::Nop(uintptr_t moduleBase, uintptr_t address, uint size)
{
    DWORD oldprotect;
    VirtualProtect((LPVOID)(moduleBase + address), size, PAGE_EXECUTE_READWRITE, &oldprotect);
    memset((void*)(moduleBase + address), 0x90, size);
    VirtualProtect((LPVOID)(moduleBase + address), size, oldprotect, &oldprotect);
}

void mem::PatchA(void* address, vByte OpBytes)
{
    DWORD oldprotect;
    VirtualProtect(address, OpBytes.size(), PAGE_EXECUTE_READWRITE, &oldprotect);
    memcpy(address, &OpBytes[0], OpBytes.size());
    VirtualProtect(address, OpBytes.size(), oldprotect, &oldprotect);
}

void mem::NopA(void* address, uint size)
{
    DWORD oldprotect;
    VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldprotect);
    memset(address, 0x90, size);
    VirtualProtect(address, size, oldprotect, &oldprotect);
}

//External ---------------------------------------------------------------------------------------------------
uintptr_t mem::getModuleBase(DWORD pID, const char* moduleName)
{
    uintptr_t ModuleBaseAddress = 0; //declare ModuleBaseAddress
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pID); //make snapshot of all active modules in process
    if (hSnapshot != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 ModuleEntry32; //defines ModuleEntry32
        ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(hSnapshot, &ModuleEntry32))
        {
            do
            {
                if (strcmp((const char*)ModuleEntry32.szModule, moduleName) == 0) //compares the current selected module to szModuleName (MonsterHunterWorld.exe)
                {
                    ModuleBaseAddress = (uintptr_t)ModuleEntry32.modBaseAddr; //if correct, it moves modBaseAddr into ModuleBaseAddress
                    break;
                }
            } while (Module32Next(hSnapshot, &ModuleEntry32));
        }
        CloseHandle(hSnapshot);
    }
    return ModuleBaseAddress;
}

uintptr_t mem::getPointerAddrEx(HANDLE pHandle, uintptr_t baseOffset, vByte offsets)
{
    uintptr_t pointerAddress = baseOffset;
    for (uint i = 0; i < offsets.size(); ++i) { //loop through each of the offsets provided in the passed vector
        ReadProcessMemory(pHandle, (LPVOID)(pointerAddress + offsets[i]), &pointerAddress, sizeof(pointerAddress), 0); //read memory at address+current-offset and put into pointerAddress
        pointerAddress += offsets[i]; //add offset to pointerAddress
    }
    return pointerAddress;
}

void mem::nopEx(HANDLE pHandle, uintptr_t moduleBase, uintptr_t address, uint size)
{
    const uint8_t sc_nop[]{ //declare nop asm variable
        0x90 // 90 = nop (no operation)
    };
    DWORD oldprotect;
    VirtualProtectEx(pHandle, (LPVOID)address, size, PAGE_EXECUTE_READWRITE, &oldprotect); //remove memory protection
    for (uint i = 0; i < size; i++) { 
        WriteProcessMemory(pHandle, (LPVOID)(address + i), sc_nop, sizeof(sc_nop), 0); //write nop however many times specified in 'size'
    }
    VirtualProtectEx(pHandle, (LPVOID)address, size, oldprotect, &oldprotect); //restore memory protection
}

void mem::writeMemEx(HANDLE pHandle, uintptr_t moduleBase, uintptr_t address, vByte input)
{
    DWORD oldprotect;
    VirtualProtectEx(pHandle, (LPVOID)address, sizeof(input), PAGE_EXECUTE_READWRITE, &oldprotect);
    WriteProcessMemory(pHandle, (LPVOID)address, &input[0], sizeof(input), 0); //write asm code specified in 'input' to address
    VirtualProtectEx(pHandle, (LPVOID)address, sizeof(input), oldprotect, &oldprotect);
}

void mem::writeValEx(HANDLE pHandle, uintptr_t moduleBase, uintptr_t address, uint value)
{
    WriteProcessMemory(pHandle, (LPVOID)(moduleBase + address), &value, sizeof(value), 0); //write value specified in 'value' as float to address
}