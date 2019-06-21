#Meu make

run: main.o fileSystem.o
	g++ -o system main.o fileSystem.o

main.o: main.cpp fileSystem.h
	g++ -c main.cpp

fileSystem.o: fileSystem.cpp fileSystem.h
	g++ -c fileSystem.cpp