// SPDX-License-Identifier: zlib-acknowledgement

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

