#include "chip8.hpp"

#include <cstdlib>

#include <algorithm>
#include <array>

Chip8::Chip8() {
  reset();
}

Chip8& Chip8::reset() {
  pc = 0x200; // program starts at 0x200
  opcode = 0; // reset current opcode
  I = 0; // reset index register

  // reset registers
  V.fill(0);

  // reset stack
  while (!stack.empty()) stack.pop();

  // reset memory
  memory.fill(0);

  // reset graphics
  mGFX.fill(0);

  // reset keys
  key.fill(false);

  // reset timers
  delayTimer = 0;
  soundTimer = 0;

  // draw flag
  drawFlag = false;

  return *this;
}

Chip8& Chip8::load(std::array<unsigned char, 4096>& game) {
  std::copy(game.begin(), game.end(), memory.begin());
  return *this;
}

bool Chip8::update() {
  // reset drawFlag
  drawFlag = false;

  // fetch opcode
  opcode = memory[pc] << 8 | memory[pc + 1];

  // decode opcode
  switch (opcode & 0xF000) {

    case 0x0000:
      switch (opcode & 0x000F) {
        case 0x0000: // 0x00E0: clears the screen
          mGFX.fill(0);
          drawFlag = true;
          break;

        case 0x000E: // 0x00EE: returns from subroutine
          pc = stack.top();
          stack.pop();
          break;

        default:
          printf("Unkown opcode [0x0000]: 0x%X\n", opcode);
      }
      break;

    case 0x1000: // 0x1NNN: Jumps to address NNN.
      pc = opcode & 0x0FFF;
      break;

    case 0x2000: // 0x2NNN: Calls subroutine at NNN.
      stack.push(pc);
      pc = opcode & 0x0FFF;
      break;

    case 0x3000: // 0x3XNN: Skips the next instruction if VX equals NN.
      if (V[(opcode & 0x0F00) >> 8] == opcode & 0x00FF) pc += 2;
      break;

    case 0x4000: // 0x4XNN: Skips the next instruction if VX doesn't equal NN.
      if (V[(opcode & 0x0F00) >> 8] != opcode & 0x00FF) pc += 2;
      break;

    case 0x5000: // 0x5XY0: Skips the next instruction if VX equals VY.
      if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) pc += 2;
      break;

    case 0x6000: // 0x6XNN: Sets VX to NN.
      V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
      break;

    case 0x7000: // 0x7XNN: Adds NN to VX.
      V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
      break;

    default:
      printf("Unknown opcode: 0x%X\n", opcode);
  }

  // update program counter
  pc += 2;

  return drawFlag;
}
