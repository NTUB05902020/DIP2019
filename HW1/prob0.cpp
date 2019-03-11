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



unsigned char powTrans(unsigned char initial, double exponent){
	return (unsigned char)round(pow(initial/256.0, exponent) * 256);
}

int main(){
	FILE *fpR = myOpen("raw/sample1.raw", "rb");
	
	unsigned char buffer[SIZE][SIZE];
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j) fread(&buffer[i][j], sizeof(unsigned char), 1, fpR);
		for(int j=0;j<SIZE/2;++j){
			unsigned char tmp = buffer[i][j];
			buffer[i][j] = buffer[i][SIZE-j-1];
			buffer[i][SIZE-j-1] = tmp;
		}
	}	
	fclose(fpR);
	
	FILE *fpO[9];  char name[64];
	fpR = myOpen("output/0/B.raw", "wb");
	for(int i=0;i<9;++i){
		sprintf(name, "output/0/%.1lfout.raw", 1.4+i*0.4);
		fpO[i] = myOpen(name, "wb");
	}
	
	for(int i=0;i<SIZE;++i){
		for(int j=0;j<SIZE;++j){
			fwrite(&buffer[i][j], sizeof(unsigned char), 1, fpR);
			for(int k=0;k<9;++k){
				unsigned char tmp = powTrans(buffer[i][j], 1.4+k*0.4);
				fwrite(&tmp, sizeof(unsigned char), 1, fpO[k]);
			}
		}
	}
	for(int i=0;i<9;++i) fclose(fpO[i]);
	fclose(fpR); exit(0);
}
