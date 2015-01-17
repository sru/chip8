#include "chip8.hpp"

#include <cstdio>
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
  _draw = false;

  return *this;
}

Chip8& Chip8::load(std::array<unsigned char, 4096>& game) {
  std::copy(game.begin(), game.end(), memory.begin());
  return *this;
}

bool Chip8::update() {
  // reset draw
  _draw = false;

  // fetch opcode
  opcode = memory[pc] << 8 | memory[pc + 1];

  // decode opcode
  switch (opcode & 0xF000) {

    case 0x0000:
      switch (opcode & 0x000F) {
        case 0x0000: // 00E0: clears the screen
          mGFX.fill(0);
          _draw = true;
          break;

        case 0x000E: // 00EE: returns from subroutine
          pc = stack.top();
          stack.pop();
          break;

        default:
          printf("Unkown opcode [0x0000]: 0x%X\n", opcode);
      }
      break;

    case 0x1000: // 1nnn: Jumps to address nnn.
      pc = opcode & 0x0FFF;
      break;

    case 0x2000: // 2nnn: Calls subroutine at nnn.
      stack.push(pc);
      pc = opcode & 0x0FFF;
      break;

    case 0x3000: // 3xnn: Skips the next instruction if Vx equals nn.
      if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) pc += 2;
      break;

    case 0x4000: // 4xnn: Skips the next instruction if Vx doesn't equal nn.
      if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) pc += 2;
      break;

    case 0x5000: // 5xy0: Skips the next instruction if Vx equals Vy.
      if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4]) pc += 2;
      break;

    case 0x6000: // 6xnn: Sets Vx to nn.
      V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
      break;

    case 0x7000: // 7xnn: Adds nn to Vx.
      V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
      break;

    case 0x8000:
      switch (opcode & 0x000F) {
        default:
          printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
      }
      break;

    case 0x9000: // 9xy0: Skips the next instruction if Vx doesn't equal Vy.
      if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4]) pc += 2;
      break;

    case 0xA000: // Annn: Sets I to the address nnn.
      I = opcode & 0x0FFF;
      break;

    case 0xB000: // Bnnn: Jumps to the address nnn plus V0.
      pc = (opcode & 0x0FFF) + V[0];
      break;

    case 0xC000: // Cxnn: Sets Vx to a random number AND nn.
      V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
      break;

    case 0xD000: // Dxyn: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
      break;

    case 0xE000:
      switch (opcode & 0x00FF) {
        case 0x009E: // Ex9E: Skips the next instruction if the key stored in Vx is pressed.
          if (key[V[(opcode & 0x0F00) >> 8]]) pc += 2;
          break;

        case 0x00A1: // ExA1: Skips the next instruction if the key stored in Vx isn't pressed.
          if (!key[V[(opcode & 0x0F00) >> 8]]) pc += 2;
          break;

        default:
          printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
      }
      break;

    case 0xF000:
      switch (opcode & 0x00FF) {
        case 0x0007: // Fx07: Sets Vx to the value of the delay timer.
          V[(opcode & 0x0F00) >> 8] = delayTimer;
          break;

        case 0x000A: // Fx0A: A key press is awaited, and then store in Vx.
          if (std::none_of(key.begin(), key.end(), [](bool b) { return b; })) pc -= 2;
          else V[(opcode & 0x0F00) >> 8] = std::find(key.begin(), key.end(), true) - key.begin();
          break;

        case 0x0015: // Fx15: Sets the delay timer to Vx.
          delayTimer = V[(opcode & 0x0F00) >> 8];
          break;

        case 0x0018: // Fx18: Sets the sound timer to Vx.
          soundTimer = V[(opcode & 0x0F00) >> 8];
          break;

        case 0x001E: // Fx1E: Adds Vx to I.
          I += V[(opcode & 0x0F00) >> 8];
          break;

        case 0x0029: // Fx29: Sets I to the location of the sprite for the character in VX.
          break;

        case 0x0033: // Fx33: Store BCD representation of Vx in memory locations I, I+1, and I+2.
          break;

        case 0x0055: // Fx55: Stores registers V0 through Vx in memory starting at location I.
          std::copy(V.begin(), V.begin() + ((opcode & 0x0F00) >> 8), memory.begin() + I);
          break;

        case 0x0065: // Fx65: Reads registers V0 through Vx from memory starting at location I.
          std::copy(memory.begin() + I, memory.begin() + I + ((opcode & 0x0F00) >> 8), V.begin());
          break;

        default:
          printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
      }

    default:
      printf("Unknown opcode: 0x%X\n", opcode);
  }

  // update program counter
  pc += 2;

  return _draw;
}

bool Chip8::draw() {
  return _draw;
}

bool Chip8::sound() {
  return soundTimer != 0;
}
