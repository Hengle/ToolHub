require "vstudio"

-- copy file 
function stream_copy(src, dst)
	local read_file =""
	local write_file=""
	local temp_content ="";
	-- open read file stream
	read_file = io.open(src,"r")
	-- read all content
	temp_content = read_file:read("*a")

	-- open write file stream
	write_file = io.open(dst,"w")

	-- write all content
	write_file:write(temp_content)
    -- close stream
	read_file:close()
	write_file:close()
end

-- work space path 
work_space = "./workspace/".._ACTION

-- copy dll names 
copy_dlls = {
	"VEngine_Network.dll"
	, "VEngine_DLL.dll"
	, "VEngine_Database.dll"
	, "mimalloc.dll"
}
copy_dll_src_root = "./build/"
copy_dll_dst_root = work_space.."/bin/"

-- platform 
isMacBuild = _ACTION == "xcode4"
isLinuxBuild = _ACTION == "gmake2"
isWindowsBuild = not isMacBuild and not isLinuxBuild

-- copy files 
term.pushColor(term.green)
for i, configuare in ipairs({"Debug/net5.0/", "Release/net5.0/"}) do
	for k, v in ipairs(copy_dlls) do
		local src = copy_dll_src_root..v
		local dst = copy_dll_dst_root..configuare..v
		print("copy file "..src.." to "..dst)
		stream_copy(src, dst)
	end
end
term.popColor()


-- work space 
workspace("CSharpNetWork")
    configurations { "Debug", "Release" }
	location (work_space)

	-- build flag 
    flags
	{
	}
	
	-- include dir 
    includedirs
	{
	}

	-- setup configurations 
	configuration "Debug"
		defines { "DEBUG" }
		symbols "full"
		inlining("auto")
		optimize("debug")
	
	configuration "Release"
		defines { "NDEBUG" }
		inlining("auto")
		optimize("speed")

-- add project 
project ("FileServer")
	-- setup kind 
	kind("ConsoleApp")

	-- enable unsafe 
	clr("Unsafe")
	
	-- setup language 
	language("C#")
	dotnetframework("net5.0")
	csversion("9.0")

	-- setup nuget 
	nuget
	{
		"MongoDB.Driver:2.13.1"
	}

	-- setup links 
	links
	{
		"Network"
	}
	
	-- add project file
	files
	{
		"./FileServer/**.cs"
	}

project ("InterfaceGenerator")
	-- setup kind 
	kind("ConsoleApp")
	
	-- enable unsafe 
	clr("Unsafe")
	
	-- setup language 
	language("C#")
	dotnetframework("net5.0")
	csversion("9.0")
	
	-- add project file 
	files
	{
		"./InterfaceGenerator/**.cs"
	}

project ("NetWork")
	-- setup kind 
	kind("SharedLib")
	
	-- enable unsafe 
	clr("Unsafe")
	
	-- setup language 
	language("C#")
	dotnetframework("net5.0")
	csversion("9.0")
	
	-- add project file 
	files
	{
		"./Network/**.cs"
	}