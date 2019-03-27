#include "raw.hpp"
#include "filter.hpp"
#include <cmath>
#define PI 3.14159265358979323846

const int lowPass[3][3] = {{0,1,0},{1,4,1},{0,1,0}}; const int lowPassb = 8;
const int highPass5[3][3] = {{0,-1,0},{-1,5,-1},{0,-1,0}};
const int highPass9[3][3] = {{-1,-1,-1},{-1,9,-1},{-1,-1,-1}};

template <int size>
double itox(int i){
	return (i-size/2+0.5)/127.5;
}

template <int size>
int xtoi(double x){
	return round(x*127.5-0.5+size/2);
}

template <int size>
Image<size> circulize(const Image<size> &in){
	Image<size> ret;
	for(int i=0;i<size;++i){
		for(int j=0;j<size;++j){
			ret.data[i][j] = 255;
			double x = itox<size>(i), y = itox<size>(j);
			if(x*x+y*y > 1.0) continue;
			
			double t = 1 - 4*x*x*y*y;
			if(t < 0) continue;
			t = 1 - sqrt(t);
			if(t < 0) continue;
			double tmp = x*y;
			double sign = (tmp==0.0)? 0.0:((tmp<0.0)? -1.0:1.0);
			int oi = xtoi<size>(sign*sqrt(t)/y/sqrt(2.0));
			int oj = xtoi<size>(sign*sqrt(t)/x/sqrt(2.0));
			ret.data[i][j] = in.data[oi][oj];			
		}
	}
	return ret;
}

template <int size>
Image<size> whirl(const Image<size> &in, const double t){
	Image<size> ret;
	for(int i=0;i<size;++i){
		for(int j=0;j<size;++j){
			double x = itox<size>(i), y = itox<size>(j);
			if(x*x+y*y > 1.0){ ret.data[i][j] = 255; continue;}
			double a = t/sqrt(x*x+y*y);
			double ox = x*cos(a) - y*sin(a), oy = x*sin(a) + y*cos(a);
			int oi = xtoi<size>(ox), oj = xtoi<size>(oy);
			ret.data[i][j] = in.data[oi][oj];
		}
	}
	return ret;
}

template <int size>
Image<size> edgeCrispening(const Image<size> &in, double c){
	Image<size> ret = Filter::filter<size, 3>(in, lowPass, lowPassb);
	for(int i=0;i<size;++i)
		for(int j=0;j<size;++j)
			ret.data[i][j] = (Data)round(in.data[i][j]*c/(2*c-1) - ret.data[i][j]*(1-c)/(2*c-1));
	return ret;
}

template <int size>
Image<size> edgeCrispening(const Image<size> &in, char method){
	switch(method){
		case '5': return Filter::filter(in, highPass5, 1);
		case '9': return Filter::filter(in, highPass9, 1);
		default:
			printf("No such method in edgeCrispening\nPossible methods: 5, 9\n");
			exit(1);
	}
}

int main(){
	Image<256> im("raw/sample4.raw");
	edgeCrispening<256>(im, '5').writeRaw("output/2/crispened5.raw");
	edgeCrispening<256>(im, '9').writeRaw("output/2/crispened9.raw");
	edgeCrispening<256>(im, 5.0/6).writeRaw("output/2/unsharp_56.raw");
	Image<256> crispened = edgeCrispening<256>(im, 3.0/5);
	crispened.writeRaw("output/2/unsharp_35.raw");
	Image<256> circle = circulize<256>(crispened);
	circle.writeRaw("output/2/circle.raw");
	whirl<256>(circle, 0.4).writeRaw("output/2/whirl_0.4.raw");
	whirl<256>(circle, 0.6).writeRaw("output/2/whirl_0.6.raw");
	exit(0);
}