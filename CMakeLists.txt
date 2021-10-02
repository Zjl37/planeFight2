cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0074 NEW)

project(planeFight2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(Boost_DEBUG ON)
set(Boost_ARCHITECTURE -x64) # workaround failure to find boost libraries compiled with MinGW, see <https://stackoverflow.com/questions/62010279/undefined-reference-errors-in-simple-boost-serialization>.

find_package(Boost REQUIRED COMPONENTS locale)

if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	add_executable(planeFight src/main.cpp src/pfAI.cpp src/pfConio.cpp src/pfConsole.cpp src/pfGame.cpp src/pfRemotePlayer.cpp src/pfUI.cpp src/pfLocale.cpp)
	target_link_libraries(planeFight ${Boost_LIBRARIES})
	if(WIN32)
		target_link_libraries(planeFight wsock32 ws2_32)
	endif()
endif()
