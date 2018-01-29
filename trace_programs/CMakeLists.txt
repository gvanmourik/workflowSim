add_executable(hello_world hello_world.cpp)

install(TARGETS hello_world  
      RUNTIME DESTINATION bin)