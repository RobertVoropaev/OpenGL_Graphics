macro(ADD_TASKS DIRNAME)
    file(GLOB STUDENTS ${DIRNAME}/*)
    foreach(STUDENT ${STUDENTS})
        if(IS_DIRECTORY ${STUDENT})
            message("Student task found " ${STUDENT})
            if(SELECTED_STUDENT)
                get_filename_component(STUDENT_NAME ${STUDENT} NAME)
                if(${SELECTED_STUDENT} STREQUAL ${STUDENT_NAME})
                    message("Add to build " ${STUDENT})
                    add_subdirectory("${STUDENT}")
                endif()
            else()
                add_subdirectory("${STUDENT}")
            endif()
        endif()
    endforeach()
endmacro(ADD_TASKS DIRNAME)

macro(MAKE_TASK TRGTNAME TASKNUM SOURCEFILES)
    message("Building target ${TRGTNAME}${TASKNUM}")
    add_executable(${TRGTNAME}${TASKNUM} ${SOURCEFILES})

    target_include_directories(${TRGTNAME}${TASKNUM} PUBLIC
        ${GLEW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${SOIL_INCLUDE_DIR}
    )

    target_link_libraries(${TRGTNAME}${TASKNUM}
        ${GLEW_LIBS}
        ${SOIL_LIBS}
    )

    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_link_libraries(${TRGTNAME}${TASKNUM} "-framework CoreFoundation")
    endif()

    install(TARGETS ${TRGTNAME}${TASKNUM} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/task${TASKNUM})

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${TRGTNAME}Data${TASKNUM})
        add_custom_target(${TRGTNAME}${TASKNUM}_Data${TASKNUM}
                COMMAND ${CMAKE_COMMAND} -E remove_directory
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TRGTNAME}Data${TASKNUM}
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/${TRGTNAME}Data${TASKNUM}
                ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TRGTNAME}Data${TASKNUM}
                COMMENT "Force copy resources to ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TRGTNAME}Data${TASKNUM}"
                VERBATIM)

        add_dependencies(${TRGTNAME}${TASKNUM} ${TRGTNAME}${TASKNUM}_Data${TASKNUM})
    else()
        message("Data folder not found " ${TRGTNAME}Data${TASKNUM})
    endif()
endmacro()

macro(MAKE_OPENGL_TASK TRGTNAME TASKNUM SOURCEFILES)
    MAKE_TASK(${TRGTNAME} ${TASKNUM} "${SOURCEFILES}")

    target_include_directories(${TRGTNAME}${TASKNUM} PUBLIC
            ${ASSIMP_INCLUDE_DIR}
            ${GLFW_INCLUDE_DIR}
            ${IMGUI_INCLUDE_DIR}
    )

    target_link_libraries(${TRGTNAME}${TASKNUM}
            ${ASSIMP_LIBS}
            ${GLFW_LIBS}
            ${IMGUI_LIBS}
    )

    if(USE_CORE_PROFILE)
        target_compile_definitions(${TRGTNAME}${TASKNUM} PRIVATE USE_CORE_PROFILE)
    endif(USE_CORE_PROFILE)
endmacro()
