#pragma once
#include <Windows.h>
#include <vector>
#include <TlHelp32.h>
#include <iostream>
#include <stdexcept>
#include <Psapi.h>
#include <regex>

#include "loader.h"

using namespace loader;

typedef unsigned char byte;

class searchState {
private:
    int* live_references;
    int refsize;
    std::vector<byte> search_string;
    std::vector<byte> mask;
public:
    int limit;
    searchState(std::vector<byte> string, std::vector<byte> mask = std::vector<byte>()) {
        this->live_references = new int[string.size()];
        this->refsize = 0;
        this->search_string = string;
        this->limit = string.size();
        if (mask.size() == 0) {
            this->mask = std::vector<byte>(this->limit, 0xFF);
        }
        else { this->mask = mask; }
        if (mask.size() != string.size())throw std::invalid_argument("Mask and Search String have different sizes.");
    };
    void checkInput(int index, byte input, int& i, bool& found) {
        if ((this->mask[index] & input) == (this->mask[index] & this->search_string[index])) {
            if (index + 1 == this->limit) {
                found = true;
            }
            else {
                (this->live_references)[i] = index + 1;
            }
            i += 1;
        }
    };
    bool nextInput(byte input) {
        int i = 0;
        bool found = false;
        for (int j = 0; j < this->refsize; j++) {
            this->checkInput(this->live_references[j], input, i, found);
        }
        checkInput(0, input, i, found);
        this->refsize = i;
        return found;
    };
};

template< class ForwardIt1 >
ForwardIt1 search(ForwardIt1 begin, ForwardIt1 end, searchState searcher) {
    while (begin < end) {
        if (searcher.nextInput(*begin)) return (begin + 1 - searcher.limit);
        ++begin;
    };
    return begin;
};


std::vector<void*> aobscan(searchState scanner, const bool all_matches = false, byte* start = NULL, byte* end = (byte*)MAXINT64) {
    std::vector<void*> results;
    auto module = GetModuleHandle(L"MonsterHunterWorld.exe");
    if (module == nullptr) {
        LOG(INFO) << "Failed to Acquire Module";
        return results;
    };
    MODULEINFO moduleInfo;
    if (!GetModuleInformation(GetCurrentProcess(), module, &moduleInfo, sizeof(moduleInfo))) {
        LOG(INFO) << "Module retrieval error.";
        return results;
    };
    byte* startAddr = max((byte*)module, start);
    byte* endAddr = min(startAddr + moduleInfo.SizeOfImage, end);
    byte* addr = startAddr;

    while (addr < endAddr)
    {
        MEMORY_BASIC_INFORMATION memInfo;
        if (!VirtualQuery(addr, &memInfo, sizeof(memInfo)) || memInfo.State != MEM_COMMIT || (memInfo.Protect & PAGE_GUARD))
            continue;
        byte* begin = (byte*)memInfo.BaseAddress;
        byte* end = begin + memInfo.RegionSize;


        byte* found = begin - 1;
        while ((found = search(found + 1, end, scanner)) != end) {
            results.push_back(found);
            if (!all_matches) {
                return results;
            };
        };
        addr = end;
        memInfo = {};
    }
    return results;
};

//Utility functions

std::vector<std::string> stringSplit(const std::string subject, const int ix) {
    std::vector<std::string> result;
    for (unsigned i = 0; i < subject.length(); i += ix) {
        result.push_back(subject.substr(i, ix));
    }
    return result;
};

std::vector<byte> aobToCode(const std::string CEAob) {
    std::regex r("\\s+");
    std::string s = std::regex_replace(CEAob, r, "");
    std::vector <std::string> textAoB = stringSplit(s, 2);
    std::vector <byte> data;
    for (auto& ceByte : textAoB) data.push_back(strtol(ceByte.c_str(), NULL, 16));
    return data;
}

searchState aobToSearch(const std::string CEAob) {
    std::regex r("\\s+");
    std::string s = std::regex_replace(CEAob, r, "");
    std::vector <std::string> textAoB = stringSplit(s, 2);
    std::vector <byte> data;
    std::vector <byte> mask;
    for (auto& ceByte : textAoB)
    {
        if (ceByte == "??") {
            data.push_back(0);
            mask.push_back(0);
        }
        else {
            data.push_back(strtol(ceByte.c_str(), NULL, 16));
            mask.push_back(0xFF);
        }
    }
    return searchState(data, mask);
}