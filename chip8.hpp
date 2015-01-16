#ifndef __CHIP8_HPP
#define __CHIP8_HPP

#include <array>
#include <stack>

class Chip8 {
  unsigned short opcode;

  // index register
  unsigned short I;

  // program counter
  unsigned short pc;

  // registers V0 up to VF
  std::array<unsigned char, 16> V;

  // 4k memory
  std::array<unsigned char, 4096> memory;

  // graphics 2048 pixels (64 x 32)
  std::array<unsigned char, 2048> mGFX;

  // timers
  unsigned char delayTimer;
  unsigned char soundTimer;

  // stacks
  std::stack<unsigned short> stack;

public:
  Chip8();

  // load game into memory. returns self
  Chip8& load(std::array<unsigned char, 4096>&);

  Chip8& reset();

  bool update(); // progress one opcode and return whether to draw or not
  bool drawFlag; // if you need draw flag again

  std::array<bool, 16> key; // keys pressed -> true

  unsigned char gfx(unsigned int row, unsigned int col) const {
    return mGFX[64 * row + col];
  }
};

#endif
