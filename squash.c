#include <stdio.h>
#include <emmintrin.h>
#include <stdint.h>
#include <math.h>
#include "aes.h"

#ifndef _mm_set_epi64x
	#define _mm_set_epi64x(m0, m1) _mm_set_epi64(_m_from_int64(m0), _m_from_int64(m1))
#endif

#if (defined(_CPU_X86_64_) || defined(_CPU_X86_)) && !defined(_COMPILER_MICROSOFT_) && defined(__SSE4_2__)
/* Compute CRC-32C using the SSE4.2 hardware instruction. */
uint32_t crc32(unsigned char* buf)
{
	uintptr_t crc0 = 0xffffffff;
	for(uint8_t len=0;len<4;len++){
		__asm__("crc32b\t" "(%1), %0"
				: "=r"(crc0)
				: "r"(buf), "0"(crc0));
		buf++;
	}
	return (uint32_t)crc0 ^ 0xffffffff;
}
#elif defined(_CPU_AARCH64_)
#define CRC_TARGET __attribute__((target("+crc")))
/* Compute CRC-32C using the ARMv8 CRC32 extension. */
static inline uint32_t crc32(unsigned char* in)
{
	uint32_t val = ((uint32_t*)in)[0];
	uint32_t res;
	asm("crc32cw %w0, %w1, %w2" : "=r"(res) : "r"(0xffffffff), "r"(val));
	return res;
}
#else
uint32_t crc32c_table[256] = {
 0x00000000L, 0xF26B8303L, 0xE13B70F7L, 0x1350F3F4L,
 0xC79A971FL, 0x35F1141CL, 0x26A1E7E8L, 0xD4CA64EBL,
 0x8AD958CFL, 0x78B2DBCCL, 0x6BE22838L, 0x9989AB3BL,
 0x4D43CFD0L, 0xBF284CD3L, 0xAC78BF27L, 0x5E133C24L,
 0x105EC76FL, 0xE235446CL, 0xF165B798L, 0x030E349BL,
 0xD7C45070L, 0x25AFD373L, 0x36FF2087L, 0xC494A384L,
 0x9A879FA0L, 0x68EC1CA3L, 0x7BBCEF57L, 0x89D76C54L,
 0x5D1D08BFL, 0xAF768BBCL, 0xBC267848L, 0x4E4DFB4BL,
 0x20BD8EDEL, 0xD2D60DDDL, 0xC186FE29L, 0x33ED7D2AL,
 0xE72719C1L, 0x154C9AC2L, 0x061C6936L, 0xF477EA35L,
 0xAA64D611L, 0x580F5512L, 0x4B5FA6E6L, 0xB93425E5L,
 0x6DFE410EL, 0x9F95C20DL, 0x8CC531F9L, 0x7EAEB2FAL,
 0x30E349B1L, 0xC288CAB2L, 0xD1D83946L, 0x23B3BA45L,
 0xF779DEAEL, 0x05125DADL, 0x1642AE59L, 0xE4292D5AL,
 0xBA3A117EL, 0x4851927DL, 0x5B016189L, 0xA96AE28AL,
 0x7DA08661L, 0x8FCB0562L, 0x9C9BF696L, 0x6EF07595L,
 0x417B1DBCL, 0xB3109EBFL, 0xA0406D4BL, 0x522BEE48L,
 0x86E18AA3L, 0x748A09A0L, 0x67DAFA54L, 0x95B17957L,
 0xCBA24573L, 0x39C9C670L, 0x2A993584L, 0xD8F2B687L,
 0x0C38D26CL, 0xFE53516FL, 0xED03A29BL, 0x1F682198L,
 0x5125DAD3L, 0xA34E59D0L, 0xB01EAA24L, 0x42752927L,
 0x96BF4DCCL, 0x64D4CECFL, 0x77843D3BL, 0x85EFBE38L,
 0xDBFC821CL, 0x2997011FL, 0x3AC7F2EBL, 0xC8AC71E8L,
 0x1C661503L, 0xEE0D9600L, 0xFD5D65F4L, 0x0F36E6F7L,
 0x61C69362L, 0x93AD1061L, 0x80FDE395L, 0x72966096L,
 0xA65C047DL, 0x5437877EL, 0x4767748AL, 0xB50CF789L,
 0xEB1FCBADL, 0x197448AEL, 0x0A24BB5AL, 0xF84F3859L,
 0x2C855CB2L, 0xDEEEDFB1L, 0xCDBE2C45L, 0x3FD5AF46L,
 0x7198540DL, 0x83F3D70EL, 0x90A324FAL, 0x62C8A7F9L,
 0xB602C312L, 0x44694011L, 0x5739B3E5L, 0xA55230E6L,
 0xFB410CC2L, 0x092A8FC1L, 0x1A7A7C35L, 0xE811FF36L,
 0x3CDB9BDDL, 0xCEB018DEL, 0xDDE0EB2AL, 0x2F8B6829L,
 0x82F63B78L, 0x709DB87BL, 0x63CD4B8FL, 0x91A6C88CL,
 0x456CAC67L, 0xB7072F64L, 0xA457DC90L, 0x563C5F93L,
 0x082F63B7L, 0xFA44E0B4L, 0xE9141340L, 0x1B7F9043L,
 0xCFB5F4A8L, 0x3DDE77ABL, 0x2E8E845FL, 0xDCE5075CL,
 0x92A8FC17L, 0x60C37F14L, 0x73938CE0L, 0x81F80FE3L,
 0x55326B08L, 0xA759E80BL, 0xB4091BFFL, 0x466298FCL,
 0x1871A4D8L, 0xEA1A27DBL, 0xF94AD42FL, 0x0B21572CL,
 0xDFEB33C7L, 0x2D80B0C4L, 0x3ED04330L, 0xCCBBC033L,
 0xA24BB5A6L, 0x502036A5L, 0x4370C551L, 0xB11B4652L,
 0x65D122B9L, 0x97BAA1BAL, 0x84EA524EL, 0x7681D14DL,
 0x2892ED69L, 0xDAF96E6AL, 0xC9A99D9EL, 0x3BC21E9DL,
 0xEF087A76L, 0x1D63F975L, 0x0E330A81L, 0xFC588982L,
 0xB21572C9L, 0x407EF1CAL, 0x532E023EL, 0xA145813DL,
 0x758FE5D6L, 0x87E466D5L, 0x94B49521L, 0x66DF1622L,
 0x38CC2A06L, 0xCAA7A905L, 0xD9F75AF1L, 0x2B9CD9F2L,
 0xFF56BD19L, 0x0D3D3E1AL, 0x1E6DCDEEL, 0xEC064EEDL,
 0xC38D26C4L, 0x31E6A5C7L, 0x22B65633L, 0xD0DDD530L,
 0x0417B1DBL, 0xF67C32D8L, 0xE52CC12CL, 0x1747422FL,
 0x49547E0BL, 0xBB3FFD08L, 0xA86F0EFCL, 0x5A048DFFL,
 0x8ECEE914L, 0x7CA56A17L, 0x6FF599E3L, 0x9D9E1AE0L,
 0xD3D3E1ABL, 0x21B862A8L, 0x32E8915CL, 0xC083125FL,
 0x144976B4L, 0xE622F5B7L, 0xF5720643L, 0x07198540L,
 0x590AB964L, 0xAB613A67L, 0xB831C993L, 0x4A5A4A90L,
 0x9E902E7BL, 0x6CFBAD78L, 0x7FAB5E8CL, 0x8DC0DD8FL,
 0xE330A81AL, 0x115B2B19L, 0x020BD8EDL, 0xF0605BEEL,
 0x24AA3F05L, 0xD6C1BC06L, 0xC5914FF2L, 0x37FACCF1L,
 0x69E9F0D5L, 0x9B8273D6L, 0x88D28022L, 0x7AB90321L,
 0xAE7367CAL, 0x5C18E4C9L, 0x4F48173DL, 0xBD23943EL,
 0xF36E6F75L, 0x0105EC76L, 0x12551F82L, 0xE03E9C81L,
 0x34F4F86AL, 0xC69F7B69L, 0xD5CF889DL, 0x27A40B9EL,
 0x79B737BAL, 0x8BDCB4B9L, 0x988C474DL, 0x6AE7C44EL,
 0xBE2DA0A5L, 0x4C4623A6L, 0x5F16D052L, 0xAD7D5351L
};

uint32_t crc32(unsigned char* buf)
{
	uintptr_t crc =  0xffffffff;
	for(uint8_t len=0;len<4;len++){
		crc = crc32c_table[(crc ^ *buf++) & 0xff] ^ (crc >> 8);
	}
	return (uint32_t)crc ^ 0xffffffff;
}
#endif


#ifdef UINT64_MAX
void ror128(uint32_t* in_32, uint32_t* out_32, uint16_t n){
	uint64_t* out = (uint64_t*)out_32;
	uint64_t* in = (uint64_t*)in_32;
	uint8_t num = ((n&0x40)>>6);
	uint8_t shift = n&0x3f;
	printf("%jx,%jx\n",(in[  num]>>shift), (in[1^num]<<(64-shift)));
	printf("%jx,%jx\n",(in[1^num]>>shift), (in[  num]<<(64-shift)));
	out[0] = (in[  num]>>shift) | (in[1^num]<<(64-shift));
	out[1] = (in[1^num]>>shift) | (in[  num]<<(64-shift));
	return;
}
void rol128(uint32_t* in_32, uint32_t* out_32, uint16_t n){
	uint64_t* out = (uint64_t*)out_32;
	uint64_t* in = (uint64_t*)in_32;
	uint8_t num = ((n&0x40)>>6);
	uint8_t shift = n&0x3f;
	printf("%jx,%jx\n",(in[  num]<<shift), (in[1^num]>>(64-shift)));
	printf("%jx,%jx\n",(in[1^num]<<shift), (in[  num]>>(64-shift)));
	out[0] = (in[  num]<<shift) | (in[1^num]>>(64-shift));
	out[1] = (in[1^num]<<shift) | (in[  num]>>(64-shift));
	return;
}
#else
void rol128(uint32_t* in, uint32_t* out, uint16_t n){
	uint8_t nums[4] = {0,1,2,3};
	if ((n&0x40)>>6) {
		nums[0] = 2;
		nums[1] = 3;
		nums[2] = 0;
		nums[3] = 1;
	}
	if ((n&0x20)>>5) {
		nums[0] = (nums[0]-1)&3;
		nums[1] =  nums[1]-1;
		nums[2] = (nums[2]-1)&3;
		nums[3] =  nums[3]-1;
	}
	uint8_t shift = n&0x1f;
	out[0] = (in[nums[0]]<<shift) | (in[nums[1]]>>(32-shift));
	out[1] = (in[nums[1]]<<shift) | (in[nums[2]]>>(32-shift));
	out[2] = (in[nums[2]]<<shift) | (in[nums[3]]>>(32-shift));
	out[3] = (in[nums[3]]<<shift) | (in[nums[0]]>>(32-shift));
	return;
}
void ror128(uint32_t* in, uint32_t* out, uint16_t n){
	uint8_t nums[4] = {0,1,2,3};
	if ((n&0x40)>>6) {
		nums[0] = 2;
		nums[1] = 3;
		nums[2] = 0;
		nums[3] = 1;
	}
	if ((n&0x20)>>5) {
		nums[0] =  1+nums[0];
		nums[1] = (1+nums[1])&3;
		nums[2] =  1+nums[2];
		nums[3] = (1+nums[3])&3;
	}
	uint8_t shift = n&0x1f;
	out[0] = (in[nums[0]]>>shift) | (in[nums[1]]<<(32-shift));
	out[1] = (in[nums[1]]>>shift) | (in[nums[2]]<<(32-shift));
	out[2] = (in[nums[2]]>>shift) | (in[nums[3]]<<(32-shift));
	out[3] = (in[nums[3]]>>shift) | (in[nums[0]]<<(32-shift));
	return;
}
#endif

#ifndef UINT64_MAX
uint32_t add32(uint32_t a, uint32_t b, uint8_t* carry){
	if((a>>31)&&(b>>31))carry[0]=1;
	return(a+b);
}
void div32(uint64_t* a, uint32_t b) {
#  ifdef __i386__  /* u64 / u32 division with little i386 machine code. */
	uint32_t upper = ((uint32_t*)a)[1], r;
	((uint32_t*)a)[1] = 0;
	if (upper >= b) {   
		((uint32_t*)a)[1] = upper / b;
		upper %= b;
	}
	__asm__("divl %2" : "=a" (((uint32_t*)a)[0]), "=d" (r) :
		"rm" (b), "0" (((uint32_t*)a)[0]), "1" (upper));
#  else
	a[0] = a[0] / b;  /* Calls __udivdi3 in libgcc. */
#  endif
	a[0]=a[0]>>32;
}
#endif

void hash(uint8_t* data, uint8_t* scratchpad, uint8_t* out){
	uint16_t  crc_16[16] =	{0};
	uint32_t* crc_32 = (uint32_t*)crc_16;
	uint64_t* crc_64 = (uint64_t*)crc_16;
	uint64_t* data_64 = (uint64_t*)data;
	uint32_t iv[4] = {0};
	uint32_t key[4] = {0};
	crc_32[0] = crc32(data);
	crc_32[1] = crc32(&data[4]);
	crc_32[2] = ((uint32_t*)&scratchpad[crc_16[0]])[0];
	crc_32[3] = ((uint32_t*)&scratchpad[crc_16[2]])[0];
	#ifdef UINT64_MAX
	crc_64[1] ^= data_64[1];
	crc_64[2] = (crc_64[1] + data_64[2]) ^ (crc_64[1] / data_64[2]);
	crc_64[3] = data_64[3];
	ror128(crc_32    ,iv ,crc_16[15]);
	ror128(&crc_32[4],key,crc_16[ 0]);
	#else
	uint32_t* data_32 = (uint32_t*)data;
	crc_32[2] ^= data_32[2];
	crc_32[3] ^= data_32[3];
	uint8_t carry = 0;
	uint64_t div_out = crc_64[1];
	div32(&div_out, data_32[5]);
	crc_32[5] = add32(crc_32[3],data_32[5],&carry) ^ div_out;
	div_out = crc_64[1];
	div32(&div_out, data_32[4]);
	crc_32[4] = (crc_32[2]+data_32[4]+carry) ^ div_out;
	crc_32[6] = data_32[6];
	crc_32[7] = data_32[7];
	ror128(crc_32    ,iv ,crc_16[15]);
	rol128(&crc_32[4],key,crc_16[ 0]);
	#endif

	printf("%u,%u|%jx,%jx,%jx,%jx|%jx,%jx|%jx,%jx\n",crc_16[15]&0x7F,crc_16[ 0]&0x7F,crc_64[0],crc_64[1],crc_64[2],crc_64[3],((uint64_t*)&iv)[0],((uint64_t*)&iv)[1],((uint64_t*)&key)[0],((uint64_t*)&key)[1]);
	aes((uint8_t*)crc_16, out, (uint8_t*)&key, (uint8_t*)&iv);
	return;
}

int main(){
	uint64_t iterations = (uint64_t)pow(2.0,2);
	uint8_t data[32] = {[0 ... 31] = 6};
	uint8_t scratchpad[65536] = {[0 ... 65535] = 5};
	uint8_t out[32] = {[0 ... 31] = 6};
	for(uint64_t i=0; i<iterations;i++) hash(out, scratchpad, out);
	printf("%x,%x,%x,%x,%x,%x,%x,%x",((uint32_t*)out)[0],((uint32_t*)out)[1],((uint32_t*)out)[2],((uint32_t*)out)[3],((uint32_t*)out)[4],((uint32_t*)out)[5],((uint32_t*)out)[6],((uint32_t*)out)[7]);
}