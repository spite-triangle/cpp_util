-- 设置 mingw 为编译链
if is_host("windows") then
    set_toolchains("mingw")
end

target("test")
    set_kind("binary")
    add_includedirs("/src")
    add_files("/src/*.cpp")

    set_targetdir("bin/")

