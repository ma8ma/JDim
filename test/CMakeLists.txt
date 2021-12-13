include(GoogleTest)
enable_testing()

add_executable(gtest_jdim $<TARGET_OBJECTS:core_objects>)
gtest_discover_tests(gtest_jdim)

target_include_directories(
  gtest_jdim PRIVATE $<TARGET_PROPERTY:jdim,INCLUDE_DIRECTORIES>
                     ${GTEST_MAIN_INCLUDE_DIRS})

target_link_libraries(gtest_jdim PRIVATE $<TARGET_PROPERTY:jdim,LINK_LIBRARIES>
                                         ${GTEST_MAIN_LINK_LIBRARIES})

target_sources(
  gtest_jdim
  PRIVATE gtest_jdlib_cookiemanager.cpp gtest_jdlib_jdiconv.cpp
          gtest_jdlib_misctime.cpp gtest_jdlib_misctrip.cpp
          gtest_jdlib_miscutil.cpp gtest_xml_dom.cpp)
