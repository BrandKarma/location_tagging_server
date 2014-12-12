all: server.exe analyzer_test.exe

server.exe: Server.cpp Server.hpp marshal.h marshal.cpp Utils.hpp locationAnalyzer.cpp locationAnalyzer.hpp
	g++ -Wno-deprecated Server.cpp marshal.cpp locationAnalyzer.cpp -o server.exe

analyzer_test.exe: locationAnalyzer.cpp locationAnalyzer.hpp Utils.hpp
	g++ -Wno-deprecated -DLOCATIONANALYZER_TEST locationAnalyzer.cpp -o analyzer_test.exe

clean:
	rm -rf *.exe *~ 

