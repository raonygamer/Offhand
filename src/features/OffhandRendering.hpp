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

float t = 0.0f;

void _transformOffhandItem(ItemInHandRenderer* self, MatrixStack::MatrixStackRef& matrixStack) {
	ClientInstance& client = *Amethyst::GetContext().mClientInstance;
	LocalPlayer* player = client.getLocalPlayer();
	if (!player) return;

	ActorEquipmentComponent* equipment = player->tryGetComponent<ActorEquipmentComponent>();
	if (!equipment || equipment->mHand->mItems.size() <= 1) return;
	const ItemStack& offhand = equipment->mHand->mItems[1];

	if (offhand.isNull() || !offhand.isBlock() || offhand.getItem()->isBlockPlanterItem() || !self->_canTessellateAsBlockItem(offhand)) {
		_ItemInHandRenderer_transformOffhandItem.call<void, ItemInHandRenderer*, MatrixStack::MatrixStackRef&>(self, matrixStack);
		return;
	}

	float depth = -1.8f;   // depth
	float side = -1.5f;   // left
	float height = -1.3f;  // down

	// Apply translation
	matrixStack->translate(side, height, depth);
	//matrixStack->rotate(glm::radians(45.f), 0.0f, 1.0f, 0.0f); // rotate

	//t += 0.1f;
	float attackAnim = player->getAttackAnim(t);

	float sqrt = mce::Math::sqrt(attackAnim);
	float rotSin = mce::Math::sin(attackAnim * attackAnim * 3.14159f);
	float posSin = mce::Math::sin(sqrt * 3.14159f);

	float sinAnimSqrtTimesPi = mce::Math::sin(sqrt * 3.14159f);

	attackAnim = mce::Math::sqrt(attackAnim);
	attackAnim = mce::Math::sin(attackAnim * 3.14159f + attackAnim * 3.14159f);

	matrixStack->rotate(45.0f, 0.0f, 1.0f, 0.0f);
	matrixStack->translate(sinAnimSqrtTimesPi * 0.3f, attackAnim * 0.4f, posSin * -0.4f);
	matrixStack->rotate(sinAnimSqrtTimesPi * 70.f, 0.0f, 1.0f, 0.0f);
	matrixStack->rotate(rotSin * -20.f, 0.0f, 0.0f, 1.0f);

	//mce::Math::sqrt();
	//matrixStack->rotate()
}

void RegisterOffhandRendering() {
	Amethyst::HookManager& hooks = Amethyst::GetHookManager();

	//hooks.RegisterFunction<&ItemInHandRenderer_renderOffhandItem>("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 44 88 4D");
	//hooks.CreateHook<&ItemInHandRenderer_renderOffhandItem>(_ItemInHandRenderer_renderOffhandItem, &ItemInHandRenderer_renderOffhandItem);

	hooks.RegisterFunction<&_transformOffhandItem>("40 53 48 83 EC ? 48 8B 02 0F 57 DB");
	hooks.CreateHook<&_transformOffhandItem>(_ItemInHandRenderer_transformOffhandItem, &_transformOffhandItem);
}