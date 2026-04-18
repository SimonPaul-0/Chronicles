# Chronicles of the Abyss

A Pure C++17 Terminal RPG

## Description

Chronicles of the Abyss is a text-based role-playing game implemented entirely in C++17. It serves as both an entertaining game and a comprehensive demonstration of modern C++ programming concepts and techniques.

## Features

- **Turn-based Combat System**: Engage in battles with various enemies using different strategies
- **Character Progression**: Level up your character, gain experience, and improve stats
- **Inventory Management**: Collect and manage items, weapons, and armor
- **Save/Load System**: Save your progress and resume later
- **Multiple Enemy Types**: Face different monsters with unique abilities
- **Colorful Terminal Interface**: Enhanced visual experience with colored text output

## C++ Concepts Demonstrated

This project showcases the following C++17 features and design patterns:

- Abstract Base Classes & Pure Virtual Functions
- Inheritance Hierarchies & Runtime Polymorphism
- Smart Pointers (unique_ptr, shared_ptr, dynamic_pointer_cast)
- STL Containers (vector, map, unordered_map, tuple, pair)
- Function Templates & Template Specialization
- Lambda Functions & std::function (Observer Pattern)
- File I/O with Custom Serialization (Save/Load System)
- Custom Exception Hierarchy
- Operator Overloading (Stats +, +=, ==)
- Move Semantics & Perfect Forwarding
- STL Algorithms (any_of, find_if, transform, min, max, sort)
- Scoped Enum Classes
- Factory, Observer & Strategy Design Patterns
- Mersenne Twister RNG (mt19937)

## Requirements

- C++17 compatible compiler (g++, clang++, MSVC)
- Windows or Unix-like operating system
- Terminal that supports ANSI escape codes (most modern terminals)

## Building

Compile the single source file with C++17 standard:

```bash
g++ chronicles.cpp -o chronicles -std=c++17
```

Or with clang:

```bash
clang++ chronicles.cpp -o chronicles -std=c++17
```

## Running

Execute the compiled binary:

```bash
./chronicles
```

On Windows:

```cmd
chronicles.exe
```

## Gameplay

- Follow the on-screen prompts to navigate through the game
- Make choices by entering the corresponding numbers
- Save your progress anytime to resume later
- Explore, fight enemies, and level up your character

## Save Files

The game automatically saves progress to `chronicles_save.dat` in the current directory.

## Contributing

This is an educational project demonstrating C++ concepts. Feel free to fork and experiment with the code!

## License

This project is for educational purposes. Use and modify as needed.