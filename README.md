# Adventure Quest: Spiral Path 🛡️💎

**Description:** A 2D graphical adventure game built in C++ using the **SFML** library. Players navigate a dynamic grid, manage inventories, and use strategy to survive hazards while racing to the center of a spiral board.

---

## 🖼️ Screenshots

### The Game Board
![Game Board](board.png)
*Active 7x7 grid showing the player positions and various game items.*

### Victory Screen
![Victory Screen](victory.png)
*The result screen triggered when a player successfully reaches the goal.*

---

## 🎮 Gameplay Features
* **Dual Movement Systems**: Player 1 uses standard grid navigation while Player 2 follows a complex spiral path algorithm to reach the center.
* **Dynamic Level Scaling**: The board size and item density increase as you progress through levels.
* **Strategic Shop**: Use collected Gold and Silver coins to purchase Shields and Swords to negate hazards.
* **Interactive Hazards**: Encounter Snakes, Fire, and Lions that force players to move backward or skip turns.

---

## 🛠️ Technical Stack
* **Language**: C++17
* **Graphics Library**: SFML (Simple and Fast Multimedia Library)
* **OOP Concepts**: Extensive use of Inheritance and Polymorphism for `GameItem`, `Hurdle`, and `HelperObject`.
* **Memory Management**: Utilizes dynamic arrays and pointers for the `GameBoard` structure.

---

## 🚀 Installation & Setup

### Prerequisites
1. **MinGW-w64** (GCC 13.1.0 SEH)
2. **SFML 2.6.1**

### Compiling with VS Code
Ensure your `tasks.json` includes the following paths to link the SFML headers and libraries:

```json
"-I", "C:/SFML-2.6.1/include",
"-L", "C:/SFML-2.6.1/lib",
"-lsfml-graphics",
"-lsfml-window",
"-lsfml-system"
```
### Running the App
1. Compile the code using Ctrl+Shift+B.
2. Place the .dll files from SFML-2.6.1/bin into the same directory as your .exe.
3. Execute AdventureQuest.exe.

### 👨‍💻 Author
***Abeer*** CS Student at FAST University
