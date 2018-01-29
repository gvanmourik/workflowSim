add_library(tracer traceTool.cpp)
target_include_directories(tracer PUBLIC ${PIN_INCLUDE_DIRS})

foreach(DEF ${PIN_DEFINES})
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


install (FILES ../build/pintools/libtracer.a DESTINATION bin)