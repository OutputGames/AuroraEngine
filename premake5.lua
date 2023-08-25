
-- Clean Function --
newaction {
    trigger     = "clean",
    description = "clean the software",
    execute     = function ()
       print("extract the build...")
       os.rmdir("./generated")
       print("empty the bin...")
       os.rmdir("./bin")
       print("done.")
    end
 }

function string.starts(String,Start)
    return string.sub(String,1,string.len(Start))==Start
end

function link_to(lib)
    links (lib)
    includedirs ("../"..lib.."/include")
    includedirs ("../"..lib.."/" )
end

function download_progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);
    print("Download progress (" .. percent .. "%/100%)")
end

function check_imgui()
    if(os.isdir("imgui") == false and os.isdir("imgui-docking") == false) then
        if(not os.isfile("imgui-docking.zip")) then
            print("ImGui not found, downloading from github")
            local result_str, response_code = http.download("https://github.com/UntitledOutput/imgui/archive/refs/heads/docking.zip", "imgui-docking.zip", {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract("imgui-docking.zip", os.getcwd())
        os.remove("imgui-docking.zip")
    end
    --includedirs {"imgui-docking"}
end

-- premake5.lua
workspace "AuroraEngine"
   configurations { "Debug", "Release" }
   platforms { "x32", "x64" }

filter { "platforms:x32" }
    system "Windows"
    architecture "x86"

filter { "platforms:x64" }
    system "Windows"
    architecture "x64"

project "AuroraEngine"

    --os.rmdir("generated\\")

   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin\\%{cfg.buildcfg}"
   location "generated\\"
   compileas "C++"

   defines {"GRAPHICS_OPENGL"}

   files { "**.h", "**.hpp", "**.cpp", "**.c" }

   files {"resources\\**", "**.json", "lib\\**"}

   includedirs {"include\\", "include\\aurora\\"}

   links {
        "glfw3_mt.lib","glew32.dll","glew32.lib","opengl32.lib","OpenAL32.lib","freetype.lib","assimp-vc143-mt.lib"
    }

    libdirs {
        "lib\\",
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
    

    if (_ACTION == "vs2022") then
        libdirs { 
            "lib\\glfw\\lib-vc2022"
        }    
    end

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