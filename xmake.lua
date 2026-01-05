-- 设置 mingw 为编译链
if is_host("windows") then
    set_toolchains("mingw")
end

add_rules("mode.debug","mode.release")
add_cxflags("-std=c++11")

target("test")
    set_kind("binary")
    add_includedirs("/src")
    add_files("/src/*.cpp")

    set_targetdir("bin/")

