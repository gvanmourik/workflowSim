add_executable(mmapPinSim mmapPinSim.cpp)
target_include_directories(mmapPinSim PUBLIC ${MY_SST_CORE}/include)

install(TARGETS mmapPinSim  
      RUNTIME DESTINATION bin)