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

void OnRegisterItems(RegisterItemsEvent& event)
{
    for (auto& pair : event.itemRegistry.mNameToItemMap) {
		//Log::Info("Item ID: {:s}", pair.first.getString());
        pair.second.get()->setAllowOffhand(true);
    }
}

class IGameModeTimer;
class IGameModeMessenger;

class GameMode {
public:
    class BuildContext {
    public:
        bool mHasBuildDirection;
        bool mHasLastBuiltPosition;
        bool mLastBuildBlockWasInteractive;
        bool mLastBuildBlockWasSnappable;
        BlockPos mLastBuiltBlockPosition;
        BlockPos mBuildDirection;
        BlockPos mNextBuildPos;
        unsigned char mContinueFacing;
    };

    uint64_t vtable;
    Player* mPlayer;
    BlockPos mDestroyBlockPos;
    uint8_t mDestroyBlockFace;
    float mOldDestroyProgress;
    float mDestroyProgress;
    long double mLastDestroyTime;
    float mDistanceTravelled;
    Vec3 mPlayerLastPosition;
    GameMode::BuildContext mBuildContext;
    float mMinPlayerSpeed;
    int mContinueBreakBlockCount;

    std::chrono::steady_clock::time_point mLastBuildTime;
    std::chrono::steady_clock::time_point mNoDestroyUntil;
    std::chrono::steady_clock::time_point mNoDestroySoundUntil;

    std::chrono::milliseconds creativeDestructionTickDelay;
    std::chrono::milliseconds buildingTickDelay;
    std::chrono::milliseconds destroySoundDelay;

    std::unique_ptr<IGameModeTimer> mTimer;
    std::unique_ptr<IGameModeMessenger> mMessenger;

    BlockPos _calculatePlacePos(ItemStack& heldStack, const BlockPos& pos, FacingID& face) {
        const Item* item = heldStack.getItem();

        if (item) {
            BlockPos result = pos;
            item->calculatePlacePos(heldStack, *this->mPlayer, face, result);
            return result;
        }

        BlockSource& blockSource = mPlayer->getDimensionBlockSource();
        const Block& block = blockSource.getBlock(pos);

        if (block.canBeBuiltOver(blockSource, pos)) {
            face = Facing::Name::UP;
            return pos;
        }
        
        return pos.neighbor(face);
    }
};

class SurvivalMode;

enum class InventoryTransactionError : uint32_t
{
    Unknown = 0x0,
    NoError = 0x1,
    BalanceMismatch = 0x2,
    SourceItemMismatch = 0x3,
    InventoryMismatch = 0x4,
    SizeMismatch = 0x5,
    AuthorityMismatch = 0x6,
    StateMismatch = 0x7,
    ApiDenied = 0x8,
};

class ItemUseInventoryTransaction {
    std::byte padding0[30];
    BlockRuntimeId mTargetBlockId;
    FacingID mFace;
    int32_t mSlot;
    std::byte padding44[212];


public:
    ItemUseInventoryTransaction() {
		using function = ItemUseInventoryTransaction *(__thiscall*)(ItemUseInventoryTransaction*);
        static auto func = reinterpret_cast<function>(SigScan("48 89 5C 24 ? 57 48 83 EC ? 48 8D 59 ? C7 41 ? ? ? ? ? 48 8D 05 ? ? ? ? 48 89 5C 24 ? 48 89 01 48 8B F9 48 8B CB E8 ? ? ? ? 33 C9 48 8D 05 ? ? ? ? 48 89 4B ? 0F 57 C0 48 89 4B ? 48 89 4B ? 48 8B 5C 24 ? 48 89 07 48 8D 05 ? ? ? ? 48 89 4F ? 48 89 4F"));
		func(this);
    }

    void setTargetBlock(const Block& block) {
        this->mTargetBlockId = block.getRuntimeId();
    }

//    virtual ~ItemUseInventoryTransaction() = default;
//    virtual void unk1(); // read 8 
//	virtual void unk2(); // write 16
//	virtual void unk3(); // postLoadItems 24
//	virtual void unk4(); // handle 32
//	virtual void onTransactionError(Player& player, InventoryTransactionError error); // 40
};

static_assert(sizeof(ItemUseInventoryTransaction) == 0x100);

class ItemStackNetManagerBase {
public:
	virtual ~ItemStackNetManagerBase() = default;
	virtual void unk1();
	virtual void unk2();
	virtual void unk3();
	virtual void unk4();
	virtual gsl::final_action<std::function<void(void)>> _tryBeginClientLegacyTransactionRequest();
};


SafetyHookInline _GameMode_buildBlock;

void Test(Container& container, int unkn, const ItemStack& item, const ItemStack& item2) {
    Log::Info("Test: {} {} {}", item.mItem->getRawNameId(), item2.mItem->getRawNameId(), unkn);
}

void Test2() {
	Log::Info("Test2"); 
}

class ItemDescriptor {
public:
    struct ItemEntry {
        const Item* mItem;
		short mAuxValue;
    };

    struct BaseDescriptor {
		virtual std::unique_ptr<BaseDescriptor> clone() const;
    };

    std::unique_ptr<BaseDescriptor> mImpl;

    virtual std::unique_ptr<BaseDescriptor> clone() const;

    ItemDescriptor(const ItemDescriptor& other) {
		mImpl = other.mImpl->clone();
    }

	ItemDescriptor& operator=(const ItemDescriptor& other) {
		if (this != &other) {
            if (other.mImpl) {
                mImpl = other.mImpl->clone();
            }
            else {
				mImpl.reset();
            }
		}

		return *this;
	}

    ItemDescriptor(ItemDescriptor&& other) noexcept = default;
	ItemDescriptor& operator=(ItemDescriptor&& other) noexcept = default;
	~ItemDescriptor() = default;

    ItemDescriptor(const Item& item, short auxValue) {
		
    }
};

class InternalItemDescriptor : ItemDescriptor::BaseDescriptor {
public:
    ItemDescriptor::ItemEntry mItemEntry;
};


void GameMode_buildBlock(GameMode* self, BlockPos* pos, FacingID face, bool isSimTick) {
    Player* player = self->mPlayer;
    PlayerInventory* playerInv = player->playerInventory;
	Inventory* inv = playerInv->mInventory.get();
    const ItemStack& mainItem = inv->getItem(playerInv->mSelected);

	// Get the held item in the offhand
    //ActorEquipmentComponent* equipment = player->tryGetComponent<ActorEquipmentComponent>();
    //ItemStack& offhandItem = equipment->mHand->mItems[1];

    // Todo: this should use more complex logic...
    // I.e. stick in mainhand, stone in offhand should use offhand since it has an action, while stick doesn't.
    //bool shouldUseOffhand = mainItem.isNull();
    //if (shouldUseOffhand && offhandItem.isNull()) return;

    // there should also be the logic with isSimTick here..

	//const ItemStack& itemToUse = shouldUseOffhand ? offhandItem : mainItem;
	const ItemStack& itemToUse = mainItem;
	//Log::Info("Using item: {}", itemToUse.mItem->getRawNameId());

    self->mPlayerLastPosition = *player->getPosition();

    std::unique_ptr<ItemUseInventoryTransaction> transaction = std::make_unique<ItemUseInventoryTransaction>();

    if (player && (player->mLevel == nullptr || (player->mLevel->isClientSide() && player->mItemStackNetManager != nullptr))) {
		player->mItemStackNetManager->_tryBeginClientLegacyTransactionRequest();
    }
    else {
        Log::Info("other not impl!");
		// TODO!
    }

    playerInv->createTransactionContext(
        [](Container&, int, const ItemStack&, const ItemStack) {
            Log::Info("callback called! todo impl");
        }, 
        [&transaction, &self, pos, face]() {
            Player* player = self->mPlayer;
            ILevel* level = player->mLevel;

			ItemStack selectedItemCopy = player->getSelectedItem();
            //ItemInstance itemInstance(selectedItemCopy);
            // NetworkItemStackDescriptor networkDescriptor(selectedItemCopy);
            
			HitResult hitResult = level->getHitResult();
			HitResult liquidHit = level->getLiquidHitResult();

			const BlockSource& region = player->getDimensionBlockSourceConst();
            const Block& block = region.getBlock(*pos);

            FacingID faceDir = face;
            BlockPos calculatedPlacePos = self->_calculatePlacePos(selectedItemCopy, *pos, faceDir);

            Log::Info("{} {}", calculatedPlacePos, faceDir);

            // todo: I think this is possibly only used for scripting...
            // maybe ignore for now
            // PlayerEventCoordinator* eventCoordinator = player->getPlayerEventCoordinator();
        }
    );
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

    hooks.RegisterFunction<&GameMode_buildBlock>("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 41 0F B6 F1");
	hooks.CreateHook<&GameMode_buildBlock>(_GameMode_buildBlock, &GameMode_buildBlock);
}
