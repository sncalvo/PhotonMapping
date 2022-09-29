workspace "PhotonMapping"
  configurations { "Debug", "Release" }

  location "build"

outputdir = "%{cfg.buildcfg}-%{cfg.system}"

project "PhotonMapping"
  location "build/PhotonMapping"
  kind "WindowedApp"
  language "C++"
  cppdialect "C++20"

  targetdir("bin/" .. outputdir .. "/%{prj.name}")
  objdir("bin-int/" .. outputdir .. "/%{prj.name}")

  files { "%{prj.name}/src/**.hpp", "%{prj.name}/src/**.cpp", "vendor/**.h" }

  includedirs { "%{prj.name}/vendor/includes" }

  libdirs { "%{prj.name}/vendor/libraries" }

  links { "embree.3" }

  postbuildcommands "{COPY} %{prj.name}/vendor/libraries/%{cfg.system}-%{cfg.architecture}/*.dylib %{wks.location}/bin/"

  filter { "action:make" }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter {}
