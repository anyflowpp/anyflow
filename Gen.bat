pushd %~dp0

if not exist build (
    mkdir build
) else (
    del /q /f build\cmakecache.txt
    rd /S /Q build
    mkdir build
)

pushd build
cmake -Tv140 .. -Awin32
popd 
popd