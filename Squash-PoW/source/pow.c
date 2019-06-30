// Copyright (c) 2019, The CCLib Developers
//
// Please see the included LICENSE file for more information.


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "squash.h"
#include "error.h"
#if !defined(HAVE_SSE2)
#include "blake2/neon/blake2.h"
#else
#include "blake2/sse/blake2.h"
#endif

#define HASH_BYTES      32                // hash length in bytes
#define EPOCH_LENGTH    1                 // blocks per epoch
#define DATASET_PARENTS 16                // number of hashes before calculating dataset entry
// Assuming 4 blocks per second, an epoch estimates 15 minutes


void make_cache(uint8_t* seed, uint8_t* cache){
	/* 64MiB cache is allocated before executing
	   this function */
	uint32_t* se32          = (uint32_t*)seed;
	uint32_t* cache_32      = (uint32_t*)cache; 
	uint64_t* cache_64      = (uint64_t*)cache; 
	uint32_t  i64           = 0;
	crc32p(se32,       cache_32);
	crc32p(&se32[1],  &cache_32[1]);
	crc32p(&se32[2],  &cache_32[2]);
	crc32p(&se32[3],  &cache_32[3]);
	crc32p(&se32[4],  &cache_32[4]);
	crc32p(&se32[5],  &cache_32[5]);
	crc32p(&se32[6],  &cache_32[6]);
	crc32p(&se32[7],  &cache_32[7]);
	for(uint32_t i=0;i<0xfffff7;i+=8){
		i64 >>= 1;
		crc32p(&cache_32[i  ],  &cache_32[i+ 8]);
		crc32p(&cache_32[i+1],  &cache_32[i+ 9]);
		crc32p(&cache_32[i+2],  &cache_32[i+10]);
		crc32p(&cache_32[i+3],  &cache_32[i+11]);
		crc32p(&cache_32[i+4],  &cache_32[i+12]);
		crc32p(&cache_32[i+5],  &cache_32[i+13]);
		crc32p(&cache_32[i+6],  &cache_32[i+14]);
		crc32p(&cache_32[i+7],  &cache_32[i+15]);
		cache_64[i64  ] += cache_64[i64+1];
		cache_64[i64+2] += cache_64[i64+3];
		cache_64[i64+1] += cache_64[i64+2];
		cache_64[i64+3] += cache_64[i64  ];
	}
}

void calc_dataset_item(uint8_t* cache, uint32_t item_number, uint64_t* out){
	uint32_t* cache_32 = (uint32_t*)cache; 
	uint64_t  mix[4]   = {0};
	uint32_t* mix_32   = (uint32_t*)mix;
	uint32_t* mix_32_s = (uint32_t*)&(((uint16_t*)mix)[1]);
	uint32_t  x        = 0;
	item_number>>=2;
	*mix_32    = cache_32[(item_number  )&0x1fffff];
	mix_32[1]  = cache_32[(item_number+1)&0x1fffff];
	mix_32[2]  = cache_32[(item_number+2)&0x1fffff];
	mix_32[3]  = cache_32[(item_number+3)&0x1fffff];
	mix_32[4]  = cache_32[(item_number+4)&0x1fffff];
	mix_32[5]  = cache_32[(item_number+5)&0x1fffff];
	mix_32[6]  = cache_32[(item_number+6)&0x1fffff];
	mix_32[7]  = cache_32[(item_number+7)&0x1fffff];
	*mix_32   ^= item_number; mix_32[1] ^= item_number;
	mix_32[2] ^= item_number; mix_32[3] ^= item_number;
	mix_32[4] ^= item_number; mix_32[5] ^= item_number;
	mix_32[6] ^= item_number; mix_32[7] ^= item_number;
	for(uint16_t j=0;j<DATASET_PARENTS;j++){
		x = j^item_number;
		*mix_32 &= 0x1fffff; mix_32[1] &= 0x1fffff;
		mix_32[2] &= 0x1fffff; mix_32[3] &= 0x1fffff;
		mix_32[4] &= 0x1fffff; mix_32[5] &= 0x1fffff;
		mix_32[6] &= 0x1fffff; mix_32[7] &= 0x1fffff;
		*mix_32   = cache_32[*mix_32];
		mix_32[1] = cache_32[mix_32[1]];
		mix_32[2] = cache_32[mix_32[2]];
		mix_32[3] = cache_32[mix_32[3]];
		mix_32[4] = cache_32[mix_32[4]];
		mix_32[5] = cache_32[mix_32[5]];
		mix_32[6] = cache_32[mix_32[6]];
		mix_32[7] = cache_32[mix_32[7]];
		*mix_32   ^= x; mix_32[1] ^= x;
		mix_32[2] ^= x; mix_32[3] ^= x;
		mix_32[4] ^= x; mix_32[5] ^= x;
		mix_32[6] ^= x; mix_32[7] ^= x;
		crc32i(mix_32_s);
		crc32i(&mix_32_s[1]);
		crc32i(&mix_32_s[2]);
		crc32i(&mix_32_s[3]);
		crc32i(&mix_32_s[4]);
		crc32i(&mix_32_s[5]);
		crc32i(&mix_32_s[6]);
	}
	*out  =*mix;    out[1]=mix[1];
	out[2]= mix[2]; out[3]=mix[3];
}

void calc_dataset(uint8_t* cache, uint64_t* out){
	for(uint32_t i=0;i<536870912;i+=4){ // (1<<32)>>3
		calc_dataset_item(cache, i, &out[i]);
	}
}

void squash_pow_full(uint8_t* header, uint64_t nonce, uint64_t* dataset, uint8_t* result){
	uint64_t  seed_64[4] = {0}; 
	uint8_t*  seed       = (uint8_t*)seed_64; 
	memcpy(&header[80], (char*)&nonce, 8);
	blake2b(seed, 32, header, (size_t) 88, NULL, 0);
	squash_3_full(seed, dataset, result);
}

void squash_pow_light(uint8_t* header, uint64_t nonce, uint8_t* cache, uint8_t* result){
	uint64_t  seed_64[4] = {0}; 
	uint8_t*  seed       = (uint8_t*)seed_64; 
	memcpy(&header[80], (char*)&nonce, 8);
	blake2b(seed, 32, header, (size_t) 88, NULL, 0);
	squash_3_light(seed, cache, result);
}

void cache_from_seed(uint8_t* seed, uint8_t* cache){
	make_cache(seed, cache);
}
void dataset_from_seed(uint8_t* seed, uint64_t* dataset){
	uint64_t* cache_64 = (uint64_t*)calloc(8388608,8);
	if(!cache_64) error_exit(1);
	uint8_t*  cache    = (uint8_t*)cache_64;
	cache_from_seed(seed, cache);
	calc_dataset(cache, dataset);
	free(cache);
}