cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0074 NEW)

project(planeFight2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

OPTION(NO_BOOST "Build with no boost: not network game and locale" OFF)

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

add_executable(
	planeFight
	src/ai.cpp
	src/game.cpp
	src/main.cpp
	src/pfExtFtxui.cpp
	src/pfLocale.cpp
	src/ui.cpp
	src/uiCtrl.cpp
)
target_link_libraries(
	planeFight
	PRIVATE ftxui::screen
	PRIVATE ftxui::dom
	PRIVATE ftxui::component
)

if(NO_BOOST)
	add_compile_definitions(PF_NO_LOCALE PF_NO_NW)
else()
	# set(Boost_DEBUG ON)
	set(Boost_ARCHITECTURE -x64) # workaround failure to find boost libraries compiled with MinGW, see <https://stackoverflow.com/questions/62010279/undefined-reference-errors-in-simple-boost-serialization>.

	find_package(Boost REQUIRED COMPONENTS locale)

	if(Boost_FOUND)
		include_directories(${Boost_INCLUDE_DIRS})
		target_sources(
			planeFight
			PRIVATE
			src/remotePlayer.cpp
		)
		target_link_libraries(
			planeFight
			PRIVATE ${Boost_LIBRARIES}
		)
		if(WIN32)
			target_link_libraries(planeFight PRIVATE wsock32 PRIVATE ws2_32)
		endif()
	endif()
endif()
