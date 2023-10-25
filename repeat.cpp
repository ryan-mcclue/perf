// SPDX-License-Identifier: zlib-acknowledgement

// going through procedure to get idea of how data flows through (not going to remember everything)
// build up vocabularly to know what to look for, so can look up information on demand, e.g. know how it works for ryzen, how about for zen 4 etc.
// for future projects, at certain points, remember factor and then investigate more in-depth

// so much variability, e.g. caching, clock speed, peripheral latency, tons of cpu state etc.
// so, repetition testing

// IMPORTANT(Ryan): Have repetition testers run ad infinatum to ascertain if first iteration is slowest
// With profile overview ascertained, see that reading is large part. So, run in repetition test.
// See higher bandwidth in repetition tester than in our application, particularly on first read.
// Collecting OS metrics on repetition tester, see that get page faults when repeating mallocs
//
// 8086 had linear access of memory. cortex-m4 similar, however MPU to enforce some memory safety.
// modern OS also has virtual memory. malloc gives memory in our process virtual address.
// for every process cpu core is currently executing, will have unique address translation tables for that process.
// cpu also has translation look-aside buffer which caches lookup results in the translation tables
// the OS is responsible for giving the CPU the translation tables, so needs to know where is physical memory address will go
// the OS allocates in pages (typically 4kb)
// in a sense, heirarchical memory structure, with OS managing pages, malloc giving chunks of those pages to you
// OS lazily creates page mappings, e.g. on first allocation, will just return virtual addresss, no physical address assigned yet
// so, on first writing to address, CPU will trigger a page fault as OS has not population translation table yet. OS will then populate table
// linux large pages with mmap MAP_HUGETLB (perhaps easier with tunables, e.g. GLIBC_TUNABLES=glibc.malloc.hugetlb=2 ./executable)
// (OS may mark 16 pages at a time, instead of 1 each time)
// (TODO(Ryan): Page tables can be dropped in a device driver?)
// (on linux, on reading first, will just return 0 page; so OS policies affect what happens on reads)
// not necessarily contiguous in physical memory (only virtual memory)

// a memory mapped file might save memory if file is already cached.
// e.g, when page fault, OS will point to file cache, instead of new physical memory

// page file is for overcommits, i.e. exceed physical memory (really only benefit of memory mapped files; so theoretical niceties). perhaps just api changes are benefit
// also, memory mapped files don't allow for async file io, as blocks thread?

// so, remember OS is doing behind-the-scenes work clearing and giving us pages  

// IMPORTANT(Ryan): mlock() applicable for server applications as can control the machine (also for embedded say)

struct AddressArray
{
  void **addresses;
  u64 count;
  u64 page_size;
};

struct TrackedBuffer
{
  String8 base;
  AddressArray results;
};

TrackedBuffer allocate(u64 size)
{
  TrackedBuffer buf;
  buf.base.content = malloc(size); // want to read dirty bits of this stuff
  buf.base.size = size;

  buf.results.count = size / page_size;
  buf.results.addresses = malloc(page_count * sizeof(void**));
}

// so, track pages written to across calls
AddressArray get_and_clear_written_pages(buf)
{
  // https://lwn.net/Articles/940704/
  // https://docs.kernel.org/admin-guide/mm/soft-dirty.html
  // read and then reset dirty bits
  // get say page 10, 11, 20 written to
}

// paging get circular buffer, sparseness and memory recording for free 
struct CircularBuffer
{
  String8 buf;
  u32 rep_count;
};

INTERNAL CircularBuffer
create(void)
{
  u64 data_size = ALIGN_POW2_UP(size, sysconf(_SC_PAGESIZE));
  u64 total_size = rep_count * data_size;

  // don't commit, just want this much virtual address space
  u8 *addr = mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (addr == MAP_FAILED);

  // shrink to actual size to get usable handles
  u8 *first = mremap(addr, total_size, data_size, 0);
  for ()

  u8 *second = mremap(addr, 0, data_size, MREMAP_MAYMOVE, addr+data_size);
  u8 *third = mremap(addr, 0, data_size, MREMAP_MAYMOVE, addr+data_size*2);

  // return second; so second[0] = second[-size] = second[size]
  // so can now just do memcpys without splitting

  return str8(mapping, data_size);

  // free
  munmap(addr, size);
}

test_func test_functions[] = {
  {"fread", read_via_fread},
  {"read", read_via_read}
};

RepetitionTester testers[ARRAY_COUNT(test_functions)] = {};

for (u32 i = 0; i < ARRAY_COUNT(test_functions); i += 1)
{
  new_test_wave(testers[i]);
  test_functions[i].func(testers[i]);
}

while (is_testing())
{
  if (tester->open_block_count > 0) // if any begin_time() recorded
  // test here
}
