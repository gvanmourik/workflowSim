add_library(tracer SHARED 
			traceTool.cpp 
			${PIN_ROOT}/intel64/runtime/pincrt/crtbeginS.o )		
target_include_directories(tracer PUBLIC ${PIN_INCLUDE_DIRS})

message(STATUS "PIN_DEFINES = ")
foreach(DEF ${PIN_DEFINES})
	message("\t-D${DEF}")
	target_compile_definitions(tracer PUBLIC -D${DEF})
endforeach()

foreach(SYS_INCLUDE ${PIN_SYSTEM_INCLUDES})
	target_compile_options(tracer PUBLIC -isystem${SYS_INCLUDE})
endforeach()
foreach(FLAG ${PIN_CXX_FLAGS})
	target_compile_options(tracer PUBLIC ${FLAG})
endforeach()
foreach(LIB ${LIBS})
	target_compile_options(tracer PUBLIC -l${LIB})
endforeach()
foreach(LDFLAG ${LDFLAGS})
	target_compile_options(tracer PUBLIC ${LDFLAG})
endforeach()

target_link_libraries(tracer "${PIN_ROOT}/intel64/lib/libpin.a")
target_link_libraries(tracer "${PIN_ROOT}/extras/xed-intel64/lib/libxed.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/lib-ext/libpin3dwarf.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/runtime/pincrt/libstlport-dynamic.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/runtime/pincrt/libm-dynamic.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/runtime/pincrt/libc-dynamic.dylib")
target_link_libraries(tracer "${PIN_ROOT}/intel64/runtime/pincrt/libunwind-dynamic.dylib")

# Create a new directory to hold the cache sim output data
set(NEW_DIR "data")
install(DIRECTORY DESTINATION ${NEW_DIR})
install (TARGETS tracer DESTINATION bin)