add_library(dbtree STATIC)

target_include_directories(dbtree PRIVATE ${OUT_SRC_DIR} ${SRC_DIR}
                                          ${GTKMM_INCLUDE_DIRS})

target_sources(
  dbtree
  PRIVATE article2ch.cpp
          article2chcompati.cpp
          articlebase.cpp
          articlehash.cpp
          articlejbbs.cpp
          articlelocal.cpp
          articlemachi.cpp
          board2ch.cpp
          board2chcompati.cpp
          boardbase.cpp
          boardfactory.cpp
          boardjbbs.cpp
          boardlocal.cpp
          boardmachi.cpp
          frontloader.cpp
          interface.cpp
          nodetree2ch.cpp
          nodetree2chcompati.cpp
          nodetreebase.cpp
          nodetreedummy.cpp
          nodetreejbbs.cpp
          nodetreelocal.cpp
          nodetreemachi.cpp
          root.cpp
          ruleloader.cpp
          settingloader.cpp
          spchar_decoder.cpp)
