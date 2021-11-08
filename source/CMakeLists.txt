cmake_minimum_required(VERSION 3.10)
project("mvcomputeshader")



message("System: ${CMAKE_SYSTEM_NAME}")

#find_package(PkgConfig REQUIRED QUIET)
#pkg_search_module(
#    OpenGL
#    REQUIRED
#      OpenGL
#    IMPORTED_TARGET
#    )


#
# OPENGL
#
#find_package(OpenGL REQUIRED COMPONENTS OpenGL)
#if (OPENGL_FOUND)
#    message("found OPENGL")
#    include_directories($(OPENGL_INCLUDE_DIRS))
#    link_libraries(${OPENGL_LIBRARIES})
#endif()



#
# GLFW
#
find_package(GLFW3 REQUIRED)
if (GLFW3_FOUND)
    message("found GLFW3")
    include_directories($(GLFW3_INCLUDE_DIRS))
    link_libraries(${GLFW3_LIBRARIES})
endif()

add_executable(mvcomputeshader
    main.cpp
    CShaderTool.cpp
)


if (CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    #
    # Windows
    #
    target_link_libraries(${CMAKE_PROJECT_NAME}
        opengl32
        glu32
        glfw3)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #
    # Linux
    #
    target_link_libraries(${CMAKE_PROJECT_NAME}
        SDL2 
        GL
        GLEW)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    #
    # Mac OS
    #
    target_link_libraries(${CMAKE_PROJECT_NAME}
        GLEW
        GLFW
        "-framework OpenGL"
    )
endif()

#
# GLEW
#
find_package(GLEW REQUIRED)
if (GLEW_FOUND)
    message("found GLEW")
    include_directories($(GLEW_INCLUDE_DIRS))
    link_libraries(${GLEW_LIBRARIES})
endif()


target_link_directories(${CMAKE_PROJECT_NAME} PUBLIC
        "/usr/local/lib"
        )

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
    /usr/local/include
    ./source/include
    ./source
)



