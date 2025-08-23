#include "OffhandHud.hpp"
#include <amethyst/runtime/ModContext.hpp>
#include <minecraft/src-client/common/client/gui/UIResolvedDef.hpp>
#include <minecraft/src-client/common/client/gui/gui/UIControl.hpp>
#include <minecraft/src-client/common/client/gui/controls/CustomRenderComponent.hpp>
#include <minecraft/src-client/common/client/gui/controls/renderers/MinecraftUICustomRenderer.hpp>
#include <minecraft/src-deps/core/math/Color.hpp>
#include <amethyst/Formatting.hpp>
#include <minecraft/src-client/common/client/player/LocalPlayer.hpp>
#include <minecraft/src/common/world/entity/components/ActorEquipmentComponent.hpp>
#include <amethyst/Formatting.hpp>
#include <minecraft/src-client/common/client/renderer/BaseActorRenderContext.hpp>
#include <minecraft/src-client/common/client/renderer/screen/ScreenContext.hpp>
#include <minecraft/src-client/common/client/renderer/actor/ItemRenderer.hpp>

class UIResolvedDef;
class UIControl;
class UIControlFactory;

SafetyHookInline _UIControlFactory_populateCustomRenderComponent;

static HashedString flushString(0xA99285D21E94FC80, "ui_flush");

class TestRenderer : public MinecraftUICustomRenderer {
public:
	TestRenderer() : MinecraftUICustomRenderer() {};

	virtual std::shared_ptr<UICustomRenderer> clone() const override {
		return std::make_shared<TestRenderer>();
	}

	virtual void render(MinecraftUIRenderContext& ctx, IClientInstance& client, UIControl& owner, int32_t pass, RectangleArea& renderAABB) override {
		LocalPlayer* player = client.getLocalPlayer();
		if (!player) return;

		ActorEquipmentComponent* equipment = player->tryGetComponent<ActorEquipmentComponent>();
		if (!equipment || equipment->mHand->mItems.size() <= 1) return;

		const ItemStack& offhand = equipment->mHand->mItems[1];
		if (offhand.isNull()) return;

		glm::tvec2<float> pos = owner.getPosition();
		renderAABB = RectangleArea(pos.x + 5.0f, pos.x + 22.0f, pos.y + 12.0f, pos.y + 22.0f);

		BaseActorRenderContext renderCtx(ctx.mScreenContext, &client, client.minecraftGame);
		renderCtx.itemRenderer->renderGuiItemNew(&renderCtx, &offhand, 0, pos.x + 2.0f, pos.y + 6.0f, false, 1.0f, mPropagatedAlpha, 1.0f);
		ctx.flushImages(mce::Color::WHITE, 1.0f, flushString);

		TextMeasureData textData;
		memset(&textData, 0, sizeof(TextMeasureData));
		textData.fontSize = 1.0f;

		CaretMeasureData caretData;
		memset(&caretData, 1, sizeof(CaretMeasureData));

		std::string text = std::format("{}", offhand.mCount);

		ctx.

		ctx.drawDebugText(&renderAABB, &text, &mce::Color::WHITE, 1.0f, ui::Right, &textData, &caretData);
	}
};

void UIControlFactory_populateCustomRenderComponent(UIControlFactory* self, const UIResolvedDef& resolved, UIControl& control) {
	std::string rendererType = resolved.getAsString("renderer");
	// Log::Info("populateCustomRenderer {}", rendererType);

	if (rendererType == "offhand_hud_renderer") {
		Log::Info("Making offhand_hud_renderer!");

		control.setComponent<CustomRenderComponent>(
			std::make_unique<CustomRenderComponent>(control)
		);

		CustomRenderComponent* component = control.getComponent<CustomRenderComponent>();
		Log::Info("0x{:x}", (uintptr_t)component);

		component->setRenderer(std::make_shared<TestRenderer>());

		return;
	}

	_UIControlFactory_populateCustomRenderComponent.call<void, UIControlFactory*, const UIResolvedDef&, UIControl&>(self, resolved, control);
}

void RegisterOffhandHud()
{
    Amethyst::HookManager& hooks = Amethyst::GetHookManager();

    hooks.RegisterFunction<&UIControlFactory_populateCustomRenderComponent>("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 85 ? ? ? ? 4D 8B E0 4C 8B FA 4C 8B E9 49 8B D0");
	hooks.CreateHook<&UIControlFactory_populateCustomRenderComponent>(_UIControlFactory_populateCustomRenderComponent, &UIControlFactory_populateCustomRenderComponent);
} 