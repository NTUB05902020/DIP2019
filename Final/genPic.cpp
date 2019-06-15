#include <cstdio>
#include <cstdlib>
#include <cmath>
#include "opencv2/opencv.hpp"
#include "Eigen/Eigen"
#include <dlib/optimization.h>

#define MAX_SIZE 640
#define K 300
#define DIM 117
#define BIG_DIM 81
#define SMALL_DIM 36
#define LAMBDA 1.0
#define MARGIN 1e-7

#define Ma(R,C) Eigen::Matrix<double,R,C>
#define Ma_D Ma(Eigen::Dynamic,Eigen::Dynamic)
#define Ve(R) Ma(R,1)
#define Ve_D Ve(Eigen::Dynamic)

using namespace cv;

const double coff[2] = {1.0/sqrt(BIG_DIM), 1.0/sqrt(SMALL_DIM)};
int sR, sC, bR, bC, smallSize, bigSize;
Ma_D D(DIM,K);
Ma_D y_feature[4];
Ma_D bigMa;

Ma_D sD(SMALL_DIM,K), bD(BIG_DIM,K);
Ma_D upD(SMALL_DIM+54,K), leD(SMALL_DIM+54,K), bothD(SMALL_DIM+72,K);
Ma_D sDTD_INV(K,K), upDTD_INV(K,K), leDTD_INV(K,K), bothDTD_INV(K,K);

Ma_D *Dptr;
Ve_D Y;

Mat mat1, mat2;

FILE *myOpen(const char *name, const char *mode){
	FILE *ret = fopen(name, mode);
	if(ret == NULL){ printf("Failed to open %s\n", name); exit(1);}
	return ret;
}

void getInput(const char *oldName){
	FILE *fp = myOpen("dict.dat", "r");
	char buffer[32];
	if(fscanf(fp, "%s%d", buffer, &sR) != 2){ printf("Broken dict\n"); exit(1);}
	for(int i=0;i<K;++i){ for(int j=0;j<DIM;++j){
		if(fscanf(fp, "%lf", &D(j,i)) != 1){ printf("Broken dict.dat:  DIM = %d   K = %d\n", j, i);	exit(1);}
	}}
	fclose(fp);
	
	mat1 = imread(std::string(oldName), CV_LOAD_IMAGE_GRAYSCALE);
	sR = mat1.rows;  sC = mat1.cols;  bR = sR*3;  bC = sC*3;
	smallSize = sR*sC;  bigSize = bR*bC;  bigMa.resize(bR,bC);
	
	resize(mat1, mat2, Size(0,0), 3.0, 3.0, CV_INTER_LANCZOS4);
	imwrite("lanczos.jpg", mat2);
	resize(mat1, mat2, Size(0,0), 3.0, 3.0, CV_INTER_CUBIC);
	imwrite("bicubic.jpg", mat2);
	resize(mat1, mat2, Size(0,0), 3.0, 3.0, CV_INTER_LINEAR);
	imwrite("bilinear.jpg", mat2);
	
	copyMakeBorder(mat1, mat2, 2, 2, 2, 2, BORDER_REPLICATE);
	for(int i=0;i<4;++i) y_feature[i].resize(sR, sC);
	for(int i=0;i<sR;++i){
		uchar *ptr[5] = { mat2.ptr<uchar>(i), mat2.ptr<uchar>(i+1), mat2.ptr<uchar>(i+2), mat2.ptr<uchar>(i+3), mat2.ptr<uchar>(i+4)};
		for(int j=0;j<sC;++j){
			y_feature[0](i,j) = coff[1] * ((double)ptr[2][j+3] - ptr[2][j+1]);
			y_feature[1](i,j) = coff[1] * ((double)ptr[3][j+2] - ptr[1][j+2]);
			y_feature[2](i,j) = coff[1] * ((double)ptr[2][j] + ptr[2][j+4] - 2.0*ptr[2][j+2]);
			y_feature[3](i,j) = coff[1] * ((double)ptr[0][j+2] + ptr[4][j+2] - 2.0*ptr[2][j+2]);
		}
	}
}

Ma_D INV(const Ma_D &ma){
	if(ma.fullPivLu().isInvertible()) return ma.inverse();
	else return ma.completeOrthogonalDecomposition().pseudoInverse();
}

void getAllDs(){
	for(int i=0;i<BIG_DIM;++i) bD.row(i) = D.row(i);
	for(int i=0,now=BIG_DIM;i<SMALL_DIM;++i,++now) sD.row(i) = leD.row(i) = upD.row(i) = bothD.row(i) = D.row(now);
	Ma_D pickLeft = Ma_D::Zero(54,BIG_DIM), pickUp = Ma_D::Zero(54,BIG_DIM);
	Ma_D pickBoth = Ma_D::Zero(72,BIG_DIM);
	for(int i=0,now=0;i<54;i+=9) for(int j=0;j<9;++j,++now)
		pickUp(now,i+j) = pickBoth(now,i+j) = 1.0;
	for(int i=0,now=0;i<81;i+=9) for(int j=0;j<6;++j,++now)
		pickLeft(now,i+j) = 1.0;
	for(int i=54,now=54;i<81;i+=9) for(int j=0;j<6;++j,++now)
		pickBoth(now,i+j) = 1.0;
	pickLeft *= bD;  pickUp *= bD;  pickBoth *= bD;
	for(int i=0,now=SMALL_DIM;i<54;++i,++now){
		leD.row(now) = pickLeft.row(i);
		upD.row(now) = pickUp.row(i);
		bothD.row(now) = pickBoth.row(i);
	}
	for(int i=54,now=SMALL_DIM+54;i<72;++i,++now) bothD.row(now) = pickBoth.row(i);
	sDTD_INV = INV(sD.transpose()*sD);  leDTD_INV = INV(leD.transpose()*leD);
	upDTD_INV = INV(upD.transpose()*upD);  bothDTD_INV = INV(bothD.transpose()*bothD);
}

void fillY(int oriR, int oriC){
	for(int i=0,now=0;i<3;++i) for(int j=0;j<3;++j)
		for(int f=0;f<4;++f,++now) Y(now,0) = y_feature[f](oriR+i,oriC+j);
}

void writeBigMa(int oriR, int oriC){
	for(int i=0,now=0;i<9;++i) for(int j=0;j<9;++j,++now)
		bigMa(oriR+i,oriC+j) = Y(now,0);
}

double Obj(const dlib::matrix<double,K,1> &alpha){
	Ve_D tmp = -Y;  double ret = 0.0;
	for(int i=0;i<K;++i){
		tmp += alpha(i) * (Dptr->col(i));
		ret += abs(alpha(i));
	}
	return LAMBDA*ret + tmp.squaredNorm();
}

dlib::matrix<double,K,1> derObj(const dlib::matrix<double,K,1> &alpha){
	Ve_D tmp = -Y;
	for(int i=0;i<K;++i) tmp += alpha(i) * (Dptr->col(i));
	tmp = (Dptr->transpose()) * tmp * 2;
	dlib::matrix<double,K,1> ret;
	for(int i=0;i<K;++i) ret(i) = (alpha(i)<0)? (tmp(i,0)-LAMBDA):(tmp(i,0)+LAMBDA);
	return ret;
}

Ve_D getAlpha(const Ve_D &alphaV){
	dlib::matrix<double,K,1> alpha = dlib::mat<double,K,1,Eigen::ColMajor,K,1>(alphaV);
	Ve_D ret(K);  ret(0,0) = dlib::find_min(dlib::bfgs_search_strategy(), dlib::objective_delta_stop_strategy(MARGIN), Obj, derObj, alpha, 0.0);
	for(int i=0;i<K;++i) ret(i,0) = alpha(i);
	return ret;
}

void freeAllMa(){
	D.resize(0,0);
	for(int i=0;i<4;++i) y_feature[i].resize(0,0);
	sD.resize(0,0);  leD.resize(0,0);  upD.resize(0,0);  bothD.resize(0,0);
	sDTD_INV.resize(0,0);  leDTD_INV.resize(0,0);
	upDTD_INV.resize(0,0);  bothDTD_INV.resize(0,0);
}

int main(int argc, char **argv){
	if(argc != 2){ printf("Format: %s [picName]\n", argv[0]); exit(1);}
	char oldName[32];  sscanf(argv[1], "%s", oldName);
	getInput(oldName);  getAllDs();
	int rTimes = sR-2, cTimes = sC-2;  Ve_D alphaV(K);
	//learnNew
	
	//1.left-up corner
	Dptr = &sD;  Y.resize(SMALL_DIM,1);  fillY(0,0);
	alphaV = getAlpha(sDTD_INV*sD.transpose()*Y);
	Y = bD * alphaV;  writeBigMa(0,0);
	//2.only left
	Dptr = &leD;
	for(int c=1,bc=3;c<cTimes;++c,bc+=3){
		Y.resize(SMALL_DIM+54,1);  fillY(0,c);
		for(int i=0,now=SMALL_DIM;i<9;++i) for(int j=0;j<6;++j,++now) Y(now,0) = bigMa(i,bc+j);
		alphaV = getAlpha(leDTD_INV*leD.transpose()*Y);
		Y = bD * alphaV;  writeBigMa(0,bc);
	}
	//3.+4.
	for(int r=1,br=3;r<rTimes;++r,br+=3){
		printf("%d\n", r);
		//3.only up
		Dptr = &upD;  Y.resize(SMALL_DIM+54,1);  fillY(r,0);
		for(int i=0,now=SMALL_DIM;i<6;++i) for(int j=0;j<9;++j,++now) Y(now,0) = bigMa(br+i,j);
		alphaV = getAlpha(upDTD_INV*upD.transpose()*Y);
		Y = bD * alphaV;  writeBigMa(br,0);
		//4.both
		Dptr = &bothD;
		for(int c=1,bc=3;c<cTimes;++c,bc+=3){
			Y.resize(SMALL_DIM+72,1);  fillY(r,c);
			for(int i=0,now=SMALL_DIM;i<6;++i) for(int j=0;j<9;++j,++now) Y(now,0) = bigMa(br+i,bc+j);
			for(int i=6,now=SMALL_DIM+54;i<9;++i) for(int j=0;j<6;++j,++now) Y(now,0) = bigMa(br+i,bc+j);
			alphaV = getAlpha(bothDTD_INV*bothD.transpose()*Y);
			Y = bD * alphaV;  writeBigMa(br,bc);
		}
	}
	printf("%d\n", rTimes);
	//output Pic
	
	freeAllMa();
	mat2.create(bR,bC,CV_8U);
	bigMa = bigMa / coff[0];
	for(int si=0,bi=0;si<rTimes;++si,bi+=3){ for(int sj=0,bj=0;sj<cTimes;++sj,bj+=3){
		double meanVal = 0.0;
		for(int di=0;di<3;++di){
			uchar *ptr = mat1.ptr<uchar>(si+di);
			for(int dj=0;dj<3;++dj) meanVal += ptr[sj+dj];
		}
		meanVal /= 9;
		for(int di=0;di<9;++di){
			uchar *ptr = mat2.ptr<uchar>(bi+di);
			for(int dj=0;dj<9;++dj){
				double value = bigMa(bi+di,bj+dj) + meanVal;
				if(value >= 255.0) ptr[bj+dj] = 255;
				else if(value <= 0.0) ptr[bj+dj] = 0;
				else ptr[bj+dj] = (uchar)round(value);
			}
		}
	}}
	imwrite("prebigger.jpg", mat2);
	
	/*for(int i=0;i<bR;++i){
		uchar *ptr = mat2.ptr<uchar>(i);
		for(int j=0;j<bC;++j) bigMa(i,j) = ptr[j];
	}
	Ma_D lastBigMa = bigMa, sparseSmaMa = Ma_D::Zero(bR,bC);
	for(int si=0,bi=0;si<sR;++si,bi+=3){
		uchar *ptr = mat1.ptr<uchar>(si);
		for(int sj=0,bj=0;sj<sC;++sj,bj+=3) sparseSmaMa(bi,bj) = ptr[sj];
	}
	Ma_D forDer = bigMa + sparseSmaMa;
	double loss = dlib::find_min(dlib::bfgs_search_strategy(), dlib::objective_delta_stop_strategy(MARGIN), picObj, picderObj, bigDlibV, 0.0);
	printf("\nloss = %lf\n", loss);
	for(int i=0,now=0;i<bR;++i){
		uchar *ptr = mat2.ptr<uchar>(i);
		for(int j=0;j<bC;++j,++now){
			if(bigDlibV(now) >= 255.0) ptr[j] = 255;
			else if(bigDlibV(now) <= 0.0) ptr[j] = 0;
			else ptr[j] = (uchar)round(bigDlibV(now));
		}
	}
	imwrite("bigger.jpg", mat2);
	*/exit(0);
}