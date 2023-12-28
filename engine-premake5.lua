project "AuroraEngine"

    --os.rmdir("generated\\")

   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin\\%{cfg.buildcfg}"
   location "generated\\"
   compileas "C++"
   staticruntime "off"

   defines {"GRAPHICS_OPENGL"}

   files { "include/**.h", "include/**.hpp", "include/**.cpp", "include/**.c","src/**.h", "src/**.hpp", "src/**.cpp", "src/**.c" }
   removefiles { "include/aurora/**.cpp", "include/aurora/**.c"}
   --removefiles {"include/reactphysics3d/**"}
   --files {"include/reactphysics3d/include/**"}

   files {"resources\\**", "**.json", "lib\\**"}

   includedirs {"include\\", "include\\aurora\\", "include\\auroraengine"}


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
    
    links {"AuroraRuntime"}

    filter "action:vs*"
        defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
        dependson {"AuroraRuntime"}
        links {"AuroraRuntime.lib"}
        characterset ("MBCS")
    
    
    filter "system:windows"
        defines{"_WIN32"}
        links {"winmm", "kernel32", "opengl32", "gdi32"}
        libdirs {"./bin/%{cfg.buildcfg}"}
        
