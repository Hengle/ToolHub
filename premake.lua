require "vstudio"

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
for i, configuare in ipairs({"Debug/net5.0/", "Release/net5.0/"}) do
	for k, v in ipairs(copy_dlls) do
		local src = copy_dll_src_root..v
		local dst = copy_dll_dst_root..configuare..v
		
		-- check src file 
		if os.isfile(src) then
			dst_dir = path.getdirectory(dst)
			
			-- make dst dir 
			if not os.isdir(dst_dir) then
				os.mkdir(dst_dir)
			end
			
			-- print msg 
			term.pushColor(term.green)
			print("copy file "..src.." to "..dst)
			term.popColor()
			
			-- copy dir 
			ok, err = os.copyfile(src, dst)
			if not ok then
				print(err)
			end
		else 
			-- print error 
			term.pushColor(term.red)
			print("lost file "..src)
			term.popColor()
		end
	end
end


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
	removefiles
	{
		"./FileServer/obj/**",
		"./FileServer/bin/**"
	}

project ("FileClient")
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
		"./FileClient/**.cs"
	}
	removefiles
	{
		"./FileClient/obj/**",
		"./FileClient/bin/**"
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
	removefiles
	{
		"./InterfaceGenerator/obj/**",
		"./InterfaceGenerator/bin/**"
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
	removefiles
	{
		"./Network/obj/**",
		"./Network/bin/**"
	}