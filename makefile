
ThreadedTriangleCounting.out : StringTokenizer.o ThreadedTriangleCounting.cpp  graph.h  random.o 
	g++ -lpthread -O3 -g  -o ThreadedTriangleCounting.out StringTokenizer.o random.o ThreadedTriangleCounting.cpp 

StringTokenizer.o : StringTokenizer.cpp StringTokenizer.h
	g++ -O3 -c -g StringTokenizer.cpp StringTokenizer.h
	
random.o: random.h  random.cpp
	g++ -O3 -g -c  random.cpp


clean:
	rm -f *.o
	rm -f *.out
	rm -f *.gch
	rm -r *.dSYM

