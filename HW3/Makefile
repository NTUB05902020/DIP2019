# DIP Homework Assignment #3
all:
	rm -fr output/
	mkdir output/
	mkdir output/1/
	mkdir output/2/
	rm -f prob1 prob2
	g++ -std=c++11 prob1.cpp -o prob1
	g++ -std=c++11 prob2.cpp -o prob2
	./prob1
	./prob2
prob1:
	rm -fr output/
	mkdir output/1/
	rm -f prob1
	g++ -std=c++11 prob1.cpp -o prob1
	./prob1
prob2:
	rm -fr output/
	mkdir output/2/
	rm -f prob2
	g++ -std=c++11 prob2.cpp -o prob2
	./prob2
clean:
	rm -fr output/
	rm -f prob1 prob2