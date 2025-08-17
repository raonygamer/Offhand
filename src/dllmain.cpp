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

extern "C" void* ItemUseInventoryTransaction_ctor = nullptr;
extern "C" void* NetworkItemStackDescriptor_ctor = nullptr;

void OnRegisterItems(RegisterItemsEvent& event)
{
    Log::Info("OnRegisterItems");

    for (auto& pair : event.itemRegistry.mNameToItemMap) {
		//Log::Info("Item ID: {:s}", pair.first.getString());
        pair.second.get()->setAllowOffhand(true);
    }
}

class PlayerEventCoordinator {
public:
    void todoMoveToProperClass_processEvent(std::function<EventResult(class PlayerEventListener&)>& ev) {
		using function = decltype(&PlayerEventCoordinator::todoMoveToProperClass_processEvent);
        static auto func = std::bit_cast<function>(SigScan("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 4C 8B EA 48 8B F1"));
		(this->*func)(ev);
    }
};

SafetyHookInline _GameMode_buildBlock;

//bool GameMode_buildBlock(GameMode* self, BlockPos* pos, FacingID face, bool isSimTick) {
//    Player& player = self->mPlayer;
//	ILevel* level = player.mLevel;
//    PlayerInventory* playerInv = player.playerInventory;
//	Inventory* inv = playerInv->mInventory.get();
//
//    const ItemStack& selectedItem = inv->getItem(playerInv->mSelected);
//	const Item* item = selectedItem.getItem();
//
//    // The game seems to check things multiple times, i.e. selectedItem.isNull() covers some of the other cases in here
//    // Going for accuracy so do anyway ig
//    bool mayUseItem = !selectedItem.mValid
//        || !item
//        || selectedItem.isNull()
//        || selectedItem.mCount == 0
//        || !isSimTick
//        || item->canUseOnSimTick();
//
//    if (!mayUseItem) {
//        Log::Info("May not use item!!");
//        return false;
//    }
//
//    self->mPlayerLastPosition = *player.getPosition();
//    std::unique_ptr<ItemUseInventoryTransaction> transaction = std::make_unique<ItemUseInventoryTransaction>();
//
//	bool useLegacyTransaction = player.isClientSide() && player.mItemStackNetManager != nullptr;
//
//    // gsl::final_action runs code at the end of the scope
//    // Game only uses for legacy transaction
//    auto onScopeClosed = useLegacyTransaction
//        ? player.mItemStackNetManager->_tryBeginClientLegacyTransactionRequest()
//        : gsl::final_action<std::function<void()>>([]() {});
//
//    InteractionResult result;
//
//    playerInv->createTransactionContext(
//        [&transaction, &self](Container& container, unsigned int slot, const ItemStack& oldStack, const ItemStack newStack) {
//            InventorySource source(InventorySourceType::ContainerInventory, ContainerID::CONTAINER_ID_INVENTORY, InventorySource::InventorySourceFlags::NoFlag);
//            InventoryAction action(source, slot, oldStack, newStack);
//            
//			self->mPlayer.mTransactionManager.addExpectedAction(action);
//			transaction->mTransaction.addAction(action);
//        }, 
//        [&transaction, &self, pos, face, &result]() {
//            Player& player = self->mPlayer;
//            ILevel* level = player.mLevel;
//
//			ItemStack selectedItemCopy = player.getSelectedItem();
//            ItemInstance itemInstance(selectedItemCopy);
//
//            NetworkItemStackDescriptor networkDescriptor(selectedItemCopy);
//            transaction->mItem = networkDescriptor;
//            
//			HitResult hitResult = level->getHitResult();
//			HitResult liquidHit = level->getLiquidHitResult();
//
//			const BlockSource& region = player.getDimensionBlockSourceConst();
//            const Block& block = region.getBlock(*pos);
//
//            FacingID faceDir = face;
//            BlockPos calculatedPlacePos = self->_calculatePlacePos(selectedItemCopy, *pos, faceDir);
//
//            if (hitResult.mType == HitResultType::TILE || liquidHit.mLiquid.y == 0 || self->mBuildContext.mHasBuildDirection) {
//                transaction->mPos = *pos;
//				transaction->setTargetBlock(block);
//                transaction->mFace = faceDir;
//
//                Vec3 clickPos;
//
//                if (hitResult.mType == HitResultType::NO_HIT) {
//                    clickPos = Vec3::ZERO;
//                }
//                else {
//                    clickPos = hitResult.mPos - Vec3(*pos);
//                }
//                
//				transaction->mClickPos = clickPos;
//                transaction->mFromPos = *player.getPosition();
//                transaction->mActionType = face == FacingID::MAX ? ItemUseInventoryTransaction::ActionType::Use : ItemUseInventoryTransaction::ActionType::Place;
//
//                //if (block.isFenceBlock() && LeadItem::canBindPlayerMobs) {
//                if (block.isFenceBlock() && false) {
//                    result.mResult = (int)InteractionResult::Result::SUCCESS | (int)InteractionResult::Result::SWING;
//                }
//                else {
//                    // IMPORTANT WILL BREAK: GameMode::_sendUseItemOnEvents uses Player::getSelectedItem oof.
//                    // IDA detects it doesn't actually use it tho?
//                    result = self->useItemOn(selectedItemCopy, *pos, face, hitResult.mPos, nullptr);
//                }
//            }
//
//            // This section handles liquid clipped items, i.e. lillypads on water
//            bool isLiquidClippingItem = (result.mResult & (int)InteractionResult::Result::SUCCESS) == 0
//                && !selectedItemCopy.isNull()
//                && selectedItemCopy.isLiquidClipItem()
//                && !selectedItemCopy.shouldInteractionWithBlockBypassLiquid(block);
//
//            if (isLiquidClippingItem) {
//                Log::Info("Todo: Handle liquid clipping items");
//            }
//
//            // Block placement timings
//            if ((result.mResult & (int)InteractionResult::Result::SUCCESS) != 0) {
//				self->mBuildContext.mLastBuiltBlockPosition = calculatedPlacePos;
//
//                // Seems to get reset often enogu to be false each item use.
//                if (!self->mBuildContext.mHasLastBuiltPosition) {
//                    self->mMessenger->startSendItemUseOn(*pos, self->mBuildContext.mLastBuiltBlockPosition, (int)face);
//					self->mBuildContext.mHasLastBuiltPosition = true;
//                }
//
//				self->mLastBuildTime = std::chrono::steady_clock::now();
//                if ((result.mResult & (int)InteractionResult::Result::SWING) != 0) {
//					player.swing();
//                } 
//            }
//
//            PlayerEventCoordinator& eventCoordinator = player.getPlayerEventCoordinator();
//
//            std::function<EventResult(PlayerEventListener&)> ev = [&player, &itemInstance](PlayerEventListener& listener) {
//                EventResult result = listener.onPlayerItemUseInteraction(player, itemInstance);
//                return result;
//            };
//
//            eventCoordinator.todoMoveToProperClass_processEvent(ev);
//        }
//    );
//
//    Log::Info("InteractResult: {}", result.mResult);
//
//    if (player.getLevel()->isClientSide()) {
//		player.sendComplexInventoryTransaction(std::move(transaction));
//    } 
//     
//    return true;
//}

bool tryUseItem(GameMode& self, const ItemStack& stack, const Player& player, Container& container, int slot, ContainerID containerId, const BlockPos& pos, FacingID face, bool isSimTick) {
    const Item* item = stack.getItem();
    bool mayUseItem = !stack.isNull() && (!isSimTick || (isSimTick && item->canUseOnSimTick()));

    if (!mayUseItem) {
        Log::Info("May not use item: {}", stack);
        return false;
    }

    self.mPlayerLastPosition = *player.getPosition();
    std::unique_ptr<ItemUseInventoryTransaction> transaction = std::make_unique<ItemUseInventoryTransaction>();

    bool useLegacyTransaction = player.isClientSide() && player.mItemStackNetManager != nullptr;
    auto onScopeClosed = useLegacyTransaction
       ? player.mItemStackNetManager->_tryBeginClientLegacyTransactionRequest()
       : gsl::final_action<std::function<void()>>([]() {});

    InteractionResult result;

    container.createTransactionContext(
        [&transaction, &self, containerId](Container& container, unsigned int slot, const ItemStack& oldStack, const ItemStack& newStack) {
            InventorySource source(InventorySourceType::ContainerInventory, containerId, InventorySource::InventorySourceFlags::NoFlag);
            InventoryAction action(source, slot, oldStack, newStack);
           
			self.mPlayer.mTransactionManager.addExpectedAction(action);
			transaction->mTransaction.addAction(action);
        },
        [&transaction, &self, pos, face, &result, &stack, slot]() {
            Player& player = self.mPlayer;
            ILevel& level = *player.mLevel;

            ItemStack stackCopy = stack;
            ItemInstance itemInstance(stackCopy);
            NetworkItemStackDescriptor networkDescriptor(stackCopy);

            transaction->mItem = networkDescriptor;
            transaction->mSlot = slot;

            HitResult hitResult = level.getHitResult();
            HitResult liquidHit = level.getLiquidHitResult();

            const BlockSource& region = player.getDimensionBlockSourceConst();
            const Block& block = region.getBlock(pos);

            FacingID faceDir = face;
            BlockPos calculatedPlacePos = self._calculatePlacePos(stackCopy, pos, faceDir);

            if (hitResult.mType == HitResultType::TILE || liquidHit.mLiquid.y == 0 || self.mBuildContext.mHasBuildDirection) {
                transaction->mPos = pos;
                transaction->setTargetBlock(block);
                transaction->mFace = faceDir;

                Vec3 clickPos;

                if (hitResult.mType == HitResultType::NO_HIT) {
                    clickPos = Vec3::ZERO;
                }
                else {
                    clickPos = hitResult.mPos - Vec3(pos);
                }
                
                transaction->mClickPos = clickPos;
                transaction->mFromPos = *player.getPosition();
                transaction->mActionType = face == FacingID::MAX ? ItemUseInventoryTransaction::ActionType::Use : ItemUseInventoryTransaction::ActionType::Place;

                //if (block.isFenceBlock() && LeadItem::canBindPlayerMobs) {
                if (block.isFenceBlock() && false) {
                    result.mResult = (int)InteractionResult::Result::SUCCESS | (int)InteractionResult::Result::SWING;
                }
                else {
                    // IMPORTANT WILL BREAK: GameMode::_sendUseItemOnEvents uses Player::getSelectedItem oof.
                    // IDA detects it doesn't actually use it tho?
                    result = self.useItemOn(stackCopy, pos, face, hitResult.mPos, nullptr);
                }
            }

            // This section handles liquid clipped items, i.e. lillypads on water
            bool isLiquidClippingItem = (result.mResult & (int)InteractionResult::Result::SUCCESS) == 0
                && !stackCopy.isNull()
                && stackCopy.isLiquidClipItem()
                && !stackCopy.shouldInteractionWithBlockBypassLiquid(block);

            if (isLiquidClippingItem) {
                Log::Info("Todo: Handle liquid clipping items");
            }

           // Block placement timings
           if ((result.mResult & (int)InteractionResult::Result::SUCCESS) != 0) {
				self.mBuildContext.mLastBuiltBlockPosition = calculatedPlacePos;

                // Seems to get reset often enogu to be false each item use.
                if (!self.mBuildContext.mHasLastBuiltPosition) {
                    self.mMessenger->startSendItemUseOn(pos, self.mBuildContext.mLastBuiltBlockPosition, (int)face);
                        self.mBuildContext.mHasLastBuiltPosition = true;
                }

				self.mLastBuildTime = std::chrono::steady_clock::now();
                if ((result.mResult & (int)InteractionResult::Result::SWING) != 0) {
                        player.swing();
                } 
           }
        }
    );

    Log::Info("InteractResult: {}", result.mResult);

    if (player.isClientSide()) {
        player.sendComplexInventoryTransaction(std::move(transaction));
    }
 
    return true;
}

// Modded version of GameMode::buildBlock that supports offhand items
bool GameMode_buildBlock(GameMode* self, BlockPos* pos, FacingID face, bool isSimTick) {
	Player& player = self->mPlayer;
	ILevel& level = *player.mLevel;
	PlayerInventory& playerInv = *player.playerInventory;
	Inventory& inv = *playerInv.mInventory.get();

	const ItemStack& mainHandItem = playerInv.getSelectedItem();
    ActorEquipmentComponent* equipment = player.tryGetComponent<ActorEquipmentComponent>();
	const ItemStack& offHandItem = equipment->mHand->mItems[1];

    Log::Info("mainhand {}", mainHandItem);
    // bool usedMainhand = tryUseItem(*self, mainHandItem, player, inv, playerInv.mSelected, ContainerID::CONTAINER_ID_INVENTORY, *pos, face, isSimTick);
    bool usedMainhand = false;
    bool usedOffhand = false;

    if (!usedMainhand) {
        Log::Info("offhand {}", offHandItem);
        usedOffhand = tryUseItem(*self, offHandItem, player, *equipment->mHand, 1, ContainerID::CONTAINER_ID_OFFHAND, *pos, face, isSimTick);
    }

    return usedMainhand || usedOffhand;
}


// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(AmethystContext& ctx) 
{
    Amethyst::InitializeAmethystMod(ctx);

    Amethyst::GetEventBus().AddListener<RegisterItemsEvent>(&OnRegisterItems);

	Amethyst::HookManager& hooks = Amethyst::GetHookManager();
    //hooks.RegisterFunction<&SurvivalMode_useItemOn>("40 53 48 83 EC ? 80 B9 ? ? ? ? ? 48 8B DA 74 ? 80 3D");
	//hooks.CreateHook<&SurvivalMode_useItemOn>(_SurvivalMode_useItemOn, &SurvivalMode_useItemOn);

    Log::Info("PlayerInv offset: {}", offsetof(Player, playerInventory));

    ItemUseInventoryTransaction_ctor = (void*)SigScan("48 89 5C 24 ? 57 48 83 EC ? 48 8D 59 ? C7 41 ? ? ? ? ? 48 8D 05 ? ? ? ? 48 89 5C 24 ? 48 89 01 48 8B F9 48 8B CB E8 ? ? ? ? 33 C9 48 8D 05 ? ? ? ? 48 89 4B ? 0F 57 C0 48 89 4B ? 48 89 4B ? 48 8B 5C 24 ? 48 89 07 48 8D 05 ? ? ? ? 48 89 4F ? 48 89 4F");
    NetworkItemStackDescriptor_ctor = (void*)SigScan("48 89 5C 24 ? 55 56 57 41 56 41 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B FA 48 8B F1 48 89 4C 24 ? 0F B6 5A");

    hooks.RegisterFunction<&GameMode_buildBlock>("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 41 0F B6 F1");
	hooks.CreateHook<&GameMode_buildBlock>(_GameMode_buildBlock, &GameMode_buildBlock);

    // found a global which is in some item transaction logging code, false by default
    // enabling seems to have no change
    //bool* enableLogger = (bool*)SlideAddress(0x597A9B5);
	//*enableLogger = true;
}
