#ifndef FILTER_HPP
#define FILTER_HPP
#include "raw.hpp"

namespace Filter{
	const int  N4[3][3] = {{0,-1,0},{-1,4,-1},{0,-1,0}}; const int N4b = 4;
	const int N8N[3][3] = {{-1,-1,-1},{-1,8,-1},{-1,-1,-1}}; const int N8Nb = 8;
	const int N8S[3][3] = {{-2,1,-2},{1,4,1},{-2,1,-2}}; const int N8Sb = 8;
	const int LOG[5][5] = {{1,4,7,4,1},{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{1,4,7,4,1}}; const int LOGb = 273;
	const int GAU[5][5] = {{2,4,5,4,2},{4,9,12,9,4},{5,12,15,12,5},{4,9,12,9,4},{2,4,5,4,2}}; const int GAUb = 159;
	template <int size, int fiS>
	void filter(const Image<size> in, double (*out)[size], const int (*fil)[fiS], const int b){
		Image<size+fiS-1> ex = in.template extend<fiS>();
		Image<size> Imagetmp;
		for(int i=0;i<size;++i){
			for(int j=0;j<size;++j){
				out[i][j] = 0.0;
				for(int fi=0;fi<fiS;++fi)
					for(int fj=0;fj<fiS;++fj) out[i][j] += fil[fi][fj]*ex.data[i+fi][j+fj];
				out[i][j] /= (double)b;
			}
		}
	}
	
	template <int size, int fiS>
	Image<size> filter(const Image<size> in, const int (*fil)[fiS], int b){
		Image<size> ret;
		Image<size+fiS-1> ex = in.template extend<fiS>();
		for(int i=0;i<size;++i){
			for(int j=0;j<size;++j){
				int tmp = 0;
				for(int fi=0;fi<fiS;++fi)
					for(int fj=0;fj<fiS;++fj) tmp += fil[fi][fj]*ex.data[i+fi][j+fj];
				ret.data[i][j] = (Data)round((double)tmp/b);
			}
		}
		return ret;
	}
}
#endif