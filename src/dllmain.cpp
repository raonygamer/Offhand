#include "dllmain.hpp"
#include <amethyst/runtime/events/RegisterEvents.hpp>
#include <minecraft/src/common/world/item/registry/ItemRegistry.hpp>
#include <minecraft/src/common/world/actor/player/Player.hpp>
#include <minecraft/src/common/world/actor/player/PlayerInventory.hpp>
#include <minecraft/src/common/world/actor/player/Inventory.hpp>
#include <minecraft/src/common/world/level/BlockPos.hpp>
#include <minecraft/src/common/world/entity/components/ActorEquipmentComponent.hpp>

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
};

class SurvivalMode;

//SafetyHookInline _SurvivalMode_useItemOn;

//InteractionResult* SurvivalMode_useItemOn(SurvivalMode* a1, InteractionResult* result, ItemStackBase* a3, BlockPos* a4, char a5, Vec3* a6, Block* a7) {
//	Log::Info("SurvivalMode_useItemOn {}", offsetof(Player, playerInventory));
//    _SurvivalMode_useItemOn.thiscall<InteractionResult*>(a1, result, a3, a4, a5, a6, a7);;
//    return result;
//}


SafetyHookInline _GameMode_buildBlock;

void GameMode_buildBlock(GameMode* self, BlockPos* pos, uint8_t face, bool isSimTick) {
    Player* player = self->mPlayer;
    PlayerInventory* inventory = player->playerInventory;
	Inventory* inv = inventory->mInventory.get();

    const ItemStack& mainItem = inv->getItem(inventory->mSelected);

    ActorEquipmentComponent* equipment = player->tryGetComponent<ActorEquipmentComponent>();

    ItemStack& offhandItem = equipment->mHand->mItems[1];

    if (mainItem.mItem != nullptr) {
        Log::Info("Mainhand holding: {}", mainItem.mItem->getRawNameId());
    }
    else {
        Log::Info("Mainhand holding nothing");
    }

    if (offhandItem.mItem != nullptr) {
        Log::Info("Offhand holding: {}", offhandItem.mItem->getRawNameId());
    }
    else {
        Log::Info("Offhand holding nothing");
    }
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
