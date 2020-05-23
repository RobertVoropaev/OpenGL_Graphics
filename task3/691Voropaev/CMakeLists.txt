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
        common/LightInfo.hpp
        common/Texture.cpp
        common/Texture.hpp
        )

MAKE_OPENGL_TASK(691Voropaev 2 "${SRC_FILES}")

if (UNIX)
    target_link_libraries(691Voropaev2 stdc++fs)
endif()
