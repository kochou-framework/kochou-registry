set_xmakever("3.0.0")
set_project("kochou_registry_dynamic_rendering")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

includes("../third_party/ktl")
includes("../third_party/kochou")
includes("../dynamic_pipeline")

target("kochou_registry_dynamic_rendering")
    add_deps("ktl")
    add_deps("kochou")
    add_deps("kochou_registry_dynamic_pipeline")

    set_kind("static")

    add_files(
        "src/dynamic_rendering.cpp"
    )

    add_includedirs("include", {public = true})
