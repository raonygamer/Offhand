#include "dllmain.hpp"
#include <functional>
#include <amethyst/runtime/events/RegisterEvents.hpp>
#include <minecraft/src/common/world/item/registry/ItemRegistry.hpp>
#include <minecraft/src/common/world/actor/player/Player.hpp>
#include <minecraft/src/common/world/actor/player/PlayerInventory.hpp>
#include <minecraft/src/common/world/actor/player/Inventory.hpp>
#include <minecraft/src/common/world/level/BlockPos.hpp>
#include <minecraft/src/common/world/level/ILevel.hpp>
#include <minecraft/src/common/world/level/BlockSource.hpp>
#include <minecraft/src/common/world/entity/components/ActorEquipmentComponent.hpp>
#include <minecraft/src/common/world/entity/components/StateVectorComponent.hpp>
#include <minecraft/src/common/world/gamemode/GameMode.hpp>
#include <minecraft/src/common/world/item/ItemDescriptorCount.hpp>
#include <minecraft/src/common/world/inventory/transaction/ItemUseInventoryTransaction.hpp>
#include <minecraft/src/common/world/Facing.hpp>
#include <minecraft/src/common/world/gamemode/GameModeMessenger.hpp>
#include <minecraft/src/common/world/inventory/network/ItemStackNetManagerBase.hpp>
#include <minecraft/src/common/world/events/PlayerEventListener.hpp>
#include <minecraft/src/common/world/inventory/transaction/ComplexInventoryTransaction.hpp>
#include <minecraft/src/common/world/inventory/transaction/InventoryTransactionManager.hpp>
#include <minecraft/src/common/world/level/BlockSource.hpp>
#include <minecraft/src/common/world/level/Level.hpp>
#include <minecraft/src/common/world/level/BlockPalette.hpp>
#include <minecraft/src/common/world/events/BlockEventCoordinator.hpp>
#include <minecraft/src-client/common/client/renderer/game/ItemInHandRenderer.hpp>
#include <minecraft/src-deps/renderer/MatrixStack.hpp>
#include <amethyst/runtime/events/InputEvents.hpp>
#include "features/SwapOffhandKey.hpp"
#include "features/Offhand.hpp"

void OnRegisterItems(RegisterItemsEvent &event)
{
    Log::Info("OnRegisterItems");

    //for (auto &pair : event.itemRegistry.mNameToItemMap)
    //{
    //    // Log::Info("Item ID: {:s}", pair.first.getString());
    //    pair.second.get()->setAllowOffhand(true);
    //}

    for (auto& pair : event.itemRegistry.mIdToItemMap)
    {
        pair.second.get()->setAllowOffhand(true);
    }
    
}

void OnRegisterBlocks(RegisterBlocksEvent &event)
{
    //Log::Info("OnRegisterBlocks");
    //for (auto &pair : event.blockDefinitions.mNameToBlockMap)
    //{
    //    Log::Info("Block ID: {:s}", pair.first.getString());
    //}

	Log::Info("OnRegisterBlocks - Registering offhand blocks");
}

//SafetyHookInline _ItemInHandRenderer_renderMainhandItem;
//
//void ItemInHandRenderer_renderOffhandItem(ItemInHandRenderer* self, BaseActorRenderContext& ctx, Player& player, ItemContextFlags flags) {
//	//Log::Info("ItemInHandRenderer_rednerOffhandItem {}", (int)flags);
//    _ItemInHandRenderer_renderMainhandItem.call<void, ItemInHandRenderer*, BaseActorRenderContext&, Player&, ItemContextFlags>(self, ctx, player, flags);
//}

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(AmethystContext &ctx)
{
    Amethyst::InitializeAmethystMod(ctx);
    Amethyst::GetEventBus().AddListener<RegisterItemsEvent>(&OnRegisterItems);
	Amethyst::GetEventBus().AddListener<RegisterBlocksEvent>(&OnRegisterBlocks);

    //Amethyst::GetContext().mFeatures->enableInputSystem = true;
    Amethyst::EventBus& events = Amethyst::GetEventBus();
    Amethyst::HookManager& hooks = Amethyst::GetHookManager();

 //   hooks.RegisterFunction<&ItemInHandRenderer_renderOffhandItem>("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 44 88 4D");
	//hooks.CreateHook<&ItemInHandRenderer_renderOffhandItem>(_ItemInHandRenderer_renderMainhandItem, &ItemInHandRenderer_renderOffhandItem);

    RegisterSwapOffhandKey();
    RegisterOffhandHooks();
}