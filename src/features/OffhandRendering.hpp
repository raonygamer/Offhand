#pragma once
#include <amethyst/runtime/ModContext.hpp>
#include <minecraft/src-client/common/client/renderer/game/ItemInHandRenderer.hpp>
#include <minecraft/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <minecraft/src-client/common/client/world/item/ItemIconManager.hpp>
#include <minecraft/src/common/util/Timer.hpp>
#include <minecraft/src/common/Minecraft.hpp>
#include <minecraft/src-deps/core/math/Math.hpp>

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

	float interpolatedTick = Amethyst::GetContext().mClientMinecraft->mSimTimer.mAlpha;

	float depth = -1.8f;   // depth
	float side = -1.5f;   // left
	float height = -1.3f;  // down

	float v12 = ((self->mHeight - self->mOldHeight) * interpolatedTick) + self->mOldHeight;
	Log::Info("v12 {}", v12);

	// Apply translation
	//

	float attackAnim = player->getAttackAnim(interpolatedTick);
	float alsoAttackAnim = attackAnim;
	float attackAnimSqrt = mce::Math::sqrt(attackAnim);
	float rotSin = mce::Math::sin((alsoAttackAnim * alsoAttackAnim) * 3.1415927);
	float posSin = mce::Math::sin(alsoAttackAnim * 3.1415927);
	float sinAnimSqrtTimesPi = mce::Math::sin(attackAnimSqrt * 3.1415927);

	attackAnim = mce::Math::sqrt(alsoAttackAnim);
	attackAnim = mce::Math::sin((*&attackAnim * 3.1415927) + (*&attackAnim * 3.1415927));

	// original - mainhand
	/*matrixStack->translate(sinAnimSqrtTimesPi * -0.30000001, attackAnim * 0.40000001, posSin * -0.40000001);

	float v275 = (1.0 - v12) * 0.60000002;
	Matrix::translate(v274, 0.64000005, -0.60000002 - v275, -0.71999997);


	matrixStack->rotate(45.0f, 0.0f, 1.0f, 0.0f);

	matrixStack->rotate(sinAnimSqrtTimesPi * 70.0f, 0.0f, 1.0f, 0.0f);
	matrixStack->rotate(rotSin * -20.0f, 1.0f, 0.0f, 0.0f);*/

	matrixStack->translate(sinAnimSqrtTimesPi * 0.30000001, attackAnim * 0.40000001, posSin * -0.40000001);

	float v275 = (v12) * 0.60000002;
	matrixStack->translate(side, height - v275, depth);

	matrixStack->rotate(45.0f, 0.0f, 1.0f, 0.0f);

	matrixStack->rotate(sinAnimSqrtTimesPi * 70.0f, 0.0f, -1.0f, 0.0f);
	matrixStack->rotate(rotSin * -20.0f, -1.0f, 0.0f, 0.0f);
}

void RegisterOffhandRendering() {
	Amethyst::HookManager& hooks = Amethyst::GetHookManager();

	//hooks.RegisterFunction<&ItemInHandRenderer_renderOffhandItem>("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 0F 29 B4 24 ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 44 88 4D");
	//hooks.CreateHook<&ItemInHandRenderer_renderOffhandItem>(_ItemInHandRenderer_renderOffhandItem, &ItemInHandRenderer_renderOffhandItem);

	hooks.RegisterFunction<&_transformOffhandItem>("40 53 48 83 EC ? 48 8B 02 0F 57 DB");
	hooks.CreateHook<&_transformOffhandItem>(_ItemInHandRenderer_transformOffhandItem, &_transformOffhandItem);
}