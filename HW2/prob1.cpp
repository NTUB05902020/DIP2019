#include "edgeDetect.hpp"

int main(){
	Image<256> im1("raw/sample1.raw");
	Image<256> im2("raw/sample2.raw");
	Image<256> im3("raw/sample3.raw");
	/*--------------------firstOrder----------------------*/
	EdgeDetect::firstOrder<256>(im1, '3', 40).writeRaw("output/1/1-3points_1.raw");
	EdgeDetect::firstOrder<256>(im2, '3', 40).writeRaw("output/1/1-3points_2.raw");
	EdgeDetect::firstOrder<256>(im3, '3', 40).writeRaw("output/1/1-3points_3.raw");
	
	EdgeDetect::firstOrder<256>(im1, 'R', 40).writeRaw("output/1/1-roberts_1.raw");
	EdgeDetect::firstOrder<256>(im2, 'R', 40).writeRaw("output/1/1-roberts_2.raw");
	EdgeDetect::firstOrder<256>(im3, 'R', 40).writeRaw("output/1/1-roberts_3.raw");
	
	EdgeDetect::firstOrder<256>(im1, 'P', 35).writeRaw("output/1/prewitt_1.raw");
	EdgeDetect::firstOrder<256>(im2, 'P', 35).writeRaw("output/1/prewitt_2.raw");
	EdgeDetect::firstOrder<256>(im3, 'P', 35).writeRaw("output/1/prewitt_3.raw");
	
	EdgeDetect::firstOrder<256>(im1, 'S', 35).writeRaw("output/1/sobel_1.raw");
	EdgeDetect::firstOrder<256>(im2, 'S', 35).writeRaw("output/1/sobel_2.raw");
	EdgeDetect::firstOrder<256>(im3, 'S', 35).writeRaw("output/1/sobel_3.raw");
	
	/*-------------------secondOrder---------------------*/
	EdgeDetect::secondOrder<256>(im1, '4', 25.0, false).writeRaw("output/1/2-4neighbor_1.raw");
	EdgeDetect::secondOrder<256>(im2, '4', 50.0, false).writeRaw("output/1/2-4neighbor_2.raw");
	EdgeDetect::secondOrder<256>(im3, '4', 25.0, false).writeRaw("output/1/2-4neighbor_3.raw");
	
	EdgeDetect::secondOrder<256>(im1, 'N', 20.0, false).writeRaw("output/1/2-8neighbor_1.raw");
	EdgeDetect::secondOrder<256>(im2, 'N', 50.0, false).writeRaw("output/1/2-8neighbor_2.raw");
	EdgeDetect::secondOrder<256>(im3, 'N', 20.0, false).writeRaw("output/1/2-8neighbor_3.raw");
	
	EdgeDetect::secondOrder<256>(im1, 'S', 20.0, false).writeRaw("output/1/2-8neigsepa_1.raw");
	EdgeDetect::secondOrder<256>(im2, 'S', 30.0, false).writeRaw("output/1/2-8neigsepa_2.raw");
	EdgeDetect::secondOrder<256>(im3, 'S', 20.0, false).writeRaw("output/1/2-8neigsepa_3.raw");
	
	EdgeDetect::secondOrder<256>(im1, '4', 3.0, true).writeRaw("output/1/2-4neighbor_1B.raw");
	EdgeDetect::secondOrder<256>(im2, '4', 3.0, true).writeRaw("output/1/2-4neighbor_2B.raw");
	EdgeDetect::secondOrder<256>(im3, '4', 3.0, true).writeRaw("output/1/2-4neighbor_3B.raw");
	
	EdgeDetect::secondOrder<256>(im1, 'N', 3.0, true).writeRaw("output/1/2-8neighbor_1B.raw");
	EdgeDetect::secondOrder<256>(im2, 'N', 5.0, true).writeRaw("output/1/2-8neighbor_2B.raw");
	EdgeDetect::secondOrder<256>(im3, 'N', 5.0, true).writeRaw("output/1/2-8neighbor_3B.raw");
	
	EdgeDetect::secondOrder<256>(im1, 'S', 3.0, true).writeRaw("output/1/2-8neigsepa_1B.raw");
	EdgeDetect::secondOrder<256>(im2, 'S', 5.0, true).writeRaw("output/1/2-8neigsepa_2B.raw");
	EdgeDetect::secondOrder<256>(im3, 'S', 5.0, true).writeRaw("output/1/2-8neigsepa_3B.raw");
	
	EdgeDetect::canny<256>(im1, 10.0, 23.0).writeRaw("output/1/canny_1.raw");
	EdgeDetect::canny<256>(im2, 10.0, 23.0).writeRaw("output/1/canny_2.raw");
	EdgeDetect::canny<256>(im3, 10.0, 23.0).writeRaw("output/1/canny_3.raw");
	
	return 0;
}