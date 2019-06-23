// Copyright (c) 2019, The CCLib Developers
//
// Please see the included LICENSE file for more information.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "hash.h"
#include "pow.h"
#include "error.h"

void benchmark_dataset_generation(uint8_t* seed, uint64_t* dataset){
	char      buffer[65] = {0};
	uint64_t* cache_64   = (uint64_t*)calloc(8388608,8);
	uint8_t*  cache      = (uint8_t*)cache_64;
	uint32_t  temp       = 0;
	if (!cache_64) error_exit(1);
	cache_from_seed(seed, cache);
	for(uint8_t i=0;i<64;i++) buffer[i]=' ';
	printf("\rBenchmarking: [%s]",buffer); fflush(stdout);
	for(uint8_t j=0;j<64;j++){
		for(uint32_t i=0;i<8388608;i+=4){ // (1<<32)>>9
			temp = (8388608*j)+i;
			calcDatasetItem(cache, temp, &dataset[temp]);
		}
		buffer[j] = '#';
		printf("\rBenchmarking: [%s]",buffer); fflush(stdout);
	}
	printf("\r%*s\r",80,"");
	free(cache);

}

uint64_t benchmark_mine(uint64_t* seed_64, uint8_t printing, uint8_t threads, uint64_t ITERATIONS){
	uint64_t  result_64[4] = {0};
	uint8_t*  result       = (uint8_t*)result_64;
	uint64_t  temp[4]      = {0};
	uint64_t* dataset_64   = (uint64_t*)calloc(536870912,8);
	if (!dataset_64) error_exit(1);
	uint8_t*  seed         = (uint8_t*)seed_64;
	uint8_t*  dataset      = (uint8_t*)dataset_64;
	uint32_t  current_time = (uint32_t)time(NULL);
	char      buffer[65]   = {0};
	uint64_t  iterations   = ITERATIONS<<6;
	for(uint16_t i=0;i<64;i++) buffer[i]=' ';
	current_time = (uint32_t)time(NULL);
	if(printing){
		benchmark_dataset_generation(seed, dataset_64);
	} else {
		dataset_from_seed(seed, dataset_64);
	}
	printf("\tDataset generation took: %us\n",(uint32_t)time(NULL)-current_time);
	current_time = (uint32_t)time(NULL);
	if(printing){
		printf("\rBenchmarking: [%s]",buffer); fflush(stdout);
		if(!threads){
			for(uint8_t j=0;j<64;j++){
				for(uint64_t i=0;i<ITERATIONS;i++){
					mash_full(seed, dataset, result);
					temp[0] ^= result_64[0]; temp[1] ^= result_64[1];
					temp[2] ^= result_64[2]; temp[3] ^= result_64[3];
					(*seed_64)++; seed_64[1]++; seed_64[2]++; seed_64[3]++;
				}
				buffer[j] = '#';
				printf("\rBenchmarking: [%s]",buffer); fflush(stdout);
			}
		}else{
			threads++;
			for(uint8_t j=0;j<64;j++){
				for(uint64_t i=0;i<ITERATIONS;i+=threads){
					mash_full_multi(seed, dataset, threads, result);
					temp[0] ^= result_64[0]; temp[1] ^= result_64[1];
					temp[2] ^= result_64[2]; temp[3] ^= result_64[3];
					(*seed_64)++; seed_64[1]++; seed_64[2]++; seed_64[3]++;
				}
				buffer[j] = '#';
				printf("\rBenchmarking: [%s]",buffer); fflush(stdout);
			}
		}
		printf("\r%*s\r",80,"");
	} else {
		if(!threads){
			for(uint64_t i=0;i<iterations;i++){
				mash_full(seed, dataset, result);
				temp[0] ^= result_64[0]; temp[1] ^= result_64[1];
				temp[2] ^= result_64[2]; temp[3] ^= result_64[3];
				(*seed_64)++; seed_64[1]++; seed_64[2]++; seed_64[3]++;
			}
		}else{
			threads++;
			for(uint64_t i=0;i<iterations;i+=threads){
				mash_full_multi(seed, dataset, threads, result);
				temp[0] ^= result_64[0]; temp[1] ^= result_64[1];
				temp[2] ^= result_64[2]; temp[3] ^= result_64[3];
				(*seed_64)++; seed_64[1]++; seed_64[2]++; seed_64[3]++;
			}
		}
	}
	free(dataset);
	uint32_t end_time = (uint32_t)time(NULL);
	printf("\tCalculation of %lu hashes took: %us\n",iterations, end_time-current_time);
	printf("\tHashrate is approximately: %luH/s\n", iterations/(end_time-current_time));
	printf("\tResult: %016jx,%016jx,%016jx,%016jx\n",temp[0],temp[1],temp[2],temp[3]);
	return 0;
}

uint64_t benchmark_validation(uint64_t* seed_64, uint8_t printing, uint8_t threads, uint64_t ITERATIONS){
	uint64_t  result_64[4] = {0};
	uint8_t*  result       = (uint8_t*)result_64;
	uint64_t  temp[4]      = {0};
	uint64_t* cache_64     = (uint64_t*)calloc(8388608,8);
	if (!cache_64) error_exit(1);
	uint8_t*  seed         = (uint8_t*)seed_64;
	uint8_t*  cache        = (uint8_t*)cache_64;
	uint32_t  current_time = (uint32_t)time(NULL);
	uint64_t  iterations   = ITERATIONS>>6;
	char      buffer[65]   = {0};
	for(uint16_t i=0;i<64;i++) buffer[i]=' ';
	current_time = (uint32_t)time(NULL);
	cache_from_seed(seed, cache);
	printf("\tCache generation took: %us\n",(uint32_t)time(NULL)-current_time);
	current_time = (uint32_t)time(NULL);
	if(printing){
		printf("\rBenchmarking: [%s]",buffer); fflush(stdout);
		if(!threads){
			for(uint8_t j=0;j<64;j++){
				for(uint64_t i=0;i<iterations;i++){
					mash_light(seed, cache, result);
					temp[0] ^= result_64[0]; temp[1] ^= result_64[1];
					temp[2] ^= result_64[2]; temp[3] ^= result_64[3];
					(*seed_64)++; seed_64[1]++; seed_64[2]++; seed_64[3]++;
				}
				buffer[j] = '#';
				printf("\rBenchmarking: [%s]",buffer); fflush(stdout);
			}
		}else{
			threads++;
			for(uint8_t j=0;j<64;j++){
				for(uint64_t i=0;i<iterations;i+=threads){
					mash_light_multi(seed, cache, threads, result);
					temp[0] ^= result_64[0]; temp[1] ^= result_64[1];
					temp[2] ^= result_64[2]; temp[3] ^= result_64[3];
					(*seed_64)++; seed_64[1]++; seed_64[2]++; seed_64[3]++;
				}
				buffer[j] = '#';
				printf("\rBenchmarking: [%s]",buffer); fflush(stdout);
			}
		}
		printf("\r%*s\r",80,"");
	} else {
		if(!threads){
			for(uint64_t i=0;i<ITERATIONS;i++){
				mash_light(seed, cache, result);
				temp[0] ^= result_64[0]; temp[1] ^= result_64[1];
				temp[2] ^= result_64[2]; temp[3] ^= result_64[3];
				(*seed_64)++; seed_64[1]++; seed_64[2]++; seed_64[3]++;
			}
		}else{
			threads++;
			for(uint64_t i=0;i<ITERATIONS;i+=threads){
				mash_light_multi(seed, cache, threads, result);
				temp[0] ^= result_64[0]; temp[1] ^= result_64[1];
				temp[2] ^= result_64[2]; temp[3] ^= result_64[3];
				(*seed_64)++; seed_64[1]++; seed_64[2]++; seed_64[3]++;
			}
		}
	}
	free(cache);
	uint32_t end_time = (uint32_t)time(NULL);
	printf("\tCalculation of %lu hashes took: %us\n",ITERATIONS, end_time-current_time);
	printf("\tHashrate is approximately: %luH/s\n", ITERATIONS/(end_time-current_time));
	printf("\tResult: %016jx,%016jx,%016jx,%016jx\n",temp[0],temp[1],temp[2],temp[3]);
	return 0;
}



int main(int argc, char *argv[]){
	uint8_t   printing   = argc>1?atoi(argv[1]):0;
	uint8_t   threads    = argc>2?atoi(argv[2]):1;
	uint8_t   iterShifts = argc>3?atoi(argv[3]):20;
	uint32_t  seed       = argc>4?atoi(argv[4]):0x89ABCDEF;
	uint32_t  seed_32[8] = {0};
	uint64_t* seed_64    = (uint64_t*)seed_32;
	uint64_t  iterations = 1;
	threads--;
	srand(seed);
	for(uint8_t i=0; i<iterShifts; i++) iterations<<=1;
	for(uint8_t i=0;i<8;i++) seed_32[i] = rand();
	printf("\e[?25l"); // Hide cursor
	printf("Parameters\n");
	printf("\tProgressbar: %s\n", printing?"yes":"no");
	printf("\tThreads: %u\n", 1+threads);
	printf("\tSeed: %08x\n", seed);
	printf("Mining\n");
	benchmark_mine(seed_64, printing, threads, iterations);
	printf("Validation\n");
	benchmark_validation(seed_64, printing, threads, iterations);
	printf("\e[?25h");
	return 1;
}