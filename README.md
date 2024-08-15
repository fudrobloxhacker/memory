# memory
Memory serves as a tool for accessing and manipulating game memory, allowing for tasks such as process identification, module addressing, and reading and writing memory.

To use these functions, consider this example involving the game Assault Cube and its module "ac_client.exe":

```c++
// Offsets
constexpr auto localPlayer = 0x17E0A8;
constexpr auto ptrHealth = 0xEC;

// Setup the foundation for your memory operations
Memory memory{ "ac_client.exe" };

// Define the Base Address
const auto moduleBase = memory.GetModuleAddress("ac_client.exe");

// Example of how to read and write the health address using local player and an offset to the health value
const auto localPlayerPtr = memory.Read<std::uintptr_t>(moduleBase + localPlayer);
const auto healthAddress = localPlayerPtr + ptrHealth;

// Example of a god mode hack
const int newHealth = 9999;
memory.Write<int>(healthAddress, newHealth);
```
