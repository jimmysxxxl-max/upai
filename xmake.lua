set_xmakever("3.0.0")

includes("lib/commonlibf4")

set_project("UniquePlayerRedirector")
set_version("0.2.5")
set_license("GPL-3.0-or-later")
set_arch("x64")
set_languages("c++23")
set_warnings("allextra")

add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

target("UniquePlayerRedirector")
    add_rules("commonlibf4.plugin", {
        name = "UniquePlayerRedirector",
        author = "UniquePlayerRedirector contributors",
        description = "ESP-less player-only body, hands and face texture redirector"
    })

    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")
