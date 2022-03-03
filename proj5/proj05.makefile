proj05: proj05.student.o
	g++ proj05.student.o -o proj05 -lpthread

proj05.student.o: proj05.student.cpp
	g++ -Wall -c proj05.student.cpp -lpthread
