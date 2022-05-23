# CXX Make variable for compiler, Thomas Kestler 
CXX=g++

CXXFLAGS=-std=c++11 -g


rideshare : main.cpp BufferADT.h consumer.h producer.h producer.cpp consumer.cpp pc_unique.h io.cpp io.h ridesharing.h
	$(CXX) $(CXXFLAGS) -pthread -o rideshare $^ -lpthread -lrt



clean :
	rm *.o

