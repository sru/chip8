#include <cstdlib>
#include <ctime>

#include <array>
#include <fstream>
#include <iostream>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "chip8.hpp"

Chip8 chip8;

/*
 * 1 2 3 C
 * 4 5 6 D
 * 7 8 9 E
 * A 0 B F
 *    |
 *    V
 * 1 2 3 4
 * q w e r
 * a s d f
 * z x c v
 */
void handleKeys() {
  chip8.key[0x0] = sf::Keyboard::isKeyPressed(sf::Keyboard::X);
  chip8.key[0x1] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num1);
  chip8.key[0x2] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num2);
  chip8.key[0x3] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num3);
  chip8.key[0x4] = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
  chip8.key[0x5] = sf::Keyboard::isKeyPressed(sf::Keyboard::W);
  chip8.key[0x6] = sf::Keyboard::isKeyPressed(sf::Keyboard::E);
  chip8.key[0x7] = sf::Keyboard::isKeyPressed(sf::Keyboard::A);
  chip8.key[0x8] = sf::Keyboard::isKeyPressed(sf::Keyboard::S);
  chip8.key[0x9] = sf::Keyboard::isKeyPressed(sf::Keyboard::D);
  chip8.key[0xA] = sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
  chip8.key[0xB] = sf::Keyboard::isKeyPressed(sf::Keyboard::C);
  chip8.key[0xC] = sf::Keyboard::isKeyPressed(sf::Keyboard::Num4);
  chip8.key[0xD] = sf::Keyboard::isKeyPressed(sf::Keyboard::R);
  chip8.key[0xE] = sf::Keyboard::isKeyPressed(sf::Keyboard::F);
  chip8.key[0xF] = sf::Keyboard::isKeyPressed(sf::Keyboard::V);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " rom" << std::endl;
    return 0;
  }

  std::ifstream file(argv[1], std::ios::binary);
  if (!file) {
    std::cout << "Cannot open file: " << argv[1] << std::endl;
    return 0;
  }

  file.seekg(0, std::ios::end);
  std::streampos length(file.tellg());
  if (!length || length > 3584) {
    std::cout << "File has invalid size." << std::endl;
    return 0;
  }

  std::array<unsigned char, 3584> buf;
  file.seekg(0, std::ios::beg);
  file.read((char*)&buf[0], length);

  chip8.load(buf);

  sf::RenderWindow window(sf::VideoMode(640, 320), "Chip8 Emulator", sf::Style::Titlebar | sf::Style::Close);
  sf::Clock dt;

  sf::RectangleShape pix(sf::Vector2f(10, 10));
  pix.setFillColor(sf::Color::White);

  while (window.isOpen()) {

    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed) window.close();
    }

    handleKeys();

    bool draw = false;

    if (dt.restart().asSeconds())
//      chip8.debug(std::cout);
      draw = chip8.update();

    if (draw) {
      window.clear(sf::Color::Black);

      for (int row = 0; row < 32; ++row) {
        for (int col = 0; col < 64; ++col) {
          if (chip8.gfx(col, row) != 0) {
            pix.setPosition(col * 10, row * 10);
            window.draw(pix);
          }
        }
      }

      window.display();
    }
  }

  return 0;
}
