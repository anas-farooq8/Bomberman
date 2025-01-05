# Bomberman: A Hero's Journey

## Table of Contents

- [Introduction](#introduction)
- [Game Description](#game-description)
- [Gameplay Features](#gameplay-features)
- [Installation](#installation)
- [How to Compile and Run](#how-to-compile-and-run)
- [How to Play](#how-to-play)
- [Save/Load Functionality](#saveload-functionality)
- [Object-Oriented Design](#object-oriented-design)
- [Demo](#demo)

## Introduction

This terminal-based Bomberman game is a recreation of the classic arcade game, implemented in C++ with the **ncurses** library. It features dynamic gameplay, including bomb mechanics, strategic movement, and a variety of enemies.

## Game Description

In this game, you take on the role of Bomberman, a fearless hero with the ability to strategically place bombs to defeat enemies and navigate mazes. Your mission is to clear levels, find hidden exits, and bring peace back to Bomberman's pixelated world.

## Gameplay Features

### 1. Player Initialization

- The player is represented by **P**.
- The player starts at the **top-left corner** of the grid.

### 2. Enemy Initialization

- Enemies are represented by **E**.
- Enemies are randomly placed on the grid, avoiding the player’s starting position.

### 3. Bomb Mechanics

- Bombs are represented by **B**.
- Players can plant bombs using a designated key.
- Bombs explode after 3 seconds, affecting a 3-tile radius horizontally and vertically.
- Bomb explosions destroy destructible blocks and defeat enemies but do not affect indestructible blocks.

### 4. Game Loop

- Continuous gameplay until the player exits.
- Player movement uses **W, A, S, D** for up, left, down, and right.
- Bomb planting uses the **B** or **Spacebar** key.

### 5. Grid Display

- A real-time display of the grid, including the player, enemies, bombs, destructible blocks, and indestructible walls.

### 6. Menu and Save/Load Functionality

- Start a new game.
- Save the current game state to a `.txt` file.
- Load a previously saved game state from a `.txt` file.
- Exit the game.

## Installation

To run the game, ensure you have the **ncurses** library installed on your system.

### Install ncurses (Linux):

```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

## How to Compile and Run

1. Compile the game:
   ```bash
   g++ -o bomberman bomberman.cpp -lncurses
   ```
2. Run the game:
   ```bash
   ./bomberman
   ```

## How to Play

### Controls:

- **W**: Move up
- **A**: Move left
- **S**: Move down
- **D**: Move right
- **B / Spacebar**: Plant a bomb

### Objectives:

1. Navigate the maze to locate and destroy all enemies.
2. Find the hidden exit, often behind destructible blocks.
3. Avoid enemy attacks and your own bomb explosions.

## Save/Load Functionality

- **Save Game**: Save your progress to a file for later play.
- **Load Game**: Resume from a previously saved state.

## Object-Oriented Design

The game uses an Object-Oriented Programming approach, featuring:

- **Entity Base Class**: An abstract class containing shared properties and methods for game entities (e.g., Player and Enemy).
- **Player Class**: A derived class representing the player.
- **Enemy Class**: A derived class representing enemies with unique movement behaviors.
- **Composition and Polymorphism**: Employed for efficient management of game objects.

## Demo

![Screenshot from 2025-01-05 17-49-39](https://github.com/user-attachments/assets/ec9600ca-8f61-4d49-a77e-1c9a19e7c1e8)

https://github.com/user-attachments/assets/b3d9ee4e-4156-4c6c-a9e7-f16e7ed08040
