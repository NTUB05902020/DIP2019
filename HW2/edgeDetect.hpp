#ifndef EDGEDETECT_HPP
#define EDGEDETECT_HPP
#include "raw.hpp"
#include "filter.hpp"
#include <cstring>
#include <cmath>
#include <set>
#define PI 3.14159265358979323846

using std::set;

namespace EdgeDetect{
	
	char orientation(double dx, double dy){
		double tmp = atan(dy/dx);
		const double a1 = PI/8, a2 = PI*3/8, a3 = PI*5/8, a4 = PI*7/8;
		if(tmp>=-a1 && tmp<a1) return '0';
		else if(tmp>=a1 && tmp<a2) return '1';
		else if(tmp>=-a2 && tmp<-a1) return '3';
		else return '2';
	}
	
	template<int size>
	bool outBound(int r, int c){
		return (r<0 || r>=size || c<0 || c>=size);
	}
	
	template<int size>
	Image<size> firstOrder(const Image<size> &in, const char method, const int th){
		Image<size+2> ex = in.template extend<3>();
		Image<size> ret(in); int i,j; double t1,t2;
		switch(method){
			case '3':{
			for(i=0;i<size;++i){
				for(j=0;j<size;++j){
					t1 = (double)ex.data[i+1][j+1]-ex.data[i+1][j];
					t2 = (double)ex.data[i+1][j+1]-ex.data[i+2][j+1];
					ret.data[i][j] = (sqrt(t1*t1+t2*t2) > th)? 255:0;
				}
			}
			return ret;
			}
			case 'R':{
			for(i=0;i<size;++i){
				for(j=0;j<size;++j){
					t1 = (double)ex.data[i+1][j+1]-ex.data[i+2][j+2];
					t2 = (double)ex.data[i+2][j+1]-ex.data[i+1][j+2];
					ret.data[i][j] = (sqrt(t1*t1+t2*t2) > th)? 255:0;
				}
			}
			return ret;
			}
			case 'P':{
			for(i=0;i<size;++i){
				for(j=0;j<size;++j){
					t1 = ((double)ex.data[i][j+2]+ex.data[i+1][j+2]+ex.data[i+2][j+2]-ex.data[i][j]-ex.data[i+1][j]-ex.data[i+2][j])/3.0;
					t2 = ((double)ex.data[i][j]+ex.data[i][j+1]+ex.data[i][j+2]-ex.data[i+2][j]-ex.data[i+2][j+1]-ex.data[i+2][j+2])/3.0;
					ret.data[i][j] = (sqrt(t1*t1+t2*t2) > th)? 255:0;
				}
			}
			return ret;
			}
			case 'S':{
			for(i=0;i<size;++i){
				for(j=0;j<size;++j){
					t1 = (ex.data[i][j+2]+ex.data[i+1][j+2]*2.0+ex.data[i+2][j+2]-ex.data[i][j]-ex.data[i+1][j]*2.0-ex.data[i+2][j])/4.0;
					t2 = (ex.data[i][j]+ex.data[i][j+1]*2.0+ex.data[i][j+2]-ex.data[i+2][j]-ex.data[i+2][j+1]*2.0-ex.data[i+2][j+2])/4.0;
					ret.data[i][j] = (sqrt(t1*t1+t2*t2) > th)? 255:0;
				}
			}
			return ret;
			}
			default:{
			printf("No such method in EdgeDetect::firstOrder\npossible methods: 3 R P S\n");
			exit(1);
			}
		}
	}
	
	template<int size>
	Image<size> secondOrder(const Image<size> &in, const char method, const double th, bool firstG){
		Image<size> pre = (firstG)? Filter::template filter<size,5>(in, Filter::LOG, Filter::LOGb):Image<size>(in);
		//second derivative
		double sder[size][size];
		if(method == '4') Filter::template filter<size,3>(pre, sder, Filter::N4, Filter::N4b);
		else if(method == 'N') Filter::template filter<size,3>(pre, sder, Filter::N8N, Filter::N8Nb);
		else if(method == 'S') Filter::template filter<size,3>(pre, sder, Filter::N8S, Filter::N8Sb);
		else{ printf("No such method in secondOrder\npossible methods: 4 N S\n"); exit(1);}
		//zero-crossing check
		Image<size> ret;
		for(int i=0;i<size;++i){
			for(int j=0;j<size;++j){
				if(!outBound<size>(i,j-1) && !outBound<size>(i,j+1) && sder[i][j-1]*sder[i][j+1]<0 && abs(sder[i][j-1]-sder[i][j+1])>th){
					ret.data[i][j] = 255; continue;
				}
				if(!outBound<size>(i-1,j+1) && !outBound<size>(i+1,j-1) && sder[i-1][j+1]*sder[i+1][j-1]<0 && abs(sder[i-1][j+1]-sder[i+1][j-1])>th){
					ret.data[i][j] = 255; continue;
				}
				if(!outBound<size>(i-1,j) && !outBound<size>(i+1,j) && sder[i-1][j]*sder[i+1][j]<0 && abs(sder[i-1][j]-sder[i+1][j])>th){
					ret.data[i][j] = 255; continue;
				}
				if(!outBound<size>(i-1,j-1) && !outBound<size>(i+1,j+1) && sder[i-1][j-1]*sder[i+1][j+1]<0 && abs(sder[i-1][j-1]-sder[i+1][j+1])>th){
					ret.data[i][j] = 255; continue;
				}
				ret.data[i][j] = 0;
			}
		}
		return ret;
	}
	
	template<int size>
	Image<size> canny(const Image<size> &in, const double tl, const double th){
		//step1
		Image<size> pre = Filter::filter(in, Filter::GAU, Filter::GAUb);
		//step2
		double f[size][size]; char o[size][size];
		Image<size+2> ex = pre.template extend<3>();
		for(int i=0;i<size;++i){
			for(int j=0;j<size;++j){
				double g1 = ((double)ex.data[i][j+2]+2*ex.data[i+1][j+2]+ex.data[i+2][j+2]-ex.data[i][j]-2*ex.data[i+1][j]-ex.data[i+2][j])/4.0;
				double g2 = ((double)ex.data[i+2][j]+2*ex.data[i+2][j+1]+ex.data[i+2][j+2]-ex.data[i][j]-2*ex.data[i][j+1]-ex.data[i][j+2])/4.0;
				f[i][j] = sqrt(g1*g1+g2*g2);
				o[i][j] = orientation(g1, g2);
			}
		}
		//step3,4
		set<int> candidate; Image<size> ret;
		for(int i=1;i<size-1;++i){
			for(int j=1;j<size-1;++j){
				if(f[i][j]>th){
					ret.data[i][j] = 255;
					continue;
				}else if(f[i][j]<tl) continue;
				switch(o[i][j]){
					case '0':
						if(f[i][j]<f[i][j-1] || f[i][j]<f[i][j+1]) f[i][j] = 0.0;
						else candidate.insert(i*size+j);
						break;
					case '1':
						if(f[i][j]<f[i-1][j+1] || f[i][j]<f[i+1][j-1]) f[i][j] = 0.0;
						else candidate.insert(i*size+j);
						break;
					case '2':
						if(f[i][j]<f[i-1][j] || f[i][j]<f[i+1][j]) f[i][j] = 0.0;
						else candidate.insert(i*size+j);
						break;
					case '3':
						if(f[i][j]<f[i-1][j-1] || f[i][j]<f[i+1][j+1]) f[i][j] = 0.0;
						else candidate.insert(i*size+j);
				}
			}
		}
		//step5
		bool changed;
		do{
			changed = false;
			for(set<int>::iterator iter=candidate.begin();iter!=candidate.end();){
				int i = *iter / size, j = *iter % size;
				if(ret.data[i-1][j-1]==255 || ret.data[i-1][j]==255 || ret.data[i-1][j+1]==255 || ret.data[i][j-1]==255
				|| ret.data[i][j+1]==255 || ret.data[i+1][j-1]==255 || ret.data[i+1][j]==255 || ret.data[i+1][j+1]==255){
					ret.data[i][j] = 255; changed = true;
					iter = candidate.erase(iter); continue;
				}else ++iter;
			}
		}while(changed);
		
		return ret;
	}
};

#endif