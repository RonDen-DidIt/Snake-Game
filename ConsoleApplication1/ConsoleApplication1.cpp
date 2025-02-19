#include <algorithm>
#include <conio.h>
#include <chrono>
#include <iostream>
#include <thread>
#include <time.h>
#include <vector>
#include <windows.h>

#define BACKGROUND '.'
#define BACKGROUND2 ' '

class map_construct {
private:
    char** map;
    unsigned int map_rows = 0, map_columns = 0;
public:
    map_construct(const unsigned int rows = 10, unsigned int columns = 20) : map_rows(rows), map_columns(columns) {
        map = new char* [rows];
        int l = 0;
        for (int i = 0; i < rows; i++) {
            map[i] = new char[columns];
            for (int j = 0; j < columns; j++) {
                map[i][j] = getBackground(j); // Assinging a background character to all empty spaces on the map
            }
        }
    }
    std::pair<unsigned int, unsigned int> getSize() {
        return { map_columns, map_rows };
    }
    char get(std::pair<unsigned int, unsigned int> pos) {
        return map[pos.second][pos.first];
    }
    char getBackground(unsigned int positionX) {
        return (positionX % 2 == 0) ? BACKGROUND : BACKGROUND2; // Different background for every second character
    }
    void set(char character, std::pair<unsigned int, unsigned int> pos) {
        map[pos.second][pos.first] = character;
    }
    void draw() {  // printing the map to the console
        for (int j = 0; j < map_columns + 2; j++) {
            std::cout << "_"; // Roof
        }
        std::cout << "\n";
        for (int i = 0; i < map_rows; i++) {
            std::cout << "|"; // Left side
            for (int j = 0; j < map_columns; j++) {
                if (map[i][j] > 'A' && map[i][j] < 'Z') {
                    std::cout << "\033[38;5;84m";
                }
                else if (map[i][j] > 'a' && map[i][j] < 'z') {
                    std::cout << "\033[38;5;196m";
                }
                std::cout << map[i][j]; // Background characters/snake/food
                std::cout << "\033[38;5;255m";
            }
            std::cout << "|\n"; // Right side
        }
        for (int j = 0; j < map_columns + 2; j++) {
            std::cout << "-"; // Bottom
        }
    }
    void destroy() {
        for (int i = 0; i < map_rows; i++) {
            delete[] map[i];
        }
        delete[] map;
    }
};


class foods {
private:

    map_construct* currentMap;
    std::pair<unsigned int, unsigned int> pos = { 0,0 };

    char character;

public:
    foods(map_construct* map) : currentMap(map) {
        makeNew();
    }
    void makeNew() {
        std::pair<unsigned int, unsigned int> map_size = currentMap->getSize();
        do {
            pos = { rand() % map_size.first, rand() % map_size.second };            // Finding a random position for the food 
        } while (currentMap->get(pos) != currentMap->getBackground(pos.first));     // Making sure the snake isn't on that positon
        character = 97 + rand() % 26; // Random lowercase letter
        currentMap->set(character, pos);
    }
    std::pair<unsigned int, unsigned int> getPosition() {
        return pos;
    }
    char getCharacter() {
        return character;
    }
};

class snakes {
private:
    int size = 1;
    map_construct* currentMap;
    foods* currentFood;

    std::vector<char> chars;
    std::vector<std::pair<unsigned int, unsigned int>> positions;
    std::pair<int, int> direction = { 1,0 };
    std::pair<int, int> currentlyFacing = { 1, 0 };

    bool extending = false;
    bool alive = true;
    char extention;

public:
    snakes(map_construct* map, foods* food, char ch = 'O') : currentMap(map), currentFood(food) {
        chars.push_back(ch);
        positions.push_back({ 0,0 });
    }
    void changeDirection(std::pair<int, int> dir) {
        if ((dir.first != 0 && dir.first + currentlyFacing.first == 0) || (dir.second != 0 && dir.second + currentlyFacing.second == 0)) { // Checking that the new directioin isn't directly opposite to the snakes current direction, which isn't allowed.
            return;
        }
        direction = dir;
    }
    void extendSnake() {
        chars.push_back(extention - 32);
        positions.push_back(positions.back());
        extending = false;
    }
    void updatePos() {
        std::pair<unsigned int, unsigned int> newPos = { positions[0].first + direction.first, positions[0].second + direction.second }; // Snakes new position next refresh
        if ((newPos.first < 0 || newPos.first > currentMap->getSize().first - 1) || (newPos.second < 0 || newPos.second > currentMap->getSize().second - 1)) {
            alive = false; // Hit edge of map
            return;
        }

        if (extending) {
            extendSnake(); // Will extend the snake if it ate food on previous refresh
        }

        currentMap->set(currentMap->getBackground(positions.back().first), positions.back()); // Making sure the snake doesn't leave a trail of letters
        for (int i = chars.size() - 1; i > 0; i--) {
            positions[i] = positions[i - 1]; // Moving snake characters up to the next position
            currentMap->set(chars[i], positions[i]);
        }


        if (newPos == currentFood->getPosition()) { // Hit Food
            extending = true;
            size++;
            extention = currentFood->getCharacter();
            currentFood->makeNew();
        }
        else if (currentMap->get(newPos) != currentMap->getBackground(newPos.first)) {
            alive = false; // Hit self
        }

        positions[0].first = newPos.first;
        positions[0].second = newPos.second;
        currentMap->set(chars[0], positions[0]);

        currentlyFacing = direction;
    }
    bool isAlive() {
        return alive;
    }
    int getSize() {
        return size;
    }
};

class key {
public:
    bool enabled = true;
    short keyCode;
    std::pair<int, int> keyDirection;

    key(std::pair<int, int> cords, short code) : keyDirection(cords), keyCode(code) {}
};

void detectInput(snakes* snake) {
    key right({ 1, 0 }, VK_RIGHT);
    key left({ -1, 0 }, VK_LEFT);
    key up({ 0, -1 }, VK_UP);
    key down({ 0, 1 }, VK_DOWN);
    key directions[4] = { right, left, up, down };

    while (snake->isAlive()) {
        for (int i = 0; i < 4; i++) {
            if (GetAsyncKeyState(directions[i].keyCode)) { // Checking if key is held
                if (directions[i].enabled) { // Making sure it doesn't change the directioin twice for the same keypress
                    snake->changeDirection(directions[i].keyDirection); // Change snake direction
                    directions[i].enabled = false;
                }
            }
            else {
                directions[i].enabled = true;
            }
        }
    }
}

int getDimension(std::string dimension, int rec) {
    int tmp = 0;
    do {
        std::cout << "Enter arena " << dimension << " (in characters) Min: " << rec << ".\n";
        std::cin >> tmp;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    } while (tmp < rec);
    return tmp;
}

int main()
{
    srand(time(NULL));

    map_construct map(getDimension("height", 5), getDimension("width", 10));
    foods food(&map);
    snakes snake(&map, &food);

    const short refreshTime = 100;

    std::thread threadIn(detectInput, &snake); // Watching for input on a seperate thread so it will detect input even while the main thread sleeps
    while (snake.isAlive()) {
        snake.updatePos();
        map.draw(); // Output to command prompt

        std::cout << "\n" << snake.getSize() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(refreshTime));
        system("cls");
    }
    map.destroy(); // Free memory
    threadIn.join();
    std::cout << "Score: " << snake.getSize() << "\n";
    return 0;
}
