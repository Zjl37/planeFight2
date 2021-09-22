obj = src/main.o src/pfGame.o src/pfConsole.o src/pfUI.o src/pfLocale.o src/pfAI.o src/vtsFilter.o src/pfRemotePlayer.o
flags = -std=c++17 -Wall -g -I$(BOOST_ROOT) -pthread
LIB_PATH = -L/home/stdent/boost_1_77_0/stage/lib
LIBS = -Wl,-Bstatic -lboost_locale -Wl,-Bdynamic

all: check-env planeFight

check-env:
ifndef BOOST_ROOT
	$(error Please define environment variable BOOST_ROOT.)
endif

planeFight: $(obj)
	g++ $(obj) -o $@ $(flags) $(LIB_PATH) $(LIBS) 

src/main.o: src/main.cpp \
            src/pfRemotePlayer.hpp src/pfGame.hpp src/pfUI.hpp src/pfLocale.hpp src/pfAI.hpp src/vtsFilter.hpp
	g++ -c $< -o $@ $(flags)

src/pfGame.o: src/pfGame.cpp src/pfGame.hpp
	g++ -c $< -o $@ $(flags)

src/pfConsole.o: src/pfConsole.cpp src/pfConsole.hpp src/vtsFilter.hpp
	g++ -c $< -o $@ $(flags)

src/pfUI.o: src/pfUI.cpp src/pfUI.hpp src/vtsFilter.hpp
	g++ -c $< -o $@ $(flags)

src/pfLocale.o: src/pfLocale.cpp src/pfUI.hpp src/pfLocale.hpp
	g++ -c $< -o $@ $(flags)

src/pfAI.o: src/pfAI.cpp src/pfGame.hpp src/pfUI.hpp
	g++ -c $< -o $@ $(flags)

src/vtsFilter.o: src/vtsFilter.cpp src/vtsFilter.hpp
	g++ -c $< -o $@ $(flags)

src/pfRemotePlayer.o: src/pfRemotePlayer.cpp src/pfRemotePlayer.hpp
	g++ -c $< -o $@ $(flags)

