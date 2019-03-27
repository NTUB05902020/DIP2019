CC = g++

all:
	rm -fr output
	rm -f prob1 prob2prob1
	mkdir output
	mkdir output/1/
	mkdir output/2/
	$(CC) prob1.cpp -o prob1
	./prob1
	$(CC) prob2.cpp -o prob2
	./prob2
prob1:
	rm -fr output
	rm -f prob1 prob2
	mkdir output
	mkdir output/1/
	$(CC) prob1.cpp -o prob1
	./prob1
prob2:
	rm -fr output
	rm -f prob1 prob2
	mkdir output
	mkdir output/2/
	$(CC) prob2.cpp -o prob2
	./prob2
