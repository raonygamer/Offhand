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

extern "C" void* ItemUseInventoryTransaction_ctor = nullptr;

void OnRegisterItems(RegisterItemsEvent& event)
{
    for (auto& pair : event.itemRegistry.mNameToItemMap) {
		//Log::Info("Item ID: {:s}", pair.first.getString());
        pair.second.get()->setAllowOffhand(true);
    }
}

//class IGameModeTimer;
//class IGameModeMessenger;
//class SurvivalMode;

//enum class InventoryTransactionError : uint32_t
//{
//    Unknown = 0x0,
//    NoError = 0x1,
//    BalanceMismatch = 0x2,
//    SourceItemMismatch = 0x3,
//    InventoryMismatch = 0x4,
//    SizeMismatch = 0x5,
//    AuthorityMismatch = 0x6,
//    StateMismatch = 0x7,
//    ApiDenied = 0x8,
//};


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


bool GameMode_buildBlock(GameMode* self, BlockPos* pos, FacingID face, bool isSimTick) {
    Player& player = self->mPlayer;
	ILevel* level = player.mLevel;
    PlayerInventory* playerInv = player.playerInventory;
	Inventory* inv = playerInv->mInventory.get();

    const ItemStack& selectedItem = inv->getItem(playerInv->mSelected);
	const Item* item = selectedItem.getItem();

    // The game seems to check things multiple times, i.e. selectedItem.isNull() covers some of the other cases in here
    // but monkey see, monkey do, can always cleanup later
    bool mayUseItem = !selectedItem.mValid
        || !item
        || selectedItem.isNull()
        || selectedItem.mCount == 0
        || !isSimTick
        || item->canUseOnSimTick();

    if (!mayUseItem) return false;

    self->mPlayerLastPosition = *player.getPosition();
    std::unique_ptr<ItemUseInventoryTransaction> transaction = std::make_unique<ItemUseInventoryTransaction>();

    if (player.mLevel == nullptr || (player.mLevel->isClientSide() && player.mItemStackNetManager != nullptr)) {
		player.mItemStackNetManager->_tryBeginClientLegacyTransactionRequest();
    }
    else {
        Log::Info("other not impl!");
    }

    InteractionResult result;

    playerInv->createTransactionContext(
        [](Container&, int, const ItemStack&, const ItemStack) {
            Log::Info("callback called! todo impl");
        }, 
        [&transaction, &self, pos, face, &result]() {
            Log::Info("Execute!!");

            Player& player = self->mPlayer;
            ILevel* level = player.mLevel;

			ItemStack selectedItemCopy = player.getSelectedItem();
            //ItemInstance itemInstance(selectedItemCopy);
            // NetworkItemStackDescriptor networkDescriptor(selectedItemCopy);
            
			HitResult hitResult = level->getHitResult();
			HitResult liquidHit = level->getLiquidHitResult();

			const BlockSource& region = player.getDimensionBlockSourceConst();
            const Block& block = region.getBlock(*pos);

            FacingID faceDir = face;
            BlockPos calculatedPlacePos = self->_calculatePlacePos(selectedItemCopy, *pos, faceDir);

			// From the dissasembly, this will always run... because v83 and v74 are always false. sus
            bool v83 = false;
            bool v74 = false;
            if (!v83 || !v74 || self->mBuildContext.mHasBuildDirection) {
                transaction->mPos = *pos;
				transaction->setTargetBlock(block);
                transaction->mFace = faceDir;

                Vec3 clickPos;

                // I have no idea what this logic is is, IDA probably doesn't know what v83 is, and neither do I.
                if ((v83 - 2) <= 1) {
                    clickPos = Vec3::ZERO;
                }
                else {
                    clickPos = Vec3::ZERO; // THIS IS TEMP WRONG!!!!
                }
                
				transaction->mClickPos = clickPos;
                transaction->mFromPos = *player.getPosition();

                //if (block.isFenceBlock() && LeadItem::canBindPlayerMobs) {
                if (block.isFenceBlock() && false) {
                    result.mResult = InteractionResult::Result::UNKN_3;
                }
                else {
                    result = self->useItemOn(selectedItemCopy, *pos, face, calculatedPlacePos, nullptr);
                }
            }

            //transaction->

            // todo: I think this is possibly only used for scripting...
            // maybe ignore for now
            // PlayerEventCoordinator* eventCoordinator = player->getPlayerEventCoordinator();
        }
    );

    Log::Info("Result {}", (int)result.mResult);

    // info: the execute lambda of createTransactionContext is called before this function
    // crashes if there are no transactions.
    if (player.getLevel()->isClientSide()) {
        Log::Info("SEND!!");
		player.sendInventoryTransaction(transaction->mTransaction);
    }

    return true;
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

    hooks.RegisterFunction<&GameMode_buildBlock>("48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 41 0F B6 F1");
	hooks.CreateHook<&GameMode_buildBlock>(_GameMode_buildBlock, &GameMode_buildBlock);

    
}
