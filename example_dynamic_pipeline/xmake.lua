set_xmakever("3.0.0")
set_project("kochou_registry_example_dynamic_pipeline")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

includes("../third_party/ktl")
includes("../third_party/kochou")

target("kochou_registry_example_dynamic_pipeline")
    add_deps("ktl")
    add_deps("kochou")

    set_kind("static")

    add_files(
        "src/example_dynamic_pipeline.cpp"
    )

    add_includedirs("include", {public = true})
