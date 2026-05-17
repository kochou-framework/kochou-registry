set_xmakever("3.0.0")
set_project("kochou-registry-metal_window")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

includes("../third_party/ktl")
includes("../third_party/kochou")

target("kochou-registry-metal_window")
    add_deps("ktl")
    add_deps("kochou")

    set_kind("static")

    local platform = os.host()
    if platform == "macosx" then
        add_frameworks("Cocoa", "QuartzCore", "Metal", "MetalKit")
        add_files("src/metal_window.mm")
    else
        add_files("src/metal_window.cpp")
    end

    add_includedirs("include", {public = true})
