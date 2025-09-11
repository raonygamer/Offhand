-- Mod Options
local mod_name = "Offhand"
local mod_version = "1.0.7"

-- Minecraft version
local major, minor, patch = 1, 21, 3

option("automated_build")
    set_default(false)
    set_showmenu(true)
    set_description("Flag to indicate this is an automated build")
option_end()

set_languages("c++23")
set_project(mod_name)
set_version(string.format("%d.%d.%d", major, minor, patch))

local automated = is_config("automated_build", true)
local modFolder
local amethystApiPath

if automated then
    print("==> Automated build mode")
    modFolder = path.join(os.projectdir(), "dist")
    amethystApiPath = path.join(os.projectdir(), "Amethyst", "AmethystAPI")
else
    print("==> Real/local build mode")
    set_symbols("debug")
    local amethystSrc = os.getenv("AMETHYST_SRC")
    amethystApiPath = amethystSrc and path.join(amethystSrc, "AmethystAPI") or nil

    local amethystFolder = path.join(
        os.getenv("localappdata"),
        "Packages",
        "Microsoft.MinecraftUWP_8wekyb3d8bbwe",
        "LocalState",
        "games",
        "com.mojang",
        "amethyst"
    )

    modFolder = path.join(
        amethystFolder,
        "mods",
        string.format("%s@%s", mod_name, mod_version)
    )
end

-- Only include AmethystAPI if present on disk at configure-time
if amethystApiPath and os.isdir(amethystApiPath) then
    includes(amethystApiPath)
    includes(path.join(amethystApiPath, "packages", "libhat"))
end

-- RelWithDebInfo flags
add_cxxflags("/O2", "/DNDEBUG", "/MD", "/EHsc", "/FS", "/MP")
add_ldflags("/OPT:REF", "/OPT:ICF", "/INCREMENTAL:NO", {force = true})

set_targetdir(modFolder)
set_toolchains("msvc", {asm = "nasm"})

target(mod_name)
    set_kind("shared")
    add_deps("AmethystAPI", "libhat")

    -- Hard fail if AmethystAPI is missing
    on_load(function (t)
        if not (amethystApiPath and os.isdir(amethystApiPath)) then
            raise("AmethystAPI not found at: " .. tostring(amethystApiPath) ..
                  "\nCI: ensure repo is checked out to Amethyst/AmethystAPI" ..
                  "\nLocal: set AMETHYST_SRC to point to your Amethyst clone.")
        end
    end)
    
    -- Force rebuild when any source file changes
    set_policy("build.optimization.lto", true )
    set_policy("build.across_targets_in_parallel", true )

    add_files("src/**.cpp")

    add_files("src/**.asm")
        set_toolset("as", "nasm")
        add_asflags("-f win64", { force = true })

    add_defines(
        string.format('MOD_VERSION="%d.%d.%d"', major, minor, patch),
        string.format('MOD_TARGET_VERSION_MAJOR=%d', major),
        string.format('MOD_TARGET_VERSION_MINOR=%d', minor),
        string.format('MOD_TARGET_VERSION_PATCH=%d', patch),
        'ENTT_PACKED_PAGE=128',
        'AMETHYST_EXPORTS'
    )

    -- Deps
    add_packages("AmethystAPI", "libhat")
    add_links("user32", "oleaut32", "windowsapp")

    add_includedirs("src", {public = true})
    add_headerfiles("src/**.hpp")

    after_build(function (target)
        local src_json = path.join("mod.json")
        local dst_json = path.join(modFolder, "mod.json")
        if not os.isdir(modFolder) then
            os.mkdir(modFolder)
        end
        os.cp(src_json, dst_json)
    end)