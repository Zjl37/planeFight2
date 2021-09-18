obj = tmp/main.o tmp/pfGame.o tmp/pfConsole.o tmp/pfUI.o tmp/pfLang.o tmp/pfAI.o tmp/vtsFilter.o tmp/pfRemotePlayer.o
flags = -Wall -DUNICODE -D_WIN32_WINNT=0x0a00 -g -I$(BOOST_ROOT)

all: check-env planeFight.exe

check-env:
ifndef BOOST_ROOT
	$(error Please define environment variable BOOST_ROOT.)
endif

planeFight.exe: $(obj)
	g++ $(obj) -o $@ $(flags) -lws2_32 -lwsock32

tmp/main.o: src/main.cpp \
            src/pfRemotePlayer.hpp src/pfGame.hpp src/pfUI.hpp src/pfLang.hpp src/pfAI.hpp src/vtsFilter.hpp
	g++ -c $< -o $@ $(flags)

tmp/pfGame.o: src/pfGame.cpp src/pfGame.hpp
	g++ -c $< -o $@ $(flags)

tmp/pfConsole.o: src/pfConsole.cpp src/pfConsole.hpp src/vtsFilter.hpp
	g++ -c $< -o $@ $(flags)

tmp/pfUI.o: src/pfUI.cpp src/pfUI.hpp src/vtsFilter.hpp
	g++ -c $< -o $@ $(flags)

tmp/pfLang.o: src/pfLang.cpp src/pfUI.hpp src/pfLang.hpp
	g++ -c $< -o $@ $(flags)

tmp/pfAI.o: src/pfAI.cpp src/pfGame.hpp src/pfUI.hpp
	g++ -c $< -o $@ $(flags)

tmp/vtsFilter.o: src/vtsFilter.cpp src/vtsFilter.hpp
	g++ -c $< -o $@ $(flags)

tmp/pfRemotePlayer.o: src/pfRemotePlayer.cpp src/pfRemotePlayer.hpp
	g++ -c $< -o $@ $(flags)

