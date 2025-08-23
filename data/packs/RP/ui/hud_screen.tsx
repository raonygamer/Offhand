import { createFile, UiFile, createMinecraftElement, Panel, GetRef, Variable, Size2D, ImageProps, Image, Custom } from "Regolith-Generators"

const hud_screen = new UiFile("hud");

const HotbarSlotImage = GetRef<{ "$hotbar_slot_image_size": Variable<Size2D> } & ImageProps>("hud", "hotbar_slot_image");

hud_screen.addControl("offhand_renderer",
    <Image 
        texture="textures/ui/hotbar_0" 
        layer={1}
        size="$hotbar_renderer_size"
        offset={[-105, -1.5]}
        anchors="bottom_middle"
        defaults={{
            "$hotbar_renderer_size": [20, 22],
        }}
    >
        <Custom
            renderer="offhand_hud_renderer"
            size="$hotbar_renderer_size"
            layer={2}
        />
    </Image>
)

hud_screen.addControl("root_panel",
    <Panel modifications={[
        {
            operation: "insert_front",
            array_name: "controls",
            value: [
                { "offhand_renderer@hud.offhand_renderer": {} }
            ]
        }
    ]} />
)

createFile(hud_screen.toJson());