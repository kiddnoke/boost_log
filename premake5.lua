solution "BoostLog"
    configurations { "Release" , "Debug" }
    if os.get() == "linux" then 
        defines{"BOOST_LOG_DYN_LINK"}
        buildoptions { "-std=c++11","-fPIC","-pthread"}
        includedirs {"/usr/local/include/",  "/usr/include",}
        libdirs {"/usr/local/lib64", "/usr/local/lib", "/usr/lib64", "/usr/lib"}
        links {"pthread"}
        links {"boost_system" , "boost_thread" , "boost_log_setup","boost_log"}
    else 
        defines{"WIN"}
        includedirs{"D:/boost_1_55_0"}
    end

    if os.get() == "windows" then 
        platforms{"x32" , "x64"}
        configuration "x32" 
            architecture "x86" 
            libdirs{"D:/boost_1_55_0/lib32-msvc-11.0"}
        configuration "x64"
            architecture "x64"
            libdirs{"D:/boost_1_55_0/lib64-msvc-11.0"}
    end 

    filter "configurations:Debug"
        defines { "DEBUG", "_DEBUG" }
        flags{"symbols"}
    filter "configurations:Release"
        defines { "NDEBUG", "NODEBUG" }
        flags {"OptimizeSpeed"}

project "BoostLog_DLL"
    kind "SharedLib"
    language "C++"
    defines {"LOG_EXPORTS"}
    files {
        "./log.cpp" ,
         "./log.h"
        }

project "BoostLog_Lib"
    kind "StaticLib"
    language "C++"
    defines {"LOG_EXPORTS"}
    files {
        "./log.cpp",
         "./log.h"
        }