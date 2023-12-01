// SPDX-License-Identifier: zlib-acknowledgement
## Introduction
Know how to say:
  1. How expensive an operation is
  2. How hard to implement 
  3. How hard to optimise

1. Optimisation requires specialised knowledge of the platform and is time consuming.
2. Algorithmic optimisation is not necessarily performance aware programming, e.g. could be asymptotic analysis
3. Performance aware programming works towards optimisation, just not all the way by utilising CPU model
   Performance is related to resource consumption (could be various metrics, e.g. network, memory, battery, etc.)
   Virtually all successful software companies like Facebook, Google, Apple do customer research to show that all product lines gain business value if faster, either by customer engagement or compute cost

Want to know how to go through full procedure of performance aware to get an idea of how data flows.
This will build up vocabularly to know what to look for, so can look up information on demand, e.g. know how it works for ryzen, how about for zen 4 etc.
For future projects, at certain points, remember factor and then investigate more in-depth

IMPORTANT: Apply a mindset to do some upfront work to preserve future performance options
e.g, some languages don't allow compiler to make optimisations, e.g. garbage collection, exceptions, dynamic typing, etc.

## Misconceptions
Early on easily obtained optimisations are for hotspots are good.

Compilers using templates introduce added build-time cost.

**CLEAN CODE** (should be accompanied with, your code will become a lot slower if you do them):
* Polymorphism (no ifs/switches)
  - additional memory access into polymorphic object vtables is roughly 1.5x slower than switch statement (this is like erasing 3-4 years of hardware gains)
* Black box (don't know internals of anything)
  - keeping things explicit forces things together, allowing for commonalities to be seen and optimised (another reason against having classes in distinct files)
* Functions should be small and do one thing
  - code is much more readable if local and don't have to open 30 tabs (then you have to understand package structure to understand functionality)

## Links
http://danglingpointers.com/tags/data-oriented-design/
http://danglingpointers.com/tags/concurrency/

## Cache
when timing code, repeat; this will give time to show what it's like when cache is primed
count operations/cycle
IMPORTANT: theoretical increases like 4x with 4 cores etc. only seen if inside cache

1. reduce number of instructions (waste, SIMD)
2. increase speed of instructions (same instructions can be faster) (IPC, cache, multithreading)

don't think about source language. think about what it becomes, e.g. templates, garbage-collection are not a zero-cost
it's only about what the source turns into, is when we can talk about its speed

main source of modern program slowdown is wasteful instructions
consider python. if we compile interpreter from source to inspect what assembly is run, find that incredible amount of waste
to first determine operator, then more waste to store result. all that was required was a single 'add' instruction
instruction stream is hugely widened for bytecode interpreter to do its work
literally just naive 'add' program see 100x slowdown
so, we don't need to do 'optimisation' just come back to our senses
could offload to numpy, or use JIT?
NOTE: 10x overhead considered good for interpreter (this would be with bytecode) 
loop unrolling can remove say `cmp` overhead, but may increase instruction cache usage
even if stuck in a high-level, can gain some performance benefits by rearranging code
furthermore, could offload sections to C

strictly, waste is if it was necessary for the computation.
not if it might be used sometime in another problem, as this would mean technically nothing is waste

modern CPUs are superscalar/overlapped, i.e. instruction level parallelism allowing for multiple instructions per cycle
this is acheived as the CPU will look for instructions it can execute at the same time
so, instructions must not be serially dependent
the CPU does not have complex logic in ascertaining dependency chains. 
it only looks at operands. so, assumes series of 'add' on same register are serially dependent
`add a, input[i]; add a, input[i + 1]`
we can help the CPU however, noting that say a summation can be broken up into multiple dependency chains
`add a, input[i]; add b, input[i + 1]`
`TODO: quadPtrScalar faster as less expensive microps for address generation?` (compiler might not be smart enough as we have disabled SLP vectorisation?)
so, a CPU will have a max. IPC value
generally, if can reduce dependency chain, always applicable
branch misprediction is costly because of this, as have to look ahead in instruction stream to make things pipelined

JIT varied term. JIT debugging, is debugger automatically opening when program crashes
JIT compilation first compiles to something like bytecode. then at runtime, converts to machine language
this is powerful as can generate varied codepaths for same function based on run time, 
e.g. if paramater small inline, if large do something else, if called a certain way do this etc.
languages that use JIT like Javascript are not very slow because of JIT, rather language design decisions that mean it can't be optimised like C (like garbage collection)
Rust has a strong-compile language model like C
Java JVM incorporates JIT in bytecode interpretation

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

IMPORTANT: multithreading is largely an architectural issue as oppose to technical like SIMD?
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


## Assembly
Register files like SRAM but laid out differently then conventional SRAM with flip-flops, latches and multiplexing logic so as to optimise surface area

mov instruction more appropriate to call a copy, as original data still there
Instruction decoding not as simple as looking at opcode and operand fields as often control bits affect what operands mean 

TODO: incorporate comp95.. into assembly understanding, e.g. overflow flag for signed jumps, cmp is a sub, etc.

## Steps
1. Profiling (do regularly when have functionality)
rdtsc is an invariant tsc broadcast to all cores regardless of individual core frequency differences
So, not literally counting core cycles but more like a precise wall clock
(tsc is on CPU, hpet is external on motherboard and would read from I/O port using `in`)

As `clock_gettime()` is vDSO syscall, cannot step in naively.
Looking at disassembly:
  * Uses 'pause' instruction (most likely a spin lock for synchronisation)
  * Kernel uses hpet over tsc as clock source if tsc considered unstable (`dmesg | rg -i clocksource`)
  * Noticed that granularity of rdtsc was reduced, so better to use rdtsc directly

Instrumentation profiling is adding timing code as opposed to sampling which periodically interrupts (better for small pieces of code?)

1.1. Run complete profiler and ascertain overall program profile/hotspots
1.2. As anything called say 1mil. times will always give a performance penalty want a way to modularly bring in profiler to see how it affects program runtime.
     Remove sections until not affecting performance as much, e.g. hotspots, nested block removals (essentially perform A/B testing)
     (to time function with no nested overheads, just `TIME_BLOCK("S") { func() };`)
1.3. CPU state is highly variable, e.g. cache primed, clock speed, peripheral latency etc.
     So, perform repetition testing on hotspots and compare with application to roughly know what practical peak performance is.
     May see higher bandwidth in repetition tester than in our application, particularly on first iteration.
     This could be repeated page faults on malloc causing first iteration to be slow.
(could have a separate thread on startup that pre-touches memory (in a sense, prefetching), before saturating it with work)
1.4. 
We want to know: 
1. how close is this to theoretical maximum? 
2. is this most efficient way?

Inspect hotspot assembly to establish cycles per hotspot?:
We know that file will probably be in OS cache. So, effectively a memory read and memory write
IMPORTANT: the best way to look at dissassembly is in debugger, to ensure compiled with same flags etc.
IMPORTANT: when inspecting assembly in debugger want to verify section looking at; can have registers (rax/al etc.) in watch to aid this
also want to have -O1 flag to remove cruft, but not go all the way to auto-vectorisation

how much CPU bring in, process and decode for this loop?
how much are we asking the CPU to process each loop?
In assembly, see that loop is decoding 11 bytes of instructions per iteration
Running loop under profiler know bandwidth
Know each loop writing 8 bytes; so (bandwidth / 8) gives loop throughput
So: machine-freq / (bandwidth / 8) = num-cycles per loop
TODO: fractional cycle counts typical for superscalar cpus?

when thinking about how fast instructions can occur are looking at:
latency (from start to end for 1 job), 
throughput (how many jobs started per unit time; reciprocal throughput often used in CPU context?) 
and dependency chains (independent n-steps)

adding more work units increases the throughput of the system (useful if bottleneck is throughput)

look at longest dependency chain to ascertain order for work unit schedules
see which work unit is most saturated

the CPU instruction decoder (front-end?) sees loop iteration as one giant stream of instructions.
it will try and extract as much ILP by analysing loop iteration dependency chains
often, the `inc` is the bottleneck in a loop?






##### OS
8086 had linear access of memory. cortex-m4 similar, however MPU to enforce some memory safety.
modern OS also has virtual memory. malloc gives memory in our process virtual address.
for every process cpu core is currently executing, will have unique address translation tables for that process.
cpu also has translation look-aside buffer which caches lookup results in the translation tables
the OS is responsible for giving the CPU the translation tables, so needs to know where is physical memory address will go
the OS allocates in pages (typically 4kb)
in a sense, heirarchical memory structure, with OS managing pages, malloc giving chunks of those pages to you
OS lazily creates page mappings, e.g. on first allocation, will just return virtual addresss, no physical address assigned yet
so, on first writing to address, CPU will trigger a page fault as OS has not population translation table yet. OS will then populate table
linux large pages with mmap MAP_HUGETLB (perhaps easier with tunables, e.g. GLIBC_TUNABLES=glibc.malloc.hugetlb=2 ./executable)
(OS may mark 16 pages at a time, instead of 1 each time)
(TODO(Ryan): Page tables can be dropped in a device driver?)
(on linux, on reading first, will just return 0 page; so OS policies affect what happens on reads)
not necessarily contiguous in physical memory (only virtual memory)

a memory mapped file might save memory if file is already cached.
e.g, when page fault, OS will point to file cache, instead of new physical memory

page file is for overcommits, i.e. exceed physical memory (really only benefit of memory mapped files; so theoretical niceties). perhaps just api changes are benefit
also, memory mapped files don't allow for async file io, as blocks thread?

so, remember OS is doing behind-the-scenes work clearing and giving us pages  

IMPORTANT(Ryan): mlock() applicable for server applications as can control the machine (also for embedded say)
