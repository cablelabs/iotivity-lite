# Patch mbedtls
set(OC_REAPPLY_MBEDTLS_PATCHES ON CACHE BOOL "")
if(OC_REAPPLY_MBEDTLS_PATCHES)
    include(${PROJECT_SOURCE_DIR}/deps/mbedtls-patch.cmake)
    set(OC_REAPPLY_MBEDTLS_PATCHES OFF CACHE BOOL
        "By default, mbedTLS patches are applied upon the first CMake Configure. Set this to ON to reapply the patches on the next configure."
         FORCE
    )
endif()

# file(GLOB MBEDTLS_SRC
#     ${PROJECT_SOURCE_DIR}/deps/mbedtls/library/*.c
# )
# list(REMOVE_ITEM MBEDTLS_SRC ${PROJECT_SOURCE_DIR}/deps/mbedtls/library/x509_crl.c
# )
# if(OC_DYNAMIC_ALLOCATION_ENABLED)
#     list(REMOVE_ITEM MBEDTLS_SRC ${PROJECT_SOURCE_DIR}/deps/mbedtls/library/memory_buffer_alloc.c)
# endif()

# add_library(mbedtls OBJECT ${MBEDTLS_SRC})
# target_include_directories(mbedtls PRIVATE
#     ${PROJECT_SOURCE_DIR}
#     ${PROJECT_SOURCE_DIR}/include
#     ${PORT_INCLUDE_DIR}
#     ${PROJECT_SOURCE_DIR}/deps/mbedtls/include
# )
# target_compile_definitions(mbedtls PUBLIC ${PUBLIC_COMPILE_DEFINITIONS} PRIVATE ${PRIVATE_COMPILE_DEFINITIONS} __OC_RANDOM)
# # do not treat warnings as errors on Windows
# if(MSVC)
#     target_compile_options(mbedtls PRIVATE /W1 /WX-)
# endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/deps/mbedtls)

set(COMPILABLE_TYPES STATIC_LIBRARY MODULE_LIBRARY SHARED_LIBRARY OBJECT_LIBRARY EXECUTABLE)
function(get_all_targets_in_directory dir out_var)
    get_property(all_targets DIRECTORY ${dir} PROPERTY BUILDSYSTEM_TARGETS)
    get_property(subdirs DIRECTORY ${dir} PROPERTY SUBDIRECTORIES)
    foreach(subdir ${subdirs})
        get_all_targets_in_directory(${subdir} subdir_targets)
        list(APPEND all_targets ${subdir_targets})
    endforeach()

    set(targets)
    foreach(target ${all_targets})
        get_target_property(target_type ${target} TYPE)
        if (NOT (${target_type} IN_LIST COMPILABLE_TYPES))
            continue()
        endif()
        list(APPEND targets ${target})
    endforeach()

    set(${out_var} ${targets} PARENT_SCOPE)
endfunction()

get_all_targets_in_directory(${PROJECT_SOURCE_DIR}/deps/mbedtls/library mbedtls_library)
set(mbedtls_targets ${mbedtls_library})

if(ENABLE_TESTING OR ENABLE_PROGRAMS)
    set(mbedtls_testing_targets)
    if(ENABLE_TESTING)
        set(mbedtls_testing_targets mbedtls_test)
        get_all_targets_in_directory(${PROJECT_SOURCE_DIR}/deps/mbedtls/tests mbedtls_tests)
        list(APPEND mbedtls_testing_targets ${mbedtls_tests})
    endif()
    get_all_targets_in_directory(${PROJECT_SOURCE_DIR}/deps/mbedtls/programs mbedtls_programs)
    list(APPEND mbedtls_testing_targets ${mbedtls_programs})

    set_target_properties(${mbedtls_testing_targets}
        PROPERTIES
            EXCLUDE_FROM_ALL 1
            EXECLUTE_FROM_DEFAULT_BUILD 1)
endif()

foreach(target ${mbedtls_targets})
    target_compile_definitions(${target} PUBLIC ${PUBLIC_COMPILE_DEFINITIONS} PRIVATE ${PRIVATE_COMPILE_DEFINITIONS} __OC_RANDOM)
    # do not treat warnings as errors on Windows
    if(MSVC)
        target_compile_options(${target} PRIVATE /W1 /WX-)
    endif()

    target_include_directories(${target} PRIVATE
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/include
        ${PORT_INCLUDE_DIR}
    )
endforeach()
