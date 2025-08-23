#include "OffhandHud.hpp"
#include <amethyst/runtime/ModContext.hpp>
#include <minecraft/src-client/common/client/gui/UIResolvedDef.hpp>
#include <minecraft/src-client/common/client/gui/gui/UIControl.hpp>
#include <minecraft/src-client/common/client/gui/controls/CustomRenderComponent.hpp>
#include <minecraft/src-deps/core/math/Color.hpp>
#include <amethyst/Formatting.hpp>

class UIResolvedDef;
class UIControl;
class UIControlFactory;

SafetyHookInline _UIControlFactory_populateCustomRenderComponent;

class TestRenderer : public UICustomRenderer {
public:
	TestRenderer() : UICustomRenderer() {};

	virtual std::shared_ptr<UICustomRenderer> clone() const override {
		return std::make_shared<TestRenderer>();
	}

	virtual void frameUpdate(UIFrameUpdateContext& frameCtx, UIControl& control) {

	};

	virtual void render(UIRenderContext& renderCtx, IClientInstance& client, UIControl& control, int32_t unkn, RectangleArea& area) {
		// calls the virtual that comes after frameUpdate
		MinecraftUIRenderContext& ctx = (MinecraftUIRenderContext&)(renderCtx);

		RectangleArea newArea{ 0.0f, 20.0f, 0.0f, 20.0f };

		ctx.drawRectangle(area, mce::Color::WHITE, 1.0f, 1);

		Log::Info("area {}", area);
		area = newArea;

		//Log::Info("render");
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