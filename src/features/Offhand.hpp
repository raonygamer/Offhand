#pragma once
#include <format>
#include <minecraft/src/common/world/item/ItemStack.hpp>

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

void RegisterOffhandHooks();