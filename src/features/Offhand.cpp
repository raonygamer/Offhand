#include "features/Offhand.hpp"
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
#include <amethyst/runtime/HookManager.hpp>
#include <amethyst/runtime/ModContext.hpp>
#include "features/OffhandSwingComponent.hpp"

extern "C" void *ItemUseInventoryTransaction_ctor = nullptr;
extern "C" void *NetworkItemStackDescriptor_ctor = nullptr;

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

    bool itemCanInteractWithBlock = item.hasTag("minecraft:is_axe")
        || item.hasTag("minecraft:is_shovel")
        || item.hasTag("minecraft:is_hoe")
        || item.isInstance("minecraft:honeycomb", false);

    if (!isSneakDown || itemCanInteractWithBlock || !item) {
		blockToUse = block.mLegacyBlock;
        bool isAir = blockToUse->isAir();

        if (!isAir && block.use(self->mPlayer, at, face, hit)) {
            Log::Info("block.use() currently not implemented");
            return result;
        }
    }
     
    if (!item) {
        Log::Info("Todo handle !item scenario");
		return result;
    }

    if (!item.isBlock()) {
        self->mBuildContext.mLastBuildBlockWasSnappable = blockLegacy.isSnappableBlock();

        bool isCreative = false;
		InteractionResult useResult;

        if (isCreative) {
            Log::Info("todo handle creative");
        }
        else {
            useResult.mResult = item.useOn(self->mPlayer, at.x, at.y, at.z, face, hit).mResult;
            Log::Info("[item.isBlock] item.useOn in return {}", useResult.mResult);
        }

        if ((useResult.mResult & (int)InteractionResult::Result::SUCCESS) != 0) {
            
            result->mResult = useResult.mResult; 
            return result;
		}

        if (item) {
            Log::Info("Player::setSelectedItem()");
        }
        else {
            Log::Info("PlayerInventory::clearSlot()");
        }

        Log::Info("GameMode::_sendPlayerInteractWithBlockAfterEvent");

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
            result->mResult = item.useOn(self->mPlayer, at.x, at.y, at.z, face, hit).mResult;
        }
    }

    return result;
}

bool tryUseItem(GameMode &self, const ItemStack &stack, const Player &player, Container &container, int slot, ContainerID containerId, const BlockPos &pos, FacingID face, bool isSimTick, bool isOffhand)
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
        [&transaction, &self, pos, face, &result, &stack, slot, isOffhand]()
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
                    if (!isOffhand) player.swing();
                    else {
                        if (!player.hasComponent<OffhandSwingComponent>()) {
							player.addComponent<OffhandSwingComponent>();
                        }

						OffhandSwingComponent* swingComp = player.tryGetComponent<OffhandSwingComponent>();
                        
                        // player.swing() implemented using OffhandSwingComponent instead off mainhand.
                        int currentSwingDuration = player.getCurrentSwingDuration();
                        int time = swingComp->mOffhandSwingTime;

                        if (!swingComp->mOffhandSwinging || time >= currentSwingDuration / 2 || time < 0) {
							swingComp->mOffhandSwinging = true;
							swingComp->mOffhandSwingTime = -1;
                        }
                    }
                }
            }
        });

    Log::Info("[tryUseItem] InteractResult: {}", result.mResult);

    if (player.isClientSide())
    {
        player.sendComplexInventoryTransaction(std::move(transaction));
    }

    if (result.mResult & (int)InteractionResult::Result::SUCCESS)
    {
        return true;
	}

    return false;
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

    const ItemStack &mainHandItem = playerInv.getSelectedItem();
    ActorEquipmentComponent *equipment = player.tryGetComponent<ActorEquipmentComponent>();
    const ItemStack &offHandItem = equipment->mHand->mItems[1];

    bool usedMainhand = tryUseItem(*self, mainHandItem, player, inv, playerInv.mSelected, ContainerID::CONTAINER_ID_INVENTORY, *pos, face, isSimTick, false);
    if (usedMainhand) return true;
    
    return tryUseItem(*self, offHandItem, player, *equipment->mHand, 1, ContainerID::CONTAINER_ID_OFFHAND, *pos, face, isSimTick, true);
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
    Log::Info("ItemUseInventoryTransaction_handle stack {} actionType {}", stack, (int)self->mActionType);

    PlayerInventory& playerInv = *player.playerInventory;
    Inventory& inv = *playerInv.mInventory.get();
    const ItemStack& mainHandItem = playerInv.getSelectedItem();
    ActorEquipmentComponent* equipment = player.tryGetComponent<ActorEquipmentComponent>();
    const ItemStack& offHandItem = equipment->mHand->mItems[1];

    // This could probably be more verbose in checking, but the logic is if the two stacks are the same, it was probably the mainhand?
    // There probably is some weird edgecase around this
    bool usedMainhand = stack.mItem == mainHandItem.mItem;
    const ItemStack& stackToUse = usedMainhand ? mainHandItem : offHandItem;
    Container* containerToUse = usedMainhand ? &inv : (Container*)equipment->mHand.get();

    //Log::Info("ItemUseInventoryTransaction_handle stackToUse is offhand: {}", stackToUse.isOffhandItem());

    bool itemsMatch = true; // InventoryTransaction::checkTransactionItemsMatch
	bool slotsMatch = true; // self->mSlot == playerInv.mSelected; <- to check properly probs need to switch between offhand and inv slots

    bool areItemsAndSlotsValid = isSenderAuthority || itemsMatch && slotsMatch;

    if (!areItemsAndSlotsValid) {
        Log::Info("ItemUseInventoryTransaction_handle: Invalid transaction, isSenderAuthority: {}, itemsMatch: {}, slotsMatch: {}", isSenderAuthority, itemsMatch, slotsMatch);
        return InventoryTransactionError::ProbablyError;
    }

    GameMode& gameMode = player.getGameMode();
    float maxPickRange = gameMode.getMaxPickRange() + 0.5f;
    bool distanceCheck = true; // todo add

    bool isValidAction = self->mActionType == ItemUseInventoryTransaction::ActionType::Use || distanceCheck || isSenderAuthority;

    if (!isValidAction) {
        Log::Info("ItemUseInventoryTransaction_handle: Invalid action type: {}", (int)self->mActionType);
        return InventoryTransactionError::ProbablyError;
    }

	InventorySource source(
        InventorySourceType::ContainerInventory, 
        usedMainhand ? ContainerID::CONTAINER_ID_INVENTORY : ContainerID::CONTAINER_ID_OFFHAND, 
        InventorySource::InventorySourceFlags::NoFlag
    );

	const std::vector<InventoryAction>& actions = self->mTransaction.getActions(source);
    
    for (auto& action : actions) {
        // todo: game does something, there are no actions sent for placing blocks it seems?
    }

    playerInv.createTransactionContext(
        [&self](Container& container, unsigned int slot, const ItemStack& oldStack, const ItemStack newStack) {
            Log::Info("top half");
            // todo: what is supposed to be in here!!!
        },
        [&self, &gameMode, &stackToUse, &level, &containerToUse]() {
            Log::Info("bottom half");

            if (self->mActionType == ItemUseInventoryTransaction::ActionType::Use) {
				ItemStack stackCopy = ItemStack(stackToUse);

                // Seems to return false for things the server needs to update the client on
                // i.e. stone block returns true
                // but torch item returns false
                if (!gameMode.baseUseItem(stackCopy)) {
                    Log::Info("calling ItemUseInventoryTransaction::resendPlayerState");
					self->resendPlayerState(gameMode.mPlayer);
                }

                Log::Info("before {}, after: {}", stackToUse, stackCopy);
            }
            else if (self->mActionType == ItemUseInventoryTransaction::ActionType::Destroy) {
				Log::Info("todo impl self->mActionType == ItemUseInventoryTransaction::ActionType::Destroy");
            }
            else if (self->mActionType == ItemUseInventoryTransaction::ActionType::Place) {
				bool isClientSide = level.isClientSide;
                BlockPalette& blockPalette = level.getBlockPalette();
                ItemStack stackCopy = ItemStack(stackToUse);

                // Idk why this exists or what the fuck it does... 
                //if (!Actor::isSneaking(*(a1 + 8)))
                //{
                //    v8 = Player::getSelectedItem(*(a1 + 8));
                //    ItemStack::operator=(&itemStack, v8);
                //}

				const Block& blockToPlaceOn = blockPalette.getBlock(self->mTargetBlockId);
                Log::Info("blockToPlace {}", blockToPlaceOn.mLegacyBlock->mNameInfo.mFullName.getString());

                Block* v18 = nullptr;
                //Actor::setPos(*(a1 + 8), (*a1 + 232i64));

                /*while (1)
                {
                    v18 = *v17;
                    if ((*v17)->mSerializationIdHash == v14->mSerializationIdHash)
                        break;
                    mLegacyBlock = v18->mLegacyBlock;
                    if (!mLegacyBlock || (v16 = v14->mLegacyBlock) == 0i64)
                        gsl::details::terminate(v16);
                    if (mLegacyBlock == v16 && Block::allowStateMismatchOnPlacement(*v17, v14))
                        break;
                    if (++v17 == &v42)
                        goto LABEL_17;
                }*/

                InteractionResult result = gameMode.useItemOn(stackCopy, self->mPos, self->mFace, self->mClickPos, v18);
                // Actor::setPos(*(a1 + 8), v39);

                if ((result.mResult & (int)InteractionResult::Result::SUCCESS) != 0)
                {
                    bool itemsMatch = false; // todo: ItemStackBase::operator!=(&itemStack, v25)

                    if (!itemsMatch) {
                        containerToUse->setItem(self->mSlot, stackCopy);
                    }   

                    if (gameMode.isLastBuildBlockInteractive()) {
                        BlockEventCoordinator& blockEvents = level.getBlockEventCoordinator();
                        blockEvents.sendBlockInteractedWith(gameMode.mPlayer, self->mPos);
                    }
                }
                else {
                    Log::Info("resending");
					self->resendBlocksAroundArea(gameMode.mPlayer, self->mPos, self->mFace);
					self->resendPlayerState(gameMode.mPlayer);
                }
            }
        }
    );

    return InventoryTransactionError::ProbablyError; // todo: return correct error code
}

void RegisterOffhandHooks()
{
    Amethyst::HookManager &hooks = Amethyst::GetHookManager();

    ItemUseInventoryTransaction_ctor = (void *)SigScan("48 89 5C 24 ? 57 48 83 EC ? 48 8D 59 ? C7 41 ? ? ? ? ? 48 8D 05 ? ? ? ? 48 89 5C 24 ? 48 89 01 48 8B F9 48 8B CB E8 ? ? ? ? 33 C9 48 8D 05 ? ? ? ? 48 89 4B ? 0F 57 C0 48 89 4B ? 48 89 4B ? 48 8B 5C 24 ? 48 89 07 48 8D 05 ? ? ? ? 48 89 4F ? 48 89 4F");
    NetworkItemStackDescriptor_ctor = (void *)SigScan("48 89 5C 24 ? 55 56 57 41 56 41 57 48 83 EC ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? 48 8B FA 48 8B F1 48 89 4C 24 ? 0F B6 5A");

    hooks.RegisterFunction<&GameMode_buildBlock>("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 41 0F B6 F1");
    hooks.CreateHook<&GameMode_buildBlock>(_GameMode_buildBlock, &GameMode_buildBlock);

    hooks.RegisterFunction<&GameMode_useItemOn>("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4D 8B E1 49 8B F0 4C 8B EA");
	hooks.CreateHook<&GameMode_useItemOn>(_GameMode_useItemOn, &GameMode_useItemOn);

    hooks.RegisterFunction<&ItemUseInventoryTransaction_handle>("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 45 0F B6 E0");
	hooks.CreateHook<&ItemUseInventoryTransaction_handle>(_ItemUseInventoryTransaction_handle, &ItemUseInventoryTransaction_handle);
}