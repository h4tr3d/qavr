if(WIN32)
  set(additional_LINK_FLAGS -Wl,-subsystem,windows -Wl,-enable-auto-import -Wl,-enable-runtime-pseudo-reloc)
endif(WIN32)

add_definitions(-DDATA_PREFIX=\"${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}\")

set(qavr_SRCS
  util.cpp
  bitscellwidget.cpp
  avrdudeprocess.cpp
  main.cpp
  qavr.cpp
)

set(qavr_HDRS
  bitscellwidget.h
  version.h
  util.h
  avrdudeprocess.h
  qavr.h
)

set(qavr_UIS
  qavr.ui
)

set(qavr_RES
    ../qavr.qrc
)

include(${QT_USE_FILE})

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR} # for include headers generated from *.ui
    ${QT_INCLUDE_DIR}
)

#qt4_automoc(${qavr_SRCS})
qt4_wrap_ui(qavr_UIS_H ${qavr_UIS})
qt4_wrap_cpp(qavr_MOC_SRCS ${qavr_HDRS})
qt4_add_resources(qavr_RES_SRCS ${qavr_RES})

add_definitions(
    ${QT_DEFINITIONS}
)

# Generate gitinfo
if(EXISTS ${CMAKE_SOURCE_DIR}/.git/)
    message(WARNING "This is look like GIT version, use on your own risk!")
    set(qavr_gitinfo_HDR ${CMAKE_CURRENT_BINARY_DIR}/gitinfo.h)
    set(qavr_gitinfo_SRC ${CMAKE_CURRENT_BINARY_DIR}/gitinfo.c)
    add_custom_command(
        OUTPUT ${qavr_gitinfo_SRC} ${qavr_gitinfo_HDR}
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/git-gen-gitinfo.sh ${qavr_gitinfo_SRC} ${qavr_gitinfo_HDR}
    )
    add_definitions(-DGIT=1)
endif()

add_executable(
    qavr
    ${qavr_SRCS}
    ${qavr_MOC_SRCS}
    ${qavr_UIS_H}
    ${qavr_RES_SRCS}
    ${qavr_gitinfo_SRC}
)

target_link_libraries(
    qavr
    ${QT_LIBRARIES}
    ${additional_LINK_FLAGS}
)

########### install files ###############

install(TARGETS qavr            DESTINATION bin)
#install(FILES   ${docs_DATA}    DESTINATION share/doc/${CMAKE_PROJECT_NAME})
#install(FILES   ${desktop_DATA} DESTINATION share/applications)
