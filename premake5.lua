workspace "PhotonMapping"
  configurations { "Debug", "Release" }
  platforms { "x64" }
  debugdir "%{cfg.targetdir}"
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

  if os.host() ~= "macosx" then
    links { "embree3" }
    links { "assimp" }
    links { "freeimage" }
  end

  if os.host() == "macosx" then
    links { "embree3", "assimp", "%{wks.location}/../%{prj.name}/vendor/libraries/%{cfg.system}/libfreeimage.3.18.0.dylib" }
    externalincludedirs { "%{prj.name}/vendor/includes" }
    sysincludedirs { "%{prj.name}/vendor/includes" }
    frameworkdirs { "%{prj.name}/vendor/libraries/%{cfg.system}" }
    embedAndSign { "embree3", "assimp", "%{wks.location}/../%{prj.name}/vendor/libraries/%{cfg.system}/libfreeimage.3.18.0.dylib" }

    postbuildcommands "{COPYFILE} %{wks.location}/../%{prj.name}/vendor/libraries/%{cfg.system}/*.dylib* %{cfg.targetdir}"
    postbuildcommands "{COPY} %{wks.location}/../%{prj.name}/assets %{cfg.targetdir}/"
  end

  if os.host() == "winows" then
    postbuildcommands "{COPYFILE} %{wks.location}../%{prj.name}/vendor/libraries/%{cfg.system}/*.dll %{cfg.targetdir}"
    postbuildcommands "{COPY} %{wks.location}../%{prj.name}/assets %{cfg.targetdir}/assets/"
  end

  filter { "action:make" }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "On"

  filter "system:macosx"
    runpathdirs { "%{cfg.targetdir}" }
