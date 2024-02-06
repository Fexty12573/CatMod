#include "pch.h"
#include <Windows.h>
#include <vector>
#include <iostream>
#include <string>
#include <ctime>
#include <fstream>
#include <sstream>

#include "loader.h"
#include "mainFunctions.h"
#include "findPattern.h"
#include "json.hpp"

#define log(x) loader::LOG(INFO) << "[UST] " << x;
#define debug(x) loader::LOG(DEBUG) << "[UST Debug] " << x;

using namespace mem;

typedef unsigned int uint;
typedef unsigned char byte;
typedef std::vector<void*> aobResults;
typedef unsigned char undefined;
typedef unsigned long long undefined8;

namespace Chat {
    static void(*BuildShowGameMessage)(undefined8, uint, byte, uint) = (void(*)(undefined8, uint, byte, uint))0x1601d8440;
    static void* MainPtr = (void*)0x144fb4070;
    static void(*ShowGameMessage)(undefined*, char*, float, uint, byte) = (void(*)(undefined*, char*, float, uint, byte))0x16070dd70;
}

void gameLog(std::string msg)
{
    Chat::ShowGameMessage(*(undefined**)Chat::MainPtr, &msg[0], -1, -1, 0);
}

void sstream(int select);
void LoadConfig();
bool CheckSteamID(undefined8 steamid64);
bool CheckSteamID2(undefined8 steamdid_1, undefined8 steamid_2);
void PatchHP(void* dst, vByte src);
uintptr_t GetAtkAddr(uintptr_t modBase, uintptr_t playerPtr, std::vector<unsigned int> offsets);
VOID startup(TCHAR* argv[]);

bool regularMode = false, randomMode = false, heroicsMode = false, alwaysAtkMode = false, setGaji = false, setBoa = false, drunkBird = false, launchHelloWorld = true, IgNotif = false, speedrunMode = false, disableTimer = false;
std::string catDefaultStr, regModeKeyStr, rndmModeKeyStr, hModeKeyStr, aAModeKeyStr, cGKeyStr, cBKeyStr, dbKeyStr, IgNotifKeyStr, speedrunKeyStr;
uint catDefault, regModeKey, rndmModeKey, hModeKey, aAModeKey, cGKey, cBKey, dbKey, debugOverride, IgNotifKey, speedrunKey, regModeTimer;
//hMode = heroics mode, aAMode = always atk mode, cG = cat Gaji, cB = cat Boa, db = drunk bird


DWORD WINAPI MainThread(HMODULE hModule)
{
    Sleep(1000);

    log("Initializing...");
    bool playHEHE = false;

    //Setup config
    LoadConfig();

    //playHEHE = true;

    uintptr_t modBase = (uintptr_t)GetModuleHandle(L"MonsterHunterWorld.exe");

    uintptr_t playerData = 0x4FB3FA0;
    uintptr_t* playerPtr = (uintptr_t*)(modBase + playerData);
    uintptr_t nullptrcheck = NULL;

    while (true)
    {
        nullptrcheck = *(uintptr_t*)playerPtr;
        if (nullptrcheck == NULL) {
            Sleep(1000);
            continue;
        }
        nullptrcheck = *(uintptr_t*)(nullptrcheck + 0x80);
        if (nullptrcheck == NULL) {
            Sleep(1000);
            continue;
        }
        break;
    }
    
    /*
    if (!CheckSteamID2(76561199016152780, 76561198449393073)) {
        log("Invalid ID");
        log("Exiting...");
        return 0;
    }
    */

    uintptr_t atkAddr = getPtrAddr(modBase, playerData, { 0x80, 0x7D20, 0x124 });
    uintptr_t affAddr = getPtrAddr(modBase, playerData, { 0x80, 0x7D20, 0x130 });
    uintptr_t sessAddr = getPtrAddr(modBase, playerData, { 0x90, 0x46A0, 0x14 });

    float atkVal;
    float affVal;
    float sessTime;
    float tempWrite = 111;
    float tempDel = 111;
    uint randomBuff;
    uint MR_toggle_switch = 0;
    uint HR_toggle_switch = 0;
    uint RandomModeCounter = 0;
    uint HeroicsModeCounter = 0;
    uint regModeCounter = 0;
    bool buffActive = false;
    bool speedrunModeSwitch = false;

    //Coral Cheerhorn
    vByte insert_attack = { 0xB0, 0x06, 0x90 };
    vByte insert_affinity = { 0xB0, 0x02, 0x90 };
    vByte insert_stamina = { 0xB0, 0x05, 0x90 };
    vByte insert_defense = { 0xB0, 0x07, 0x90 };
    vByte insert_recovery = { 0xB0, 0x03, 0x90 };
    vByte original_code = { 0x41, 0x8B, 0xC7 };
    vByte heroics_toSta = { 0xBA, 0x41, 0x00, 0x00, 0x00 };
    vByte heroics_toAff = { 0xBA, 0x3E, 0x00, 0x00, 0x00 };
    vByte heroics_toRec = { 0xBA, 0x3F, 0x00, 0x00, 0x00 };
    vByte heroics_toAtkS = { 0xBA, 0x3A, 0x00, 0x00, 0x00 };
    vByte uninst_defS = { 0xBA, 0x3C, 0x00, 0x00, 0x00 };
    vByte uninst_defL = { 0xBA, 0x3D, 0x00, 0x00, 0x00 };
    vByte uninst_hp = { 0xBA, 0x40, 0x00, 0x00, 0x00 };
    vByte nop = { 0x90, 0x90, 0x90 };

    //Gajalakas & Boaboa
    vByte setCatGaji = { 0xB3, 0x06 };
    vByte setCatBoa = { 0xB3, 0x08 };
    vByte catDelJne = { 0x8B, 0xFE };
    vByte catResJne = { 0x75, 0x06 };
    vByte restoreCat = { 0x8B, 0xD3 };

    //Bugfix for special cases
    vByte catForceJE = { 0xB9, 0x08, 0x00, 0x00, 0x00 };
    vByte catResJE_1 = { 0x8B, 0x88, 0xF0, 0x24, 0x10, 0x00 };
    vByte catResJE_2 = { 0xC1, 0xE9, 0x0C };
    vByte catResJNE_fix = { 0x75, 0x02 };

    //Player spawn
    vByte cmpedi1 = { 0x83, 0xFF, 0x39 };
    vByte movecx = { 0xB9, 0x64, 0x00, 0x00, 0x00 };
    vByte movsil = { 0x40, 0xB6, 0x01 };
    vByte cmpedi2 = { 0xBf, 0x01, 0x00, 0x00, 0x00 };
    vByte plNop = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    vByte cmpedi1Res = { 0x83, 0xFF, 0x01 };
    vByte movecxRes = { 0xB9, 0x14, 0x00, 0x00, 0x00 };
    vByte movsilRes = { 0x0F, 0x1F, 0x00 };
    vByte cmpedi2Res = { 0x41, 0x8B, 0xBC, 0xC6, 0x84, 0x56, 0x18 ,0x03 };

    //MR Multiplier
    vByte add_r15d = { 0x45, 0x01, 0xFF };
    vByte add_ecx = { 0x44, 0x01, 0xF9 };
    vByte mov_eax = { 0xB8, 0xFF, 0xE0, 0xF5, 0x05 };
    vByte cmp_ecx = { 0x39, 0xC1 };
    vByte cmova_ecx = { 0x0F, 0x47, 0xC8 };

    //HR Multiplier
    vByte add_esiHR = { 0x01, 0xF6 };
    vByte add_ecxHR = { 0x01, 0xF1 };

    //Create a searchstate object
    searchState sSCoralMod = aobToSearch("49 03 C8 FF E1 BA 3C000000 8B C2");
    searchState sSCatMod = aobToSearch("33 D2 F7 F5 8B FA ???????? 33 D2 8B CB ?????????? 3C 01");
    searchState sSCatMod2 = aobToSearch("41 3A C5 8B EE BF 08000000 41 0F44 ED 8B CF 33 D2");
    searchState sSdb = aobToSearch("0F1F 00 41 3B C8 77 15 FF C3 44 2B C1 8B C3");
    /*
    searchState sSMR = aobToSearch("C1 E1 04 F7 E1 41 89 D7 41 C1 EF 05 ?? ?? ???????? B8 FF E0 F5 05");
    searchState sSHR = aobToSearch("41 54 41 56 41 57 48 83 EC 20 48 89 CF 89 D6 48 83 C1 08");
    */

    //Aobscan using searchstate object
    aobResults aobResCoralMod = aobscan(sSCoralMod);
    aobResults aobResCatMod = aobscan(sSCatMod);
    aobResults aobResCatMod2 = aobscan(sSCatMod2);
    aobResults aobResDb = aobscan(sSdb);
    /*
    aobResults aobResMR = aobscan(sSMR);
    aobResults aobResHR = aobscan(sSHR);
    */

    //Add offsets to returned addresses
    uintptr_t mainAddr = (uintptr_t)aobResCoralMod.at(0) - 0xB;
    uintptr_t defSAddr = (uintptr_t)aobResCoralMod.at(0) + 0x5;
    uintptr_t defLAddr = (uintptr_t)aobResCoralMod.at(0) + 0x83;
    uintptr_t hpAddr = (uintptr_t)aobResCoralMod.at(0) + 0x44;

    uintptr_t catSetAddr = (uintptr_t)aobResCatMod.at(0) + 0xC;
    uintptr_t catmovedi = (uintptr_t)aobResCatMod.at(0) + 0x15;
    uintptr_t catDelJNE_fix = (uintptr_t)aobResCatMod.at(0) - 0x1B;

    uintptr_t catMovEcx = (uintptr_t)aobResCatMod2.at(0) - 0xAB;
    uintptr_t catNopShr = (uintptr_t)aobResCatMod2.at(0) - 0xA5;

    uintptr_t cmp1Addr = (uintptr_t)aobResDb.at(0) - 0x2D;
    uintptr_t movecxAddr = (uintptr_t)aobResDb.at(0) - 0x5;
    uintptr_t movsilAddr = (uintptr_t)aobResDb.at(0);
    uintptr_t cmp2Addr = (uintptr_t)aobResDb.at(0) + 0x1D;

    /*
    uintptr_t MR_0x12 = (uintptr_t)aobResMR.at(0) + 0x12;
    uintptr_t MR_0x15 = (uintptr_t)aobResMR.at(0) + 0x15;
    uintptr_t MR_0x17 = (uintptr_t)aobResMR.at(0) + 0x17;
    uintptr_t MR_0x1A = (uintptr_t)aobResMR.at(0) + 0x1A;
    uintptr_t MR_0x18 = (uintptr_t)aobResMR.at(0) + 0x18;
    uintptr_t MR_0x1C = (uintptr_t)aobResMR.at(0) + 0x1C;
    uintptr_t MR_0x1B = (uintptr_t)aobResMR.at(0) + 0x1B;

    uintptr_t HR_0x2C = (uintptr_t)aobResHR.at(0) + 0x2C;
    uintptr_t HR_0x2E = (uintptr_t)aobResHR.at(0) + 0x2E;
    uintptr_t HR_0x30 = (uintptr_t)aobResHR.at(0) + 0x30;
    uintptr_t HR_0x32 = (uintptr_t)aobResHR.at(0) + 0x32;
    uintptr_t HR_0x31 = (uintptr_t)aobResHR.at(0) + 0x31;
    uintptr_t HR_0x33 = (uintptr_t)aobResHR.at(0) + 0x33;
    uintptr_t HR_0x35 = (uintptr_t)aobResHR.at(0) + 0x35;
    */

    Sleep(1500);

    log("Initialized");
    log("Ready!");

    if (IgNotif) {
        gameLog("[UST] Ready!");
    }

    if (regularMode && heroicsMode || regularMode && randomMode || regularMode && alwaysAtkMode || heroicsMode && randomMode || heroicsMode && alwaysAtkMode || randomMode && alwaysAtkMode) {
        regularMode = false;
        randomMode = false;
        heroicsMode = false;
        alwaysAtkMode = false;
        log("Can't have multiple types enabled by default");
        log("Everything disabled");
    }

    //Apply config values
    if (setGaji) {
        NopA((void*)catMovEcx, 9);
        PatchA((void*)catMovEcx, catForceJE);
        NopA((void*)catDelJNE_fix, 2);
        PatchA((void*)catmovedi, catDelJne);
        PatchA((void*)catSetAddr, setCatGaji);
    }
    else if (setBoa) {
        NopA((void*)catMovEcx, 9);
        PatchA((void*)catMovEcx, catForceJE);
        NopA((void*)catDelJNE_fix, 2);
        PatchA((void*)catmovedi, catDelJne);
        PatchA((void*)catSetAddr, setCatBoa);
    }

    if (drunkBird) {
        PatchA((void*)cmp1Addr, cmpedi1);
        PatchA((void*)movecxAddr, movecx);
        PatchA((void*)movsilAddr, movsil);
        NopA((void*)cmp2Addr, 8);
        PatchA((void*)cmp2Addr, cmpedi2);
    }

    //main loop
    while (true)
    {
        Sleep(10);
        //All toggles
        if (GetAsyncKeyState(regModeKey) & 1) { //regular mode
            regularMode = !regularMode;
            if (regularMode) {
                PatchA((void*)mainAddr, original_code);
                log("Regular Mode activated and all other modes deactivated");
                if (IgNotif) {
                    gameLog("[UST] Regular Mode On");
                }
                randomMode = false;
                if (heroicsMode) {
                    uninstallHeroics(defSAddr, defLAddr, hpAddr, uninst_defS, uninst_defL, uninst_hp);
                    heroicsMode = false;
                }
                alwaysAtkMode = false;
            }
            else if (!regularMode) {
                log("Regular Mode deactivated");
                if (IgNotif) {
                    gameLog("[UST] Regular Mode Off");
                }
                regModeCounter = 0;
                buffActive = false;
                PatchA((void*)mainAddr, original_code);
            }
        }

        if (GetAsyncKeyState(hModeKey) & 1) { //heroics mode
            heroicsMode = !heroicsMode;
            if (heroicsMode) {
                PatchA((void*)mainAddr, original_code);
                log("Heroics Mode activated and all other modes deactivated");
                if (IgNotif) {
                    gameLog("[UST] Heroics Mode On");
                }
                regularMode = false;
                regModeCounter = 0;
                buffActive = false;
                randomMode = false;
                alwaysAtkMode = false;
                if (speedrunMode) {
                    speedrunMode = false;
                    speedrunModeSwitch = false;
                }
            }
            else if (!heroicsMode) {
                log("Heroics Mode deactivated");
                if (IgNotif) {
                    gameLog("[UST] Heroics Mode Off");
                }
                PatchA((void*)mainAddr, original_code);
                uninstallHeroics(defSAddr, defLAddr, hpAddr, uninst_defS, uninst_defL, uninst_hp);
            }
        }

        if (GetAsyncKeyState(speedrunKey) & 1) {
            speedrunMode = !speedrunMode;
            if (speedrunMode) {
                PatchA((void*)mainAddr, original_code);
                regularMode = false;
                regModeCounter = 0;
                buffActive = false;
                randomMode = false;
                alwaysAtkMode = false;
                if (heroicsMode) {
                    heroicsMode = false;
                    uninstallHeroics(defSAddr, defLAddr, hpAddr, uninst_defS, uninst_defL, uninst_hp);
                }
                PatchA((void*)hpAddr, heroics_toSta);
                log("Speedrun Mode activated and all other modes deactivated");
                if (IgNotif) {
                    gameLog("Speedrun Mode activated");
                }
            }
            else {
                log("Speedrun Mode deactivated");
                if (IgNotif) {
                    gameLog("Speedrun Mode deactivated");
                }
                uninstallHeroics(defSAddr, defLAddr, hpAddr, uninst_defS, uninst_defL, uninst_hp);
                PatchA((void*)mainAddr, original_code);
            }
        }

        if (GetAsyncKeyState(cGKey) & 1) { //guarantee gajalakas
            setGaji = !setGaji;
            if (setGaji) {
                setBoa = false;
                NopA((void*)catMovEcx, 9);
                PatchA((void*)catMovEcx, catForceJE);
                NopA((void*)catDelJNE_fix, 2);
                PatchA((void*)catmovedi, catDelJne);
                PatchA((void*)catSetAddr, setCatGaji);
                log("Cat Helpers: Always Gajalakas");
                if (IgNotif) {
                    gameLog("[UST] Forced Gajalakas");
                }
            }
            else if (!setGaji) {
                NopA((void*)catMovEcx, 9);
                PatchA((void*)catMovEcx, catResJE_1);
                PatchA((void*)catNopShr, catResJE_2);
                PatchA((void*)catDelJNE_fix, catResJNE_fix);
                PatchA((void*)catmovedi, catResJne);
                PatchA((void*)catSetAddr, restoreCat);
                log("Cat Helpers: Normal");
                if (IgNotif) {
                    gameLog("[UST] Disabled Gajalakas");
                }
            }
        }

        if (GetAsyncKeyState(cBKey) & 1) { //guarantee boaboa
            setBoa = !setBoa;
            if (setBoa) {
                setGaji = false;
                NopA((void*)catMovEcx, 9);
                PatchA((void*)catMovEcx, catForceJE);
                NopA((void*)catDelJNE_fix, 2);
                PatchA((void*)catmovedi, catDelJne);
                PatchA((void*)catSetAddr, setCatBoa);
                log("Cat Helpers: Always Boaboas");
                if (IgNotif) {
                    gameLog("[UST] Forced Boaboas");
                }
            }
            else if (!setBoa) {
                NopA((void*)catMovEcx, 9);
                PatchA((void*)catMovEcx, catResJE_1);
                PatchA((void*)catNopShr, catResJE_2);
                PatchA((void*)catDelJNE_fix, catResJNE_fix);
                PatchA((void*)catmovedi, catResJne);
                PatchA((void*)catSetAddr, restoreCat);
                log("Cat Helpers: Normal");
                if (IgNotif) {
                    gameLog("[UST] Disabled Boaboas");
                }
            }
        }

        if (GetAsyncKeyState(dbKey) & 1) {
            drunkBird = !drunkBird;
            if (drunkBird) {
                //rewrite cmp instructions to always get drunk bird
                PatchA((void*)cmp1Addr, cmpedi1);
                PatchA((void*)movecxAddr, movecx);
                PatchA((void*)movsilAddr, movsil);
                NopA((void*)cmp2Addr, 8);
                PatchA((void*)cmp2Addr, cmpedi2);
                log("Drunk Bird enabled");
                if (IgNotif) {
                    gameLog("[UST] Drunk Bird Enabled");
                }
            }
            else if (!drunkBird) {
                //restore original instructions
                PatchA((void*)cmp1Addr, cmpedi1Res);
                PatchA((void*)movecxAddr, movecxRes);
                PatchA((void*)movsilAddr, movsilRes);
                NopA((void*)cmp2Addr, 8);
                PatchA((void*)cmp2Addr, cmpedi2Res);
                log("Drunk Bird disabled");
                if (IgNotif) {
                    gameLog("[UST] Drunk Bird Disabled");
                }
            }
        }

        if (GetAsyncKeyState(IgNotifKey) & 1) {
            IgNotif = !IgNotif;
            if (IgNotif) {
                log("Ingame Messages Enabled");
                gameLog("[UST] Ingame Messages Enabled");
            }
            if (!IgNotif) {
                log("Ingame Messages Disabled");
            }
        }

        if (regularMode) {
            atkVal = *(float*)atkAddr; //read timer (cast address to float pointer (float*) and dereference *(float*))
            if (buffActive && atkVal != 0 && disableTimer == false) {
                regModeCounter++;
                if (regModeCounter != regModeTimer) {
                    continue;
                }
                else {
                    regModeCounter = 0;
                    buffActive = false;
                }
            }

            if (atkVal < 0 || atkVal == 0) { //when buff inactive
                buffActive = false;
                regModeCounter = 0;
                if (tempWrite == atkVal) {}
                else {
                    PatchA((void*)mainAddr, insert_attack);
                }
                tempWrite = atkVal; //multi-write safeguard
            }
            else if (atkVal > 89.8) { //when buff active
                if (tempDel == atkVal) {}
                else {
                    PatchA((void*)mainAddr, original_code);
                    if (!disableTimer) {
                        buffActive = true;
                    }
                }
                tempDel = atkVal;
            }
            else {}
            if (atkVal == 0) {
                tempWrite = 111;
            }
            continue;
        }

        if (speedrunMode) {
            atkVal = *(float*)atkAddr;
            if (atkVal == 0) {
                speedrunModeSwitch = false;
            }
            if (!speedrunModeSwitch) {
                if (atkVal == 0 || atkVal < 0) {
                    PatchA((void*)mainAddr, insert_attack);
                    continue;
                }
                else if (atkVal > 80) {
                    PatchA((void*)mainAddr, insert_affinity);
                    /*gameLog("Hehe boi");
                    if (playHEHE) {
                        PlaySound(L"C:\\Program Files\\Steam\\steamapps\\common\\Monster Hunter World\\nativePC\\plugins\\Hehe_boi.wav", NULL, SND_FILENAME);
                    }*/
                    speedrunModeSwitch = true;
                }
                else {
                    continue;
                }
            }

            if (speedrunModeSwitch) {
                affVal = *(float*)affAddr;

                if (affVal < 0 || affVal == 0) { //when buff inactive
                    PatchA((void*)mainAddr, insert_affinity);
                    continue;
                }
                else if (affVal > 115) { //when buff active
                    PatchA((void*)mainAddr, insert_attack);
                    speedrunModeSwitch = false;
                    continue;
                }
                else {
                    continue;
                }
            }
        }

        if (heroicsMode) {
            atkVal = *(float*)atkAddr;
            HeroicsModeCounter++;
            if (atkVal < 0 || atkVal == 0) { //when attack buff inactive
                if (tempWrite == atkVal) {}
                else {
                    PatchA((void*)mainAddr, insert_attack);
                }
                tempWrite = atkVal; //multi-write safeguard
            }
            else if (atkVal > 89.5) { //when buff active
                if (tempDel == atkVal) {}
                else {
                    PatchA((void*)mainAddr, original_code);
                }
                tempDel = atkVal;
            }
            else {
                if (HeroicsModeCounter >= 99) {
                    HeroicsModeCounter = 0;
                    randomBuff = randomNum(4);
                    blockDefHp(randomBuff, defSAddr, defLAddr, hpAddr, heroics_toAff, heroics_toRec, heroics_toAtkS, heroics_toSta);
                }
            }
            if (atkVal == 0) {
                tempWrite = 111;
            }
            continue;
        }
    }
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


//============================================================================================//
//============================================================================================//


void sstream(int select)
{
    std::stringstream ss;
    switch (select)
    {
    case 0: {
        if (catDefaultStr == "disabled") {
            setGaji = false;
            setBoa = false;
        }
        else if (catDefaultStr == "gajalaka") {
            setGaji = true;
            setBoa = false;
        }
        else if (catDefaultStr == "boaboa") {
            setBoa = true;
            setGaji = false;
        }
        break;
    }
    case 1: {
        ss << std::hex << regModeKeyStr;
        ss >> regModeKey;
        break;
    }
    case 99: {
        ss << std::hex << rndmModeKeyStr;
        ss >> rndmModeKey;
        break;
    }
    case 2: {
        ss << std::hex << hModeKeyStr;
        ss >> hModeKey;
        break;
    }
    case 98: {
        ss << std::hex << aAModeKeyStr;
        ss >> aAModeKey;
        break;
    }
    case 3: {
        ss << std::hex << cGKeyStr;
        ss >> cGKey;
        break;
    }
    case 4: {
        ss << std::hex << cBKeyStr;
        ss >> cBKey;
        break;
    }
    case 5: {
        ss << std::hex << dbKeyStr;
        ss >> dbKey;
        break;
    }
    case 6: {
        ss << std::hex << IgNotifKeyStr;
        ss >> IgNotifKey;
        break;
    }
    case 7: {
        ss << std::hex << speedrunKeyStr;
        ss >> speedrunKey;
    }
    default:
        break;
    }
    return;
}

void LoadConfig()
{
    std::ifstream config("nativePC\\plugins\\SpeedrunUtilityConfig.json");
    if (config.fail()) {
        log("Could not find/read config file");
        return;
    }

    log("Found config file");
    nlohmann::json cFile = nlohmann::json::object();
    config >> cFile;

    //retrieve values from config
    {
        regularMode = cFile.value<bool>("regularModeDefaultOn", false);
        //randomMode = cFile.value<bool>("randomModeDefaultOn", false);
        heroicsMode = cFile.value<bool>("heroicsModeDefaultOn", false);
        //alwaysAtkMode = cFile.value<bool>("alwaysAtkModeDefaultOn", false);
        drunkBird = cFile.value<bool>("drunkBirdDefaultOn", false);
        IgNotif = cFile.value<bool>("IngameMessageDefaultOn", false);
        speedrunMode = cFile.value<bool>("speedrunModeDefaultOn", false);
        catDefaultStr = cFile.value<std::string>("catHelpersDefault", "disabled");
        regModeKeyStr = cFile.value<std::string>("regularModeHotkey", "60");
        //rndmModeKeyStr = cFile.value<std::string>("randomModeHotkey", "61");
        hModeKeyStr = cFile.value<std::string>("heroicsModeHotkey", "62");
        //aAModeKeyStr = cFile.value<std::string>("alwaysAtkModeHotkey", "63");
        cGKeyStr = cFile.value<std::string>("catHelperGajiHotkey", "64");
        cBKeyStr = cFile.value<std::string>("catHelperBoaHotkey", "65");
        dbKeyStr = cFile.value<std::string>("drunkBirdHotkey", "66");
        IgNotifKeyStr = cFile.value<std::string>("IngameMessageHotkey", "67");
        speedrunKeyStr = cFile.value<std::string>("speedrunModeHotkey", "68");
        regModeTimer = cFile.value<uint>("regModeTimer", 45);
        //launchHelloWorld = cFile.value<bool>("autolaunchHelloWorld", true);
        debugOverride = cFile.value<uint>("debug_IGNORE", 0);
    }

    if (regModeTimer == 0) {
        disableTimer = true;
    }
    else if (regModeTimer != 0) {
        regModeTimer *= 90;
        regModeTimer += 8100;
    }

    log("Retrieved values from config");

    for (uint i = 0; i < 8; i++) {
        sstream(i);
    }
}

bool CheckSteamID(undefined8 steamid64)
{
    uintptr_t steam_api_base = (uintptr_t)GetModuleHandle(L"steam_api64.dll");
    uintptr_t steam_api = steam_api_base + 0x3A118;

    DWORD oldprotect;
    VirtualProtect((LPVOID)steam_api, 8, PAGE_EXECUTE_READWRITE, &oldprotect);
    undefined8 steamID = *(undefined8*)steam_api;
    VirtualProtect((LPVOID)steam_api, 8, oldprotect, &oldprotect);

    if (steamID != steamid64) {
        return false;
    }

    return true;
}

bool CheckSteamID2(undefined8 steamdid_1, undefined8 steamid_2)
{
    uintptr_t base = (uintptr_t)GetModuleHandle(L"steam_api64.dll");
    uintptr_t steam_api = base + 0x3a118;

    DWORD protect;
    VirtualProtect((LPVOID)steam_api, 8, PAGE_EXECUTE_READWRITE, &protect);
    undefined8 steamdID_read = *(undefined8*)steam_api;
    VirtualProtect((LPVOID)steam_api, 8, protect, &protect);

    if (steamdID_read == steamdid_1 || steamdID_read == steamid_2) {
        return true;
    }
    else {
        return false;
    }
}

uintptr_t GetAtkAddr(uintptr_t modBase, uintptr_t playerPtr, std::vector<unsigned int> offsets) //calculates attack address
{
    uintptr_t pointerAddr = modBase + playerPtr;
    for (unsigned int i = 0; i < offsets.size(); ++i) { //loop however many times specified in passed vector
        pointerAddr = *(uintptr_t*)pointerAddr; //cast to pointer and dereference to get value (address)
        //ESSENTIALLY: Read at address and store in pointerAddr
        pointerAddr += offsets[i]; //add current offset
    }
    return pointerAddr;
}

VOID startup(TCHAR* argv[])
{
    // additional information
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    // set the size of the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // start the program up
    CreateProcess(NULL,   // the path
        argv[1],        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
    );
    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}