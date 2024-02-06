#include "pch.h"

#include <iostream>
#include <MinHook.h>
#include "util.h"
#include "loader.h"
#include "Timer.hpp"
#include "json.hpp"
#include "ghidra_export.h"
#include "game_functions.h"

#include "buff_handler.h"

#include <Windows.h>
#include <string>

using namespace loader;
#define log(x)          LOG(INFO) << "[FextyMod] " << x
#define BASE_ADDRESS    0x140000000
#define PL_DATA_OFFSET  0x500ab60

#define GAJALAKA        6
#define BOABOA          8
#define HEX             16

#ifndef GET_BYTE(x)
#define GET_BYTE(x)     (*(BYTE*)(x))
#define GET_WORD(x)     (*(WORD*)(x))
#define GET_DWORD(x)    (*(DWORD*)(x))
#define GET_QWORD(x)    (*(QWORD*)(x))
#define GET_FLOAT(x)    (*(float*)(x))
#endif

#define INITIAL_LOAD    true
#define RELOAD          false

struct Mode {
    bool RegMode;
    bool HeroicsMode;
    bool AffinityMode;
    bool SetGaji;
    bool SetBoa;
    bool DrunkBird;
    bool GameMsg;
    bool SRMode;
    bool NoTimer;
};

struct StringKey {
    std::string CatDefault;
    std::string RegModeKey;
    std::string HeroicsModeKey;
    std::string GajiKey;
    std::string BoaKey;
    std::string DrunkBirdKey;
    std::string GameMsgKey;
    std::string SRModeKey;
};

struct Key {
    uint RegMode;
    uint HeroicsMode;
    uint Gaji;
    uint Boa;
    uint DrunkBird;
    uint GameMsg;
    uint SRMode;
};

struct Cooldown {
    tmr::ulong RegularMode ;
    tmr::ulong HeroicsMode ;
    tmr::ulong AffinityMode;
};

Mode gModes = { };
StringKey gStringKeys;
Key gKeys = { };
Cooldown gCooldowns = { };
float* atkPtr;
tmr::Timer tCooldown;

BuffHandler buffhandler;

void LoadConfig(bool bInitialLoad);
void ReloadConfig(void);
void GameLog(const std::string& msg);
bool ConfigModified();
undefined8 RandomHeroics();
inline bool MultipleModes();

namespace memory {
    uint RandomNum(uint num = 10);
    uintptr_t GetPtrAddr(uintptr_t, std::vector<uint>);
}

CreateHook(MH::Palico::PalicoBuff, ChangeBuff, undefined8, ulonglong param_1)
{
    undefined8 ret = original(param_1);
    float atkVal = *atkPtr;

    if (gModes.RegMode)
    {
        if (tCooldown.TimePassed(gCooldowns.RegularMode))
        {
            buffhandler.NextBuff<BuffHandler::Mode::REGULAR>(atkVal, ret);
        }
    }
    else if (gModes.HeroicsMode)
    {
        if (tCooldown.TimePassed(gCooldowns.HeroicsMode))
        {
            buffhandler.NextBuff<BuffHandler::Mode::HEROICS>(atkVal, ret);
        }
    }
    else if (gModes.AffinityMode)
    {
        if (tCooldown.TimePassed(gCooldowns.AffinityMode))
        {
            buffhandler.NextBuff<BuffHandler::Mode::AFFINITY>(atkVal, ret);
        }
    }
    else if (gModes.SRMode)
    {
        buffhandler.NextBuff<BuffHandler::Mode::SPEEDRUN>(atkVal, ret);
        GameLog("[FextyMod] Using Speedrun Mode");
    }

    return ret;
}

CreateHook(MH::EmSetter::CatHelperSetter, SetCatHelper, void, longlong param_1)
{
    original(param_1);

    if (gModes.SetGaji)     *(int*)(param_1 + 0x73E0) = GAJALAKA;
    else if (gModes.SetBoa) *(int*)(param_1 + 0x73E0) = BOABOA;

    return;
}

CreateHook(MH::Player::DrunkBird, SetDrunkBird, ulonglong, undefined8 stageId, undefined8 flag)
{
    ulonglong ret = original(stageId, flag);

    if (gModes.DrunkBird)
    {
        ret = 1;
        GET_QWORD(GET_QWORD(0x14500caf0) + (QWORD)0x291d0) = ret;
    }

    return ret;
}

void WINAPI Load(HMODULE hModule)
{
    MH_Initialize();

    QueueHook(ChangeBuff);
    QueueHook(SetCatHelper);
    QueueHook(SetDrunkBird);

    MH_ApplyQueued();

    LoadConfig(INITIAL_LOAD);

    QWORD PlayerData = (QWORD)(BASE_ADDRESS + PL_DATA_OFFSET); // 0x4FFF758
    QWORD MidPlayerData = NULL;

    // this is godawful but whatever
    while ((MidPlayerData = GET_QWORD(PlayerData)) == 0) Sleep(20); /* Wait until player data is initialized */
    while (GET_QWORD(MidPlayerData + 0x80) == 0) Sleep(20);

    atkPtr = (float*)memory::GetPtrAddr((uintptr_t)(BASE_ADDRESS + PL_DATA_OFFSET), { 0x80, 0x7D20, 0x124 }); /* Pointer to player attack buff timer */

    if (gModes.GameMsg)
        GameLog("[FextyMod] Initialized");

    log("Initialized");

    if (MultipleModes()) /* Check if multiple modes are enabled simultaneously, which they shouldn't be */
    {
        gModes.RegMode = false;
        gModes.HeroicsMode = false;
        gModes.SRMode = false;
        loader::LOG(loader::ERR) << "[FextyMod] Can't have multiple Modes enabled. All Modes disabled.";
    }

    if (gModes.SetGaji && gModes.SetBoa)
        gModes.SetBoa = false;

    tmr::Timer tConfig;

    tConfig.Start();
    tCooldown.Start();
    
    while (true)
    {
        Sleep(10);

        /* Toggles for all of the modules */
        if (GetAsyncKeyState(gKeys.RegMode) & 1) {
            gModes.RegMode = !gModes.RegMode;

            gModes.HeroicsMode = false;
            gModes.SRMode = false;

            if (gModes.RegMode) {
                log("Regular Mode Enabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Regular Mode Enabled");
                if (gCooldowns.RegularMode != 0)
                    tCooldown.Start();
            }
            else {
                log("Regular Mode Disabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Regular Mode Disabled");
            }
        }

        if (GetAsyncKeyState(gKeys.HeroicsMode) & 1) {
            gModes.HeroicsMode = !gModes.HeroicsMode;

            gModes.RegMode = false;
            gModes.SRMode = false;

            if (gModes.HeroicsMode) {
                log("Heroics Mode Enabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Heroics Mode Enabled");
                if (gCooldowns.HeroicsMode != 0)
                    tCooldown.Start();
            }
            else {
                log("Heroics Mode Disabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Heroics Mode Disabled");
            }
        }

        //if (GetAsyncKeyState(VK_F1) & 1) /* Affinity Mode */
        //{
        //    gModes.AffinityMode = !gModes.AffinityMode;

        //    if (gModes.AffinityMode)
        //    {
        //        log("Affinity Mode Enabled");
        //        if (gModes.GameMsg) GameLog("[FextyMod] Affinity Mode Enabled");
        //    }
        //    else
        //    {
        //        log("Affinity Mode Disabled");
        //        if (gModes.GameMsg) GameLog("[FextyMod] Affinity Mode Disabled");
        //    }
        //}

        if (GetAsyncKeyState(gKeys.SRMode) & 1) {
            gModes.SRMode = !gModes.SRMode;

            gModes.RegMode = false;
            gModes.HeroicsMode = false;
            gModes.AffinityMode = false;

            if (gModes.SRMode) {
                log("Speedrun Mode Enabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Speedrun Mode Enabled");
            }
            else {
                log("Speedrun Mode Disabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Speedrun Mode Disabled");
            }
        }

        if (GetAsyncKeyState(gKeys.DrunkBird) & 1) {
            gModes.DrunkBird = !gModes.DrunkBird;

            if (gModes.DrunkBird) {
                log("Drunk Bird Forcer Enabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Drunk Bird Forcer Enabled");
            }
            else {
                log("Drunk Bird Forcer Disabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Drunk Bird Forcer Disabled");
            }
        }

        if (GetAsyncKeyState(gKeys.Gaji) & 1) {
            gModes.SetGaji = !gModes.SetGaji;

            if (gModes.SetGaji) {
                gModes.SetBoa = false;
                log("Gajalaka Forcer Enabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Gajalaka Forcer Enabled");
            }
            else {
                log("Gajalaka Forcer Disabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Gajalaka Forcer Disabled");
            }
        }

        if (GetAsyncKeyState(gKeys.Boa) & 1) {
            gModes.SetBoa = !gModes.SetBoa;

            if (gModes.SetBoa) {
                gModes.SetGaji = false;
                log("Boaboa Forcer Enabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Boaboa Forcer Enabled");
            }
            else {
                log("Boaboa Forcer Disabled");
                if (gModes.GameMsg)
                    GameLog("[FextyMod] Boaboa Forcer Disabled");
            }
        }

        if (GetAsyncKeyState(gKeys.GameMsg) & 1) {
            gModes.GameMsg = !gModes.GameMsg;

            if (gModes.GameMsg) {
                log("Game Messages Enabled");
                GameLog("[FextyMod] Game Messages Enabled");
            }
            else {
                log("Game Messages Disabled");
                GameLog("[FextyMod] Game Messages Disabled");
            }
        }

        if (tConfig.TimePassed(5)) { /* Reload config every few seconds */
            tConfig.Start();
            if (ConfigModified())
                LoadConfig(RELOAD);
        }
    }

    FreeLibraryAndExitThread(hModule, 0);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)Load, hModule, 0, nullptr);

    return TRUE;
}

inline bool MultipleModes()
{
    if (gModes.RegMode && gModes.HeroicsMode || gModes.RegMode && gModes.SRMode || gModes.HeroicsMode && gModes.SRMode)
        return true;

    return false;
}

void LoadConfig(bool bInitialLoad)
{
    std::ifstream config("nativePC\\plugins\\FextyModConfig.json");
    if (config.fail())
        return;

    nlohmann::json cFile = nlohmann::json::object();
    config >> cFile;

    auto defaults = cFile["Defaults"];
    auto hotkeys = cFile["Hotkeys"];
    auto cooldowns = cFile["Cooldowns"];

    //retrieve values from config
    if (bInitialLoad) {
        gModes.RegMode = defaults.value<bool>("RegularModeDefaultOn", false);
        gModes.HeroicsMode = defaults.value<bool>("HeroicsModeDefaultOn", false);
        gModes.DrunkBird = defaults.value<bool>("DrunkBirdDefaultOn", false);
        gModes.GameMsg = defaults.value<bool>("IngameMessageDefaultOn", false);
        gModes.SRMode = defaults.value<bool>("SpeedrunModeDefaultOn", false);

        if (defaults.value<std::string>("CatHelpersDefault", "disabled") == "gajalaka")
            gModes.SetGaji = true;
        else if (defaults.value<std::string>("CatHelpersDefault", "disabled") == "boaboa")
            gModes.SetBoa = true;
    }
    
    gCooldowns.RegularMode = cooldowns.value<tmr::ulong>("RegularModeCooldown", 0);
    gCooldowns.HeroicsMode = cooldowns.value<tmr::ulong>("HeroicsModeCooldown", 0);

    gKeys.RegMode = std::stoi(hotkeys.value<std::string>("RegularModeHotkey", "60"), nullptr, HEX);
    gKeys.HeroicsMode = std::stoi(hotkeys.value<std::string>("HeroicsModeHotkey", "61"), nullptr, HEX);
    gKeys.Gaji = std::stoi(hotkeys.value<std::string>("CatHelperGajiHotkey", "64"), nullptr, HEX);
    gKeys.Boa = std::stoi(hotkeys.value<std::string>("CatHelperBoaHotkey", "65"), nullptr, HEX);
    gKeys.DrunkBird = std::stoi(hotkeys.value<std::string>("DrunkBirdHotkey", "66"), nullptr, HEX);
    gKeys.GameMsg = std::stoi(hotkeys.value<std::string>("IngameMessageHotkey", "67"), nullptr, HEX);
    gKeys.SRMode = std::stoi(hotkeys.value<std::string>("SpeedrunModeHotkey", "68"), nullptr, HEX);

    gCooldowns.RegularMode += 90;
    gCooldowns.HeroicsMode += 90;

    log("Successfully Read Config");
}

uint memory::RandomNum(uint num)
{
    srand((unsigned)time(0));
    int number;
    for (uint i = 0; i < 10; i++) {
        number = rand() % num + 1;
    }
    return number;
}

uintptr_t memory::GetPtrAddr(uintptr_t basePtr, std::vector<uint> offsets)
{
    uintptr_t addr = basePtr;
    for (uint i = 0; i < offsets.size(); ++i) {
        addr = *(uintptr_t*)addr;
        addr += offsets[i];
    }
    return addr;
}

void GameLog(const std::string& msg)
{
    MH::sMhGUI::DisplayPopup(MH::sMhGUI::GetInstance(), msg.c_str(), 3.0f, 0.0f, false, 0.0f, 0.0f);
}

bool ConfigModified()
{
    bool check = false;
    static time_t cModTime;
    struct _stat64i32 res;

    if (_stat("nativePC\\plugins\\FextyModConfig.json", &res) == 0) {
        if (res.st_mtime != cModTime) {
            cModTime = res.st_mtime;
            return true;
        }
    }

    return false;
}

undefined8 RandomHeroics()
{
    switch (memory::RandomNum(4))
    {
    case 1: return 0x3A;     // Atk S
    case 2: return 0x3E;     // Aff
    case 3: return 0x3F;     // RecSpd
    case 4: return 0x41;     // Sta
    default: return 0x3E;    // Aff
    }
}
