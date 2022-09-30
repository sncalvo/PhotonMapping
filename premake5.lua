workspace "PhotonMapping"
  configurations { "Debug", "Release" }
  platforms { "Win64" }

  location "build"

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

project "PhotonMapping"
  flags "RelativeLinks"

  location "build/PhotonMapping"
  kind "ConsoleApp"
  language "C++"
  cppdialect "C++20"

  targetdir("bin/" .. outputdir .. "/%{prj.name}")
  objdir("bin-int/" .. outputdir .. "/%{prj.name}")

  files { "%{prj.name}/src/**.hpp", "%{prj.name}/src/**.cpp", "vendor/**.h" }

  includedirs { "%{prj.name}/vendor/includes" }

  libdirs { "%{prj.name}/vendor/libraries/%{cfg.system}" }

  links { "embree3" }

  filter "system:macosx"
    postbuildcommands "{COPYDIR} %{wks.location}../%{prj.name}/vendor/libraries/%{cfg.system}/*.dylib %{wks.location}/bin/"

  filter "system:windows"
    postbuildcommands "{COPYFILE} %{wks.location}../%{prj.name}/vendor/libraries/%{cfg.system}/*.dll %{cfg.targetdir}"

  filter { "action:make" }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter "system:macosx"
    runpathdirs { "./" }
