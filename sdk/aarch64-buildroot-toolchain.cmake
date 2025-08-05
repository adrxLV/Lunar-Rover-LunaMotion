# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Check for buildroot host binary path SDK_PATH
# 1. Environment varialbe SDK_PATH
#       Which you can define in command line, or in vscode cmake.environment
# 2. A "dotenv" file with the SDK_PATH variable
#       File must be named .sdkconfig and exist in this file parent directory
# 3.
set(SDK_CONFIG_FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/../.sdkconfig" CACHE FILEPATH "Please set it to your buildroot host binary folder. e.g. ~/buildroot/output/host/bin") 
if(DEFINED ENV{SDK_PATH})
    set(SDK_BIN_PATH $ENV{SDK_PATH})
elseif(EXISTS ${SDK_CONFIG_FILE_PATH})
    file(STRINGS ${SDK_CONFIG_FILE_PATH} sdk_entry REGEX "^SDK_PATH=")
    if (sdk_entry MATCHES "^SDK_PATH=(.*)$")
        set(SDK_BIN_PATH "${CMAKE_MATCH_1}")
    else ()
        message(FATAL_ERROR "Malformed dotenv SDK_PATH entry.\n")
    endif ()
endif()

if(NOT DEFINED SDK_BIN_PATH)
    message(SEND_ERROR  "SDK_PATH must be defined as a environment variable or in .sdkconfig file. Please set it to your buildroot host binary folder. e.g. ~/buildroot/output/host/bin")
endif()

# which compilers to use for C and C++
set(CMAKE_CXX_COMPILER ${SDK_BIN_PATH}/aarch64-buildroot-linux-gnu-g++)
set(CMAKE_C_COMPILER ${SDK_BIN_PATH}/aarch64-buildroot-linux-gnu-gcc)

# where is the target environment located
#set(CMAKE_FIND_ROOT_PATH  )

# adjust the default behavior of the FIND_XXX() commands:
# search programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# search headers and libraries in the target environment
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)