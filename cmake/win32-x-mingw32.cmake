#include(CMakeForceCompiler)

# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER i486-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i486-mingw32-g++)
SET(CMAKE_RC_COMPILER i486-mingw32-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH  /usr/i486-mingw32)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_EXE_LINKER_FLAGS 
    ${CMAKE_EXE_LINKER_FLAGS}
    -Wl,-subsystem,windows 
    -Wl,-enable-auto-import 
    -Wl,-enable-runtime-pseudo-reloc)

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# Qt
set(QT_QMAKE_EXECUTABLE i486-mingw32-qmake CACHE INTERNAL "" FORCE)
set(QT_BINARY_DIR /usr/i486-mingw32/bin/ /usr/bin)
set(QT_HEADERS_DIR "/usr/i486-mingw32/include")
set(QT_LIBRARY_DIR "/usr/i486-mingw32/lib")
