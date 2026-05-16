set_xmakever("3.0.0")
set_project("kochou-registry-template")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

includes("../third_party/ktl")
includes("../third_party/kochou")

target("kokochou-registry-templatechou")
    add_deps("ktl")
    add_deps("kochou")

    set_kind("static")

    add_files(
        "src/template.cpp"
    )

    add_includedirs("include", {public = true})
