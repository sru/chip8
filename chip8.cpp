#include <cstdio>
#include <cstdlib>

#include <algorithm>
#include <array>

#include "chip8.hpp"

unsigned char x;
unsigned char y;

std::array<unsigned char, 80> fontset {
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() {
  reset();
}

Chip8::Chip8(std::array<unsigned char, 3584>& game) {
  reset();
  load(game);
}

Chip8& Chip8::reset() {
  pc = 0x200; // program starts at 0x200
  opcode = 0; // reset current opcode
  I = 0; // reset index register

  // clear registers
  V.fill(0);

  // clear stack
  while (!stack.empty()) stack.pop();

  // clear memory
  memory.fill(0);

  // load fontset
  std::copy(fontset.begin(), fontset.end(), memory.begin());

  // reset graphics
  mGFX.fill(0);

  // reset keys
  key.fill(false);

  // reset timers
  delayTimer = 0;
  soundTimer = 0;

  // draw flag
  mDraw = false;

  return *this;
}

Chip8& Chip8::load(std::array<unsigned char, 3584>& game) {
  std::copy(game.begin(), game.end(), memory.begin() + 0x200);
  return *this;
}

bool Chip8::update() {
  // reset draw
  mDraw = false;

  // fetch opcode
  opcode = memory[pc] << 8 | memory[pc + 1];

  // use x and y if applicable
  // usually in the form of 0xy0
  x = (opcode & 0x0F00) >> 8;
  y = (opcode & 0x00F0) >> 4;

  // decode opcode
  switch (opcode & 0xF000) {

    case 0x0000:
      switch (opcode & 0x000F) {
        case 0x0000: // 00E0: Clear the screen
          mGFX.fill(0);
          mDraw = true;
          break;

        case 0x000E: // 00EE: Return from subroutine
          pc = stack.top();
          stack.pop();
          break;

        default:
          printf("Unkown opcode [0x0000]: 0x%X\n", opcode);
      }
      break;

    case 0x1000: // 1nnn: Jump to address nnn.
      pc = opcode & 0x0FFF;
      break;

    case 0x2000: // 2nnn: Call subroutine at nnn.
      stack.push(pc);
      pc = opcode & 0x0FFF;
      break;

    case 0x3000: // 3xnn: Skip the next instruction if Vx equals nn.
      if (V[x] == (opcode & 0x00FF)) pc += 2;
      break;

    case 0x4000: // 4xnn: Skip the next instruction if Vx doesn't equal nn.
      if (V[x] != (opcode & 0x00FF)) pc += 2;
      break;

    case 0x5000: // 5xy0: Skip the next instruction if Vx equals Vy.
      if (V[x] == V[y]) pc += 2;
      break;

    case 0x6000: // 6xnn: Set Vx to nn.
      V[x] = opcode & 0x00FF;
      break;

    case 0x7000: // 7xnn: Add nn to Vx.
      V[x] += opcode & 0x00FF;
      break;

    case 0x8000:
      switch (opcode & 0x000F) {
        case 0x0000: // 8xy0: Set Vx = Vy.
          V[x] = V[y];
          break;

        case 0x0001: // 8xy1: Set Vx = Vx OR Vy.
          V[x] |= V[y];
          break;

        case 0x0002: // 8xy2: Set Vx = Vx AND Vy.
          V[x] &= V[y];
          break;

        case 0x0003: // 8xy3: Set Vx = Vx XOR Vy.
          V[x] ^= V[y];
          break;

        case 0x0004: // 8xy4: Set Vx = Vx + Vy, set VF = carry.
          if (V[x] > 0xFF - V[y]) V[0xF] = 1;
          else V[0xF] = 0;
          V[x] += V[y];
          break;

        case 0x0005: // 8xy5: Set Vx = Vx - Vy, set VF = NOT borrow.
          if (V[x] > V[y]) V[0xF] = 1;
          else V[0xF] = 0;
          V[x] -= V[y];
          break;

        case 0x0006: // 8xy6: Shift Vx right by one. VF = least signifcant bit of Vx before the shift.
          V[0xF] = V[x] & 1;
          V[x] >>= 1;
          break;

        case 0x0007: // 8xy7: Set Vx = Vy - Vx, set VF = NOT borrow.
          if (V[y] > V[x]) V[0xF] = 1;
          else V[0xF] = 0;
          V[x] = V[y] - V[x];
          break;

        case 0x000E: // 8xyE: Shift Vx left by one. VF = most significant bit of Vx before the shift.
          V[0xF] = (V[x] & 0x80) >> 7;
          V[x] <<= 1;
          break;

        default:
          printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
      }
      break;

    case 0x9000: // 9xy0: Skip the next instruction if Vx doesn't equal Vy.
      if (V[x] != V[y]) pc += 2;
      break;

    case 0xA000: // Annn: Set I to the address nnn.
      I = opcode & 0x0FFF;
      break;

    case 0xB000: // Bnnn: Jump to the address nnn plus V0.
      pc = (opcode & 0x0FFF) + V[0];
      break;

    case 0xC000: // Cxnn: Set Vx to a random number AND nn.
      V[x] = (rand() % 0xFF) & (opcode & 0x00FF);
      break;

    case 0xD000: // Dxyn: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
      for (int row = 0; row < (opcode & 0x000F); ++row) {
        for (int col = 0; col < 8; ++col) {
          if (memory[I + row] & (0x80 >> col) != 0) {
            if (mGFX[((V[y] + row) * 64 + V[x] + col)] == 1)
              V[0xF] = 1;
            mGFX[((V[y] + row) * 64 + V[x] + col)] ^= 1;
          }
        }
      }
      mDraw = true;
      break;

    case 0xE000:
      switch (opcode & 0x00FF) {
        case 0x009E: // Ex9E: Skip the next instruction if the key stored in Vx is pressed.
          if (key[V[x]]) pc += 2;
          break;

        case 0x00A1: // ExA1: Skip the next instruction if the key stored in Vx isn't pressed.
          if (!key[V[x]]) pc += 2;
          break;

        default:
          printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
      }
      break;

    case 0xF000:
      switch (opcode & 0x00FF) {
        case 0x0007: // Fx07: Set Vx to the value of the delay timer.
          V[x] = delayTimer;
          break;

        case 0x000A: // Fx0A: A key press is awaited, and then store in Vx.
          if (std::none_of(key.begin(), key.end(), [](bool b) { return b; })) pc -= 2;
          else V[x] = std::find(key.begin(), key.end(), true) - key.begin();
          break;

        case 0x0015: // Fx15: Set the delay timer to Vx.
          delayTimer = V[x];
          break;

        case 0x0018: // Fx18: Set the sound timer to Vx.
          soundTimer = V[x];
          break;

        case 0x001E: // Fx1E: Add Vx to I.
          I += V[x];
          break;

        case 0x0029: // Fx29: Set I to the location of the sprite for the character in VX.
          I = V[x] * 0x5;
          break;

        case 0x0033: // Fx33: Store BCD representation of Vx in memory locations I, I+1, and I+2.
          memory[I] = V[x] / 100;
          memory[I + 1] = (V[x] / 10) % 10;
          memory[I + 2] = (V[x] % 100) % 10;
          break;

        case 0x0055: // Fx55: Store registers V0 through Vx in memory starting at location I.
          std::copy(V.begin(), V.begin() + (x), memory.begin() + I);
          break;

        case 0x0065: // Fx65: Read registers V0 through Vx from memory starting at location I.
          std::copy(memory.begin() + I, memory.begin() + I + (x), V.begin());
          break;

        default:
          printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
      }

    default:
      printf("Unknown opcode: 0x%X\n", opcode);
  }

  // update timers
  if (delayTimer > 0) --delayTimer;
  if (soundTimer > 0) --soundTimer;

  // update program counter
  pc += 2;

  return mDraw;
}

bool Chip8::draw() {
  return mDraw;
}

bool Chip8::sound() {
  return soundTimer != 0;
}
