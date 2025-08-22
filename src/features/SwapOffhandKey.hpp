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
        stream.writeUnsignedVarInt32(0); // game shits itself if the packet has nothing so add some shit
    }

    virtual Bedrock::Result<void, std::error_code> read(ReadOnlyBinaryStream& stream) override {
        Log::Info("[SwapOffhandPacket::read]");
        return Bedrock::Result<void, std::error_code>();
    }

    virtual Bedrock::Result<void, std::error_code> _read(ReadOnlyBinaryStream& stream) override {
        return Bedrock::Result<void, std::error_code>();
    }
};

template <>
class PacketHandlerDispatcherInstance<SwapOffhandPacket, false> : public IPacketHandlerDispatcher {
public:
    virtual void handle(const NetworkIdentifier& networkId, NetEventCallback& netEvent, std::shared_ptr<Packet> _packet) const {
        SwapOffhandPacket& packet = *(SwapOffhandPacket*)_packet.get();
        ServerNetworkHandler& serverNetwork = (ServerNetworkHandler&)netEvent;

        Log::Info("handle!");

        uintptr_t** address = *(uintptr_t***)&serverNetwork;
		Log::Info("ServerNetworkHandler address: 0x{:x}", (uintptr_t)address - GetMinecraftBaseAddress());
		
        ServerPlayer* serverPlayer = serverNetwork._getServerPlayer(networkId, SubClientId::PrimaryClient);

        if (serverPlayer == nullptr) {
            Log::Info("SwapOffhandPacket: ServerPlayer not found?");
            return;
		}

		const PlayerInventory& inventory = serverPlayer->getSupplies();
		const ItemStack& mainhandStack = inventory.getSelectedItem();

        ActorEquipmentComponent& equipment = *serverPlayer->tryGetComponent<ActorEquipmentComponent>();
        const ItemStack& offhandStack = equipment.mHand->mItems[1];

		Log::Info("mainhand: {}, offhand: {}", mainhandStack, offhandStack);

        //Log::Info("pos {}", *serverPlayer->getPosition());
    }
};

static PacketHandlerDispatcherInstance<SwapOffhandPacket, false> swapOffhandPacketHandler;

SafetyHookInline _createPacket;

std::shared_ptr<Packet> createPacket(MinecraftPacketIds id) {
    //Log::Info("createPacket {}", (int)id);

    // Vanilla packets.
    if (id <= MinecraftPacketIds::EndId) {
        return _createPacket.call<std::shared_ptr<Packet>>(id);
    }

    // Custom packets
    if ((int)id == (int)MinecraftPacketIds::EndId + 1) {
        Log::Info("createPacket made SwapOffhandPacket");
        auto shared = std::make_shared<SwapOffhandPacket>();
        shared->mHandler = &swapOffhandPacketHandler;
        return shared;
    }

    //Assert("Recieved packet with unknown id: {}", (int)id);
}

void RegisterKeyListener(RegisterInputsEvent& ev) {
    Log::Info("Registering swap offhand key");

    ev.inputManager.RegisterNewInput("swap_offhand", { 70 }, true);

    ev.inputManager.AddButtonDownHandler("swap_offhand", [](FocusImpact focus, IClientInstance& client) {
        LocalPlayer& player = *client.getLocalPlayer();
        ILevel& level = *player.getLevel();

        PacketSender& packetSender = *level.getPacketSender();

        SwapOffhandPacket swapOffhandPacket;
        packetSender.sendToServer(swapOffhandPacket);
        Log::Info("Sent SwapOffhandPacket to server");

    }, true);
}

void RegisterSwapOffhandKey() {
    Amethyst::HookManager& hooks = Amethyst::GetHookManager();
	Amethyst::EventBus& events = Amethyst::GetEventBus();

	//events.AddListener<RegisterInputsEvent>(&RegisterKeyListener);

    hooks.RegisterFunction<&MinecraftPackets::createPacket>("40 53 48 83 EC ? 45 33 C0 48 8B D9 FF CA 81 FA");
    hooks.CreateHook<&MinecraftPackets::createPacket>(_createPacket, &createPacket);

	Amethyst::PatchManager& patches = Amethyst::GetPatchManager();

    // Need to patch the max packet ID check in ServerNetworkHandler::allowIncomingPacketId
    // original instruction - cmp r8d, 136h
    uintptr_t maxPacketCheck = GetMinecraftBaseAddress() + 0x1737C2B;
    uint8_t patch[7] = { 
        0x41, 0x81, 0xF8, // cmp r8d
        0x38, 0x01, 0, 0 // new max (312 in hex (0x0138), then reversed order)
    };

    patches.ApplyPatch(maxPacketCheck, patch, sizeof(patch));
}