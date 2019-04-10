#ifndef RAW_HPP
#define RAW_HPP

#include <cstdio>
#include <cstdlib>
#include <cmath>

#define Data unsigned char

FILE *myOpen(const char *name, const char *mode){
	FILE *ret = fopen(name, mode);
	if(ret == NULL){
		printf("Failed to open %s\n", name);
		exit(1);
	}
	return ret;
}

template<int size>
class Image{
	public:
	Data data[size][size];
	Image(){
		for(int i=0;i<size;++i)
			for(int j=0;j<size;++j) data[i][j] = 0;
	}
	Image(const char *name){
		FILE *fp = myOpen(name, "rb");
		for(int i=0;i<size;++i)
			for(int j=0;j<size;++j) fread(&data[i][j], sizeof(Data), 1, fp);
		fclose(fp);
	}
	Image(const Image &other){
		for(int i=0;i<size;++i)
			for(int j=0;j<size;++j) data[i][j] = other.data[i][j];
	}
	template<int fiS> Image<size+fiS-1> extend() const{
		const int ext = fiS/2;
		Image<size+fiS-1> ret;
		//four corners
		for(int i=0;i<ext;++i){
			for(int j=0;j<ext;++j){
				ret.data[i][j] = data[i][j];
				ret.data[ext+size+i][j] = data[size-ext+i][j];
				ret.data[i][ext+size+j] = data[i][size-ext+j];
				ret.data[ext+size+i][ext+size+j] = data[size-ext+i][size-ext+j];	
			}
		}
		//four edges
		for(int i=0;i<ext;++i){
			for(int j=0;j<size;++j){
				ret.data[i][ext+j] = data[i][j];
				ret.data[ext+size+i][ext+j] = data[size-ext+i][j];
				ret.data[ext+j][i] = data[j][i];
				ret.data[ext+j][ext+size+i] = data[j][size-ext+i];
			}
		}
		//center
		for(int i=0;i<size;++i)
			for(int j=0;j<size;++j) ret.data[i][j] = data[i][j];
		return ret;
	}
	void writeRaw(const char *name) const{
		FILE *fp = myOpen(name, "wb");
		for(int i=0;i<size;++i)
			for(int j=0;j<size;++j) fwrite(&data[i][j], sizeof(Data), 1, fp);
		fclose(fp);
	}
};
#endif