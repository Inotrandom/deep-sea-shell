errored=0
PROJECT="DeepSeaShell"

cmake --version || errored=1
cd build || errored=2

if [ $errored -eq 1 ]; then
    echo "run: cmake was not found"
    return 1
elif [ $errored -eq 2 ]; then
    echo "run: build directory was not found"
    return 1
fi

cmake --build . || errored=3

if [ $errored -eq 3 ]; then
    echo "run: build failed, exiting"
    return 1
fi

./$PROJECT || errored=4

if [ $errored -eq 4 ]; then
    echo "run: executable not located (invalid project name)"
    return 1
fi