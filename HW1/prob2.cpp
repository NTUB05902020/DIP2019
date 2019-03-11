#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <algorithm>

using std::sort;

#define SIZE 256

unsigned char pic3[SIZE][SIZE];
FILE *fp;

FILE *myOpen(const char *name, const char *mode){
	FILE *ret = fopen(name, mode);
	if(ret == NULL){
		printf("Failed to open %s\n", name);
		exit(1);
	}
	return ret;
}

void *writeToPicture(unsigned char (*output)[SIZE], const char *fileName){
	FILE *fpTmp = myOpen(fileName, "wb");
	double mse= 0, tmp;
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			tmp = output[i][j] - pic3[i][j];
			mse += tmp*tmp;
			fwrite(&output[i][j], sizeof(unsigned char), 1, fpTmp);
		}
	}
	fclose(fpTmp);
	mse = 10*log(255.0*255*SIZE*SIZE/mse)/log(10);
	fprintf(fp, "%s  %lf\n", fileName, mse);
}


void extendBound(unsigned char (*in)[SIZE], unsigned char (*out)[2*SIZE], const int ext){
	const int tmp1 = ext+SIZE, tmp2 = SIZE-ext;
	//four corners
	for(int i=0;i<ext;++i){
		for(int j=0;j<ext;++j){
			out[i][j] = in[i][j];
			out[i][tmp1+j] = in[i][tmp2+j];
			out[tmp1+i][j] = in[tmp2+i][j];
			out[tmp1+i][tmp1+j] = in[tmp2+i][tmp2+j];			
		}
	}
	//four edges
	for(int i=0;i<ext;++i)
		for(int j=0;j<SIZE;++j) out[i][ext+j] = in[i][j];
	for(int i=tmp2;i<SIZE;++i)
		for(int j=0;j<SIZE;++j) out[ext+i][ext+j] = in[i][j];
	for(int j=0;j<ext;++j)
		for(int i=0;i<SIZE;++i) out[ext+i][j] = in[i][j];
	for(int j=tmp2;j<SIZE;++j)
		for(int i=0;i<SIZE;++i) out[ext+i][ext+j] = in[i][j];
	//middle
	for(int i=0;i<SIZE;++i)
		for(int j=0;j<SIZE;++j) out[ext+i][ext+j] = in[i][j];
}

template <class filType> void lowPass(unsigned char (*in)[SIZE], unsigned char (*out)[SIZE], filType filter, const int filterSize){
	/*---------------------bound extend----------------------*/
	int ext = filterSize/2;
	unsigned char extended[SIZE*2][SIZE*2];
	extendBound(in, extended, ext);
	
	/*---------------------convolution-----------------------*/
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			double sum = 0.0;
			for(int fi=0;fi<filterSize;++fi)
				for(int fj=0;fj<filterSize;++fj) sum += extended[i+fi][j+fj] * filter[fi][fj];
			out[i][j] = (unsigned char) round(sum);
		}
	}
}

void medianFilter(unsigned char (*in)[SIZE], unsigned char (*out)[SIZE], const int filterSize){
	unsigned char extended[SIZE*2][SIZE*2], tmp[SIZE];
	extendBound(in, extended, filterSize/2);
	for(int i=0;i<filterSize;++i){
		for(int j=0;j<filterSize;++j){
			int count = 0;
			for(int fi=0;fi<filterSize;++fi)
				for(int fj=0;fj<filterSize;++fj) tmp[count++] = extended[i+fi][j+fj];
			sort(tmp, &tmp[count]);
			out[i][j] = tmp[count/2];
		}
	}
}

void outlier(unsigned char (*in)[SIZE], unsigned char (*out)[SIZE], const int threshold){
	unsigned char ex[SIZE*2][SIZE*2];
	extendBound(in, ex, 1);
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			double sum = (ex[i][j] + ex[i][j+1] + ex[i][j+2] + ex[i+1][j] + ex[i+1][j+2] + ex[i+2][j] + ex[i+2][j+1] + ex[i+2][j+2])/8.0;
			out[i][j] = (abs(in[i][j]-sum) > threshold)? sum:in[i][j];
		}
	}
}

int main(){
	const double F1[3][3] = {{0.1, 0.1, 0.1}, {0.1, 0.2, 0.1}, {0.1, 0.1, 0.1}},
		F2[7][7] = {{1/84.0, 1/84.0, 1/84.0, 1/84.0, 1/84.0, 1/84.0, 1/84.0},
					{1/84.0, 2/84.0, 2/84.0, 2/84.0, 2/84.0, 2/84.0, 1/84.0},
					{1/84.0, 2/84.0, 3/84.0, 3/84.0, 3/84.0, 2/84.0, 1/84.0},
					{1/84.0, 2/84.0, 3/84.0, 4/84.0, 3/84.0, 2/84.0, 1/84.0},
					{1/84.0, 2/84.0, 3/84.0, 3/84.0, 3/84.0, 2/84.0, 1/84.0},
					{1/84.0, 2/84.0, 2/84.0, 2/84.0, 2/84.0, 2/84.0, 1/84.0},
					{1/84.0, 1/84.0, 1/84.0, 1/84.0, 1/84.0, 1/84.0, 1/84.0}};
	unsigned char pic4[SIZE][SIZE], pic5[SIZE][SIZE], out4[SIZE][SIZE], out5[SIZE][SIZE];
	
	/*-----------------------------read input image--------------------------------*/
	fp = myOpen("raw/sample3.raw", "wb");
	FILE *fp1 = myOpen("raw/sample4.raw", "rb"), *fp2 = myOpen("raw/sample5.raw", "rb");
	double tmp1=0.0, tmp2=0.0, tmp;
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			fread(&pic3[i][j], sizeof(unsigned char), 1, fp);
			fread(&pic4[i][j], sizeof(unsigned char), 1, fp1);
			fread(&pic5[i][j], sizeof(unsigned char), 1, fp2);
			tmp = pic4[i][j]-pic3[i][j];   tmp1 += tmp*tmp;
			tmp = pic5[i][j]-pic3[i][j];   tmp2 += tmp*tmp;
		}
	}
	fclose(fp); fclose(fp1); fclose(fp2);
	fp = myOpen("output/2/PSNR.txt", "w");
	fprintf(fp, "sample4.raw  %lf\n", 10*log(255.0*255*SIZE*SIZE/tmp1)/log(10));
	fprintf(fp, "sample5.raw  %lf\n", 10*log(255.0*255*SIZE*SIZE/tmp2)/log(10));
	/*----------------------------low pass filtering-------------------------------*/
	//================F1
	lowPass(pic4, out4, F1, 3);
	lowPass(pic5, out5, F1, 3);
	writeToPicture(out4, "output/2/4_low3.raw");
	writeToPicture(out5, "output/2/5_low3.raw");
	//================F2
	lowPass(pic4, out4, F2, 7);
	lowPass(pic5, out5, F2, 7);
	writeToPicture(out4, "output/2/4_low7.raw");
	writeToPicture(out5, "output/2/5_low7.raw");
	
	/*------------------------------median filtering-------------------------------*/
	//=================3X3 med
	medianFilter(pic4, out4, 3);
	medianFilter(pic5, out5, 3);
	writeToPicture(out4, "output/2/4_med3.raw");
	writeToPicture(out5, "output/2/5_med3.raw");
	//=================7X7 med
	medianFilter(pic4, out4, 7);
	medianFilter(pic5, out5, 7);
	writeToPicture(out4, "output/2/4_med7.raw");
	writeToPicture(out5, "output/2/5_med7.raw");
	
	/*-------------------------------outlier-------------------------------------------*/
	//=================20 threshold
	outlier(pic4, out4, 20);
	outlier(pic5, out5, 20);
	writeToPicture(out4, "output/2/4_out20.raw");
	writeToPicture(out5, "output/2/5_out20.raw");
	//=================80 threshold
	outlier(pic4, out4, 80);
	outlier(pic5, out5, 80);
	writeToPicture(out4, "output/2/4_out80.raw");
	writeToPicture(out5, "output/2/5_out80.raw");
	//=================160 threshold
	outlier(pic4, out4, 160);
	outlier(pic5, out5, 160);
	writeToPicture(out4, "output/2/4_out160.raw");
	writeToPicture(out5, "output/2/5_out160.raw");
	exit(0);
}