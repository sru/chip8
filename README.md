# chip8
Chip8 emulator written in C++.

## Dependancies
- C++11
- SFML (for main.cpp only)

## Compilation
This uses SFML for graphics, window, and keyboard management, so you need to specify as library when compiling. For example:
```
g++ -std=c++11 main.cpp chip8.cpp -lsfml-graphics -lsfml-window -lsfml-system
```

Chip8 emulator itself is entirely written only using C++ and STL.

## Usage
```
chip8 rom\_to\_play
```

## Todo
- More bug fixes
- seg fault on VERS
- sound
- Super Chip8 instructions

## License
See [LICENSE](LICENSE).
