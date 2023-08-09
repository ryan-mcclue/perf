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

### Waste
This is anything not necessary for computation.
Instruction stream unecessarily widened leading to binary bloat and added runtime cost e.g. templates, interpreters (10x overhead considered good, e.g. rigmarole determining types and how to add; 1 add instruction in C)
Binary bloat is one factor that could lead to instruction cache misses. So, not using templates is a simple way to reduce this and runtime cost.

even if stuck in a high-level, can gain some performance benefits by rearranging code
furthermore, could offload sections to C

### IPC


### Cache

when timing code, repeat; this will give time to show what it's like when cache is primed
count operations/cycle

IMPORTANT: theoretical increases like 4x with 4 cores etc. only seen if inside cache

### SIMD
core-compentency is optimising math operations

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

### Questions
`TODO: quadPtrScalar faster as less expensive microps for address generation?` (compiler might not be smart enough as we have disabled SLP vectorisation?)
SLP (superword-level parallelism) a.k.a compiler auto-vectorisation optimisation 
