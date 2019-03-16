*This readme will be replaced*

# Squash
A provably fast and expensive hash algorithm</br>

## ToDo
The following things have to be done
- [ ] Check if ASM CRCs have the same result as C CRC
- [x] Add AES ASM instructions
- [ ] Security Analysis (Review)
- [ ] Benchmark on ARMv8, SSE4.2 compatible CPU and AMD GPUs.

## Benchmarking
To test out squash, it is recommended to use the shipped python scripts which can be found in `tests/`. Please note that the C code in this folder writes to a file and therefore is a lot slower than the C code in squash.c.
### Parameters
Atleast one algorithm has to be tested, atleast one test option has to be specified. 
```
Algorithm options
-k, --keccak              Perform tests on keccak
-s, --squash              Perform tests on squash
-i, --iterations number   Set the number of iterations


Test options
-t, --time                Enable runtime testing
-c, --collisions          Enable collision testing
-b, --bit                 Enable bit histogram
-B, --bucket              Enable bucket histogram

Output options
-w, --write               Write to a file instead of console outputs
-P, --plot                Show plots instead of writing them
-o, --out directory       Specify out directory

-h, --help                Display this help message
```
By default the out directory is set to "results" and iterations are set to 65536 to enable quick, relatively accurate testing. All other values are disabled by default.</br>
An example for a command could be the following: `python3 tests.py -s -i 65536 -t -c -w -b -B`. It performs all tests on squash while using 65536 (2**16) iterations.</br>
**ATTENTION** due to recent changes of the sorting habits of the hashes (sort before all tasks are done instead of afterwords), the "similarity" test became somewhat obsolete for squash. To check the similarity of the hashes, go to docs/hash_sample.txt or generate one yourself using the python module in tests/.


### Dependencies
Those scripts use [clang](https://releases.llvm.org/download.html) and [Python3.7](https://www.python.org/downloads/release/python-372/), please make sure you use it instead of the older versions.
To create plots (bucket histogram and bit histogram), you will need to install matplotlib.
You can install it using the following command `python3.7 -m pip install matplotlib`.</br>
A CPU has to be capable of performing 64bit operations and needs the [AES-NI](https://en.wikipedia.org/wiki/AES_instruction_set), speedups for SSE4.2 and ARMv8 are available.

### Plots
The plots have no labeling and therefore are described in here.</br> 
In the bit histogram you can see the average value of a group of four bits at a specific position. The Y-Axis shows the average value where the X-Axis displays the position. Ideally it is a straight line at the value of 8.</br>
The bucket histogram cuts of the first part of every hash value and only takes the last 16bit. Those are then used to fit everything into a bucket. The X-Axis displays the buckets, where each bucket corresponds to a value (such as 0, 1 or 65535). The Y-Axis corresponds to the absolute number of entities in this bucket. Ideally this is a straight line aswell at ITERATIONS/65536.</br>

### Speed
On a [Xeon E3-1225v2](https://ark.intel.com/content/www/us/en/ark/products/65733/intel-xeon-processor-e3-1225-v2-8m-cache-3-20-ghz.html), the algorithm has an average speed of 165.8 in C, 21000ns in Python. For comparision, keccak takes about 800ns on average. Which results in 16.5 cpb for squash, 80 cpb for keccak - on an Ivy Bridge CPU.

**Please note, that this Project is still under development. It works perfectly fine on all machines machines**

If you have got any issues, please open a case.
If you know how to improve parts of the code, make sure to send a pull request.





