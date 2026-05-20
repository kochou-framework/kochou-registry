set_xmakever("3.0.0")
set_project("kochou_registry_rcu_fence_frame")
set_languages("c++23")

add_rules("mode.debug", "mode.release")

includes("../third_party/ktl")
includes("../third_party/kochou")

target("kochou_registry_rcu_fence_frame")
    add_deps("ktl")
    add_deps("kochou")

    set_kind("static")

    add_files(
        "src/rcu_fence_frame.cpp"
    )

    add_includedirs("include", {public = true})
