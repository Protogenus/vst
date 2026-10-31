# Toolchain for cross-compiling the VST3 to Windows x64 with MinGW-w64.
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER   x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER  x86_64-w64-mingw32-windres)

# Don't try to run built executables during the build.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Static libstdc++ for a self-contained binary.
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")

# JUCE assumes <cstring> transitively on MSVC; MinGW needs it forced (C++ only).
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -include cstring")
