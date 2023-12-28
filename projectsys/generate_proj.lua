project ""

    location "generated/"
    kind "SharedLib"
    language "C#"
    dotnetframework "4.7.2"

    targetdir "Bin\\%{cfg.buildcfg}"

    files {
        "**.cs"
    }

    links {
        "Aurora-ScriptCore"
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"
        debugdir "./"
        staticruntime "off"
        runtime "Debug"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        staticruntime "off"
        runtime "Release"