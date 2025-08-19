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

extern "C" void *ItemUseInventoryTransaction_ctor = nullptr;
extern "C" void *NetworkItemStackDescriptor_ctor = nullptr;

void OnRegisterItems(RegisterItemsEvent &event)
{
    //Log::Info("OnRegisterItems");

    for (auto &pair : event.itemRegistry.mNameToItemMap)
    {
        // Log::Info("Item ID: {:s}", pair.first.getString());
        pair.second.get()->setAllowOffhand(true);
    }
}

class PlayerEventCoordinator
{
public:
    void todoMoveToProperClass_processEvent(std::function<EventResult(class PlayerEventListener &)> &ev)
    {
        using function = decltype(&PlayerEventCoordinator::todoMoveToProperClass_processEvent);
        static auto func = std::bit_cast<function>(SigScan("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 4C 8B EA 48 8B F1"));
        (this->*func)(ev);
    }
};

SafetyHookInline _GameMode_buildBlock;

// bool GameMode_buildBlock(GameMode* self, BlockPos* pos, FacingID face, bool isSimTick) {
//     Player& player = self->mPlayer;
//	ILevel* level = player.mLevel;
//     PlayerInventory* playerInv = player.playerInventory;
//	Inventory* inv = playerInv->mInventory.get();
//
//     const ItemStack& selectedItem = inv->getItem(playerInv->mSelected);
//	const Item* item = selectedItem.getItem();
//
//     // The game seems to check things multiple times, i.e. selectedItem.isNull() covers some of the other cases in here
//     // Going for accuracy so do anyway ig
//     bool mayUseItem = !selectedItem.mValid
//         || !item
//         || selectedItem.isNull()
//         || selectedItem.mCount == 0
//         || !isSimTick
//         || item->canUseOnSimTick();
//
//     if (!mayUseItem) {
//         Log::Info("May not use item!!");
//         return false;
//     }
//
//     self->mPlayerLastPosition = *player.getPosition();
//     std::unique_ptr<ItemUseInventoryTransaction> transaction = std::make_unique<ItemUseInventoryTransaction>();
//
//	bool useLegacyTransaction = player.isClientSide() && player.mItemStackNetManager != nullptr;
//
//     // gsl::final_action runs code at the end of the scope
//     // Game only uses for legacy transaction
//     auto onScopeClosed = useLegacyTransaction
//         ? player.mItemStackNetManager->_tryBeginClientLegacyTransactionRequest()
//         : gsl::final_action<std::function<void()>>([]() {});
//
//     InteractionResult result;
//
//     playerInv->createTransactionContext(
//         [&transaction, &self](Container& container, unsigned int slot, const ItemStack& oldStack, const ItemStack newStack) {
//             InventorySource source(InventorySourceType::ContainerInventory, ContainerID::CONTAINER_ID_INVENTORY, InventorySource::InventorySourceFlags::NoFlag);
//             InventoryAction action(source, slot, oldStack, newStack);
//
//			self->mPlayer.mTransactionManager.addExpectedAction(action);
//			transaction->mTransaction.addAction(action);
//         },
//         [&transaction, &self, pos, face, &result]() {
//             Player& player = self->mPlayer;
//             ILevel* level = player.mLevel;
//
//			ItemStack selectedItemCopy = player.getSelectedItem();
//             ItemInstance itemInstance(selectedItemCopy);
//
//             NetworkItemStackDescriptor networkDescriptor(selectedItemCopy);
//             transaction->mItem = networkDescriptor;
//
//			HitResult hitResult = level->getHitResult();
//			HitResult liquidHit = level->getLiquidHitResult();
//
//			const BlockSource& region = player.getDimensionBlockSourceConst();
//             const Block& block = region.getBlock(*pos);
//
//             FacingID faceDir = face;
//             BlockPos calculatedPlacePos = self->_calculatePlacePos(selectedItemCopy, *pos, faceDir);
//
//             if (hitResult.mType == HitResultType::TILE || liquidHit.mLiquid.y == 0 || self->mBuildContext.mHasBuildDirection) {
//                 transaction->mPos = *pos;
//				transaction->setTargetBlock(block);
//                 transaction->mFace = faceDir;
//
//                 Vec3 clickPos;
//
//                 if (hitResult.mType == HitResultType::NO_HIT) {
//                     clickPos = Vec3::ZERO;
//                 }
//                 else {
//                     clickPos = hitResult.mPos - Vec3(*pos);
//                 }
//
//				transaction->mClickPos = clickPos;
//                 transaction->mFromPos = *player.getPosition();
//                 transaction->mActionType = face == FacingID::MAX ? ItemUseInventoryTransaction::ActionType::Use : ItemUseInventoryTransaction::ActionType::Place;
//
//                 //if (block.isFenceBlock() && LeadItem::canBindPlayerMobs) {
//                 if (block.isFenceBlock() && false) {
//                     result.mResult = (int)InteractionResult::Result::SUCCESS | (int)InteractionResult::Result::SWING;
//                 }
//                 else {
//                     // IMPORTANT WILL BREAK: GameMode::_sendUseItemOnEvents uses Player::getSelectedItem oof.
//                     // IDA detects it doesn't actually use it tho?
//                     result = self->useItemOn(selectedItemCopy, *pos, face, hitResult.mPos, nullptr);
//                 }
//             }
//
//             // This section handles liquid clipped items, i.e. lillypads on water
//             bool isLiquidClippingItem = (result.mResult & (int)InteractionResult::Result::SUCCESS) == 0
//                 && !selectedItemCopy.isNull()
//                 && selectedItemCopy.isLiquidClipItem()
//                 && !selectedItemCopy.shouldInteractionWithBlockBypassLiquid(block);
//
//             if (isLiquidClippingItem) {
//                 Log::Info("Todo: Handle liquid clipping items");
//             }
//
//             // Block placement timings
//             if ((result.mResult & (int)InteractionResult::Result::SUCCESS) != 0) {
//				self->mBuildContext.mLastBuiltBlockPosition = calculatedPlacePos;
//
//                 // Seems to get reset often enogu to be false each item use.
//                 if (!self->mBuildContext.mHasLastBuiltPosition) {
//                     self->mMessenger->startSendItemUseOn(*pos, self->mBuildContext.mLastBuiltBlockPosition, (int)face);
//					self->mBuildContext.mHasLastBuiltPosition = true;
//                 }
//
//				self->mLastBuildTime = std::chrono::steady_clock::now();
//                 if ((result.mResult & (int)InteractionResult::Result::SWING) != 0) {
//					player.swing();
//                 }
//             }
//
//             PlayerEventCoordinator& eventCoordinator = player.getPlayerEventCoordinator();
//
//             std::function<EventResult(PlayerEventListener&)> ev = [&player, &itemInstance](PlayerEventListener& listener) {
//                 EventResult result = listener.onPlayerItemUseInteraction(player, itemInstance);
//                 return result;
//             };
//
//             eventCoordinator.todoMoveToProperClass_processEvent(ev);
//         }
//     );
//
//     Log::Info("InteractResult: {}", result.mResult);
//
//     if (player.getLevel()->isClientSide()) {
//		player.sendComplexInventoryTransaction(std::move(transaction));
//     }
//
//     return true;
// }

SafetyHookInline _GameMode_useItemOn;

//InteractionResult GameMode_useItemOn(GameMode* self, ItemStack& item, const BlockPos& at, FacingID face, const Vec3& hit, const Block* targetBlock)
// Returning an InteractionResult directly is for some reason returning in a different register than the one used by the game
// force it lmao
InteractionResult* GameMode_useItemOn(GameMode* self, InteractionResult* result, ItemStack& item, const BlockPos& at, FacingID face, const Vec3& hit, const Block* targetBlock)
{
    // todo: seems to set self+0x40 to 0
    bool isSneakDown = false; // todo: get this

    result->mResult = 0;

    //Log::Info("GameMode_useItemOn {}", self->mPlayer.isClientSide() ? "client" : "server");

    // idk what this does but it doesn't seem to have anything to do with networking / decreasing item stack
    if ((self->_sendUseItemOnEvents(item, at, face, hit).mResult & (int)InteractionResult::Result::SUCCESS) == 0) return result;

    // todo: check that the player has use ability
    bool hasPermissions = true; 
    if (!hasPermissions) return result;

    BlockSource& region = self->mPlayer.getDimensionBlockSource();
	const Block& block = region.getBlock(at);
	const Block& liquidBlock = region.getLiquidBlock(at);
	const BlockLegacy& blockLegacy = *block.mLegacyBlock.get();

    // todo: game checks if its invisible bedrock

    if (targetBlock) {
		uint64_t blockSerializationHash = targetBlock->mSerializationIdHash;

        if (block.mSerializationIdHash != blockSerializationHash && liquidBlock.mSerializationIdHash != blockSerializationHash)
			return result;
    }

	const BlockSource& constRegion = self->mPlayer.getDimensionBlockSourceConst();

    AABB blockCollision = AABB::BLOCK_SHAPE;
    bool isBlockSolid = block.getCollisionShape(blockCollision, constRegion, at, nullptr);
    bool isItemBucket = item.isInstance("minecraft:bucket", false);

	const HashedString& blockFullName = blockLegacy.mNameInfo.mFullName;
	bool isPowderedSnow = blockFullName == "minecraft:powder_snow";
    bool canContainLiquid = blockLegacy.canContainLiquid();

	// Handles clicking with buckets on non-powdered snow blocks, or blocks that cannot contain liquids
	if (isBlockSolid && isItemBucket && !isPowderedSnow && !canContainLiquid) return result;

	BlockLegacy* blockToUse = nullptr;

    //if (!isSneakDown
    //    || ItemStackBase::hasTag(item, &VanillaItemTags::Hatchet)
    //    || ItemStackBase::hasTag(item, &VanillaItemTags::Shovel)
    //    || ItemStackBase::hasTag(item, &VanillaItemTags::Hoe)
    //    || ItemStackBase::isInstance(item, &VanillaItemNames::HoneyComb, 0)
    //    || (v29 = (*(*this->mPlayer + 664i64))(this->mPlayer), !ItemStackBase::operator bool(v29)))
    // todo: handle all these cases
    if (!isSneakDown) {
        bool isAir = true;
		blockToUse = block.mLegacyBlock;

        if (!isAir) {
            Log::Info("Non air blocks not currently handled");
            return result;
        }

        Log::Info("clicked air");
    }

    if (!item) {
        Log::Info("Todo handle !item scenario");
		return result;
    }

    if (!item.isBlock()) {
		Log::Info("Todo handle !item.isBlock() scenario");
        return result;
    }

    blockToUse = item.getLegacyBlock();
    const Block& renderBlock = blockToUse->getRenderBlock();

    if (self->_canUseBlock(renderBlock)) {
        bool isInCreative = false;

        if (isInCreative) {
			Log::Info("Todo handle Using block in creative mode");
            return result;
        }
        else {
            //Log::Info("Calling item::useOn {}", item);
            result->mResult = item.useOn(self->mPlayer, at.x, at.y, at.z, face, hit).mResult;
        }
    }

    return result;
}

bool tryUseItem(GameMode &self, const ItemStack &stack, const Player &player, Container &container, int slot, ContainerID containerId, const BlockPos &pos, FacingID face, bool isSimTick)
{
    const Item *item = stack.getItem();
    bool mayUseItem = !stack.isNull() && (!isSimTick || (isSimTick && item->canUseOnSimTick()));

    if (!mayUseItem)
    {
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
        // This lambda gets called whenever GameMode::useItemOn is called within the 2nd lambda
        [&transaction, &self, containerId](Container &container, unsigned int slot, const ItemStack &oldStack, const ItemStack &newStack)
        {
            //Log::Info("Top bit");
            InventorySource source(InventorySourceType::ContainerInventory, containerId, InventorySource::InventorySourceFlags::NoFlag);
            InventoryAction action(source, slot, oldStack, newStack);

            self.mPlayer.mTransactionManager.addExpectedAction(action);
            transaction->mTransaction.addAction(action);
        },
        // This lambda is the first to get called
        [&transaction, &self, pos, face, &result, &stack, slot]()
        {
			//Log::Info("Bottom bit");
            Player &player = self.mPlayer;
            ILevel &level = *player.mLevel;

            ItemStack stackCopy = stack;
            ItemInstance itemInstance(stackCopy);
            NetworkItemStackDescriptor networkDescriptor(stackCopy);

            transaction->mItem = networkDescriptor;
            transaction->mSlot = slot;

            HitResult hitResult = level.getHitResult();
            HitResult liquidHit = level.getLiquidHitResult();

            const BlockSource &region = player.getDimensionBlockSourceConst();
            const Block &block = region.getBlock(pos);

            FacingID faceDir = face;
            BlockPos calculatedPlacePos = self._calculatePlacePos(stackCopy, pos, faceDir);

            if (hitResult.mType == HitResultType::TILE || liquidHit.mLiquid.y == 0 || self.mBuildContext.mHasBuildDirection)
            {
                transaction->mPos = pos;
                transaction->setTargetBlock(block);
                transaction->mFace = faceDir;

                Vec3 clickPos;

                if (hitResult.mType == HitResultType::NO_HIT)
                {
                    clickPos = Vec3::ZERO;
                }
                else
                {
                    clickPos = hitResult.mPos - Vec3(pos);
                }

                transaction->mClickPos = clickPos;
                transaction->mFromPos = *player.getPosition();
                transaction->mActionType = face == FacingID::MAX ? ItemUseInventoryTransaction::ActionType::Use : ItemUseInventoryTransaction::ActionType::Place;

                // if (block.isFenceBlock() && LeadItem::canBindPlayerMobs) {
                if (block.isFenceBlock() && false)
                {
                    result.mResult = (int)InteractionResult::Result::SUCCESS | (int)InteractionResult::Result::SWING;
                }
                else
                {
                    // IMPORTANT WILL BREAK: GameMode::_sendUseItemOnEvents uses Player::getSelectedItem oof.
                    // IDA detects it doesn't actually use it tho?
					//Log::Info("Send self.useItemOn");
                    //result = self.useItemOn(stackCopy, pos, face, hitResult.mPos, nullptr);
					GameMode_useItemOn(&self, &result, stackCopy, pos, face, hitResult.mPos, nullptr);
					//Log::Info("self.useItemOn Result: {}", result.mResult);
                }
            }

            // This section handles liquid clipped items, i.e. lillypads on water
            bool isLiquidClippingItem = (result.mResult & (int)InteractionResult::Result::SUCCESS) == 0 && !stackCopy.isNull() && stackCopy.isLiquidClipItem() && !stackCopy.shouldInteractionWithBlockBypassLiquid(block);

            if (isLiquidClippingItem)
            {
                Log::Info("Todo: Handle liquid clipping items");
            }

            // Block placement timings
            if ((result.mResult & (int)InteractionResult::Result::SUCCESS) != 0)
            {
                //Log::Info("Test");
                self.mBuildContext.mLastBuiltBlockPosition = calculatedPlacePos;

                // Seems to get reset often enogu to be false each item use.
                if (!self.mBuildContext.mHasLastBuiltPosition)
                {
                    //self.mMessenger->startSendItemUseOn(pos, self.mBuildContext.mLastBuiltBlockPosition, (int)face);
                    self.mBuildContext.mHasLastBuiltPosition = true;
                }

                self.mLastBuildTime = std::chrono::steady_clock::now();
                if ((result.mResult & (int)InteractionResult::Result::SWING) != 0)
                {
                    player.swing();
                }
            }
        });

    //Log::Info("InteractResult: {}", result.mResult);

    if (player.isClientSide())
    {
        player.sendComplexInventoryTransaction(std::move(transaction));
    }

    return true;
}

// Modded version of GameMode::buildBlock that supports offhand items
// note: the game only seems to call this on the client?
// I wasn't able to see it called on the server when hooking it and logging the caller
bool GameMode_buildBlock(GameMode *self, BlockPos *pos, FacingID face, bool isSimTick)
{
    
    Player &player = self->mPlayer;
    ILevel &level = *player.mLevel;
    PlayerInventory &playerInv = *player.playerInventory;
    Inventory &inv = *playerInv.mInventory.get();
    Log::Info("GameMode::buildBlock {}", player.isClientSide() ? "client" : "server");

    const ItemStack &mainHandItem = playerInv.getSelectedItem();
    ActorEquipmentComponent *equipment = player.tryGetComponent<ActorEquipmentComponent>();
    const ItemStack &offHandItem = equipment->mHand->mItems[1];

    Log::Info("mainhand {}", mainHandItem);
    bool usedMainhand = tryUseItem(*self, mainHandItem, player, inv, playerInv.mSelected, ContainerID::CONTAINER_ID_INVENTORY, *pos, face, isSimTick);
    bool usedOffhand = false;

    if (!usedMainhand)
    {
        Log::Info("offhand {}", offHandItem);
        usedOffhand = tryUseItem(*self, offHandItem, player, *equipment->mHand, 1, ContainerID::CONTAINER_ID_OFFHAND, *pos, face, isSimTick);
    }

    return usedMainhand || usedOffhand;
}

enum InventoryTransactionError : uint64_t {
    Unknown0 = 0,
    Success = 1,
    Unknown2 = 2,
    ProbablyError = 3,


    StateMismatch = 7
};

SafetyHookInline _ItemUseInventoryTransaction_handle;

InventoryTransactionError ItemUseInventoryTransaction_handle(ItemUseInventoryTransaction* self, Player& player, bool isSenderAuthority) {
	// todo: game checks if player is dead, if so returns StateMismatch

	Level& level = *player.getLevel()->asLevel();
	bool isClientSide = level.isClientSide;
    BlockPalette& blockPalette = level.getBlockPalette();

    ItemStack stack = ItemStack::fromDescriptor(self->mItem, blockPalette, isClientSide);
    Log::Info("ItemUseInventoryTransaction_handle stack {}", stack);

    PlayerInventory& playerInv = *player.playerInventory;
    Inventory& inv = *playerInv.mInventory.get();
    const ItemStack& mainHandItem = playerInv.getSelectedItem();
    ActorEquipmentComponent* equipment = player.tryGetComponent<ActorEquipmentComponent>();
    const ItemStack& offHandItem = equipment->mHand->mItems[1];

    // This could probably be more verbose in checking, but the logic is if the two stacks are the same, it was probably the mainhand?
    // There probably is some weird edgecase around this
    bool usedMainhand = stack.mItem == mainHandItem.mItem;
    const ItemStack& stackToUse = usedMainhand ? mainHandItem : offHandItem;

    Log::Info("usedStack {}", stackToUse);
    

    return InventoryTransactionError::Success;
}

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(AmethystContext &ctx)
{
    Amethyst::InitializeAmethystMod(ctx);

    Amethyst::GetEventBus().AddListener<RegisterItemsEvent>(&OnRegisterItems);

    Amethyst::HookManager &hooks = Amethyst::GetHookManager();
    // hooks.RegisterFunction<&SurvivalMode_useItemOn>("40 53 48 83 EC ? 80 B9 ? ? ? ? ? 48 8B DA 74 ? 80 3D");
    // hooks.CreateHook<&SurvivalMode_useItemOn>(_SurvivalMode_useItemOn, &SurvivalMode_useItemOn);

    //Log::Info("PlayerInv offset: {}", offsetof(Player, playerInventory));

    ItemUseInventoryTransaction_ctor = (void *)SigScan("48 89 5C 24 ? 57 48 83 EC ? 48 8D 59 ? C7 41 ? ? ? ? ? 48 8D 05 ? ? ? ? 48 89 5C 24 ? 48 89 01 48 8B F9 48 8B CB E8 ? ? ? ? 33 C9 48 8D 05 ? ? ? ? 48 89 4B ? 0F 57 C0 48 89 4B ? 48 89 4B ? 48 8B 5C 24 ? 48 89 07 48 8D 05 ? ? ? ? 48 89 4F ? 48 89 4F");
    NetworkItemStackDescriptor_ctor = (void *)SigScan("48 89 5C 24 ? 55 56 57 41 56 41 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B FA 48 8B F1 48 89 4C 24 ? 0F B6 5A");

    hooks.RegisterFunction<&GameMode_buildBlock>("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 41 0F B6 F1");
    hooks.CreateHook<&GameMode_buildBlock>(_GameMode_buildBlock, &GameMode_buildBlock);

    // found a global which is in some item transaction logging code, false by default
    // enabling seems to have no change
    // bool* enableLogger = (bool*)SlideAddress(0x597A9B5);
    //*enableLogger = true;

    hooks.RegisterFunction<&GameMode_useItemOn>("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4D 8B E1 49 8B F0 4C 8B EA");
	hooks.CreateHook<&GameMode_useItemOn>(_GameMode_useItemOn, &GameMode_useItemOn);

    hooks.RegisterFunction<&ItemUseInventoryTransaction_handle>("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 0F B6 E0");
	hooks.CreateHook<&ItemUseInventoryTransaction_handle>(_ItemUseInventoryTransaction_handle, &ItemUseInventoryTransaction_handle);
}
