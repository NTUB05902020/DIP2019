#include "Eigen/Eigen"
#include <dlib/optimization.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <iostream>
#include <thread>
#include <mutex>
#include <set>
#include <fstream>
#include <chrono>
#include <functional>
#include <random>
#include <limits>

#define N 8468
#define DIM 117
#define K 300
#define THREAD_MAX 128
#define LAMBDA 1.0
#define MARGIN 1e-7

#define Ma(R,C) Eigen::Matrix<double,R,C>
#define Ve(R) Eigen::Matrix<double,R,1>
#define Ma_D Ma(Eigen::Dynamic,Eigen::Dynamic)
#define Ve_D Ve(Eigen::Dynamic)


using std::mutex;
using std::thread;
using std::set;

Ma_D X(DIM,N), D(DIM,K), Z(K,N);
Ma_D DTD_INV(K,K), DIMKtmp(DIM,K);
Ma_D XZT(DIM,K), ZZT(K,K);

int haveItered;
const double inf = std::numeric_limits<double>::infinity();

double loss;
int semaphore;
mutex semMutex;
thread threads[THREAD_MAX];


FILE *myOpen(const char *name, const char *mode){
	FILE *ret = fopen(name, mode);
	if(ret == NULL){ printf("Failed to open %s\n", name); exit(1);}
	return ret;
}

void getData(){
	//get X
	FILE *fp = myOpen("train.dat", "r");
	for(int i=0;i<N;++i){
		for(int j=0;j<DIM;++j){
			if(fscanf(fp, "%lf", &X(j,i)) != 1){
				printf("train.dat has problem:  N = %d  DIM = %d\n", i, j);
				exit(1);
			}
		}
	}
	fclose(fp);
	//get D
	fp = myOpen("dict.dat", "r");
	char buffer[32];
	if(fscanf(fp, "%s%d", buffer, &haveItered) != 2){
		printf("dict.dat has problem: Iteration: [Num]\n");
		exit(1);
	}
	for(int i=0;i<K;++i){
		for(int j=0;j<DIM;++j){
			if(fscanf(fp, "%lf", &D(j,i)) != 1){
				printf("dict.dat has problem: K = %d  DIM = %d\n", i, j);
				exit(1);
			}
		}
	}
	fclose(fp);
}

void zThFunc(){
	while(1){
		semMutex.lock();
		if(semaphore == N){
			semMutex.unlock();
			break;
		}
		int now = semaphore++;
		semMutex.unlock();
		//do some thing
		Ve_D nX = X.col(now);
		dlib::matrix<double,K,1> nZ = dlib::mat<double,K,1,Eigen::ColMajor,K,1>(DTD_INV*D.transpose()*nX);
		std::function<double(const dlib::matrix<double,K,1> &)> objZ =
			[&](const dlib::matrix<double,K,1> &nZ){
				Ve_D tmp = -nX;
				double ret = 0.0;
				for(int i=0;i<K;++i){
					ret += abs(nZ(i));
					tmp += nZ(i) * D.col(i);
				}
				return LAMBDA*ret + tmp.squaredNorm();
			};
		std::function<dlib::matrix<double,K,1>(const dlib::matrix<double,K,1> &)> derObjZ =
			[&](const dlib::matrix<double,K,1> &nZ){
				Ve_D tmp = -nX;
				for(int i=0;i<K;++i) tmp += nZ(i) * D.col(i);
				tmp = D.transpose()*tmp*2;
				dlib::matrix<double,K,1> ret;
				for(int i=0;i<K;++i)
					if(nZ(i) < 0) ret(i) = tmp(i,0) - 1;
					else ret(i) = tmp(i,0) + 1;
				return ret;
			};
		
		double min_value = dlib::find_min(dlib::bfgs_search_strategy(), dlib::objective_delta_stop_strategy(MARGIN), objZ, derObjZ, nZ, 0.0);
		for(int i=0;i<K;++i) Z(i,now) = nZ(i);
		//if(now %500 == 499) printf("%d  %lf\n", now, min_value);
	}
}

void get_ZZTL_INV(const dlib::matrix<double,K,1> &L, Ma_D &ZZTL_INV){
	ZZTL_INV = ZZT;
	for(int i=0;i<K;++i) ZZTL_INV(i,i) += L(i);
	if(ZZTL_INV.fullPivLu().isInvertible()) ZZTL_INV = ZZTL_INV.inverse();
	else ZZTL_INV = ZZTL_INV.completeOrthogonalDecomposition().pseudoInverse();
}

double objL(const dlib::matrix<double,K,1> &L){
	get_ZZTL_INV(L, DTD_INV);
	return -(dlib::sum(L) + (XZT * DTD_INV * XZT.transpose()).trace());
}

dlib::matrix<double,K,1> derObjL(const dlib::matrix<double,K,1> &L){
	get_ZZTL_INV(L, DTD_INV);
	DIMKtmp = XZT * DTD_INV;
	dlib::matrix<double,K,1> ret;
	for(int i=0;i<K;++i) ret(i) = DIMKtmp.col(i).squaredNorm() - 1;
	return ret;
}


void lossThFunc(){
	while(1){
		semMutex.lock();
		if(semaphore == N){
			semMutex.unlock();
			break;
		}
		int now = semaphore++;
		semMutex.unlock();
		//do some thing
		double tmp = (D*Z.col(now) - X.col(now)).squaredNorm() + LAMBDA*(Z.col(now).template lpNorm<1>());
		semMutex.lock();
		loss += tmp;
		semMutex.unlock();
	}
}

int main(int argc, char **argv){
	if(argc != 3){
		printf("Format: %s [iteration] [threadNum]\n", argv[0]);
		exit(1);
	}
	int iteration = atoi(argv[1]), threadNum = atoi(argv[2]);
	if(threadNum > THREAD_MAX){
		printf("THREAD_MAX = %d\n", THREAD_MAX);
		exit(1);
	}
	getData();
	///*
	std::chrono::system_clock::time_point nowTime = std::chrono::system_clock::now();
	std::time_t nowCTIME = std::chrono::system_clock::to_time_t(nowTime);
	std::cout << std::ctime(&nowCTIME) << std::endl;
	//============================Start learning===============
	for(int t=0;t<iteration;++t){
		//learn Z
		DTD_INV = D.transpose() * D;
		if(DTD_INV.fullPivLu().isInvertible()) DTD_INV = DTD_INV.inverse();
		else DTD_INV = DTD_INV.completeOrthogonalDecomposition().pseudoInverse();
		
		semaphore = 0;
		for(int i=0;i<threadNum;++i) threads[i] = thread(zThFunc);
		for(int i=0;i<threadNum;++i) threads[i].join();
		
		//learn D
		XZT = X * Z.transpose();
		ZZT = Z * Z.transpose();
		
		dlib::matrix<double,K,1> L;
		double max_value = dlib::find_max_box_constrained
			(dlib::bfgs_search_strategy(), dlib::objective_delta_stop_strategy(MARGIN), objL, derObjL, L, 0.0, inf);
		get_ZZTL_INV(L, DTD_INV);
		D = XZT * DTD_INV.transpose();
		
		//write D
		FILE *fp = myOpen("dict.dat", "w");
		fprintf(fp, "Iteration: %d\n", ++haveItered);
		for(int i=0;i<K;++i){
			for(int j=0;j<DIM;++j) fprintf(fp, "%lf ", D(j,i));
			fprintf(fp, "\n");
		}
		fclose(fp);
		
		//compute loss
		semaphore = 0;  loss = 0.0;
		for(int i=0;i<threadNum;++i) threads[i] = thread(lossThFunc);
		for(int i=0;i<threadNum;++i) threads[i].join();
		fp = myOpen("loss.txt", "a");
		fprintf(fp, "%lf\n", loss);
		fclose(fp);
		
		nowTime = std::chrono::system_clock::now();
		nowCTIME = std::chrono::system_clock::to_time_t(nowTime);
		std::cout << t+1 << "    " << std::ctime(&nowCTIME) << std::endl;
	}
	//*/
	exit(0);
}
