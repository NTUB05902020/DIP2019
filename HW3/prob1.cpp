#include "morph.hpp"

int main(){
	Image<256> pic("raw/sample1.raw");
	//edge extraction
	Morph::boundary<256>(pic, '4').writeRaw("output/1/edge4.raw");
	Morph::boundary<256>(pic, '8').writeRaw("output/1/edge8.raw");
	//connected component labeling
	int label[256][256] = {{0}};
	int groupNum = Morph::labeling<256>(pic, label, '8');
	Morph::generateColor<256>(label, groupNum, "output/1/label.raw");
	//thinning and skeletonizing
	Morph::thinning<256>(pic).writeRaw("output/1/thinning.raw");
	Morph::skeletonizing<256>(pic).writeRaw("output/1/skeletonizing.raw");
	Morph::skeletonizing2<256>(pic).writeRaw("output/1/anotherSkele.raw");
	exit(0);
}
