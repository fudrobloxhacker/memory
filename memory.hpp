#ifndef INCLUDE_MEMORY_HPP
#define INCLUDE_MEMORY_HPP

#pragma once

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <TlHelp32.h>

#include <cstdint>
#include <string_view>
#include <functional>
#include <memory>

class Memory
{
private:
    std::uintptr_t procId = 0;
    HANDLE processHandle = nullptr;

    auto closeHandle = [](HANDLE handle) {
        if (handle) ::CloseHandle(handle);
    };

public:
    Memory(const std::string_view processName) noexcept
    {
        ::PROCESSENTRY32 entry = {};
        entry.dwSize = sizeof(::PROCESSENTRY32);

        const auto procSnap = std::unique_ptr<void, decltype(closeHandle)>(
            ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0), closeHandle);

        auto findProcessId = [&](auto&& name) -> std::uintptr_t {
            while (::Process32Next(procSnap.get(), &entry)) {
                if (!name.compare(entry.szExeFile)) {
                    return entry.th32ProcessID;
                }
            }
            return 0;
        };

        procId = findProcessId(processName);

        processHandle = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, procId);
    }

    ~Memory()
    {
        closeHandle(processHandle);
    }

    const std::uintptr_t GetModuleAddress(const std::string_view moduleName) const noexcept
    {
        ::MODULEENTRY32 entry = {};
        entry.dwSize = sizeof(::MODULEENTRY32);

        const auto procSnap = std::unique_ptr<void, decltype(closeHandle)>(
            ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId), closeHandle);

        auto findModuleBase = [&](auto&& name) -> std::uintptr_t {
            while (::Module32Next(procSnap.get(), &entry)) {
                if (!name.compare(entry.szModule)) {
                    return reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
                }
            }
            return 0;
        };

        return findModuleBase(moduleName);
    }

    template <typename T>
    constexpr const T Read(const std::uintptr_t& address) const noexcept
    {
        T value = {};
        ::ReadProcessMemory(processHandle, reinterpret_cast<const void*>(address), &value, sizeof(T), NULL);
        return value;
    }

    template <typename T>
    constexpr void Write(const std::uintptr_t& address, const T& value) const noexcept
    {
        ::WriteProcessMemory(processHandle, reinterpret_cast<void*>(address), &value, sizeof(T), NULL);
    }

    LPVOID AllocateMemory(SIZE_T size, DWORD allocationType = MEM_COMMIT | MEM_RESERVE, DWORD protect = PAGE_READWRITE) const noexcept
    {
        return ::VirtualAllocEx(processHandle, nullptr, size, allocationType, protect);
    }

    BOOL FreeMemory(LPVOID address, SIZE_T size, DWORD freeType = MEM_RELEASE) const noexcept
    {
        return ::VirtualFreeEx(processHandle, address, size, freeType);
    }
};

#endif // INCLUDE_MEMORY_HPP
