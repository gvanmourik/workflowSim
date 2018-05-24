add_library(tracer SHARED 
			traceTool.cpp
			${PIN_ROOT}/intel64/runtime/pincrt/crtbeginS.o)	

# target_include_directories(tracer PRIVATE ${PIN_INCLUDE_DIRS})
# target_include_directories(tracer SYSTEM PRIVATE ${PIN_SYSTEM_INCLUDES})

# Directories of linked libraries
# set(LIB_DIRS
# 	${PIN_ROOT}/intel64/lib/
# 	${PIN_ROOT}/extras/xed-intel64/lib)

message(STATUS "PIN_INCLUDE_DIRS = ")
foreach(PIN_INCLUDE_DIR ${PIN_INCLUDE_DIRS})
	message("\t-I${PIN_INCLUDE_DIR}")
endforeach()

message(STATUS "PIN_SYSTEM_INCLUDES = ")
foreach(SYS_INCLUDE ${PIN_SYSTEM_INCLUDES})
	message("\t-isystem ${SYS_INCLUDE}")
endforeach()

message(STATUS "PIN_DEFINES = ")
foreach(DEF ${PIN_DEFINES})
	message("\t-D${DEF}")
	target_compile_definitions(tracer PRIVATE -D${DEF})
endforeach()


# # Try this...
# set_target_properties(tracer
# 	PROPERTIES 
# 	INTERFACE_INCLUDE_DIRECTORIES "${PIN_INCLUDE_DIRS}"
# )

# set_target_properties(tracer
# 	PROPERTIES 
# 	INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${PIN_SYSTEM_INCLUDES}"
# )

# message("STATUS INTERFACE_INCLUDE_DIRECTORIES = ${PIN_INCLUDE_DIRS}")
# message("STATUS INTERFACE_SYSTEM_INCLUDE_DIRECTORIES = ${PIN_SYSTEM_INCLUDES}")

# # 


# target_include_directories(tracer PUBLIC $<BUILD_INTERFACE:${PIN_INCLUDE_DIRS}> )
foreach(PIN_INCLUDE_DIR ${PIN_INCLUDE_DIRS})
	target_include_directories(tracer PUBLIC 
		${PIN_INCLUDE_DIR}
		)
	# target_include_directories(tracer PRIVATE ${PIN_INCLUDE_DIR})
endforeach()

# target_include_directories(tracer PUBLIC $<BUILD_INTERFACE:${PIN_SYSTEM_INCLUDES}> )
foreach(PIN_SYSTEM_INCLUDE ${PIN_SYSTEM_INCLUDES})
	# target_compile_options(tracer PUBLIC -isystem${PIN_SYSTEM_INCLUDE})
	# if (NOT "${PIN_SYSTEM_INCLUDE}" STREQUAL "${PIN_ROOT}/extras/crt/include")
	# 	message("----->>>${PIN_SYSTEM_INCLUDE} added to -isystem linking")
	# 	target_include_directories(tracer SYSTEM PUBLIC ${PIN_SYSTEM_INCLUDE})
	# endif()
	target_include_directories(tracer 
		SYSTEM
		PUBLIC 
		${PIN_SYSTEM_INCLUDE} 
		)
	# target_include_directories(tracer SYSTEM PRIVATE ${PIN_SYSTEM_INCLUDE})
endforeach()

# get_target_property(current_include_directories tracer INCLUDE_DIRECTORIES)
# set(appended_include_directories "${current_include_directories} -I${PIN_ROOT}/extras/crt/include")




## Attempt to overwrite the target_include_directories list 
# get_target_property(tracer_inc_dirs tracer INCLUDE_DIRECTORIES )
# set(tracer_inc_dirs "${PIN_ROOT}/extras/crt/include ${tracer_inc_dirs}")
# foreach(tracer_inc_dir ${tracer_inc_dirs})
# 	set(tracer_inc_dirs "${tracer_inc_dirs} ${tracer_inc_dir}")	
# endforeach(tracer_inc_dir)
# set(tracer_inc_dirs "${tracer_inc_dirs} ${PIN_ROOT}/extras/crt/include")

# message("tracer_inc_dirs = ${tracer_inc_dirs}")
# set_target_properties(tracer PROPERTIES INCLUDE_DIRECTORIES tracer_inc_dirs)


target_compile_options(tracer PRIVATE -nodefaultlibs -nostdlib)
foreach(FLAG ${PIN_CXX_FLAGS})
	target_compile_options(tracer PRIVATE ${FLAG})
endforeach()

foreach(DEF ${PIN_DEFINES})
	target_compile_definitions(tracer PRIVATE -D${DEF})
endforeach()


message(STATUS "Libs = ")
foreach(LIB ${LIBS})
	message("\t${LIB}")
	target_compile_options(tracer PRIVATE -l${LIB})
endforeach()

foreach(LDFLAG ${LDFLAGS})
	target_compile_options(tracer PRIVATE ${LDFLAG})
endforeach()

message("CMAKE_CXX_IMPLICIT_LINK_LIBRARIES = ${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES}")
message("CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES = ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}")

# Disable the libc++ linking
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")
set_target_properties(tracer PROPERTIES LINKER_LANGUAGE C)

# **************************************
# STILL NEED TO DIABLE libSystem linking
# **************************************
# set_target_properties(tracer PROPERTIES LINK_FLAGS "-nostdlib" -nodefaultlibs)


# message("CMAKE_C_IMPLICIT_LINK_DIRECTORIES = ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES}")
# set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")
# message("CMAKE_C_IMPLICIT_LINK_DIRECTORIES = ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES}")

# set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib -v")
# message("CMAKE_SHARED_LINKER_FLAGS = ${CMAKE_SHARED_LINKER_FLAGS}")
# message("CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES = ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}")

# set(CMAKE_FIND_FRAMEWORK "NEVER")


get_target_property(result1 tracer COMPILE_OPTIONS)
# if(result1 EQUAL NOTFOUND)
message("COMPILE_OPTIONS = ${result1}")
# endif()

# get_property(result2 tracer INCLUDE_DIRECTORIES)
# message("INCLUDE_DIRECTORIES = ${resul2}")

get_target_property(result5 tracer SYSTEM_INCLUDE_DIRECTORIES)
# if(result5 EQUAL NOTFOUND)
message("SYSTEM_INCLUDE_DIRECTORIES = ${resul5}")
# endif()

get_target_property(result3 tracer INTERFACE_INCLUDE_DIRECTORIES)
# if(result3 EQUAL NOTFOUND)
message("INTERFACE_INCLUDE_DIRECTORIES = ${resul3}")
message("INCLUDE_DIRECTORIES(global) = ${INCLUDE_DIRECTORIES}")

get_target_property(result4 tracer INTERFACE_SYSTEM_INCLUDE_DIRECTORIES)
# if(result4 EQUAL NOTFOUND)
message("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES = ${resul4}")
# endif()

message("BUILD_INTERFACE = ${BUILD_INTERFACE}")

# # don't add the automatically determined parts of the RPATH
# # which point to directories outside the build tree to the install RPATH
# set(CMAKE_INSTALL_RPATH_USE_LINK_PATH false)


# set(CFLAGS
# 	-std=c++11
# 	-dynamiclib
# 	-nostdlib
# 	-Wl,-headerpad_max_install_names
# 	)
# set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-std=c++11 -dynamiclib -Wl,-headerpad_max_install_names")

# set(CMAKE_SKIP_RPATH true)

# # the RPATH to be used when installing
# SET(CMAKE_INSTALL_RPATH "")

# target_link_libraries(tracer ${LIBS})
# target_link_libraries(tracer "-I${PIN_ROOT}/extras/crt/include")

# target_link_libraries(tracer "-nostdlib")

# foreach(LIB ${LIBS})
# 	find_library(LIB_IS_FOUND ${LIB}
# 				 HINTS ${PIN_ROOT}/intel64/lib/)
# 	if(NOT LIB_IS_FOUND)
# 		message(FATAL_ERROR "${LIB} library not found")
# 	endif()
# 	message("\tlinking ${LIB} to tracer...")
# 	target_link_libraries(tracer ${LIB})
# endforeach(LIB)
target_link_libraries(tracer "${PIN_ROOT}/intel64/lib/libpin.a")
target_link_libraries(tracer "${PIN_ROOT}/extras/xed-intel64/lib/libxed.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/lib-ext/libpin3dwarf.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/runtime/pincrt/libstlport-dynamic.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/runtime/pincrt/libm-dynamic.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/runtime/pincrt/libc-dynamic.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/runtime/pincrt/libunwind-dynamic.dylib")

# get_target_property(result tracer LINK_LIBRARIES)
# message("result = ${result}")

# Create a new directory to hold the cache sim output data
set(NEW_DIR "data")
install(DIRECTORY DESTINATION ${NEW_DIR})

install (TARGETS tracer DESTINATION bin)