project "AuroraRuntime"

    --os.rmdir("generated\\")

   kind "StaticLib"
   language "C++"
   cppdialect "C++17"
   targetdir "bin\\%{cfg.buildcfg}"
   location "generated\\"
   compileas "C++"
   staticruntime "on"

   defines {"GRAPHICS_OPENGL"}

   files { "include/**.h", "include/**.hpp", "include/**.cpp", "include/**.c","src/**.h", "src/**.hpp" }

   files {"resources\\**", "**.json", "lib\\**"}

   includedirs {"include\\", "include\\aurora\\", "vcpkg\\installed\\x64-windows\\include\\", "vcpkg\\installed\\x64-windows\\include\\bullet"}

   links {
        "glfw3_mt.lib","glew32.dll","glew32.lib","opengl32.lib","OpenAL32.lib","freetype.lib","assimp-vc143-mt.lib"
    }

    libdirs {
        "lib\\",
        "lib\\glfw\\lib-vc2022"
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
--check_imgui()
