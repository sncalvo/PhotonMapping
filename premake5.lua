workspace "PhotonMapping"
  configurations { "Debug", "Release" }

  location "build"

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

project "PhotonMapping"
  flags "RelativeLinks"

  buildoptions "-Wl,-rpath=."

  location "build/PhotonMapping"
  kind "WindowedApp"
  language "C++"
  cppdialect "C++20"

  targetdir("bin/" .. outputdir .. "/%{prj.name}")
  objdir("bin-int/" .. outputdir .. "/%{prj.name}")

  files { "%{prj.name}/src/**.hpp", "%{prj.name}/src/**.cpp", "vendor/**.h" }

  includedirs { "%{prj.name}/vendor/includes" }

  libdirs { "%{prj.name}/vendor/libraries/%{cfg.system}" }

  links { "embree3" }

  postbuildcommands "{COPYDIR} %{wks.location}/%{prj.name}/vendor/libraries/%{cfg.system}/*.dylib %{wks.location}/bin/"
  -- filter "files:%{prj.name}/vendor/libraries/%{cfg.system}/*.dylib"
  --   buildaction "Embed"

  filter { "action:make" }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

