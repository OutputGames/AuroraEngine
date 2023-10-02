
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

flags { "MultiProcessorCompile"}
    
group "Core"
	include "core-premake5.lua"
group ""

group "Misc"
	include "engine-premake5.lua"
group ""

group "ScriptCore"
	include "scriptcore-premake5.lua"
group ""