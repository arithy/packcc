cmake_minimum_required(VERSION 3.14)

project(
    packcc
    VERSION 3.0.0
    DESCRIPTION "A parser generator for C"
    HOMEPAGE_URL https://github.com/arithy/packcc
    LANGUAGES C
)

if(MSVC)
    string(REGEX REPLACE "/W[0-3]" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}") # To avoid the warning D9025.
endif()

function(add_common_compile_options target)
    target_compile_features(${target} PRIVATE c_std_90)
    target_compile_options(
        ${target} PRIVATE
        $<$<COMPILE_LANGUAGE:C>:
            $<$<C_COMPILER_ID:GNU>:
                -fsigned-char -Wall -Wextra -Wno-unused-parameter -Wno-overlength-strings -pedantic -Werror
                $<$<CONFIG:Debug>:
                    -fsanitize=undefined,address -fno-sanitize-recover=all -fno-omit-frame-pointer
                >
            >
            $<$<C_COMPILER_ID:Clang>:
                -fsigned-char -Wall -Wextra -Wno-unused-parameter -Wno-overlength-strings -pedantic -Werror
                $<$<CONFIG:Debug>:
                    -fsanitize=undefined,address -fno-sanitize-recover=all -fno-omit-frame-pointer
                >
            >
            $<$<AND:$<C_COMPILER_ID:Intel>,$<NOT:$<PLATFORM_ID:Windows>>>:
                -Wall -Wextra -Wno-unused-parameter -pedantic -Werror
            >
            $<$<C_COMPILER_ID:MSVC>:
                /W4 /WX /wd4100 /wd4456
            >
            $<$<AND:$<C_COMPILER_ID:Intel>,$<PLATFORM_ID:Windows>>:
                /W4 /WX
            >
        >
    )
endfunction()

function(add_common_link_options target)
    target_link_options(
        ${target} PRIVATE
        $<$<COMPILE_LANGUAGE:C>:
            $<$<C_COMPILER_ID:GNU>:
                $<$<CONFIG:Debug>:
                    -fsanitize=undefined,address -fno-sanitize-recover=all -fno-omit-frame-pointer
                >
            >
            $<$<C_COMPILER_ID:Clang>:
                $<$<CONFIG:Debug>:
                    -fsanitize=undefined,address -fno-sanitize-recover=all -fno-omit-frame-pointer
                >
            >
        >
    )
endfunction()

add_executable(
    packcc
    src/packcc.c
)
add_common_compile_options(packcc)
add_common_link_options(packcc)

add_custom_target(
    check
    COMMAND ${CMAKE_COMMAND} -E env "PACKCC=$<TARGET_FILE:packcc>" "bash" "${PROJECT_SOURCE_DIR}/tests/test.sh"
    DEPENDS packcc
    VERBATIM
)
set_property(
    TARGET check
    PROPERTY
    ENVIRONMENT "PCC_IMPORT_PATH=${PROJECT_SOURCE_DIR}/import" "CC=\"${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS}\""
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/tests"
)

install(TARGETS packcc)
install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/import"
    DESTINATION "."
    PATTERN ".*" EXCLUDE
    PATTERN "*.txt" EXCLUDE
    PATTERN "*.md" EXCLUDE
)
