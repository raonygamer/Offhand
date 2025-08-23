import { createFile, Custom, UiFile, createMinecraftElement, Panel } from "Regolith-Generators"

const hud_screen = new UiFile("hud");

hud_screen.addControl("offhand_renderer", 
    <Custom renderer="offhand_hud_renderer" size={[20, 20]} anchors="center" />
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