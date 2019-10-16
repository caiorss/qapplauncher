cmake_minimum_required(VERSION 3.9)
project(applauncher)

#====== Global Configurations ==================#

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt5 COMPONENTS Core Widgets Network UiTools REQUIRED)

#========== Functions =======================================#

# Copy target file to current directory whenerver it is rebuilt
function(copy_after_build TARGET_NAME )
    # Note: CMAKE_CURRENT_LIST_DIR is the directory where is this
    # CMakeLists.txt file.
    set(DESTDIR ${CMAKE_CURRENT_LIST_DIR}/bin/)
    file(MAKE_DIRECTORY ${DESTDIR})

    # Copy binary file to <CMakeLists.txt didctory>./bin
    # after target is compiled.
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
                               $<TARGET_FILE:${TARGET_NAME}> ${DESTDIR}
                               )
endfunction()

#=============== Target Configurations ============#

#------- Target applauncher ----------------------------#
#


# QT Utilities
# qtutils.hpp
# RecordTableModel.hpp
#FormLoader.hpp
 # serialization.hpp

# Brief: Header-only libraries with header-only and template utitlites for QT.
#-----------------------------------------------------------------------
# add_library(qtutils_ide qtutils/qtutils.hpp
#                        qtutils/serialization.hpp
#                        qtutils/FormLoader.hpp
#                        qtutils/RecordTableModel.hpp)


# Brief: Main executable - GUI Graphical User Interface
#-----------------------------------------------------------
add_executable( applauncher

                src/main.cpp

                src/appmainwindow.cpp
                src/appmainwindow.hpp

                src/tab_desktopbookmarks.cpp
                src/tab_desktopbookmarks.hpp

                # Class FileBookMarkItem
                src/FileBookmarkItem.hpp

                # Class: FileBookMarkItemModel
                src/filebookmarkitemmodel.cpp
                src/filebookmarkitemmodel.hpp

                # Class Tab_ApplicationLauncher
                src/tab_applicationlauncher.cpp
                src/tab_applicationlauncher.hpp

                resources.qrc
               )           
# target_include_directories(applauncher PUBLIC .)
# target_link_libraries(applauncher qtutils_ide)
copy_after_build( applauncher )

target_link_libraries(applauncher
    Qt5::Core Qt5::Widgets Qt5::UiTools Qt5::Network)

copy_after_build( applauncher )
