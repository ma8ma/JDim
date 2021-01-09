# TODO: cmake v3.9 から追加されるGoogleTestモジュールを検証する
include(CTest)
enable_testing()
add_executable(gtest_jdim)
add_test(NAME jdim_test COMMAND $<TARGET_FILE:gtest_jdim>)
add_custom_target(test COMMAND $<TARGET_FILE:gtest_jdim>)

get_target_property(jdim_incs jdim INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(gtest_jdim PRIVATE ${jdim_incs}
                                              ${GTEST_MAIN_INCLUDE_DIRS})

get_target_property(jdim_libs jdim LINK_LIBRARIES)
target_link_libraries(gtest_jdim PRIVATE ${jdim_libs}
                                         ${GTEST_MAIN_LINK_LIBRARIES})

target_sources(
  gtest_jdim
  PRIVATE ${fullpath_core_sources}
          gtest_jdlib_cookiemanager.cpp
          gtest_jdlib_jdiconv.cpp
          gtest_jdlib_misctime.cpp
          gtest_jdlib_misctrip.cpp
          gtest_jdlib_miscutil.cpp
          gtest_xml_dom.cpp)
