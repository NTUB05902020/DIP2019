#include <experimental/filesystem>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <random>
#include "opencv2/opencv.hpp"

#define MAX_SIZE 640
#define SMALL_DIM 36
#define BIG_DIM 81

using namespace cv;
using std::string;

string path;
char currentName[64];
int dataCount = 0;
FILE *fp = NULL;
double smaArr[MAX_SIZE/3][MAX_SIZE/3][4];
double bigArr[MAX_SIZE][MAX_SIZE];
double coff[2] = {1.0/sqrt(BIG_DIM), 1.0/sqrt(SMALL_DIM)};

void writeData(const Mat &im){
	Mat blurred, small, extended;
	GaussianBlur(im, blurred, Size(3,3), 4.0, 4.0);
	int bR = blurred.rows, bC = blurred.cols;
	int sR = bR/3, sC = bC/3;
	
	small.create(sR,sC,CV_8U);
	for(int si=0,bi=0;si<sR;++si,bi+=3){
		uchar *src = blurred.ptr<uchar>(bi);
		uchar *dst = small.ptr<uchar>(si);
		for(int sj=0,bj=0;sj<sC;++sj,bj+=3) dst[sj] = src[bj];
	}
	imwrite("blurred.jpg", blurred);
	imwrite("small.jpg", small);
	copyMakeBorder(small, extended, 2, 2, 2, 2, BORDER_REPLICATE);
	
	for(int i=0;i<bR;++i){
		const uchar *ptr = im.ptr<uchar>(i);
		for(int j=0;j<bC;++j) bigArr[i][j] = coff[0] * ptr[j];
	}
	
	for(int i=0;i<sR;++i){
		uchar *ptr[5] = {extended.ptr<uchar>(i), extended.ptr<uchar>(i+1),
			extended.ptr<uchar>(i+2), extended.ptr<uchar>(i+3), extended.ptr<uchar>(i+4)};
		for(int j=0;j<sC;++j){
			smaArr[i][j][0] = coff[1] * ((double)ptr[2][j+3] - ptr[2][j+1]);
			smaArr[i][j][1] = coff[1] * ((double)ptr[3][j+2] - ptr[1][j+2]);
			smaArr[i][j][2] = coff[1] * ((double)ptr[2][j] + ptr[2][j+4] - 2.0*ptr[2][j+2]);
			smaArr[i][j][3] = coff[1] * ((double)ptr[0][j+2] + ptr[4][j+2] - 2.0*ptr[2][j+2]);
		}
	}
	
	int rTimes = sR-4, cTimes = sC-4;
	for(int rn=0,si=2,bi=6;rn<rTimes;++rn,++si,bi+=3){ for(int cn=0,sj=2,bj=6;cn<cTimes;++cn,++sj,bj+=3,++dataCount){
		double mean = 0.0;
		for(int di=0;di<9;++di) for(int dj=0;dj<9;++dj) mean += bigArr[bi+di][bj+dj];
		mean /= 81;
		for(int di=0;di<9;++di) for(int dj=0;dj<9;++dj) fprintf(fp, "%lf ", bigArr[bi+di][bj+dj]-mean);
		for(int di=0;di<3;++di) for(int dj=0;dj<3;++dj) fprintf(fp, "%lf %lf %lf %lf ", smaArr[bi+di][bj+dj][0], smaArr[bi+di][bj+dj][1], smaArr[bi+di][bj+dj][2], smaArr[bi+di][bj+dj][3]);
		fprintf(fp, "\n");
	}}
	printf("dataCount = %d\n", dataCount);
}

int main(int argc, char **argv){
	if(argc != 2){ printf("Format: %s [picFile]\n", argv[0]); exit(1);}
	if((fp = fopen("train.dat", "w")) == NULL){	printf("Failed to open train.dat\n"); exit(1);}
	
	writeData(imread(std::string(argv[1]), CV_LOAD_IMAGE_GRAYSCALE));  exit(0);
}