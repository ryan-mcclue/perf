// SPDX-License-Identifier: zlib-acknowledgement
https://git.some.website/aolo2/compooter-enhensa


modern CPUs are superscalar/overlapped, i.e. instruction level parallelism allowing for multiple instructions per cycle
this is acheived as the CPU will look for instructions it can execute at the same time
so, instructions must not be serially dependent
the CPU does not have complex logic in ascertaining dependency chains. 
it only looks at operands. so, assumes series of 'add' on same register are serially dependent
`add a, input[i]; add a, input[i + 1]`
we can help the CPU however, noting that say a summation can be broken up into multiple dependency chains
`add a, input[i]; add b, input[i + 1]`
so, a CPU will have a max. IPC value
generally, if can reduce dependency chain, always applicable
branch misprediction is costly because of this, as have to look ahead in instruction stream to make things pipelined

Not as simple to say if 4 add units then unroll 4 times. 
To determine loop unrolling factor must analyse port usage
ports perform multiple operations, e.g. add or cmp. but can only do 1 at a time
so, there might be port contention

Given a codebase, not really a constructive question to ask why written in a language
What's done is done. Just be performance-aware. Perhaps was better in another language, but that decision is made. 
If I know how fast something can run, strive to make it as fast as I can

Make decisions you know it will make a difference on say, Skylake chip
Know performance characteristics of microarchtictures and how they apply to say the pipeline execution

performance analysis one of 2 things:
1. determine behaviour of microarchitecture, i.e. peak fastest:
   if only take mean, median we don't see what it converges to
   we want to take minimum cycles, to see what will happen if 'all the stars align'
2. see what is typical
   given a mix of what is cache state, what else in pipeline, other cores doing 
   look at mean/median 

times where compiler want produce what is optimally possible, e.g. staggered parallelism

IMPORTANT: sse will still end up doing 4 add instructions internally (via a SIMD adder unit, not a scalar unit)
its main benefit is removing all the front-end work for CPU, i.e.
less instruction decoding, less dependency determinations etc.
Will still expect almost 4x speed increase

Lots of elbow grease organising data with SIMD
Can also do 16 bit lane size or even 8 bit  

Can combine IPC with SIMD
With just IPC and SIMD multipliers, can literally get 1000x increase compared to Python 


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

This great speedup from Python to C just for a naive add loop. 
So, for larger more complex programs that can utilise more hardware, even greater speedups!

Python speed up:
Using builtins faster like sum() as get rid of iteration overhead
reduce type deduction
call out to C library like numpy
integrate C directly like cython (fastest possible with SIMD etc.)
(in effect, on really need to know enough C to write optimised loops to insert into a higher level language)


1. Operational, e.g: First step is optimising math operations (as easiest. this is core competency)
2. Input: how did the data get here, e.g. points for math operation (could be JSON file, which is not favourable for performance)
(probably optimising use of file, rather than OS actually opening file)
So, time how long input takes and 'math' takes  

Break down problem into parts, and find what are performance bottlenecks
Learn what we expect for parsing etc. Then make estimate for theoretical maximum
