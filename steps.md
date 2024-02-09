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
IPC (instructions per clock) measure of how much IPC
often, the `inc` is the bottleneck in a loop?

threading and SIMD are explicit parallelism
VLIW is a form of explicit ILP

(ILP means that the IP register is really just a conceptual thing)

front-end of CPU is responsible for decoding instructions and passing micro-op queue to back-end
micro-ops are specific to back-end/architecture, e.g. zen 3, skylake all different
so, front-end convert instructions to 0 (in case of NOP) or more micro-ops
L1 cache is separated into two parts; I$ is instruction cache, D$ is data cache
also have decoded micro-op cache
inserting NOPs can throttle the front-end as it has to decode extraneous instructions to determine dependency chains (although should be removed by optimising compiler)

TODO: after scheduling, front-end has ports?

so, clearly can decode more than 1 instruction per cycle (and then execute)

x86 instruction decoding tries to guess where instructions end (as variable lengthed) to acheive multiple decodes on a cycle

the front-end schedules uops

TODO: how does front-end know to decode beyond a cmp and a jmp? i.e. how does it sustain rate of multiple cycle decodes if encounters cmps?

to facilitate ILP (requiring decoding instructions ahead of things affecting flags register), branch prediction is required on the front-ends part.
after the back-end executes a cmp, it will look to see if the address matches what the front-end guessed
if wrong, the back-end has to wait for front-end to refill uop queue, i.e. front-end must flush its queue (very costly, like 10 cycles)
(sometimes, get better performance with more instructions and less ifs if takes less than 10 cycles)

modern branch predictors utilise perceptrons and can in-fact guess CRT randomness

just taking a branch is costly?
cost of a branch misprediction is size of architecture pipeline, so maybe 12 cycles?

intel optimisation manual for specific microarchitecture, e.g. skylake client
amd documentation hub "software optimisation guide" (sort by relevence)
amd microarch numbers: https://en.wikipedia.org/wiki/List_of_AMD_CPU_microarchitectures

placement of branches greatly affects front-end

alignment of code can affect front-ends ability?

setting bottom 12 bits to 0, implies an alignment of 4k
(saves on hardware and more efficient)

cpu uses 64-byte alignment for caches.
so, for data alignment, don't won't say a 4byte integer to straddle cache lines
the front-end also accesses code via i-cache, so code alignment important (more so for large code bases)
hopefully, front-end just pulling from uop-cache for your loop (loops are mainly what code is)


TODO: error handling: if (unlikely(res) != OK) ...????


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

Saying one instruction is faster than the other ignores context of execution.
e.g. mul and add same latency, however due to pipelining mul execution unit might be full

Frequent context-switching will give terrible cache coherency

adding `restrict` also useful to prevent aliasing and thereby might allow compiler to vectorise say array loops

Before cpus increased in single thread execution speed. Now more cores. It's a topic of research to convert single threaded into multithreaded for emulation. 
This is why emulation of something like the GameCube (powerpc) is slow. 
If actually a simple translation, then should run close to native speed. This is reality of emulating hardware with hardware

CISC gives reduced cache pressure for high-intensive, sustained loops

log2(n) number of bits for decimal

twos complement (-1 all 1s)
Branch less programming is essentially SIMD

Not memory bound is best case for hyper threading
Intel speeds optimised for GPR arithmetic, boolean and flops
Intel deliberately makes mmx slow

Low cache associativity means fast lookup as less slots, but a lot of misses 
LRU eviction policy in-play when miss occurs. 

Hyperthreads useful only if different execution unit
Cpu reads memory from cache and ram in cache lines (due to programmer access patterns).
Each item in cache set is cache line size

CISC simplifies compiler optimisation (more versatile instructions), RISC simplifies hardware (less transistors)

Optimiser allows for lexical scoping of stack variables, i.e. variable referenced inside of for loop
However, to eliminate aliasing, try to use non-pointers

memory faster to read than write

2.5bln * 8 (simd) * n (execution units) * 2 (cores)
assuming instructions have throughput of 1
so 64 / 4 gives how many floats per second from L1 cache
in general, not streaming from memory the entire time
(would probably hit a cache bandwidth limit)

times when manual inlining is required (SIMD): (discovered this only by looking at counters) 
https://www.youtube.com/watch?v=B2BFbs0DJzw

As single threaded performance is largely stagnant you will have to utilise parallelism if want performance. 
Multithreading is building up a new discipline to single threaded. 
There are a lot of pitfalls for performance (balancing want things local, however must share to utilise)
If you care about performance for anything, you should care about cache misses
Memory bandwidth and caches are major reasons for a cpu attaining performance. 
You have to think about where does memory actually live and how is it transferred around

Computer faster than you think, e.g instructions, clock cycles, cores.
With M2 drive should be quick

We see contiguous memory in virtual memory, however in physical memory it is almost certainly going to be fragmented

compilers can auto-vectorize loops for us (and other operations if we perform say 20 of them).
so, floats will be twice as fast as doubles (more space, even though same latency and throughput)

with pure compiler optimisations, i.e. code we have not optimised ourselves, a 2x increase is not unexpected.
Code we optimised not as much

virtually never use lookup tables as ram memory is often 100x slower 
so unless you can't compute in 100 instructions

1. Optimisation (this is rarely done due to time involved)
Do back-of-envelope calculations; look at algorithm to see if wasteful; 
benchmark to see if hitting theoretical calculations and then use timings etc. 
to isolate why our code isn't hitting these theoretical numbers
Very importantly need to know what the theoretical maximum is, 
which will be dependent on the program, 
e.g bounded by number of bytes sent to graphics card, kernel stdout etc. 
2. Non-pessimisation (do this all the time)
Don't introduce unecessary work (interpreter; complex libraries; constructs like polymorphism/classes people have convinced themselves are necessary) for the CPU to do to solve the problem
3. Fake optimisation (very bad philosophy)
Repeating context-specific statements 
e.g. use memcpy as it's optimised 
(however the speed of something is so context specific, so non-statement), 
arrays are faster than linked lists (again, so dependent on what your usage patterns are)
IMPORTANT: In programming, preface the suitability of something to a particular environment/context

When many people say too much effort involved in optimisation, 
they are generally thinking of point 1

sometimes things should be running faster than they should, however only so much we can replace
e.g. the structure of many OSs are based on legacy code, so simply outputting to stdout may go through shell then kernel etc.
also, may have to deal with pessimised libraries. in these cases:
* isolate bad code, i.e. draw hard boundary between your code and theirs by caching calls to them
* do as little modification to the data coming in from them (no need to say put in a string class etc.)

Performance is critical in getting people excited about what you do, 
e.g. windows ce was laggy, iphone performant but less features changes the market 
low latency is more desirable, even if less features

branches can get problematic if they're unpredictable

so, FMUL may have latency of 11, however throughput of 0.5, so can start 2 every cycle, 
i.e. issue again (cpus these days are incredibly overlapped even if single-threaded)
so throughput is more of what we care about for sustained execution, e.g. in a loop

however, these numbers are all assuming the data is in the chip.
it's just as important to see how long it takes to get data to the chip.
look at cache parameters for microarchitecture; 
how many cycles to get from l1 cache, l2 cache, etc. 
when get to main memory potentially hundreds of cycles

bandwidth of L1 cache say is 80 bytes/cycle, 
so can get 20 floats per cycle (however, based on size of L1 cache, 
not really sustainable for large data)

http://igoro.com/archive/gallery-of-processor-cache-effects/

(could just shortcut this and just see if flops is recorded for our chip)

so, using these rough numbers we should be able to look at an algorithm 
(and dissect what operations it's performing, like FMULs), know how much data it's taking,
and give a rough estimate as to how long it could optimally take 
(will never hit optimal however)
IT'S CRITICAL TO KNOW HOW FAST SOMETHING SHOULD RUN

REASONS SOFTWARE IS SLOW:
1. No back-of-envelope calculations (people aren't concerned that they are running up to 1000 times slower thn what the hardware is capable of)
These calculations involve say looking at number of math ops to be performed in the algoirthm and comparing that to the perfect hardware limit
2. Reusing code (20LOC is a lot; use things that do what you want to do, but not in the way you want them to be done; 
often piling up code that is ill-fitting to what the task was, e.g. we know this isn't a regular ray cast, it's a ray cast that is always looking down)
3. When writing, thinking of goals ancillary to task 
(not many places taught how to actually write code; all high level abstractions about clean code
there are thinking about templates/classes etc. not just what does the computer actually have to do to do this task?)
(there is no metric for clean code; it's just some fictional thing people made up)

WHENEVER UNDERSTANDING CODE EXAMPLES:
1. COMPILE AND STEP INTO (NOT OVER) IN DEBUGGER AND MAKE HIGH LEVEL STEPS PERFORMED
look at these steps for duplicated/unecessary work (may pollute cache). (perhaps even asking why was the code written the way it was)
could we gather things up in a prepass, i.e. outside of loop?
if allocating memory each cycle that's game over for performance.
do we actually have to perform the same action to get the same result, e.g. a full raycast is not necessary, just segment on grid
O(n·m) is multiplicative, not linear O(n). 
big oh is just indication of how it scales. could be less given some input threshold
(big oh ignores constants, hence looking at aymptotic behaviour, i.e. limiting behaviour)
now, once code reduced, look at minimising number of ops 

cpu front end is figuring out what work it has to do, i.e. instruction decoding

e.g simd struct is 288 bytes, 4.5 cache lines, able to store 8 triangles

understanding assembly language is essential in understanding why the code might not be performing well

branch prediction necessary to ensure that the front-end can keep going and not have to wait on the back-end

execution ports execute uops.
however, the days of assembly language registers actually mapping to real registers is gone.
instead, the registers from the uops are passed through a register allocation table (if we have say 16 general purpose registers, table has about 192 entries; so a lot more) in the back-end
this is because in many programs, things can happen in any order. so to take advantage of this, the register allocation table stores dependency chains of operations
(wikichips.org for diagram)
from execution port, could be fed back into scheduler or to load/store in actual memory

when looking at assembly, when we say from memory, we actually mean from the L1 cache

xmm is a sse register (4 wide, 16 bytes); m128 is a memory operand of 128 bits
ymm is 8 wide
1*p01 + 1*p23 is saying issue 1 microop on either port0 or port1 and one microop on either port2 or port3
so, we could issue the same instruction multiple times, i.e. throughput of 0.5

microop fusion is where a microop doesn't count for your penalty as it's fused with another. with combined memory ops, e.g. `vsubps ymm8, ymm3, ymmword ptr [rdx]` this is the case 
so, if a compiler were to separate this out into a mov and then a sub, not only does this put unecessary strain on the front-end decoder it also removes microop fusion as they are now separate microops
(important to point out that I'm not the world best optimiser, or the worlds best optimisers assistant, so perhaps best not to outrightly say bad codegen, just say makes nervous)

godbolt.org good for comparing compiler outputs and possible detecting a spurious load etc.

macrop fusion is where you have an instruction that the front-end will handle for you, e.g. add and a jne will merge to addjne which will just send the 1 microp of add through 

uica.uops.info gives percentage of time instruction was on a port 
(this is useful for determining bottle-necks, e.g. series of instructions all require port 1 and 2, so cannot paralleise easily)
so, although best case say is issue instruction every 4 cycles, 
this bottleneck will give higher throughput

some levels of abstraction are necessary and good, e.g. higher level languages to assembly

Optimise: gather stats -> make estimate -> analyse efficiency and performance

file size
https://justine.lol/sizetricks
https://codegolf.stackexchange.com/questions/215216/high-throughput-fizz-buzz/236630#236630

more important to understand how CPU and memory work than language involved
in an OS, you will get given a zero-page due to security concerns


likely() macros for branch prediction compiler optimisations 
(https://akkadia.org/drepper/cpumemory.pdf, pg 56)

recording information:
We want to understand where slow with vtune, amd uprof, arm performance reports 
Next, determine if IO bound, memory bound etc.

To determine performance must have some stable metric, e.g. ops/sec to compare to
e.g measure total time and number of operations
Hyper-threading useful in alleviating memory latency, e.g. one thread is waiting to get content from RAM, 
the other hyper-thread can execute
However, as we are not memory bound (just going through pixel by pixel and not generating anything intermediate; 
will all probably stay in L1 cache), we are probably saturating the core's ALUs, so hyper-threading not as useful

Inspecting the assembly of our most expensive loop, we see that rand() is not inlined and is a call festival. This must be replaced
Essentially we are looking for mathematical functions that could be inlined and aren't that are in our hot-path.
When you want something to be fast, it should not be calling anything. If it does, probably made a mistake
Also note that using SIMD instructions, however not to their widest extent, i.e. single scalar 'ss'. Want to replace with 'ps' packed scalar

we have the option of constructor/destructor pairs
if we want to determine best possible time if all caches align etc. 'hunt for mininum', e.g. record mininum time execution in loop iteration or re-run if smaller time yielded
alternatively, we could develop a statistical breakdown of values (could see moments when kernel switches us out etc.)

(IMPORTANT save out configuration and timing information for various optimisation stages, e.g. ./app > 17-04-2022-image.txt)


agnerfog optimise website 'what's a creel'

threading
Observe CPU percentage use is not close to 100%
For multithreading, often have to pack into 64 bit value to perform single operation on it,
e.g. `delta = (val1 << 32 | val2); interlocked_add(&val, delta)`

When making multi-threaded, segregate task by writing prototype 'chunk' function, e.g. `render_tile`
Then write a for loop combining these chunk functions
Before entering the chunk function, good to have a configuration printout, e.g. num chunks, num cores, chunk dim, chunk size, etc.

When dividing a whole into pieces, an uneven divisor will give less than what's needed.
so `(total + divisor - 1) / divisor` to ensure always enough.
We will want this calculation to be in the last dividing operation, e.g. tile_width then tile_count calculated, so use on tile_count
Associated with this calculation is clamping to handle adding extra exceeding original dimensions
For getting proper place in chunk, call function wrapper for pointer location per row

(May have to inline functions?)
Next we want to pass each chunk onto a queue and then dequeue them from each logical core?
So, have a WorkOrder that will store all information required to perform operation on chunk, i.e. all parameters in `render_chunk` function
(may also store entropy for each chunk, i.e. random number series)
Then a WorkQueue that contains an array of WorkOrders with total number equalling number of chunks
So, the original loop iterating of chunks now just populates the WorkOrders 
Now in a while loop that runs while there are still chunks to execute, we call the `render_chunk` function and pass in the WorkQueue
The `render_chunk` function will increment the next_work_order_index and return true if more to be done

When spawning the actual worker thread functions, have same while loop calling the `render_chunk` function as for core 0
(the amount of threads to spawn would think should be equal to number of logical cores? however exceeding them may increase performance?)
(this debate of manually prescribing the core count applies to the chunk size as well. perhaps the sweet-spot for my machine in balancing context switching and drain out
is to manually prescribe their size as oppose to computing them off the core count)
(Collating information into the WorkQueue struct helsp for printing out configuration)
(Setting up this way, we can easily turn multithreading off)

As creating threads will require platform specific, put prototypes in main.h and the implementations in linux_main.cpp
Then include linux_main.cpp based on macro definition of platform in the build script at the bottom of main.cpp

hyperthreading, architecture specific information becomes more 
important when in a situation where memory is constrained in relation to the cache
(hyper-threads share same L1-L2 cache)

volatile says code other than what is generated by this compiler run, could modify this value.
it's required for multithreaded, as compiler may not re-read value that it may have cached in a register if changed elsewhere
when incrementing volatiles, must use a locked_add_and_return_previous_value (could return new value, just be clear)


simd
clamp can be re-written as min() and max() combination, which are instructions in SSE
Although looking at the system monitor shows cpus maxed out, we could be wasting cycles, e.g. not using SIMD

Define lane width, and divide with this to get the new loop count
Go through loop and loft used values e.g. lane_r32, lane_v3, lane_u32
(IMPORTANT at first we are only concerned with getting single values to work, later can worry about n-wide loading of values)
(TODO the current code has the slots for each lane generated, rather than unpacked. look at handmade hero for this unpacking mode)
If parameters to functions, loft them also (not functions? just parameters? however we do random_bilateral_lane() so yes to functions?)
If using struct or struct member references, take out values and loft them also, e.g. sphere.radius == lane_r32 sphere_r; 
(group struct remappings together)
Remap if statement conditions into a lane_u32 mask and remove enclosing brace hierarchy
(IMPORTANT you can still have if statements if they apply to lanes, e.g. if mask_is_zeroed() break;)
(TODO for mask_is_zeroed() we want the masks to be either all 1's or 0's)
(call mask_is_zeroed() on all masks to early out as often as possible to get a speed up)
Once lofted all if statements, & all the masks into a single mask 
(it seems if there is large amounts of code inside the if statements, you don't want to do it this way and rather check if needing to execute?)
(IMPORTANT to only & dependent masks, e.g. if there is an intermediate if like a pick_mask or clamp, then don't include it, but do the conditional assign directly on this mask)
Then enclose remaining assignments in a conditional assignment function using this single mask? 
(conditional_assign(&var, final_mask, value); this uses positive mask to get source and negative mask to get dest?)
(also discover the work around to perform binary operations on floating point numbers)
So, by end of this all values operated upon should be a lane type? (can have some scalar types if appropriate)
We may have situation where some items in a lane may finish before others.  So, introduce a lane_mask variable that indicates this. 
To indicate say a break, we can do (lane_mask = lane_mask & (hit_value == 0));
For incrementing, will have to introduce an incrementor value that will be zeroed out for the appropriate lane item that has finished.
Have horizontal_add()?
Next once everything remapped create a lane.h. Here, typedef the lane types to their single variants to ensure working before adding actual simd instructions
Also do simd helper functions like horizontal_add(), mask_is_zeroed() in one dimension first
Wrap the single lane helper functions and types in an if depending on the lane width set
(IMPORTANT any functions that we are to SIMD, place here. 
if it comes that we want actual scalar, then rename with func_lane prefix) 

for simd typically have to organically transition from AOS to SOA

Debug in single lane, single threaded mode (easier and debugger works)
However, can increase lane width as needed (threading not so much?)

For bitwise SIMD instructions, the compiler does not need to know how we are segmenting the register, e.g. 4x8, 8x8 etc. 
as the same result is obtained performing on the entire register at once.
So they only provide one version of it, i.e. no epi32 only si128
Naming convention have types: `__m128 (float), __m128i (integer), __m128d (double)` 
and names in functions: `epi32/si128 (integer), ps (float), pd (double)`
Overload operators on actual wide lane structs
(IMPORTANT remember to do both orders, e.g. (val / scalar) and (scalar / val))
Also have conversion functions

Lane agnostic functions go at bottom (like +=, -=, &=, most v3 functionality)
(IMPORTANT it seems we can replace logical && and || with binary for same functionality in simd)


(IMPORTANT simd does not handle unsigned conversions, may have to cut off sign bit, e.g. >> 1)

process of casting type to pointer to access individual bytes or containing elements (used in file reading too)

(IMPORTANT masks in SIMD will either be all 1's or 0's. perhaps have a specific name for this to distinguish?)

(IMPORTANT seems that not all operations are provided in SSE, like !=, so have to implement with some bitwise operations)

SIMD allows divide by zeros by default? (because nature of SIMD have to allow divide by zeroes?)


To get over the fact that C doesn't allow & floating point, reinterpret bit paradigm `*(u32 *)&a` as oppose to cast
(IMPORTANT in SIMD cast is reinterpreting bits, so the opposite of cast in C)



caching
https://akkadia.org/drepper/cpumemory.pdf
1. know cache sizes to have data fit in it
2. know cache line sizes to ensure data is close together (may have to separate components of structs to allow loops to access less cache lines) 
i.e. understand what you operate on frequently. may also have to align struct 
3. simple, linear access patterns (or prefetch instructions) for things larger than cache size 

inline assembly (raw syscalls from github)
HAVE TO INSPECT/VERIFY ASSEMBLY IS SANE FIRST THEN LOOK AT TIMING INFORMATION
inspecting compiler generated assembly loops, look for JMP to ascertain looping condition
due to macro-op fusion (relevent to say Skylake), e.g. cmp-jmp non-programmable instructions could be executed by the cpu
similarly, instructions that only exist on the frontend but exist programmatically e.g. xmm to xmm might just be a renaming in register allocation table
also due to concurrent port usage, can identify parts of code as relatively 'free'
struct access typically off a [base pointer]
in assembly, 1.0f might be large number e.g. 1065353216
in assembly loop, repeated instructions may be due to loop unrolling
we might see:
* superfluous loading of values off stack
* more instructions required, e.g not efficiently using SIMD 
(often this exposes the misconception that compilers are better than programmers; so better to handwrite intrinsics)


comparing unoptimised assembly to 'wc' see noticeable speed increase.
example of non-pessimisation

Following the basic principles of non-pessimisation, 
I make a note of the huge amount of cruft in the C STL.
The output buffering, hidden `malloc()` 'optimisations' (uncommitted memory, encounter expensive page faults later; prefer reliability/clarity over edge-performance benefits), 
OS line ending conversions, non-obvious use of mutexes etc.
Whilst these may seem like minor inconveniences, they can be insidious for performance, e.g.
`rand()` has a huge call-stack that if we replace with a simple xor shift, 
results in 3x speed up.
Although easy to criticise, it may be the situation that the CRT had to be that way 
because of C standards.
To isolate use of the CRT, wrap in functions so we can hopefully replace with system calls, 
intrinisics, etc.
Although generally okay to use STL, it forces you to use their patterns (e.g. memory allocation, locks etc.).
This is true for STLs in general across all languages.
Some may have bloatness from other areas, e.g. C++ templates
To avoid the compiler having to generate a large export table of all functions, 
make them `static`
To avoid large amounts of linking and ∴ increase compilation time, have a unity build. 
Furthermore, garunteed ability to inline functions (as with multiple translation units, possible one might only have function declaration and not definition)
(issues may occur with slower incremental builds when including 3rd party libraries; yet, can still work around this possibly using ccache)
