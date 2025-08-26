#pragma once
#include <amethyst/runtime/ModContext.hpp>
#include <minecraft/src-client/common/client/renderer/game/ItemInHandRenderer.hpp>
#include <minecraft/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <minecraft/src-client/common/client/world/item/ItemIconManager.hpp>

//SafetyHookInline _ItemInHandRenderer_renderOffhandItem;
//
//void ItemInHandRenderer_renderOffhandItem(ItemInHandRenderer* self, BaseActorRenderContext& ctx, Player& player, ItemContextFlags flags) {
//	//Log::Info("ItemInHandRenderer_rednerOffhandItem {}", (int)flags);
//	_ItemInHandRenderer_renderOffhandItem.call<void, ItemInHandRenderer*, BaseActorRenderContext&, Player&, ItemContextFlags>(self, ctx, player, flags);
//}

SafetyHookInline _ItemInHandRenderer_transformOffhandItem;

void _transformOffhandItem(ItemInHandRenderer* self, MatrixStack::MatrixStackRef& matrixStack) {
	ClientInstance& client = *Amethyst::GetContext().mClientInstance;
	LocalPlayer* player = client.getLocalPlayer();
	if (!player) return;

	ActorEquipmentComponent* equipment = player->tryGetComponent<ActorEquipmentComponent>();
	if (!equipment || equipment->mHand->mItems.size() <= 1) return;
	const ItemStack& offhand = equipment->mHand->mItems[1];

	// todo: this shouldn't apply for "item" block items

	if (offhand.isNull() || !offhand.isBlock() || offhand.getItem()->isBlockPlanterItem() || !self->_canTessellateAsBlockItem(offhand)) {
		_ItemInHandRenderer_transformOffhandItem.call<void, ItemInHandRenderer*, MatrixStack::MatrixStackRef&>(self, matrixStack);
		return;
	}

	//ResolvedItemIconInfo iconInfo = offhand.getItem()->getIconInfo(offhand, 0, false);
	//TextureAtlasItem* atlasItem = ItemIconManager::getIcon(iconInfo);

	//Log::Info("0x{:x}", (uint64_t)atlasItem);

	//Log::Info("atlas {:s}", offhand.getItem()->mTextureAtlasFile);

	//Log::Info("{}", atlasItem->mName);

	//if (iconInfo.mIconType != ItemIconInfoType::LegacyBlock) {
	//	_ItemInHandRenderer_transformOffhandItem.call<void, ItemInHandRenderer*, MatrixStack::MatrixStackRef&>(self, matrixStack);
	//	return;
	//}

	float depth = -1.8f;   // depth
	float side = -1.5f;   // left
	float height = -1.3f;  // down

	// Apply translation
	matrixStack->translate(side, height, depth);
	matrixStack->rotate(45.f, 0.0f, 1.0f, 0.0f); // rotate
}

void RegisterOffhandRendering() {
	Amethyst::HookManager& hooks = Amethyst::GetHookManager();

	//hooks.RegisterFunction<&ItemInHandRenderer_renderOffhandItem>("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 44 88 4D");
	//hooks.CreateHook<&ItemInHandRenderer_renderOffhandItem>(_ItemInHandRenderer_renderOffhandItem, &ItemInHandRenderer_renderOffhandItem);

	hooks.RegisterFunction<&_transformOffhandItem>("40 53 48 83 EC ? 48 8B 02 0F 57 DB");
	hooks.CreateHook<&_transformOffhandItem>(_ItemInHandRenderer_transformOffhandItem, &_transformOffhandItem);
}