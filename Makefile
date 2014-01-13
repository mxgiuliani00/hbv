

####### Compiler, tools and options

CC            = gcc
CXX           = g++
CXXFLAGS      = -c  
TARGET	      = SimHBV

####### Compile
all: $(TARGET)

$(TARGET): main_HBV.o hbv_model.o utils.o moeaframework.o
	$(CXX) main_HBV.o hbv_model.o utils.o moeaframework.o -o $@

main_HBV.o: main_HBV.cpp hbv_model.h utils.h moeaframework.h
	$(CXX) $(CXXFLAGS) main_HBV.cpp

hbv_model.o: hbv_model.cpp hbv_model.h
	$(CXX) $(CXXFLAGS) hbv_model.cpp

utils.o: utils.cpp utils.h
	$(CXX) $(CXXFLAGS) utils.cpp

moeaframework.o: moeaframework.c moeaframework.h
	$(CXX) $(CXXFLAGS) moeaframework.c 

clean:
	rm -rf *.o 
	rm $(TARGET)
