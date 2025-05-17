errored=0

cmake --version || errored=1
(mkdir build && cd build) || errored=2

if [ $errored -eq 1 ]; then
    echo "setup: cmake is not found"
    return 1;
elif [ $errored -eq 2 ]; then
    echo "setup: unable to create build directory"
    return 1;
fi

cmake ..
echo "setup: build system successfully configured for $PWD"
