obj = tmp/main.o tmp/pfUI.o tmp/pfLang.o tmp/pfAI.o tmp/vtsFilter.o
flags = -Wall -DUNICODE -g

planeFight.exe: $(obj)
	g++ $(obj) -o $@ $(flags) -lwsock32

tmp/main.o: src/main.cpp
	g++ -c $< -o $@ $(flags)

tmp/pfUI.o: src/pfUI.cpp
	g++ -c $< -o $@ $(flags)

tmp/pfLang.o: src/pfLang.cpp
	g++ -c $< -o $@ $(flags)

tmp/pfAI.o: src/pfAI.cpp
	g++ -c $< -o $@ $(flags)

tmp/vtsFilter.o: src/vtsFilter.cpp
	g++ -c $< -o $@ $(flags)