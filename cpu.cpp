// SPDX-License-Identifier: zlib-acknowledgement

// register files like SRAM but laid out differently then conventional SRAM with flip-flops, latches and multiplexing logic so as to optimise surface area

// mov instruction more appropriate to call a copy, as original data still there
// instruction decoding not as simple as looking at opcode and operand fields as they are, as often control bits affect what operands mean 
// output text assembly and run it through nasm, then use diff utility to verify

#include "base-inc.h"

#if 0
 String8List args_list = {0};
 for(u32 i = 1; i < argc; i += 1)
 {
  Str8ListPush(tctx.arenas[0], &args_list, Str8C(arguments[argument_idx]));
 }

 CmdLine cmdline = CmdLineFromStringList(tctx.arenas[0], args_list);
 some_func(&cmdline);
#endif


int
main(int argc, char *argv[])
{
  global_debugger_present = linux_was_launched_by_gdb();
  MemArena *arena = mem_arena_allocate(GB(1), GB(1)); 

  String8 single_register_mov = str8_read_entire_file(arena, str8_lit("../listings/single-register-mov"));

  u16 instruction = *(u16 *)single_register_mov.content;
  instruction = u16_endianness_swap(instruction);

  u32 opcode_mask = 0xfc00;
  u32 opcode_shift = 10;
  u32 opcode = (instruction & opcode_mask) >> opcode_shift;
  ASSERT(opcode == 0x22);

  u32 direction_mask = ;
  u32 direction_shift = ;
  u32 direction = ;
  ASSERT(direction == );

  u32 mode_mask = ;
  u32 mode_shift = ;
  u32 mode = ;
  ASSERT(mode == );


  return 0;
}

