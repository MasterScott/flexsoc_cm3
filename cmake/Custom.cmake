#
# Custom cmake functions to support this project
#
# Set test directory
set( TEST_DIR ${PROJECT_SOURCE_DIR}/test )
  
if( CMAKE_CROSSCOMPILING )
  
  function( target_bin TARGET SOURCES )
    
    # Create executable
    add_executable( ${TARGET} ${SOURCES} ${ARGN} )
    
    # Link to common lib
    target_link_libraries( ${TARGET} common c )

    # Set link properties
    set_target_properties( ${TARGET} PROPERTIES
      LINK_FLAGS
      "-Wl,-zmax-page-size=4 -T ${TEST_DIR}/arm/linker.ld -Wl,-Map=${TARGET}.map"
      COMPILE_FLAGS
      "-O0 -ggdb" )
    
    # Create binary and copy back
    add_custom_command( TARGET ${TARGET} POST_BUILD
      DEPENDS ${TARGET}
      COMMENT "Creating binary: ${TARGET}.bin"
      COMMAND arm-none-eabi-objcopy -O binary ${TARGET} ${TARGET}.bin
      COMMAND cmake -E copy $<TARGET_FILE:${TARGET}>.bin ${TEST_DIR}/arm/
      )
    
    # Cleanup
    set_property( DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${TARGET}.bin )
    
  endfunction( target_bin )
  
else( NOT CROSSCOMPILING )
  
  # Create plugin
  function( plugin NAME SOURCES )
    
    # Create shared library
    add_library( ${NAME} SHARED ${SOURCES} ${ARGN} )
    
    # Make sure our export structure doesn't get compiled out
    set_target_properties( ${NAME} PROPERTIES
      COMPILE_FLAGS "-O0 -ggdb"
      LINK_FLAGS "-Wl,-entry=__plugin"
      PREFIX ""
      SUFFIX ".plg" )
    
    # Link against plugin for helper fns
    target_link_libraries( ${NAME} plugin )
    
  endfunction( plugin )

  macro( test_setup )
    # Decide where to run tests
    if( DEFINED ENV{FLEXSOC_HW} )
      set( FLEXSOC_HW $ENV{FLEXSOC_HW} )
      message( STATUS "Running on hw: ${FLEXSOC_HW}" )
    else ()
      set( FLEXSOC_HW "127.0.0.1:5555" )
      message( STATUS "Running on simulator: ${FLEXSOC_HW}" )
      # TODO: codify version if possible
      set( VERILATOR_SIM build/${CMAKE_PROJECT_NAME}_0.1/sim-verilator/V${CMAKE_PROJECT_NAME} )
      add_custom_command(
        OUTPUT ${VERILATOR_SIM}
        COMMENT "Generating verilated sim for ${CMAKE_PROJECT_NAME}"
        COMMAND ${FUSESOC_EXECUTABLE} --target=sim ${CMAKE_PROJECT_NAME}
        )
    endif ()
    # Turn on testing
    enable_testing()
  endmacro( test_setup )

  macro( test_finalize )
    if( DEFINED ENV{FLEXSOC_HW} )
      add_custom_target( check
        COMMAND ${CMAKE_CTEST_COMMAND} )
    else ()
      add_custom_target( check
        DEPENDS ${VERILATOR_SIM}
        COMMAND ${CMAKE_CTEST_COMMAND} )
    endif ()
  endmacro( test_finalize )
  
  # Create test
  function( hw_test NAME SOURCES)
    add_executable( ${NAME} ${SOURCES} ${ARGN} )
    target_link_libraries( ${NAME} flexsoc test )
    add_test(
      NAME ${NAME}
      COMMAND ${PROJECT_SOURCE_DIR}/test/scripts/hw_test.sh ${VERILATOR_SIM} $<TARGET_FILE:${NAME}> ${FLEXSOC_HW} )    
  endfunction( hw_test )
  
endif()
