// SPDX-License-Identifier: zlib-acknowledgement

// register files like SRAM but laid out differently then conventional SRAM with flip-flops, latches and multiplexing logic so as to optimise surface area

// mov instruction more appropriate to call a copy, as original data still there
// instruction decoding not as simple as looking at opcode and operand fields as they are, as often control bits affect what operands mean 
// output text assembly and run it through nasm, then use diff utility to verify
