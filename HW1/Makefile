#把source files與raw資料夾放在一起

all:
	rm -f prob0 prob1 prob2
	rm -fr output
	mkdir output
	mkdir output/0
	mkdir output/1
	mkdir output/2
	g++ prob0.cpp -o prob0
	g++ prob1.cpp -o prob1
	g++ prob2.cpp -o prob2
	./prob0
	./prob1
	./prob2


prob0:
	rm -f prob0 prob1 prob2
	rm -fr output
	mkdir output
	mkdir output/0
	g++ prob0.cpp -o prob0
	./prob0


#prob1 會產生7個.histdat檔, 各有256行數字依序代表亮度為0~255的pixel個數
prob1:
	rm -f prob0 prob1 prob2
	rm -fr output
	mkdir output
	mkdir output/1
	g++ prob1.cpp -o prob1
	./prob1


#prob2 會產生一個PSNR.txt  裡面紀錄所有圖片與sample3.raw算PSNR出來的值
prob2:
	rm -f prob0 prob1 prob2
	rm -fr output
	mkdir output
	mkdir output/2
	g++ prob2.cpp -o prob2
	./prob2


clean:
	rm -fr output
	rm -f prob0 prob1 prob2
