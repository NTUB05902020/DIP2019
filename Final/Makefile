all:
	rm -f getData learnD genDict genPic
	g++ -std=c++17 -Os getData.cpp -o getData `pkg-config --cflags --libs opencv`
	g++ -std=c++17 learnDict.cpp -Os -IEigen3/ -Idlib-19.17/ -o learnD -pthread -lcblas
	g++ -std=c++17 genDict.cpp -o genDict
	g++ -std=c++17 -Os genPic.cpp -IEigen3/ -Idlib-19/ -o genPic -pthread -lcblas `pkg-config --cflags --libs opencv`
clean:
	rm -f getData learnD genDict genPic