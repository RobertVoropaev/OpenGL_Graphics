set(SRC_FILES
        main.cpp
        common/Application.cpp
        common/Camera.cpp
        common/Mesh.cpp
        common/ShaderProgram.cpp
        common/Application.hpp
        common/Camera.hpp
        common/Mesh.hpp
        common/ShaderProgram.hpp
        common/DebugOutput.h
        common/DebugOutput.cpp
        )

MAKE_OPENGL_TASK(691Voropaev 1 "${SRC_FILES}")

if (UNIX)
    target_link_libraries(691Voropaev1 stdc++fs)
endif()
