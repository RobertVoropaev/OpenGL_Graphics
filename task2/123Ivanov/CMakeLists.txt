set(SRC_FILES
    gl1_SimpleTriangle.cpp
)

MAKE_OPENGL_TASK(123Ivanov 2 "${SRC_FILES}")

if (UNIX)
    target_link_libraries(123Ivanov2 stdc++fs)
endif()
