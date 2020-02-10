cmake_minimum_required(VERSION 3.0)

project(SOIL)

set(CMAKE_DEBUG_POSTFIX "d")

set(SRC_FILES
	src/SOIL2/SOIL2.c
	src/SOIL2/etc1_utils.c
	src/SOIL2/image_DXT.c
	src/SOIL2/image_helper.c
)

set_source_files_properties(${SRC_FILES} PROPERTIES LANGUAGE CXX)

set(HEADER_FILES
	src/SOIL2/SOIL2.h
	src/SOIL2/etc1_utils.h
	src/SOIL2/image_DXT.h
	src/SOIL2/image_helper.h
	src/SOIL2/pkm_helper.h
	src/SOIL2/pvr_helper.h
	src/SOIL2/stb_image.h
	src/SOIL2/stb_image_write.h
	src/SOIL2/stbi_DDS.h
	src/SOIL2/stbi_DDS_c.h
	src/SOIL2/stbi_ext.h
	src/SOIL2/stbi_ext_c.h
	src/SOIL2/stbi_pkm.h
	src/SOIL2/stbi_pkm_c.h
	src/SOIL2/stbi_pvr.h
	src/SOIL2/stbi_pvr_c.h
)

add_library(soil STATIC ${SRC_FILES} ${HEADER_FILES})

install(TARGETS soil
	EXPORT soilTargets	
	ARCHIVE DESTINATION lib
	INCLUDES DESTINATION include
)

install(FILES src/SOIL2/SOIL2.h DESTINATION include)

install(EXPORT soilTargets FILE soilTargets.cmake			
	DESTINATION "lib/cmake/soil"
	NAMESPACE SOIL::)

install(FILES soilConfig.cmake
    DESTINATION "lib/cmake/soil"
)