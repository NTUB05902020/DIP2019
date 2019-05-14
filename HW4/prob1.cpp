#include "raw.hpp"
#include <cmath>
#include <set>

using std::set;

#define MY_PI 3.1415926

const double t_0 = 0.0;
const double t_45 = MY_PI/4, t_90 = MY_PI/2;
const double t_135 = -t_45, t_180 = -t_90;

const double t_225 = MY_PI/8, t_1575 = -t_225;
const double t_675 = t_225+t_45, t_1125 = -t_675;

enum Direction{D0, D45, D90, D135};

Direction getDir(double angle){
	if(angle >= t_1125 && angle < t_1575) return D135;
	if(angle >= t_1575 && angle < t_225) return D0;
	if(angle >= t_225 && angle < t_675) return D45;
	return D90;
}

template<int size>
Image<size> Canny(const Image<size> &im, double th1, double th2){
	//1.BLUR
	double gau[5][5] = 
	{{2/159.0, 4/159.0, 5/159.0, 4/159.0, 2/159.0},
	{4/159.0, 9/159.0, 12/159.0, 9/159.0, 4/159.0},
	{5/159.0, 12/159.0, 15/159.0, 12/159.0, 5/159.0},
	{4/159.0, 9/159.0, 12/159.0, 9/159.0, 4/159.0},
	{2/159.0, 4/159.0, 5/159.0, 4/159.0, 2/159.0}};
	Image<size+4> ext = im.template extend<5>();
	Image<size> blur;
	for(int i=0;i<size;++i){
		for(int j=0;j<size;++j){
			double sum = 0.0;
			for(int fi=0;fi<5;++fi) for(int fj=0;fj<5;++fj)
				sum += ext.data[i+fi][j+fj] * gau[fi][fj];
			blur.data[i][j] = (Data)round(sum);
		}
	}
	//2.GRADIENT CALCULATION
	int sobel1[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};
	int sobel2[3][3] = {{1,2,1},{0,0,0},{-1,-2,-1}};
	double gradient[size+2][size+2];
	Direction grDir[size+2][size+2];
	ext = blur.template extend<5>();
	for(int i=0;i<size+2;++i){
		for(int j=0;j<size+2;++j){
			int gx = 0, gy = 0;
			for(int fi=0;fi<3;++fi){
				for(int fj=0;fj<3;++fj){
					gx += ext.data[i+fi][j+fj] * sobel1[fi][fj];
					gy += ext.data[i+fi][j+fj] * sobel2[fi][fj];
				}
			}
			gradient[i][j] = sqrt(gx*gx + gy*gy);
			grDir[i][j] = getDir(atan2(gy,gx));
		}
	}
	//3.NON-MAXIMUM SUPPRESSION
	for(int i=0;i<size;++i){
		for(int j=0;j<size;++j){
			double g1, g2;
			switch(grDir[i+1][j+1]){
			case D0:
				g1 = gradient[i+1][j];  g2 = gradient[i+1][j+2];  break;
			case D45:
				g1 = gradient[i][j+2];  g2 = gradient[i+2][j];  break;
			case D90:
				g1 = gradient[i][j+1];  g2 = gradient[i+2][j+1];  break;
			default:
				g1 = gradient[i][j];  g2 = gradient[i+2][j+2];  break;
			}
			if(gradient[i+1][j+1] <= g1 || gradient[i+1][j+1] <= g2) gradient[i+1][j+1] = 0;
		}
	}
	//4.DOUBLE THRESHOLDING
	bool edge[size][size] = {{false}};
	bool cand[size][size] = {{false}};
	for(int i=0;i<size;++i){
		for(int j=0;j<size;++j){
			if(gradient[i+1][j+1] > th2) edge[i][j] = true;
			else if(gradient[i+1][j+1] > th1) cand[i][j] = true;
		}
	}
	//5.Hysteresis
	bool flag = true;
	while(flag){
		flag = false;
		for(int i=0;!flag && i<size;++i){
			for(int j=0;!flag && j<size;++j){
				if(edge[i][j]){
					int i1, i2, j1, j2;
					switch(grDir[i+1][j+1]){
					case D0:
						j1 = j-1;  j2 = j+1;
						i1 = i2 = i;  break;
					case D45:
						i1 = i-1;  j1 = j+1;
						i2 = i+1;  j2 = j-1;
						break;
					case D90:
						i1 = i-1;  i2 = i+1;
						j1 = j2 = j;  break;
					default:
						i1 = i-1;  j1 = j-1;
						i2 = i+1;  j2 = j+1;						
					}
					if(i1>=0 && i1<size && j1>=0 && j1<size && cand[i1][j1]){
						cand[i1][j1] = false;
						edge[i1][j1] = true;
						flag = true;
					}
					if(i2>=0 && i2<size && j2>=0 && j2<size && cand[j1][j2]){
						cand[i2][j2] = false;
						edge[i2][j2] = true;
						break;
					}
				}
			}
		}
	}
	Image<size> ret;
	for(int i=0;i<size;++i) for(int j=0;j<size;++j)
		if(edge[i][j]) ret.data[i][j] = 255;
	return ret;
}


template<int inSize, int outSize>
Image<outSize> Hough(const Image<inSize>&im, unsigned int (*acArray)[outSize]){
	for(int i=0;i<outSize;++i) for(int j=0;j<outSize;++j)
		acArray[i][j] = 0;
	
	double maxR = inSize/sqrt(2);
	const int outhSize = outSize/2;
	const int inhSize = inSize/2;
	
	for(int ti=0;ti<outSize;++ti){
		for(int rj=0;rj<outSize;++rj){
			double t = (double)(outhSize-ti-0.5)/outhSize * MY_PI - t_90;
			double r = (double)rj/outhSize * maxR;
			double c = cos(t), s = sin(t);
			if(t < t_45 && t > t_135){
				for(int i=0;i<inSize;++i){
					double y = inhSize - i - 0.5;
					double x = (r-y*s)/c;
					int j = (int)round(x+inhSize-0.5);
					if(j >= 0 && j < inSize && im.data[i][j]==255) ++acArray[ti][rj];
				}
			}else{
				for(int j=0;j<inSize;++j){
					double x = j - inhSize + 0.5;
					double y = (r-x*c)/s;
					int i = (int)round(inhSize-y-0.5);
					if(i >= 0 && i < inSize && im.data[i][j]==255) ++acArray[ti][rj];
				}
			}
			
		}
	}
	
	int maxCnt = -1;
	for(int i=0;i<outSize;++i) for(int j=0;j<outSize;++j)
		if(maxCnt < acArray[i][j]) maxCnt = acArray[i][j];
	
	double mul = 255.0/maxCnt;
	Image<outSize> ret;
	for(int i=0;i<outSize;++i){
		for(int j=0;j<outSize;++j){
			int tmp = (int)round(acArray[i][j]*mul);
			ret.data[i][j] = (tmp>255)? 255:tmp;
		}
	}
	return ret;
}

class Test{
public:
	int i, j;
	unsigned int num;
	
	Test(int ii, int jj, unsigned int numnum){
		i = ii;  j = jj; num = numnum;
	}
	
	bool operator < (const Test& t) const{
		if(num < t.num) return true;
		else if(num > t.num) return false;
		else if(i < t.i) return true;
		else if(i > t.i) return false;
		else return (j < t.j)? true:false;		
	}
};

template<int size>
Image<size> enhance(const Image<size> &im){
	Data min=255, max=0;
	for(int i=0;i<size;++i){
		for(int j=0;j<size;++j){
			if(im.data[i][j] < min) min = im.data[i][j];
			if(im.data[i][j] > max) max = im.data[i][j];
		}
	}
	Image<size> ret;
	for(int i=0;i<size;++i){
		for(int j=0;j<size;++j){
			int tmp = (int)round((double)(im.data[i][j]-min)/(max-min)*255.0);
			ret.data[i][j] = (tmp > 255)? 255:tmp;			
		}
	}
	return ret;
}


int main(){
	Image<256> im("raw/sample1.raw");
	//----------------------firstOrder------------------------
	Image<256> E = Canny<256>(im, 50.0, 100.0);
	E.writeRaw("output/E.raw");
	
	//------------------Hough Transform-----------------------
	unsigned int acArray[256][256] = {{0}};
	Image<256> reH = Hough<256,256>(E, acArray);
	Image<256> H1(reH);
	//H1.writeRaw("output/H1re.raw");
	for(int i=0;i<256;++i){
		for(int j=0;j<i;++j){
			Data tmp = reH.data[255-i][j];
			reH.data[255-i][j] = reH.data[255-j][i];
			reH.data[255-j][i] = tmp;
		}
	}
	reH.writeRaw("output/H1.raw");
	
	
	enhance<256>(reH).writeRaw("output/H2.raw");
	
	
	
	
	set<Test> s;
	for(int i=0;i<256;++i) for(int j=0;j<256;++j)
		s.insert(Test(i,j,acArray[i][j]));
	
	int Ns[2] = {10, 20};
	for(int ni=0;ni<2;++ni){
		int n = Ns[ni];
		Data out[3][256][256];
		for(int i=0;i<256;++i) for(int j=0;j<256;++j)
			out[0][i][j] = out[1][i][j] = out[2][i][j] = im.data[i][j];
		
		auto it = s.rbegin();
		for(int cnt=0;cnt<n;++cnt,++it){
			int ti=it->i, rj=it->j;
			double t = (double)(127.5-ti)/128 * MY_PI - t_90;
			double r = (double)rj*sqrt(2);
			double s = sin(t), c = cos(t);
			if(s == 0){
				int j1 = (int)round(r+127.5), j2 = (int)round(-r+127.5);
				if(j1>=0 && j1<256)
					for(int i=0;i<256;++i) out[0][i][j1] = 255;
				if(j2>=0 && j2<256)
					for(int i=0;i<256;++i) out[0][i][j2] = 255;
			}else{
				for(int j=0;j<256;++j){
					double x = j - 127.5;
					double y = (r-c*x)/s;
					int i = (int)round(127.5 - y);
					if(i >= 0 && i < 256) out[0][i][j] = 255;
				}
			}
		}
		char name[32];
		sprintf(name, "output/out%d.raw", n);
		FILE *fp = fopen(name, "wb");
		for(int k=0;k<3;++k)
			for(int i=0;i<256;++i) for(int j=0;j<256;++j)
				fwrite(&out[k][i][j], sizeof(Data), 1, fp);
		fclose(fp);
	}
	
	
	exit(0);
}