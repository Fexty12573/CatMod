#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>

#include "Memory.hpp"
#include "loader.h"
#include "json.hpp"
#include "findPattern.h"
#include "Timer.hpp"
#include "ghidra_export.h"
#include "util.h"
#include "minhook/MinHook.h"

using namespace loader;
#define log(x) loader::LOG(INFO) << "[FxTy] " << x;
#define debug(x) loader::LOG(DEBUG) << "[FxTy Debug] " << x;

using namespace memory;

typedef unsigned long ulong;
typedef std::vector<void*> aobResults;

void GameLog(std::string msg)
{
    MH::Chat::ShowGameMessage(*(undefined**)MH::Chat::MainPtr, (undefined*)&msg[0], -1, -1, 0);
}

bool regularMode = false, randomMode = false, heroicsMode = false, alwaysAtkMode = false, setGaji = false, setBoa = false, drunkBird = false, launchHelloWorld = true, IgNotif = false, speedrunMode = false, disableTimer = false;
std::string catDefaultStr, regModeKeyStr, rndmModeKeyStr, hModeKeyStr, aAModeKeyStr, cGKeyStr, cBKeyStr, dbKeyStr, IgNotifKeyStr, speedrunKeyStr;
uint catDefault, regModeKey, hModeKey, cGKey, cBKey, dbKey, IgNotifKey, speedrunKey;
ulong uRegModeCooldown = 0;

CreateHook(MH::EmSetter::CatHelperSetter, ChangeCatHelpers, void, longlong param_1)
{
    original(param_1);

    if (setBoa)
        *(int*)(param_1 + 0x73E0) = 8;
    else if (setGaji)
        *(int*)(param_1 + 0x73E0) = 6;

    return;
}

void sstream(int select);
void LoadConfig(void);
void ReloadConfig(void);
bool CheckSteamID2(undefined8 steamdid_1, undefined8 steamid_2);

DWORD WINAPI MainThread(HMODULE hModule)
{
    Sleep(1000);

    tmr::Timer t, tConfig, tHeroics;
    
    /*
    if (std::string(loader::GameVersion) != "420540") {
        log("Wrong Version of MHW. Exiting..");
        return false;
    }*/

    /*if (!CheckSteamID2(76561199016152780, 76561198449393073)) {
        log("Invalid ID");
        log("Exiting...");
        return 0;
    }*/

    log("Initializing...");

    LoadConfig();

    MH_Initialize();
    QueueHook(ChangeCatHelpers);
    MH_ApplyQueued();

    uintptr_t base = (uintptr_t)GetModuleHandle(L"MonsterHunterWorld.exe");
    uintptr_t playerPtr = 0x4FF6388; //0x50640A0
    uintptr_t* PlayerData = (uintptr_t*)(base + playerPtr);
    uintptr_t loadCheck = NULL;

    while (true)
    {
        loadCheck = *(uintptr_t*)PlayerData;
        if (loadCheck == NULL) {
            Sleep(200);
            continue;
        }
        loadCheck = *(uintptr_t*)(loadCheck + 0x80);
        if (loadCheck == NULL) {
            Sleep(200);
            continue;
        }
        break;
    }

    uintptr_t atkPtr = GetPtrAddr(base + playerPtr, { 0x80, 0x7D20, 0x124 });
    uintptr_t affPtr = atkPtr + 0xC;

    float atkVal;
    float affVal;
    bool buffActive = false;
    bool srModeSwitch = false;
    bool affAtkMode = false;

    // Instructions, Patches, Aobscans, Pointers are initialized here
    opcode insert_attack = { 0xB0, 0x06, 0x90 };
    opcode insert_attackS = { 0xB0, 0x00, 0x90 };
    opcode insert_affinity = { 0xB0, 0x02, 0x90 };
    opcode insert_stamina = { 0xB0, 0x05, 0x90 };
    opcode insert_defense = { 0xB0, 0x07, 0x90 };
    opcode insert_recovery = { 0xB0, 0x03, 0x90 };
    opcode original_code = { 0x41, 0x8B, 0xC7 };
    opcode heroics_toSta = { 0xBA, 0x41, 0x00, 0x00, 0x00 };
    opcode heroics_toAff = { 0xBA, 0x3E, 0x00, 0x00, 0x00 };
    opcode heroics_toRec = { 0xBA, 0x3F, 0x00, 0x00, 0x00 };
    opcode heroics_toAtkS = { 0xBA, 0x3A, 0x00, 0x00, 0x00 };
    opcode uninst_defS = { 0xBA, 0x3C, 0x00, 0x00, 0x00 };
    opcode uninst_defL = { 0xBA, 0x3D, 0x00, 0x00, 0x00 };
    opcode uninst_hp = { 0xBA, 0x40, 0x00, 0x00, 0x00 };
    opcode nop = { 0x90, 0x90, 0x90 };

    //Gajalakas & Boaboa
    opcode setCatGaji = { 0xB3, 0x06 };
    opcode setCatBoa = { 0xB3, 0x08 };
    opcode catDelJne = { 0x8B, 0xFE };
    opcode catResJne = { 0x75, 0x06 };
    opcode restoreCat = { 0x8B, 0xD3 };

    //Bugfix for special cases
    opcode catForceJE = { 0xB9, 0x08, 0x00, 0x00, 0x00 };
    opcode catResJE_1 = { 0x8B, 0x88, 0xF0, 0x24, 0x10, 0x00 };
    opcode catResJE_2 = { 0xC1, 0xE9, 0x0C };
    opcode catResJNE_fix = { 0x75, 0x02 };

    //Player spawn
    opcode cmpedi1 = { 0x83, 0xFF, 0x39 };
    opcode movecx = { 0xB9, 0x64, 0x00, 0x00, 0x00 };
    opcode movsil = { 0x40, 0xB6, 0x01 };
    opcode cmpedi2 = { 0xBf, 0x01, 0x00, 0x00, 0x00 };
    opcode plNop = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
    opcode cmpedi1Res = { 0x83, 0xFF, 0x01 };
    opcode movecxRes = { 0xB9, 0x14, 0x00, 0x00, 0x00 };
    opcode movsilRes = { 0x0F, 0x1F, 0x00 };
    opcode cmpedi2Res = { 0x41, 0x8B, 0xBC, 0xC6, 0x84, 0x56, 0x18 ,0x03 };

    searchState sSCoralMod = aobToSearch("49 03 C8 FF E1 BA 3C000000 8B C2"); //Updated 15.01
    searchState sSCatMod = aobToSearch("33 D2 F7 F5 8B FA ???????? 33 D2 8B CB ?????????? 3C 01"); //Updated 15.01
    searchState sSCatMod2 = aobToSearch("41 3A C5 8B EE BF 08000000 41 0F44 ED 8B CF 33 D2"); //Updated 15.01
    searchState sSdb = aobToSearch("0F1F 00 41 3B C8 77 15 FF C3 44 2B C1 8B C3"); //Updated 15.01

    aobResults aobResCoralMod = aobscan(sSCoralMod);
    aobResults aobResCatMod = aobscan(sSCatMod);
    aobResults aobResCatMod2 = aobscan(sSCatMod2);
    aobResults aobResDb = aobscan(sSdb);

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
    
    Sleep(500);
    log("Initialized");
    log("READY!");

    if (IgNotif) {
        GameLog("[FxTy] Ready!");
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
        Nop((void*)catMovEcx, 9);
        Patch((void*)catMovEcx, catForceJE);
        Nop((void*)catDelJNE_fix, 2);
        Patch((void*)catmovedi, catDelJne);
        Patch((void*)catSetAddr, setCatGaji);
    }
    else if (setBoa) {
        Nop((void*)catMovEcx, 9);
        Patch((void*)catMovEcx, catForceJE);
        Nop((void*)catDelJNE_fix, 2);
        Patch((void*)catmovedi, catDelJne);
        Patch((void*)catSetAddr, setCatBoa);
    }

    if (drunkBird) {
        Patch((void*)cmp1Addr, cmpedi1);
        Patch((void*)movecxAddr, movecx);
        Patch((void*)movsilAddr, movsil);
        Nop((void*)cmp2Addr, 8);
        Patch((void*)cmp2Addr, cmpedi2);
    }

    tConfig.Start();

    time_t cModTime;
    struct _stat64i32 result;
    _stat("nativePC\\plugins\\FextyModConfig.json", &result);
    cModTime = result.st_mtime;

    while (true)
    {
        Sleep(10);

        if (GetAsyncKeyState(regModeKey) & 1) {
            regularMode = !regularMode;
            if (regularMode) {
                Patch((void*)mainAddr, original_code);
                log("Regular Mode activated and all other modes deactivated");
                if (IgNotif) {
                    GameLog("[FxTy] Regular Mode On");
                }
                randomMode = false;
                if (heroicsMode) {
                    TriplePatch((void*)defSAddr, (void*)defLAddr, (void*)hpAddr, uninst_defS, uninst_defL, uninst_hp);
                    heroicsMode = false;
                }
                alwaysAtkMode = false;
            }
            else if (!regularMode) {
                log("Regular Mode deactivated");
                if (IgNotif) {
                    GameLog("[FxTy] Regular Mode Off");
                }
                buffActive = false;
                Patch((void*)mainAddr, original_code);
            }
        }

        if (GetAsyncKeyState(hModeKey) & 1) {
            heroicsMode = !heroicsMode;

            if (heroicsMode) {
                Patch((void*)mainAddr, original_code);
                log("Heroics Mode activated and all other modes deactivated");
                if (IgNotif) {
                    GameLog("[FxTy] Heroics Mode On");
                }
                regularMode = false;
                buffActive = false;
                randomMode = false;
                alwaysAtkMode = false;
                if (speedrunMode) {
                    speedrunMode = false;
                    srModeSwitch = false;
                }
            }
            else if (!heroicsMode) {
                log("Heroics Mode deactivated");
                if (IgNotif) {
                    GameLog("[FxTy] Heroics Mode Off");
                }
                Patch((void*)mainAddr, original_code);
                TriplePatch((void*)defSAddr, (void*)defLAddr, (void*)hpAddr, uninst_defS, uninst_defL, uninst_hp);
            }
        }

        if (GetAsyncKeyState(speedrunKey) & 1) {
            speedrunMode = !speedrunMode;
            if (speedrunMode) {
                Patch((void*)mainAddr, original_code);
                regularMode = false;
                srModeSwitch = false;
                if (heroicsMode) {
                    heroicsMode = false;
                    TriplePatch((void*)defSAddr, (void*)defLAddr, (void*)hpAddr, uninst_defS, uninst_defL, uninst_hp);
                }
                Patch((void*)hpAddr, heroics_toSta);
                log("Speedrun Mode activated and all other modes deactivated");
                if (IgNotif) {
                    GameLog("[FxTy] Speedrun Mode On");
                }
            }
            else {
                log("Speedrun Mode deactivated");
                if (IgNotif) {
                    GameLog("[FxTy] Speedrun Mode Off");
                }
                TriplePatch((void*)defSAddr, (void*)defLAddr, (void*)hpAddr, uninst_defS, uninst_defL, uninst_hp);
                Patch((void*)mainAddr, original_code);
            }
        }

        /*
        if (GetAsyncKeyState(cGKey) & 1) {
            setGaji = !setGaji;
            if (setGaji) {
                setBoa = false;
                Nop((void*)catMovEcx, 9);
                Patch((void*)catMovEcx, catForceJE);
                Nop((void*)catDelJNE_fix, 2);
                Patch((void*)catmovedi, catDelJne);
                Patch((void*)catSetAddr, setCatGaji);
                log("Cat Helpers: Always Gajalakas");
                if (IgNotif) {
                    GameLog("[FxTy] Forced Gajalakas");
                }
            }
            else if (!setGaji) {
                Nop((void*)catMovEcx, 9);
                Patch((void*)catMovEcx, catResJE_1);
                Patch((void*)catNopShr, catResJE_2);
                Patch((void*)catDelJNE_fix, catResJNE_fix);
                Patch((void*)catmovedi, catResJne);
                Patch((void*)catSetAddr, restoreCat);
                log("Cat Helpers: Normal");
                if (IgNotif) {
                    GameLog("[FxTy] Disabled Gajalakas");
                }
            }
        }

        if (GetAsyncKeyState(cBKey) & 1) {
            setBoa = !setBoa;
            if (setBoa) {
                setGaji = false;
                Nop((void*)catMovEcx, 9);
                Patch((void*)catMovEcx, catForceJE);
                Nop((void*)catDelJNE_fix, 2);
                Patch((void*)catmovedi, catDelJne);
                Patch((void*)catSetAddr, setCatBoa);
                log("Cat Helpers: Always Boaboas");
                if (IgNotif) {
                    GameLog("[FxTy] Forced Boaboas");
                }
            }
            else if (!setBoa) {
                Nop((void*)catMovEcx, 9);
                Patch((void*)catMovEcx, catResJE_1);
                Patch((void*)catNopShr, catResJE_2);
                Patch((void*)catDelJNE_fix, catResJNE_fix);
                Patch((void*)catmovedi, catResJne);
                Patch((void*)catSetAddr, restoreCat);
                log("Cat Helpers: Normal");
                if (IgNotif) {
                    GameLog("[FxTy] Disabled Boaboas");
                }
            }
        }
        */

        if (GetAsyncKeyState(cGKey) & 1) {
            setBoa = false;
            setGaji = !setGaji;

            if (setGaji) {
                log("Cat Helpers: Always Gajalakas");
                if (IgNotif)
                    GameLog("[FxTy] Forced Gajalakas");
            }
        }

        if (GetAsyncKeyState(cBKey) & 1) {
            setGaji = false;
            setBoa = !setBoa;

            if (setBoa) {
                log("Cat Helpers: Always Boaboa");
                if (IgNotif)
                    GameLog("[FxTy] Forced Boaoba");
            }
        }

        if (GetAsyncKeyState(dbKey) & 1) {
            drunkBird = !drunkBird;
            if (drunkBird) {
                Patch((void*)cmp1Addr, cmpedi1);
                Patch((void*)movecxAddr, movecx);
                Patch((void*)movsilAddr, movsil);
                Nop((void*)cmp2Addr, 8);
                Patch((void*)cmp2Addr, cmpedi2);
                log("Drunk Bird enabled");
                if (IgNotif) {
                    GameLog("[FxTy] Drunk Bird Enabled");
                }
            }
            else if (!drunkBird) {
                Patch((void*)cmp1Addr, cmpedi1Res);
                Patch((void*)movecxAddr, movecxRes);
                Patch((void*)movsilAddr, movsilRes);
                Nop((void*)cmp2Addr, 8);
                Patch((void*)cmp2Addr, cmpedi2Res);
                log("Drunk Bird disabled");
                if (IgNotif) {
                    GameLog("[FxTy] Drunk Bird Disabled");
                }
            }
        }

        if (GetAsyncKeyState(IgNotifKey) & 1) {
            IgNotif = !IgNotif;
            if (IgNotif) {
                log("Ingame Messages Enabled");
                GameLog("[FxTy] Ingame Messages Enabled");
            }
            if (!IgNotif) {
                log("Ingame Messages Disabled");
                GameLog("[FxTy] Ingame Messages Disabled");
            }
        }

        if (GetAsyncKeyState(VK_INSERT) & 1) {
            affAtkMode = !affAtkMode;

            if (affAtkMode) {
                Patch((void*)mainAddr, original_code);
                buffActive = false;
                regularMode = false;
                speedrunMode = false;
                heroicsMode = false;
                TriplePatch((void*)defSAddr, (void*)defLAddr, (void*)hpAddr, uninst_defS, uninst_defL, uninst_hp);
                log("Attack/Affinity mode enabled");
            }
            else {
                Patch((void*)mainAddr, original_code);
                log("Attack/Affinity mode disabled");
            }
        }

        if (regularMode) {

            atkVal = *(float*)atkPtr;

            if (uRegModeCooldown > 0)
            {
                if (atkVal == 0) {
                    Patch((void*)mainAddr, insert_attack);
                }

                if ((atkVal < 0 || atkVal == 0)) {
                    if (t.TimePassed(uRegModeCooldown))
                        Patch((void*)mainAddr, insert_attack);
                }

                if (atkVal >= 89.7) {
                    Patch((void*)mainAddr, original_code);
                    t.Start();
                }
            }
            else
            {
                if (atkVal == 0) {
                    Patch((void*)mainAddr, insert_attack);
                }

                if ((atkVal < 0 || atkVal == 0)) {
                    Patch((void*)mainAddr, insert_attack);
                }

                if (atkVal >= 89.7) {
                    Patch((void*)mainAddr, original_code);
                }
            }
        }

        if (heroicsMode) {
            atkVal = *(float*)atkPtr;

            if (atkVal == 0) {
                Patch((void*)mainAddr, insert_attack);
            }

            if (atkVal <= 0) {
                Patch((void*)mainAddr, insert_attack);
            }
            else if (atkVal >= 89.7) {
                Patch((void*)mainAddr, original_code);
                tHeroics.Start();
            }
            else {
                if (tHeroics.TimePassed(5)) {
                    switch (RandomNum(4))
                    {
                    case 1: {
                        Patch((void*)mainAddr, insert_attackS);
                        break;
                    }
                    case 2: {
                        Patch((void*)mainAddr, insert_stamina);
                        break;
                    }
                    case 3: {
                        Patch((void*)mainAddr, insert_recovery);
                        break;
                    }
                    case 4: {
                        Patch((void*)mainAddr, insert_affinity);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }

        if (speedrunMode) {

            atkVal = *(float*)atkPtr; // Read attack buff timer from game, atkVal holds that time afterwards

            if (atkVal == 0) {          // check if player is in seliana/gathering hub etc
                srModeSwitch = false;   // Toggle the switch
            }

            if (!srModeSwitch) {        // Check if the switch is toggled off
                if (atkVal <= 0) {      // If it is, check if attack buff has run out/is not active
                    Patch((void*)mainAddr, insert_attack);      // Patch instruction to guarantee attack
                }
                else if (atkVal >= 89.7) {  // Otherwise, if attack timer is > 89.7
                    Patch((void*)mainAddr, original_code);  // Patch Back the original code
                    GameLog("[FxTy] Using Speedrun Mode");  // Display ingame message
                    srModeSwitch = true;                    // Toggle switch on, to prevent any further attack buffs
                }
            }
        }

        if (affAtkMode) {
            atkVal = *(float*)atkPtr;
            affVal = *(float*)affPtr;

            if (atkVal == 0) {
                Patch((void*)mainAddr, insert_attack);
            }

            if (atkVal <= 0 && affVal > 0) {
                Patch((void*)mainAddr, insert_attack);
            }
            else if (affVal <= 0 && atkVal > 0) {
                Patch((void*)mainAddr, insert_affinity);
            }
            else {
                switch (RandomNum(2))
                {
                case 1:
                    Patch((void*)mainAddr, insert_attack);
                    break;
                case 2: 
                    Patch((void*)mainAddr, insert_affinity);
                    break;
                default:
                    break;
                }
            }
        }

        if (tConfig.TimePassed(5)) {
            tConfig.Start();
            if (_stat("nativePC\\plugins\\FextyModConfig.json", &result) == 0) {
                if (result.st_mtime != cModTime) {
                    cModTime = result.st_mtime;
                    ReloadConfig();
                    log("Updated Config Values");
                }
            }
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

void LoadConfig(void)
{
    std::ifstream config("nativePC\\plugins\\FextyModConfig.json");
    if (config.fail()) {
        log("Could not find/read config file");
        return;
    }

    log("Found config file");
    nlohmann::json cFile = nlohmann::json::object();
    config >> cFile;

    //retrieve values from config
    {
        regularMode = cFile.value<bool>("RegularModeDefaultOn", false);
        heroicsMode = cFile.value<bool>("HeroicsModeDefaultOn", false);
        drunkBird = cFile.value<bool>("DrunkBirdDefaultOn", false);
        IgNotif = cFile.value<bool>("IngameMessageDefaultOn", false);
        speedrunMode = cFile.value<bool>("SpeedrunModeDefaultOn", false);
        catDefaultStr = cFile.value<std::string>("CatHelpersDefault", "disabled");
        regModeKeyStr = cFile.value<std::string>("RegularModeHotkey", "60");
        hModeKeyStr = cFile.value<std::string>("HeroicsModeHotkey", "62");
        cGKeyStr = cFile.value<std::string>("CatHelperGajiHotkey", "64");
        cBKeyStr = cFile.value<std::string>("CatHelperBoaHotkey", "65");
        dbKeyStr = cFile.value<std::string>("DrunkBirdHotkey", "66");
        IgNotifKeyStr = cFile.value<std::string>("IngameMessageHotkey", "67");
        speedrunKeyStr = cFile.value<std::string>("SpeedrunModeHotkey", "68");
        uRegModeCooldown = cFile.value<ulong>("RegularModeCooldown", 0);
        debugOverride = cFile.value<uint>("Debug", 0);
    }
    uRegModeCooldown += 90;

    log("Retrieved values from config");

    for (uint i = 0; i < 8; i++) {
        sstream(i);
    }
}

void ReloadConfig(void)
{
    std::ifstream config("nativePC\\plugins\\FextyModConfig.json");
    if (config.fail())
        return;

    nlohmann::json cFile = nlohmann::json::object();
    config >> cFile;

    //retrieve values from config
    regModeKeyStr = cFile.value<std::string>("RegularModeHotkey", "60");
    hModeKeyStr = cFile.value<std::string>("HeroicsModeHotkey", "62");
    cGKeyStr = cFile.value<std::string>("CatHelperGajiHotkey", "64");
    cBKeyStr = cFile.value<std::string>("CatHelperBoaHotkey", "65");
    dbKeyStr = cFile.value<std::string>("DrunkBirdHotkey", "66");
    IgNotifKeyStr = cFile.value<std::string>("IngameMessageHotkey", "67");
    speedrunKeyStr = cFile.value<std::string>("SpeedrunModeHotkey", "68");
    uRegModeCooldown = cFile.value<ulong>("RegularModeCooldown", 0);
    debugOverride = cFile.value<uint>("Debug", 0);

    uRegModeCooldown += 90;

    for (uint i = 1; i < 8; i++) {
        sstream(i);
    }
}

bool CheckSteamID2(undefined8 steamdid_1, undefined8 steamid_2)
{
    uintptr_t base = (uintptr_t)GetModuleHandle(L"steam_api64.dll");
    uintptr_t steam_api = base + 0x3a118;

    DWORD protect;
    VirtualProtect((LPVOID)steam_api, 8, PAGE_EXECUTE_READWRITE, &protect);
    undefined8 steamdID_read = *(undefined8*)steam_api;
    VirtualProtect((LPVOID)steam_api, 8, protect, &protect);

    if (steamdID_read == steamdid_1 || steamdID_read == steamid_2)
        return true;
    else
        return false;
}
