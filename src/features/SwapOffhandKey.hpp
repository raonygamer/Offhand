#pragma once
#include <amethyst/runtime/ModContext.hpp>
#include <amethyst/runtime/events/InputEvents.hpp>
#include <minecraft/src/common/network/packet/Packet.hpp>
#include <minecraft/src/common/network/PacketHandlerDispatcherInstance.hpp>
#include <minecraft/src/common/network/PacketSender.hpp>
#include <minecraft/src-client/common/client/player/LocalPlayer.hpp>
#include <minecraft/src/common/network/ServerNetworkHandler.hpp>
#include <minecraft/src/common/server/ServerPlayer.hpp>
#include <amethyst/Formatting.hpp>
#include <minecraft/src-deps/input/MouseDevice.hpp>
#include <minecraft/src/common/world/entity/components/ActorEquipmentComponent.hpp>
#include <minecraft/src/common/world/level/ILevel.hpp>
#include <minecraft/src/common/world/actor/player/Inventory.hpp>

class SwapOffhandPacket : public Packet {
public:
    virtual MinecraftPacketIds getId() const override {
        return (MinecraftPacketIds)((int)MinecraftPacketIds::EndId + 1);
	}

    virtual std::string getName() const override {
        return "SwapOffhandPacket";
    }

    virtual void write(BinaryStream& stream) override {
        Log::Info("[SwapOffhandPacket::write]");
		stream.writeString("SwapOffhand");
    }

    virtual Bedrock::Result<void, std::error_code> read(ReadOnlyBinaryStream& stream) override {
        Log::Info("[SwapOffhandPacket::read]");

        if (stream.getString().value() != "SwapOffhand") {
            Log::Info("SwapOffhandPacket: Invalid packet data");
            return Bedrock::Result<void, std::error_code>();
		}

        return Bedrock::Result<void, std::error_code>();
    }

    virtual Bedrock::Result<void, std::error_code> _read(ReadOnlyBinaryStream& stream) override {
		Log::Info("[SwapOffhandPacket::_read]");
        return Bedrock::Result<void, std::error_code>();
    }

    virtual ~SwapOffhandPacket() override {
		Packet::~Packet();
    }
};

template <>
class PacketHandlerDispatcherInstance<SwapOffhandPacket, false> : public IPacketHandlerDispatcher {
public:
    virtual void handle(const NetworkIdentifier& networkId, NetEventCallback& netEvent, std::shared_ptr<Packet>& packet) const override {
		Log::Info("PacketHandlerDispatcherInstance<SwapOffhandPacket>::handle");

        //SwapOffhandPacket& packet = *(SwapOffhandPacket*)_packet.get();
        ServerNetworkHandler& serverNetwork = (ServerNetworkHandler&)netEvent;

        uintptr_t** address = *(uintptr_t***)&serverNetwork;
		
        ServerPlayer* serverPlayer = serverNetwork._getServerPlayer(networkId, SubClientId::PrimaryClient);

        if (serverPlayer == nullptr) {
            Log::Info("SwapOffhandPacket: ServerPlayer not found?");
            return;
		}

		const PlayerInventory& inventory = serverPlayer->getSupplies();
		const ItemStack& mainhandStack = inventory.getSelectedItem();
		ItemStack mainhandCopy = mainhandStack;

        ActorEquipmentComponent& equipment = *serverPlayer->tryGetComponent<ActorEquipmentComponent>();
        const ItemStack& offhandStack = equipment.mHand->mItems[1];
        ItemStack offhandCopy = offhandStack;

        inventory.mInventory->createTransactionContext(
            [](Container& container, int slot, ItemStack const& from, ItemStack const& to) {
                
            },
            [&inventory, offhandCopy]() {
                inventory.mInventory->setItem(inventory.mSelected, offhandCopy);
            }
        );

        equipment.mHand->createTransactionContext(
            [](Container& container, int slot, ItemStack const& from, ItemStack const& to) {

            },
            [&equipment, mainhandCopy]() {
                equipment.mHand->setItem(1, mainhandCopy);
            }
        );
    }
};

static PacketHandlerDispatcherInstance<SwapOffhandPacket, false> swapOffhandPacketHandler;

SafetyHookInline _createPacket;

std::shared_ptr<Packet> createPacket(MinecraftPacketIds id) {
    //Log::Info("createPacket {}", (int)id);

    // Vanilla packets.
    if (id < MinecraftPacketIds::EndId) {
		std::shared_ptr<Packet> packet = _createPacket.call<std::shared_ptr<Packet>>(id);
		return packet;
    }

    // Custom packets
    if ((int)id == (int)MinecraftPacketIds::EndId + 1) {
        auto shared = std::make_shared<SwapOffhandPacket>();
        shared->mHandler = &swapOffhandPacketHandler;
        return shared;
    }
}

void RegisterKeyListener(RegisterInputsEvent& ev) {
	Amethyst::InputManager& inputManager = ev.inputManager;
	Amethyst::InputAction& swapOffhandKey = inputManager.RegisterNewInput("swap_input", { 'F' }, true, Amethyst::KeybindContext::Gameplay);

    swapOffhandKey.addButtonDownHandler([](FocusImpact focus, IClientInstance& client) {
        LocalPlayer& player = *client.getLocalPlayer();
		ILevel& level = *player.getLevel();
        PacketSender& packetSender = *level.getPacketSender();

        SwapOffhandPacket packet = SwapOffhandPacket();
        packetSender.sendToServer(packet);

		// Swap items clientside for instant feedback
        const PlayerInventory& inventory = player.getSupplies();
        const ItemStack& mainhandStack = inventory.getSelectedItem();
		ItemStack mainhandCopy = mainhandStack;

        const ActorEquipmentComponent& equipment = *player.tryGetComponent<ActorEquipmentComponent>();
        const ItemStack& offhandStack = equipment.mHand->mItems[1];
		ItemStack offhandCopy = offhandStack;

        inventory.mInventory->createTransactionContext(
            [](Container& container, int slot, ItemStack const& from, ItemStack const& to) {

            },
            [&inventory, offhandCopy]() {
                inventory.mInventory->setItem(inventory.mSelected, offhandCopy);
            }
        );

        equipment.mHand->createTransactionContext(
            [](Container& container, int slot, ItemStack const& from, ItemStack const& to) {

            },
            [&equipment, mainhandCopy]() {
                equipment.mHand->setItem(1, mainhandCopy);
            }
        );
    });
}

SafetyHookInline _allowIncomingPacketId;

bool allowIncomingPacketId(ServerNetworkHandler* self, const NetworkIdentifier& networkId, MinecraftPacketIds packet) {
    if ((int)packet == (int)MinecraftPacketIds::EndId + 1) {
		Log::Info("allowIncomingPacketId: SwapOffhandPacket");
        return true;
	}

	return _allowIncomingPacketId.call<bool, ServerNetworkHandler*, const NetworkIdentifier&, MinecraftPacketIds>(self, networkId, packet);
}

void RegisterSwapOffhandKey() {
    Amethyst::HookManager& hooks = Amethyst::GetHookManager();
	Amethyst::EventBus& events = Amethyst::GetEventBus();

	events.AddListener<RegisterInputsEvent>(&RegisterKeyListener);

    hooks.RegisterFunction<&MinecraftPackets::createPacket>("40 53 48 83 EC ? 45 33 C0 48 8B D9 FF CA 81 FA");
    hooks.CreateHook<&MinecraftPackets::createPacket>(_createPacket, &createPacket);

    hooks.RegisterFunction<&allowIncomingPacketId>("48 89 5C 24 ? 48 89 6C 24 ? 56 57 41 56 48 83 EC ? 41 8B D8 48 8B F2");
	hooks.CreateHook<&allowIncomingPacketId>(_allowIncomingPacketId, &allowIncomingPacketId);

 //   hooks.RegisterFunction<&allocate>("B8 ? ? ? ? 48 3B D0 48 0F 42 D0 48 8B CA");
	//hooks.CreateHook<&allocate>(_allocate, &allocate);

	//hooks.RegisterFunction<&release>("48 8B CA 48 FF 25 ? ? ? ? CC CC CC CC CC CC 40 53 48 83 EC ? 48 8B D9");
	//hooks.CreateHook<&release>(_release, &release);

    int flags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    flags |= _CRTDBG_ALLOC_MEM_DF |    // Keep track of all allocations
        _CRTDBG_CHECK_ALWAYS_DF | // Check heap on every allocation/free
        _CRTDBG_LEAK_CHECK_DF;    // Dump leaks at exit
    _CrtSetDbgFlag(flags);
}