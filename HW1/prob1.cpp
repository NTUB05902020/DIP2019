#include <cstdio>
#include <cstdlib>
#include <cmath>

#define SIZE 256

FILE *myOpen(const char *name, const char *mode){
	FILE *ret = fopen(name, mode);
	if(ret == NULL){
		printf("Failed to open %s\n", name);
		exit(1);
	}
	return ret;
}

void histEqual(unsigned char (*in)[SIZE], unsigned char (*out)[SIZE], int rs, int cs, int R, int C){
	unsigned int histCount[256]={0};
	unsigned char min = 255;
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			min = (min>in[i][j])? in[i][j]:min;
			++histCount[in[i][j]];
		}
	}
	for(int i=1;i<256;++i) histCount[i] += histCount[i-1];
	double mul = 255.0/(R*C - min);
	unsigned char histHash[256];
	for(int i=0;i<256;++i) histHash[i] = (unsigned char) round((double)(histCount[i]-min) * mul);
	for(int i=0;i<R;++i)
		for(int j=0;j<C;++j) out[rs+i][cs+j] = histHash[in[rs+i][cs+j]];
}

int main(){
	/*------------read sample2.raw-----------------*/
	FILE *fp = myOpen("raw/sample2.raw", "rb");
	unsigned char buffer[SIZE][SIZE];
	for(int i=0;i<SIZE;++i)
		for(int j=0;j<SIZE;++j)
			fread(&buffer[i][j], sizeof(unsigned char), 1, fp);
	fclose(fp);
	/*------------generate D & E-------------------*/
	FILE *fpD, *fpE;
	unsigned int histArrI2[256] = {0}, histArrD[256] = {0}, histArrE[256] = {0};
	unsigned char D[SIZE][SIZE], E[SIZE][SIZE];
	fpD = myOpen("output/1/D.raw", "wb");
	fpE = myOpen("output/1/E.raw", "wb");
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			++histArrI2[buffer[i][j]];
			D[i][j] = (unsigned char) round(buffer[i][j]/2.0);
			E[i][j] = (unsigned char) round(buffer[i][j]/3.0);
			fwrite(&D[i][j], sizeof(unsigned char), 1, fpD);
			fwrite(&E[i][j], sizeof(unsigned char), 1, fpE);
			++histArrD[D[i][j]]; ++histArrE[E[i][j]];
		}
	}
	fclose(fpD); fclose(fpE);
	/*-----------generate histdat of D & E--------------*/
	fp = myOpen("output/1/I2.histdat", "w");
	fpD = myOpen("output/1/D.histdat", "w");
	fpE = myOpen("output/1/E.histdat", "w");
	for(int i=0;i<256;++i){
		fprintf(fp, "%u\n", histArrI2[i]);
		fprintf(fpD, "%u\n", histArrD[i]);
		fprintf(fpE, "%u\n", histArrE[i]);
	}
	fclose(fp); fclose(fpD); fclose(fpE);
	
	/*---------------global histogram equalization--------------*/
	unsigned char globalD[SIZE][SIZE], globalE[SIZE][SIZE];
	histEqual(D, globalD, 0, 0, SIZE, SIZE);
	histEqual(E, globalE, 0, 0, SIZE, SIZE);
	for(int i=0;i<256;++i) histArrD[i] = histArrE[i] = 0;
	fpD = myOpen("output/1/globalD.raw", "wb");
	fpE = myOpen("output/1/globalE.raw", "wb");
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			++histArrD[globalD[i][j]];
			++histArrE[globalE[i][j]];
			fwrite(&globalD[i][j], sizeof(unsigned char), 1, fpD);
			fwrite(&globalE[i][j], sizeof(unsigned char), 1, fpE);
		}
	}
	fclose(fpD); fclose(fpE);
	fpD = myOpen("output/1/globalD.histdat", "w");
	fpE = myOpen("output/1/globalE.histdat", "w");
	for(int i=0;i<256;++i){
		fprintf(fpD, "%u\n", histArrD[i]);
		fprintf(fpE, "%u\n", histArrE[i]);
	}
	fclose(fpD); fclose(fpE);
	
	/*----------------local histogram equalization---------------*/
	unsigned char localD[SIZE][SIZE], localE[SIZE][SIZE];
	for(int i=0;i<4;++i){
		for(int j=0;j<4;++j){
			histEqual(D, localD, i*SIZE/4, j*SIZE/4, SIZE/4, SIZE/4);
			histEqual(E, localE, i*SIZE/4, j*SIZE/4, SIZE/4, SIZE/4);
		}
	}
	for(int i=0;i<256;++i) histArrD[i] = histArrE[i] = 0;
	fpD = myOpen("output/1/localD.raw", "wb");
	fpE = myOpen("output/1/localE.raw", "wb");
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			++histArrD[localD[i][j]];
			++histArrE[localE[i][j]];
			fwrite(&localD[i][j], sizeof(unsigned char), 1, fpD);
			fwrite(&localE[i][j], sizeof(unsigned char), 1, fpE);
		}
	}
	fclose(fpD); fclose(fpE);
	fpD = myOpen("output/1/localD.histdat", "w");
	fpE = myOpen("output/1/localE.histdat", "w");
	for(int i=0;i<256;++i){
		fprintf(fpD, "%u\n", histArrD[i]);
		fprintf(fpE, "%u\n", histArrE[i]);
	}
	fclose(fpD); fclose(fpE); exit(0);
}