#pragma once
#include <format>
#include <Windows.h>
#include <amethyst/runtime/ModContext.hpp>
#include <amethyst/runtime/events/GameEvents.hpp> // OnStartJoinGameEvent
#include <minecraft/src/common/world/item/ItemStack.hpp>
#include <minecraft/src/common/world/item/Item.hpp>

#define ModFunction extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

template <>
struct std::formatter<ItemStack> {
    constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const ItemStack& stack, FormatContext& ctx) const
    {
        if (stack.isNull()) {
            return std::format_to(ctx.out(), "ItemStack(Null)");
        }

        return std::format_to(ctx.out(), "ItemStack(ID: {}, Count: {})",
            stack.getItem()->mFullName.getString(),
            stack.mCount
        );
    }
};