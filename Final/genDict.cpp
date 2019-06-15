#include <cstdio>
#include <cstdlib>
#include <random>
#include <cmath>

#define DIM 117
#define K 300

std::default_random_engine generator;
std::normal_distribution<double> distribution(0.0, 1.0);

int main(int argc, char **argv){
	FILE *fp = fopen("dict.dat", "w");
	fprintf(fp, "Iteration: 0\n");
	for(int i=0;i<K;++i){
		double arr[DIM] = {0.0}, sum = 0.0, squareSum = 0.0;
		for(int j=0;j<DIM;++j){
			arr[j] = distribution(generator);
			sum += arr[j];
			squareSum += arr[j]*arr[j];
		}
		sum /= DIM;   squareSum/=DIM;
		squareSum = sqrt(squareSum - sum*sum);
		for(int j=0;j<DIM;++j) fprintf(fp, "%lf ", (arr[j]-sum)/squareSum);
		fprintf(fp, "\n");
	}
	fclose(fp); exit(0);
}