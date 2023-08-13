<!-- SPDX-License-Identifier: zlib-acknowledgement -->
## Overview
optimisation requires specialised knowledge of platform and is time consuming
performance-aware programming works towards optimisation, just not all the way

Distinction between, algorithmic optimisation is not necessarily performance aware programming (i.e. might involve math etc.)
We are learning about CPU mental model

don't think about source language. think about what it becomes, e.g. templates, garbage-collection are not a zero-cost
it's only about what the source turns into, is when we can talk about its speed

1. reduce number of instructions (waste, SIMD)
2. increase speed of instructions (same instructions can be faster) (IPC, cache, multithreading)

Make decisions you know it will make a difference on say, Skylake chip
Know performance characteristics of microarchtictures and how they apply to say the pipeline execution

performance analysis one of 2 things:
1. determine behaviour of microarchitecture, i.e. peak fastest:
   if only take mean, median we don't see what it converges to
   we want to take minimum cycles, to see what will happen if 'all the stars align'
2. see what is typical
   given a mix of what is cache state, what else in pipeline, other cores doing 
   look at mean/median 

Break down problem into parts, and find what are performance bottlenecks
Then make estimate for theoretical maximum

### Waste
This is anything not necessary for computation.
Instruction stream unecessarily widened leading to binary bloat and added runtime cost e.g. templates, interpreters (10x overhead considered good, e.g. rigmarole determining types and how to add; 1 add instruction in C)
Binary bloat is one factor that could lead to instruction cache misses. So, not using templates is a simple way to reduce this and runtime cost.

even if stuck in a high-level, can gain some performance benefits by rearranging code
furthermore, could offload sections to C

### IPC
modern CPUs are superscalar/overlapped, i.e. instruction level parallelism (multiple instructions per cycle)
the CPU will look for instructions it can execute at the same time, i.e. not serially dependent
this lookahead makes branch misprediction so costly
the CPU does not have complex logic in ascertaining dependency chains, only looking at operands. 
So, assumes series of 'add' on same register are serially dependent
`add a, input[i]; add a, input[i + 1]`
we can help the CPU however, noting that say a summation can be broken up into multiple dependency chains
`add a, input[i]; add b, input[i + 1]`
generally, if can reduce dependency chain, always applicable
a CPU will have a max. IPC value

Not as simple to say if 4 add units then unroll 4 times. 
To determine loop unrolling factor must analyse port usage
ports perform multiple operations, e.g. add or cmp. but can only do 1 at a time
so, there might be port contention

### Cache
when timing code, repeat; this will give time to show what it's like when cache is primed
count operations/cycle

However, won't get these performance benefits if bad cache performance
So, in general we have the main problem to be optimised, i.e. the 'add'
However, we also have important overheads to this:
  * loads (reading from memory)
  so `add a, input[0]` is dependent on load
  first see if particular memory location already moved into L1 cache
  * stores (these are equivalent to loads in terms of performance)
So, want to have cache size/layout in back-of-mind to know if exceeding
We have to keep things in size of cache so that compuation performance improvements actually matter and not just waiting on loads
IMPORTANT: Peak performance when all data in L1

IMPORTANT: theoretical increases like 4x with 4 cores etc. only seen if inside cache

### SIMD
core-compentency is optimising math operations

IMPORTANT: sse will still end up doing 4 add instructions internally (via a SIMD adder unit, not a scalar unit)
its main benefit is removing all the front-end work for CPU, i.e.
less instruction decoding, less dependency determinations etc.
Will still expect almost 4x speed increase

Lots of elbow grease organising data with SIMD
Can also do 16 bit lane size or even 8 bit  

### Multithreading
times where compiler won't produce what is optimally possible, e.g. staggered parallelism

Multi-threading gives us literally distinct instruction streams
Smaller buffer sizes could mean overhead of adding more threads not advantageous
Multi-threading can help with cache performance as well, as effectively get bigger L1 cache, i.e. 32K x 4 = effective 128K L1 cache
(this is why if have 4 threads an approximate x4 increase is actually minimum)
On chips optimised for single thread execution, a single core occupies majority of memory bandwidth. 
So, adding more cores won't increase memory bandwidth that much (visible in situations exceeding cache size)
Threads are becoming almost as a great a multiple as waste.
Server machines literally have 96 cores
Cores are rapidly increasing in new machines. SIMD, IPC remaining steady
So, biggest speed decreases are waste, cache and threads


### Real-World
1. actual operations to perform, e.g. math ops on haversine (throughput would be haversines/second)
2. how input sourced (json bad due to high-deserialisation overhead)
NOTE: sourcing from file or network same mental procedure to yield performance
3. output 
4. come up with optimal performance estimate and strive to get it

### Addendum
MIPS only indicative on single-thread throughput.

loop unrolling can remove say `cmp` overhead, but may increase instruction cache usage

JIT varied term. JIT debugging, is debugger automatically opening when program crashes
JIT compilation first compiles to something like bytecode. then at runtime, converts to machine language
this is powerful as can generate varied codepaths for same function based on run time, 
e.g. if paramater small inline, if large do something else, if called a certain way do this etc.
languages that use JIT like Javascript are not very slow because of JIT, 
rather language design decisions that mean it can't be optimised like C (like garbage collection)
Java JVM incorporates JIT in bytecode interpretation

'clean code':
(should be accompanied with, your code will become a lot slower if you do them)
polymorphism (no ifs/switches)
  -- the vtables are roughly 1.5x slower than switch
  -- in perspective, this is like erasing 3-4 years of hardware gains
black box (don't know internals of anything)
  -- switch statements help align everything by function, rather than class
  -- so, we can see what commonalities we have and optimise (another reason against having classes in distinct files)
  -- knowing what we are doing specifically is key to getting optimal performance 
  -- TODO: seems that a lookup table is faster if being called repetively as values in cache
functions should be small
functions should do one thing

Python speed up:
Using builtins faster like sum() as get rid of iteration overhead
reduce type deduction
call out to C library like numpy
integrate C directly like cython (fastest possible with SIMD etc.)
(in effect, on really need to know enough C to write optimised loops to insert into a higher level language)

### Questions
`TODO: quadPtrScalar faster as less expensive microps for address generation?` (compiler might not be smart enough as we have disabled SLP vectorisation?)
SLP (superword-level parallelism) a.k.a compiler auto-vectorisation optimisation 
