cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0074 NEW)

project(planeFight2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# fetch FTXUI
include(FetchContent)

set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
FetchContent_Declare(ftxui
	GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
	GIT_TAG v3.0.0
)

FetchContent_GetProperties(ftxui)
if(NOT ftxui_POPULATED)
	FetchContent_Populate(ftxui)
	add_subdirectory(${ftxui_SOURCE_DIR} ${ftxui_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# end fetch FTXUI

# set(Boost_DEBUG ON)
set(Boost_ARCHITECTURE -x64) # workaround failure to find boost libraries compiled with MinGW, see <https://stackoverflow.com/questions/62010279/undefined-reference-errors-in-simple-boost-serialization>.

find_package(Boost REQUIRED COMPONENTS locale)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_executable(
		planeFight
		src/main.cpp
		src/ai.cpp
		src/game.cpp
		src/remotePlayer.cpp
		src/ui.cpp
		src/pfLocale.cpp
		src/uiCtrl.cpp
		src/pfExtFtxui.cpp
	)
	target_link_libraries(
		planeFight
		PRIVATE ${Boost_LIBRARIES}
		PRIVATE ftxui::screen
		PRIVATE ftxui::dom
		PRIVATE ftxui::component
	)
	if(WIN32)
		target_link_libraries(planeFight PRIVATE wsock32 PRIVATE ws2_32)
	endif()
endif()
