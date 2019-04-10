#include "raw.hpp"
#include <cmath>

#define SIZE 512
#define TOTAL 262144
#define MASKSIZE 29
#define EXT MASKSIZE/2
#define TYPENUM 4
#define PASTESIZE 128

Data **im;
float ***pre;
float x[TOTAL][9] = {{0.0}};
int label[TOTAL] = {0};
float origins[TYPENUM][9] = {{0.0}};
int i, j, k, p, fi, fj, n;

float laws[9][3][3] = 
	{{{1/36.0,2/36.0,1/36.0},{2/36.0,4/36.0,2/36.0},{1/36.0,2/36.0,1/36.0}},
	 {{1/12.0,0.0,-1/12.0},{2/12.0,0.0,-2/12.0},{1/12.0,0.0,-1/12.0}},
	 {{-1/12.0,2/12.0,-1/12.0},{-2/12.0,4/12.0,-2/12.0},{-1/12.0,2/12.0,-1/12.0}},
	 {{-1/12.0,-2/12.0,-1/12.0},{0.0,0.0,0.0},{1/12.0,2/12.0,1/12.0}},
	 {{1/4.0,0.0,-1/4.0},{0.0,0.0,0.0},{-1/4.0,0.0,1/4.0}},
	 {{-1/4.0,2/4.0,-1/4.0},{0.0,0.0,0.0},{1/4.0,-2/4.0,1/4.0}},
	 {{-1/12.0,-2/12.0,-1/12.0},{2/12.0,4/12.0,2/12.0},{-1/12.0,-2/12.0,-1/12.0}},
	 {{-1/4.0,0.0,1/4.0},{2/4.0,0.0,-2/4.0},{-1/4.0,0.0,1/4.0}},
	 {{1/4.0,-2/4.0,1/4.0},{-2/4.0,4/4.0,-2/4.0},{1/4.0,-2/4.0,1/4.0}}};

void *myMalloc(size_t size){
	void *ret = malloc(size);
	if(ret == NULL){ printf("Failed to malloc\n"); exit(1);}
	return ret;
}

Data **getIm(){
	FILE *fp = myOpen("raw/sample2.raw", "rb");
	Data **ret = (Data **)myMalloc(sizeof(Data *) * (SIZE+2));
	for(i=0;i<SIZE+2;++i)
		ret[i] = (Data *)myMalloc(sizeof(Data) * (SIZE+2));
	for(i=0;i<SIZE;++i) for(j=0;j<SIZE;++j)
		fread(&ret[i+1][j+1], sizeof(Data), 1, fp);
	fclose(fp);
	for(i=0;i<SIZE;++i){
		ret[0][i+1] = ret[1][i+1];
		ret[SIZE+1][i+1] = ret[SIZE][i+1];
		ret[i+1][0] = ret[i+1][1];
		ret[i+1][SIZE+1] = ret[i+1][SIZE];
	}
	ret[0][0] = ret[1][1];
	ret[0][SIZE+1] = ret[1][SIZE];
	ret[SIZE+1][0] = ret[SIZE][1];
	ret[SIZE+1][SIZE+1] = ret[SIZE][SIZE];
	return ret;
}

void freeIm(Data **in){
	for(i=0;i<SIZE+2;++i) free(in[i]);
	free(in);
}

float ***getPre(){
	float ***pre = (float ***)myMalloc(sizeof(float **) * (MASKSIZE+SIZE-1));
	for(i=0;i<MASKSIZE+SIZE-1;++i){
		pre[i] = (float **)myMalloc(sizeof(float *) * (MASKSIZE+SIZE-1));
		for(j=0;j<MASKSIZE+SIZE-1;++j)
			pre[i][j] = (float *)myMalloc(sizeof(float) * 9);
	}
	return pre;
}

void freePre(float ***pre){
	for(i=0;i<MASKSIZE+SIZE-1;++i){
		for(j=0;j<MASKSIZE+SIZE-1;++j) free(pre[i][j]);
		free(pre[i]);
	}
	free(pre);
}

int main(){
	im = getIm();
	pre = getPre();
	//get pre
	for(i=0;i<SIZE;++i) for(j=0;j<SIZE;++j)
		for(k=0;k<9;++k)
			for(fi=0;fi<3;++fi) for(fj=0;fj<3;++fj)
				pre[EXT+i][EXT+j][k] += im[i+fi][j+fj] * laws[k][fi][fj];
	for(i=0;i<MASKSIZE/2;++i){ for(j=0;j<MASKSIZE/2;++j){
		for(k=0;k<9;++k){
			pre[i][j][k] = pre[EXT+i][EXT+j][k];
			pre[i][EXT+SIZE+j][k] = pre[EXT+i][SIZE+j][k];
			pre[EXT+SIZE+i][j][k] = pre[SIZE+i][EXT+j][k];
			pre[EXT+SIZE+i][EXT+SIZE+j][k] = pre[SIZE+i][SIZE+j][k];
		}
	}}
	for(i=0;i<SIZE;++i){ for(j=0;j<MASKSIZE/2;++j){
		for(k=0;k<9;++k){
			pre[EXT+i][j][k] = pre[EXT+i][EXT+j][k];
			pre[EXT+i][EXT+SIZE+j][k] = pre[EXT+i][SIZE+j][k];
			pre[j][EXT+i][k] = pre[EXT+j][EXT+i][k];
			pre[EXT+SIZE+j][EXT+i][k] = pre[SIZE+j][EXT+i][k];
		}
	}}
	freeIm(im);
	//get x
	for(i=0,n=0;i<SIZE;++i){ for(j=0;j<SIZE;++j,++n){
		for(k=0;k<9;++k){
			float squareSum = 0.0; float Sum = 0.0;
			for(fi=0;fi<MASKSIZE;++fi){ for(fj=0;fj<MASKSIZE;++fj){
				squareSum += (pre[i+fi][j+fj][k] * pre[i+fi][j+fj][k]);
				Sum += pre[i+fi][j+fj][k];
			}}
			x[n][k] = squareSum / (MASKSIZE*MASKSIZE);
		}
	}}
	freePre(pre);
	//start K-means
	for(k=0;k<9;++k){
		for(i=0;i<MASKSIZE;++i){ for(j=0;j<MASKSIZE;++j){
			origins[0][k] += x[SIZE*i+j][k];
			origins[1][k] += x[SIZE*i+(SIZE-MASKSIZE+j)][k];
			origins[2][k] += x[SIZE*(SIZE-MASKSIZE+i)+j][k];
			origins[3][k] += x[SIZE*(SIZE-MASKSIZE+i)+(SIZE-MASKSIZE+j)][k];
		}}
		origins[0][k] /= (MASKSIZE*MASKSIZE);
		origins[1][k] /= (MASKSIZE*MASKSIZE);
		origins[2][k] /= (MASKSIZE*MASKSIZE);
		origins[3][k] /= (MASKSIZE*MASKSIZE);
	}
	int iteration = 1;
	
	while(1){
		//update label
		bool flag = true;
		float d, newd, tmp;
		
		for(n=0;n<TOTAL;++n){
			d  = 0.0;
			for(k=0;k<9;++k){
				tmp = x[n][k] - origins[label[n]][k];
				d += (tmp * tmp);
			}
			for(p=0;p<TYPENUM;++p){
				newd = 0.0;
				for(k=0;k<9;++k){
					tmp = x[n][k] - origins[p][k];
					newd += (tmp * tmp);
				}
				if(newd < d){
					d = newd;
					flag = false; label[n] = p;
				}
			}
		}
		if(flag) break;
		//update origins
		for(p=0;p<TYPENUM;++p){
			for(k=0;k<9;++k) origins[p][k] = 0.0;
			int cnt = 0;
			for(n=0;n<TOTAL;++n){
				if(label[n] == p){
					++cnt;
					for(k=0;k<9;++k) origins[p][k] += x[n][k];
				}
			}
			if(cnt == 0){
				printf("TYPENUM %d  has no point in group\n", p);
				exit(1);
			}
			for(k=0;k<9;++k) origins[p][k] /= (double)cnt;
		}
		++iteration;
	}
	
	Data colors[TYPENUM];
	for(i=0;i<TYPENUM;++i) colors[i] = (255/(TYPENUM-1) * i);
	
	Image<SIZE> output;
	for(i=0,n=0;i<SIZE;++i)	for(j=0;j<SIZE;++j,++n)
		output.data[i][j] = colors[label[n]];
	output.writeRaw("output/2/label.raw");
	
	Image<SIZE> pic("raw/sample2.raw");
	Data newColor[PASTESIZE][PASTESIZE][TYPENUM];
	for(p=0;p<TYPENUM;++p){
		bool found = false;
		for(i=10;i<SIZE&&!found;++i){ for(j=10;j<SIZE&&!found;++j){
			bool fit = true;
			for(k=0;k<PASTESIZE;++k){ for(n=0;n<PASTESIZE;++n){
				if(label[SIZE*(i+k)+(j+n)] != p){
					fit = false; break;
				}
			}}
			if(fit) found = true;
		}}
		if(!found){
			printf("Can't find fit\n"); exit(1);
		}
		for(k=0;k<PASTESIZE;++k) for(n=0;n<PASTESIZE;++n)
			newColor[k][n][p] = pic.data[i+k][j+n];
	}
	
	
	for(i=0;i<SIZE;i+=PASTESIZE) for(j=0;j<SIZE;j+=PASTESIZE)
		for(k=0;k<PASTESIZE;++k) for(p=0;p<PASTESIZE;++p)
			output.data[i+k][j+p] = newColor[k][p][(label[(i+k)*SIZE+(j+p)] + 1) % TYPENUM];
	
	output.writeRaw("output/2/rearranged.raw");
	return 0;
}