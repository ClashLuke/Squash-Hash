## Bidash
A bi-assymetric hash combining the birthday problem with dagger.

**Contents**

- [General](#General)
	- [Concept](#Concept)
	- [Speed](#Speed)
	- [Difficulty](#Difficulty)
- [Security](#Security)
	- [Dataset](#Dataset)
	- [Birthday-Paradox](#Birthday-Paradox)
	- [Memory](#Memory)
	- [Time-Memory-Tradeoff](#Time-Memory-Tradeoff)
- [Comparision](#Comparision)
	- [Equihash](#Equihash)
	- [Dagger](#Dagger)
	- [Balloon](#Balloon)
- [Resistance](#Resistance)
	- [ASIC](#ASIC)
	- [FPGA](#FPGA)
	- [GPU](#GPU)
- [ToDo](#ToDo)

### General
#### Concept
Bidash is a bi-assymetric hash algorithm combining the research of the [birthday problem](#Birthday-Paradox) and [ethash](https://github.com/ethereum/wiki/wiki/Ethash).

The birthday problem is used as a rate limitation for an exterior hash algorithm used to hash the block header, so that the hashrate is dependent on the speed of finding a solution to the birthday paradox instead of the speed of finding a solution to a hash as seen in most cryptocurrencies. With that in mind, bidash also does not rely on a hash-based difficulty but instead has its own difficulty mechanism. After finding one solution to the birthday paradox on a 32bit level, where one birthday is equal to the first half of the nonce, another item has to be found which meets `abs(item-nonce)<difficulty`. To ensure that a solution other than the first one has to be found, the nonce is mixed with the the 64bit behind solution to the birthday paradox. Those items being calculated similar to ethereums ethash allows a validator to calculate only the two required items and check for equality, without forcing them to calculate the entire dataset. Additionally, thanks to the speed-assymetry of the birthday problem, a validator also has a higher hashrate (validation rate) than a miner. 

### Speed
The previously mentioned speed of the algorithm is variable for the miner, thanks to the embedded [difficulty](#Difficulty), but constant for a validator at approximately eight million validations per second (8MH/s). 





In bidash, the most important thing is memory-dependency. Say `m` is the variable determining the memory intensity (for the purpose of accurate and recreatable calculations, m is measured in bytes). Unfortunately with a background of the equihash and the birthday paradox, the time complexity `t` for the algorithm to find a solution, assuming that there are no memory optimisations and a complete randomness, is equal to `2*m*(m-1)` CPU cycles in a best-case scenario. This best-case scenario assumes that all memory is on-cache and checks are performed in one CPU cycle. Considering that a minimum of 4 GiB (2^32 bytes) are recommended to achieve strong ASIC and botnet resistance, the minimum time spent calculating one nonce, assuming that there are no memory optimisations possible for the algorithm, is `t = 2*m*(m-1) = 2^65-2^33` CPU cycles. Converting CPU cycles to seconds, on a normal 4GiHz CPU with one active thread, the average best-case execution time for the algorithm to create one subnonce to one nonce is 272 years. (Further reading: [A generalized Birthday Problem](https://link.springer.com/content/pdf/10.1007%2F3-540-45708-9_19.pdf), [Lattice Problem](https://cseweb.ucsd.edu/~daniele/papers/SVP.pdf))

Therefore a different approach has to be used, which skips a lot of work yet requires the entire dataset to be existent.

The dataset is tied to 4GB (or (2^32)-31 elements of 32bit size) and generated once for every "seed hash". The seed hash is the hash of the block header including the transactions and the nonce and therefore can be changed quickly. A dataset of 4GB for mining is not enforced, but recommended to achieve the optimal performance. The upper limit is 4GiB.

Dataset generation: 4.5s/4GiB

After sub-nonces to the major nonce are found, they are attached to the block and hashed with any fast hash such as blake2s or AES-hash.

Thanks to the underlying design of the birthday paradox, an ASIC for this algorithm would be a chip with on-chip memory and a large memory bus.

Using this design, bidash is botnet resistant, has heavy ASIC resistance and low dataset generation times while being extremely fast verifyable, provably assymetric and provably secure.

Difficulty works perfectly fine. Diff * 2 = Time * 2. Minimum recommended difficulty: 2^32. 

GPUs can't mine, because a CPU finishes one nonce in 1s (dataset generation) + 3s (calculation) but a GPU needs 5+ seconds to generate the dataset itself. When copying the dataset generated on CPU to GPU, 1+ second is needed only for the process of copying it. The GPU would then have up to 2s to check all items in the dataset in parallel, find a solution and broadcast it back to the CPU. This appears to be very unlikely.

```
Parameters
	Progressbar:  yes
	Iterations
		Full:  1024
		Light:  68719476736
	Seed:  89abcdef
	Difficulty:  68719476736
Mining
        Calculation of 1024 solutions took:  4252s                              
	Hashrate is approximately:  607 s/sol
	Found 7 solutions for 1024 nonces. Rate: 0%
	Result:  a7b3906a,d1cc6688


```

###ToDo
- [ ] **General Optimisations**
- [ ] Add dataset item calculation optimisation
- [ ] **Readme**
- [ ] Add "the lower the memory the lower the chance of finding a solution (exponentially)"-text
- [ ] Explain where the expontial growth of time complexity comes from (when reducing the dataset size linearly)
- [ ] Add explanation that difficulty is inside the function itself, not about a hash outside of it
- [ ] Explain how that's an improvement compared to equihash
- [ ] Explain new approach properly
- [ ] Add references
- [ ] Add proper benchmark
- [ ] Improve GPU part
- [ ] Explain difficulty minimum of 2^32

