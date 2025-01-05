/*
-------------------------------------------------- The Legend of Bomberman --------------------------------------------------
*/


#include <iostream>
#include <ctime>
#include <thread>
#include <ncurses.h>
#include <fstream>

using namespace std;

#define WIDTH 60
#define HEIGHT 30

// Define symbols for each entity
#define PLAYER 'P'
#define ENEMY 'E'
#define BOMB 'B'
#define DESTRUCTIBLE_BLOCK '#'
#define INDESTRUCTIBLE_BLOCK 'X'
#define EXIT_DOOR 'D'
#define TRAP 'T'

#define NUM_BOMBS 3     // Number of bombs the player can plant at a time

/*
-------------------------------------------------- Entity Class --------------------------------------------------
*/

// The 2-d grid of the game world is represented by a 2D array of Entity pointers.
// Each block in the grid can contain either an Entity pointer or nullptr.

// Base Class for all entities in the game
class Entity {
protected:
    int x, y;       // Position Coordinates
    char symbol;    // Symbol to represent the entity on the grid

public:
    // Constructor
    Entity(int x, int y, char symbol) : x(x), y(y), symbol(symbol) {
        // Initialize the entity with the given position and symbol
    }
    // Destructor
    virtual ~Entity() {}

    // Getters
    int getX() const { return x; }
    int getY() const { return y; }
    char getSymbol() const { return symbol; }

    // Move the entity by dx and dy; will be same for all entities
    virtual void move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    // Update the entity; will be overridden by Player and Enemy classes
    virtual void update() {}
};

/*
-------------------------------------------------- Player Class --------------------------------------------------
*/

class Player : public Entity {
private:
    int hasBombs;           // Number of bombs the player has

public:
    // Constructor
    Player(int x, int y) : Entity(x, y, PLAYER), hasBombs(NUM_BOMBS) {}

    // Check if the player can plant a bomb
    bool canPlantBomb() const {
        return hasBombs > 0;
    }
    // Use a bomb
    void useBomb() {
        hasBombs--;
    }
    // Reload a bomb, after a bomb has exploded; restore that bomb
    void reloadBomb() {
        hasBombs++;
    }
};

/*
-------------------------------------------------- Enemy Class --------------------------------------------------
*/

class Enemy : public Entity {
private:
    int moveType;       // Type of movement for the enemy (0: Horizontal, 1: Vertical, 2: Both)
    int moveStep;       // Counter to control the movement of the enemy

public:
    // Constructor
    Enemy(int x, int y, int type) : Entity(x, y, ENEMY), moveType(type), moveStep(0) {}

    // Update the enemy's position based on the moveType
    void update() override {
        // Move the enemy every 10th update
        if(moveStep++ != 10) {
            return;
        }
        // Reset the moveStep counter
        moveStep = 0;
        int dx = 0, dy = 0;
        int randomMove = rand() % 4; // 0: up, 1: right, 2: down, 3: left

        switch (moveType) {
            case 0: // Horizontal Movement
                // (randomMove == 0 || randomMove == 1) -> Move right
                // (randomMove == 2 || randomMove == 3) -> Move left
                dx = (randomMove == 0 || randomMove == 1) ? 1 : ((randomMove == 2 || randomMove == 3) ? -1 : 0);
                break;
            case 1: // Vertical Movement
                // (randomMove == 0 || randomMove == 1) -> Move up
                // (randomMove == 2 || randomMove == 3) -> Move down
                dy = (randomMove == 0 || randomMove == 1) ? -1 : ((randomMove == 2 || randomMove == 3) ? 1 : 0);
                break;
            case 2: // Both
                dx = (randomMove == 0 || randomMove == 1) ? 1 : ((randomMove == 2 || randomMove == 3) ? -1 : 0);
                dy = (randomMove == 0 || randomMove == 1) ? -1 : ((randomMove == 2 || randomMove == 3) ? 1 : 0);
                break;
        }
        // Move the enemy
        move(dx, dy);
    }

    // Getter for moveType
    int getMoveType() const { return moveType; }
};

/*
-------------------------------------------------- Bomb Class --------------------------------------------------
*/

class Bomb : public Entity {
private:
    // Time when the bomb was planted
    chrono::steady_clock::time_point plantTime;

public:
    // Constructor
    Bomb(int x, int y) : Entity(x, y, BOMB), plantTime(chrono::steady_clock::now()) {}

    // Check if the bomb should explode
    bool shouldExplode() const {
        auto now = chrono::steady_clock::now();
        return chrono::duration_cast<chrono::seconds>(now - plantTime).count() >= 3;    // Explode after 3 seconds
    }
};

/*
-------------------------------------------------- Block Class --------------------------------------------------
*/

class Block : public Entity {
protected:
    bool destructible;  // Whether the block is destructible or not
public:
    // Constructor
    Block(int x, int y, char symbol, bool destructible) 
        : Entity(x, y, symbol), destructible(destructible) {
            // Initialize the block with the given position, symbol, and destructibility
        }

    // Getter for destructible
    bool isDestructible() const { return destructible; }
};

/*
-------------------------------------------------- Destructible Block Class --------------------------------------------------
*/

class DestructibleBlock : public Block {
private:
    bool isGreen;   // Meaning it will contain the exit door when destroyed

public:
    // Constructor
    DestructibleBlock(int x, int y, bool green = false) : Block(x, y, DESTRUCTIBLE_BLOCK, true) {
        isGreen = green;
    }
    // Getter for isGreen
    bool isGreenBlock() const { return isGreen; }
};

/*
-------------------------------------------------- Indestructible Block Class --------------------------------------------------
*/

class IndestructibleBlock : public Block {
public:
    // Constructor
    IndestructibleBlock(int x, int y) : Block(x, y, INDESTRUCTIBLE_BLOCK, false) {}
};

/*
-------------------------------------------------- Exit Door Class --------------------------------------------------
*/

class ExitDoor : public Entity {
private:
    bool visible;       // Whether the exit door is visible or not

public:
    ExitDoor(int x, int y) : Entity(x, y, EXIT_DOOR), visible(false) {
        // Initialize the exit door with the given position
        // By default, the exit door is not visible
        // When the destructible block containing the exit door is destroyed, the exit door becomes visible
    }

    // Setter and Getter for visible
    void setVisible(bool visible) { this->visible = visible; }
    bool isVisible() const { return visible; }
};

/*
-------------------------------------------------- Trap Class --------------------------------------------------
*/

class Trap : public Entity {
public:
    Trap(int x, int y) : Entity(x, y, TRAP) {}
};

/*
-------------------------------------------------- Game Class --------------------------------------------------
*/

class Game {
private:
    string saveFileName = "game_save.txt";
    
    Entity*** grid;     // 2D array of Entity pointers
    Player* player;     // Pointer to the player object
    Enemy** enemies;    // Array of pointers to enemy objects
    int enemyCount;     // Number of enemies
    Bomb** bombs;       // Array of pointers to bomb objects
    int bombCount;      // Number of bombs
    ExitDoor* exitDoor; // Pointer to the exit door object
    int bombsPlanted;   // Number of bombs planted by the player

    // Function to clear the screen and display the menu
    void displayMenu() {
        clear();
        mvprintw(HEIGHT / 2 - 2, WIDTH / 2 - 10, "1. Start a new game");
        mvprintw(HEIGHT / 2 - 1, WIDTH / 2 - 10, "2. Load previous game");
        mvprintw(HEIGHT / 2, WIDTH / 2 - 10, "3. Exit");
        refresh();
    }

    // Function to save the game state to a file
    void saveGame() {
        ofstream saveFile(saveFileName);
        if (saveFile.is_open()) {
            // Save player position
            saveFile << player->getX() << " " << player->getY() << "\n";

            // Save bombs planted
            saveFile << bombsPlanted << "\n";

            // Save enemy positions
            saveFile << enemyCount << "\n";
            for (int i = 0; i < enemyCount; i++) {
                saveFile << enemies[i]->getX() << " " << enemies[i]->getY() << " " << enemies[i]->getMoveType() << "\n";
            }

            // Save bomb positions
            saveFile << bombCount << "\n";
            for (int i = 0; i < bombCount; i++) {
                saveFile << bombs[i]->getX() << " " << bombs[i]->getY() << "\n";
            }

            // Save exit door position
            saveFile << exitDoor->getX() << " " << exitDoor->getY() << " " << exitDoor->isVisible() << "\n";

            // Save grid state
            for (int i = 0; i < HEIGHT; i++) {
                for (int j = 0; j < WIDTH; j++) {
                    if (grid[i][j]) {
                        saveFile << grid[i][j]->getSymbol();
                    } else {
                        saveFile << ' ';
                    }
                }
                saveFile << "\n";
            }

            saveFile.close();
            mvprintw(HEIGHT + 1, 0, "Game saved successfully!");
            refresh();
        } else {
            mvprintw(HEIGHT + 1, 0, "Unable to save game!");
            refresh();
        }
    }

    // Function to load the game state from a file
    bool loadGame() {
        ifstream loadFile(saveFileName);
        if (loadFile.is_open()) {
            // Clear existing game state
            for (int i = 0; i < HEIGHT; i++) {
                for (int j = 0; j < WIDTH; j++) {
                    delete grid[i][j];
                    grid[i][j] = nullptr;
                }
            }
            for (int i = 0; i < enemyCount; i++) {
                delete enemies[i];
            }
            delete[] enemies;
            for (int i = 0; i < bombCount; i++) {
                delete bombs[i];
            }
            delete[] bombs;

            // Load player position
            int playerX, playerY;
            loadFile >> playerX >> playerY;
            delete player;
            player = new Player(playerX, playerY);

            // Load bombs planted
            loadFile >> bombsPlanted;

            // Load enemy positions
            loadFile >> enemyCount;
            enemies = new Enemy*[enemyCount];
            for (int i = 0; i < enemyCount; i++) {
                int x, y, moveType;
                loadFile >> x >> y >> moveType;
                enemies[i] = new Enemy(x, y, moveType);
            }

            // Load bomb positions
            loadFile >> bombCount;
            bombs = new Bomb*[bombCount];
            for (int i = 0; i < bombCount; i++) {
                int x, y;
                loadFile >> x >> y;
                bombs[i] = new Bomb(x, y);
            }

            // Load exit door position
            int exitX, exitY;
            bool visible;
            loadFile >> exitX >> exitY >> visible;
            delete exitDoor;
            exitDoor = new ExitDoor(exitX, exitY);
            exitDoor->setVisible(visible);

            // Load grid state
            loadFile.ignore(); // Ignore newline
            for (int i = 0; i < HEIGHT; i++) {
                string line;
                getline(loadFile, line);
                for (int j = 0; j < WIDTH; j++) {
                    char symbol = line[j];
                    switch (symbol) {
                        case INDESTRUCTIBLE_BLOCK:
                            grid[i][j] = new IndestructibleBlock(j, i);
                            break;
                        case DESTRUCTIBLE_BLOCK:
                            grid[i][j] = new DestructibleBlock(j, i);
                            break;
                        case TRAP:
                            grid[i][j] = new Trap(j, i);
                            break;
                        case EXIT_DOOR:
                            grid[i][j] = exitDoor; // Place the exit door in the grid
                        break;
                    }
                }
            }

            // Make the green brick on the exit door, if it is not visible
            if(!exitDoor->isVisible())
                grid[exitY][exitX] = new DestructibleBlock(exitX, exitY, true);

            loadFile.close();
            return true;
        }
        return false;
    }

public:
    // Constructor
    Game() : player(nullptr), exitDoor(nullptr), enemyCount(0), bombCount(0) {
        // Initialize the grid with nullptr
        grid = new Entity**[HEIGHT];
        for (int i = 0; i < HEIGHT; i++) {
            grid[i] = new Entity*[WIDTH];
            for (int j = 0; j < WIDTH; j++) {
                grid[i][j] = nullptr;
            }
        }
        initializeGame();
    }

    // Destructor
    ~Game() {
        // Delete all entities and deallocate memory
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                delete grid[i][j];
            }
            delete[] grid[i];
        }
        delete[] grid;
        delete player;
        delete exitDoor;
        for (int i = 0; i < enemyCount; i++) {
            delete enemies[i];
        }
        delete[] enemies;
        for (int i = 0; i < bombCount; i++) {
            delete bombs[i];
        }
        delete[] bombs;
    }

    // Function to display the game over screen
    void gameOver(string causeOfDeath) {
        clear();
        string toDisplay = "GAME OVER! " + causeOfDeath;
        mvprintw(HEIGHT / 2, WIDTH / 2 - 5, toDisplay.c_str());
        refresh();
        nodelay(stdscr, FALSE);
        getch();
        endwin();
        exit(0);
    }

    // Function to display the game win screen
    void gameWin() {
        clear();
        mvprintw(HEIGHT / 2, WIDTH / 2 - 5, "YOU WIN!");
        refresh();
        nodelay(stdscr, FALSE);
        getch();
        endwin();
        exit(0);
    }

    // Function to initialize the game
    void initializeGame() {
        player = new Player(1, 1);
        bombsPlanted = 0;

        // Adding blocks
        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                // Adding indestructible blocks around the border
                if (i == 0 || i == HEIGHT - 1 || j == 0 || j == WIDTH - 1) {
                    grid[i][j] = new IndestructibleBlock(j, i);
                }
                // Adding destructible blocks randomly
                else if (rand() % WIDTH == 0) {
                    grid[i][j] = new IndestructibleBlock(j, i);
                }
                // Adding destructible blocks randomly
                else if (rand() % HEIGHT == 0) {
                    grid[i][j] = new DestructibleBlock(j, i);
                }
            }
        }

        // Adding traps
        for (int i = 0; i < (HEIGHT + WIDTH) / 10; i++) {
            int x, y;
            // Randomly select a position for the trap
            do {
                x = rand() % (WIDTH - 2) + 1;
                y = rand() % (HEIGHT - 2) + 1;
            } while (grid[y][x] != nullptr || (x == 1 && y == 1));
            grid[y][x] = new Trap(x, y);
        }

        // Add enemies
        enemyCount = (HEIGHT + WIDTH) / 10;
        enemies = new Enemy*[enemyCount];
        for (int i = 0; i < enemyCount; i++) {
            int x, y;
            do {
                x = rand() % (WIDTH - 2) + 1;
                y = rand() % (HEIGHT - 2) + 1;
            } while (grid[y][x] != nullptr || (x == 1 && y == 1));
            enemies[i] = new Enemy(x, y, i % 3);
        }

        // Clear player's starting area; player starts at (1, 1)
        for (int i = 1; i <= 3; i++) {
            for (int j = 1; j <= 3; j++) {
                delete grid[i][j];
                grid[i][j] = nullptr;
            }
        }

        // Adding exit door
        int exitX, exitY;
        do {
            exitX = rand() % (WIDTH - 2) + 1;
            exitY = rand() % (HEIGHT - 2) + 1;
        } while (grid[exitY][exitX] != nullptr || (exitX == 1 && exitY == 1));

        // Adding exit door
        exitDoor = new ExitDoor(exitX, exitY);
        grid[exitY][exitX] = new DestructibleBlock(exitX, exitY, true);


        // Initialize bombs array
        bombCount = 0;
        bombs = new Bomb*[NUM_BOMBS]; // Arbitrary initial size
    }

    // Function to clear the screen
    void display() {
        clear();
        start_color();
        init_pair(1, COLOR_GREEN, COLOR_BLACK);

        for (int i = 0; i < HEIGHT; i++) {
            for (int j = 0; j < WIDTH; j++) {
                if (grid[i][j]) {
                    // Check if the block is a destructible block
                    if (auto* block = dynamic_cast<DestructibleBlock*>(grid[i][j])) {
                        // Check if the destructible block is green
                        // If it is green, display it in green color
                        if (block->isGreenBlock()) {
                            attron(COLOR_PAIR(1));
                            mvaddch(i, j, block->getSymbol());
                            attroff(COLOR_PAIR(1));
                        } else {
                            mvaddch(i, j, block->getSymbol());
                        }
                    } else {
                        mvaddch(i, j, grid[i][j]->getSymbol());
                    }
                } else {
                    mvaddch(i, j, ' ');
                }
            }
        }

        // Displaying entities
        mvaddch(player->getY(), player->getX(), player->getSymbol());

        for (int i = 0; i < enemyCount; i++) {
            mvaddch(enemies[i]->getY(), enemies[i]->getX(), enemies[i]->getSymbol());
        }

        for (int i = 0; i < bombCount; i++) {
            mvaddch(bombs[i]->getY(), bombs[i]->getX(), bombs[i]->getSymbol());
        }

        if (exitDoor->isVisible()) {
            mvaddch(exitDoor->getY(), exitDoor->getX(), exitDoor->getSymbol());
        }

        mvprintw(HEIGHT, 0, "Bombs planted: %d", bombsPlanted);
        refresh();
    }

    // Function to check if a move is valid
    // Enemies can step on the traps
    // But if the player steps on it the game is over
    bool isValidMove(int x, int y) {
        return x > 0 && x < WIDTH - 1 && y > 0 && y < HEIGHT - 1 && grid[y][x] == nullptr || grid[y][x]->getSymbol() == TRAP;
    }

    // Function to move the player, given the change in x and y; in the game grid
    void movePlayer(int dx, int dy) {
        int newX = player->getX() + dx;
        int newY = player->getY() + dy;

        if (isValidMove(newX, newY)) {
            player->move(dx, dy);
        }
    }

    // Function to plant a bomb
    void plantBomb() {
        if (player->canPlantBomb()) {
            if (bombCount >= NUM_BOMBS) {
                return;
            }
            bombs[bombCount++] = new Bomb(player->getX(), player->getY());
            player->useBomb();
            bombsPlanted++;
        }
    }

    // Function to explode a bomb
    void explodeBomb(Bomb* bomb) {
        int bx = bomb->getX(), by = bomb->getY();
        // Explode in all 4 directions
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 || dy == 0) { // Only explode in straight lines
                    for (int i = 1; i <= 3; i++) { // Explode up to 3 blocks away
                        int x = bx + dx * i, y = by + dy * i;
                        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) break;
                        
                        // Check for block destruction
                        if (grid[y][x] && grid[y][x]->getSymbol() == DESTRUCTIBLE_BLOCK) {
                            delete grid[y][x];
                            grid[y][x] = nullptr;
                            if (x == exitDoor->getX() && y == exitDoor->getY()) {
                                exitDoor->setVisible(true);
                            }
                            break;
                        } else if (grid[y][x] && grid[y][x]->getSymbol() == INDESTRUCTIBLE_BLOCK) {
                            break;
                        }

                        // Check for enemy elimination
                        for (int j = 0; j < enemyCount; j++) {
                            if (enemies[j]->getX() == x && enemies[j]->getY() == y) {
                                delete enemies[j];
                                enemies[j] = enemies[--enemyCount];
                                break;
                            }
                        }

                        // Check for player elimination
                        if (player->getX() == x && player->getY() == y) {
                            // Handling player death
                            gameOver("Player was blown up by a bomb!");
                        }
                    }
                }
            }
        }
        // Reload the bomb
        player->reloadBomb();
    }

    // Function to update the game state
    void update() {
        // Player and enemy collision
        for (int i = 0; i < enemyCount; i++) {
            if (player->getX() == enemies[i]->getX() && player->getY() == enemies[i]->getY()) {
                gameOver("Player was caught by an enemy!");
                return;
            }
        }

        // Player and trap collision
        if (grid[player->getY()][player->getX()] && grid[player->getY()][player->getX()]->getSymbol() == TRAP) {
            gameOver("Player stepped on a trap!");
            return;
        }

        // Enemy and trap collision
        for (int i = 0; i < enemyCount; i++) {
            int oldX = enemies[i]->getX(), oldY = enemies[i]->getY();
            enemies[i]->update();
            // Check if the new position is valid
            if (!isValidMove(enemies[i]->getX(), enemies[i]->getY())) {
                enemies[i]->move(oldX - enemies[i]->getX(), oldY - enemies[i]->getY()); // Move back if invalid
            }
        }

        // Bomb explosion
        int i = 0;
        while (i < bombCount) {
            // Check if the bomb should explode
            if (bombs[i]->shouldExplode()) {
                explodeBomb(bombs[i]);
                delete bombs[i];
                bombs[i] = bombs[--bombCount];
            } else {
                i++;
            }
        }

        // Check for level completion
        if (player->getX() == exitDoor->getX() && player->getY() == exitDoor->getY() && exitDoor->isVisible()) {
            // Handle level completion
            gameWin();
            return;
        }
    }

    // Function to run the game
    void run() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, TRUE);
        curs_set(0);

        while (true) {
            displayMenu();
            int choice = getch() - '0';

            // Start a new game
            switch (choice) {
                case 1:
                    playGame();
                    break;
                case 2:
                    if (loadGame()) {
                        playGame();
                    } else {
                        mvprintw(HEIGHT / 2 + 2, WIDTH / 2 - 15, "No saved game found. Press any key to continue.");
                        refresh();
                        getch();
                    }
                    break;
                case 3:
                    endwin();
                    exit(0);
                default:
                    break;
            }
        }
    }


    // Function to play the game
    void playGame() {
        nodelay(stdscr, TRUE);

        while (true) {
            display();
            int ch = getch();

            switch (ch) {
                case 'w': case KEY_UP: movePlayer(0, -1); break;
                case 's': case KEY_DOWN: movePlayer(0, 1); break;
                case 'a': case KEY_LEFT: movePlayer(-1, 0); break;
                case 'd': case KEY_RIGHT: movePlayer(1, 0); break;
                case ' ': plantBomb(); break;
                case 'e': saveGame(); break;
                case 'q': case 'Q':{ 
                    endwin();
                    exit(0);
                };
            }

            update();
            this_thread::sleep_for(chrono::milliseconds(50));
        }
    }
};

// Library to install (ncurses), for continuous keyboard input
// sudo apt-get install libncurses5-dev libncursesw5-dev

// Compile and run
// g++ -o bomberman bomberman.cpp -lncurses
// ./bomberman

int main() {
    srand(time(nullptr));
    Game game;
    game.run();
    return 0;
}

