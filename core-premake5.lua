project "AuroraRuntime"

    --os.rmdir("generated\\")

   kind "SharedLib"
   language "C++"
   cppdialect "C++17"
   targetdir "bin\\%{cfg.buildcfg}"
   location "generated\\"
   compileas "C++"
   staticruntime "on"

   defines {"GRAPHICS_OPENGL", 'MONO_HOME="C:/Program Files/Mono/"'}

   files { "include/**.h", "include/**.hpp", "include/**.cpp", "include/**.c","src/**.h", "src/**.hpp" }
   removefiles {"vendor/**"}
   removefiles { "include/auroraengine/**"}
   --files {"include/bullet3/src/**.hpp", "include/bullet3/src/**.h"}



   files {"resources\\**", "**.json", "lib\\**"}

   includedirs {"include\\", "include\\aurora\\", "vcpkg\\installed\\x64-windows\\include\\", "vcpkg\\installed\\x64-windows\\include\\bullet", "C:/Program Files/Mono/include/mono-2.0", "C:/physx/include"}

   links {
        "glfw3_mt.lib","glew32.dll","glew32.lib","opengl32.lib","OpenAL32.lib","freetype.lib","assimp-vc143-mt.lib","mono-2.0-sgen.lib",
    
    }

    --filter { 'system:windows' }
        --postbuildcommands { "copy C:/Program Files/Mono/bin/mono-2.0-sgen.dll bin\\%{cfg.buildcfg}\\mono-2.0-sgen.dll" }

    libdirs {
        "lib\\",
        "lib\\glfw\\lib-vc2022",
        "C:/Program Files/Mono/lib"
    }
            --for 32 bit use these library paths
    filter "architecture:x86"
    libdirs { 
        ".\\lib\\glew\\win32\\"
        }

    filter { }
    --for x64 use these
    filter "architecture:x64"
    libdirs { 
        ".\\lib\\glew\\x64\\",
        ".\\lib\\assimp\\x64\\"
    }

    dependson {"Aurora-ScriptCore"}

   filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        optimize "Off"
        debugdir "./"
        staticruntime "off"
        runtime "Debug"
        libdirs {"C:/physx/bin/win.x86_64.vc143.mt/debug/"}

   filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        staticruntime "off"
        runtime "Release" 
        libdirs {"C:/physx/bin/win.x86_64.vc143.mt/release/"}
--check_imgui()
