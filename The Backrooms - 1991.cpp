// The Backrooms - 1991.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
using namespace std;

#include <SFML/Graphics.hpp>

// Startup settings & defaults
const string title = "The Backrooms: 1991";
bool showDebugInfo = false, toggleDebugInfo = false;
int maxFrameRate = 144;
enum keys { up, dn, lt, rt, start, select, a, b, x, y, lb, rb};

// State data
int screen, inputTimer, frameTime, selection = 0;
bool keysPressed[12]; // up, dn, lt, rt, start, select, a, b, x, y, lb, rb

// Global SFML & graphics objects
sf::Clock clk;
sf::RenderTexture buffer;
sf::RenderWindow window(sf::VideoMode(512, 448), title);
sf::Sprite bufferObj;

int tilemap[4][64][64];

// Graphics assets
sf::Texture font;
sf::Texture titleScreen;
sf::Texture menu;

// Engine functions
void drawTilemapScreen(sf::Texture, int layer);
void drawText(int x, int y, string text, sf::Color color);
void drawText(int x, int y, string text);
void loadTilemap(string filename, int layer);
void readInput();
int updateFrameTime();

// Game States
void TitleScreen();
void MainMenu();



int main() {
    // Game state

    // Print startup info to terminal
    cout << "Opening " << title << ". Press TAB to toggle debug information.\n";

    // Create window
    if(showDebugInfo) cout << "Creating window...";

    window.setFramerateLimit(maxFrameRate);
    buffer.create(256, 224);
    bufferObj.setScale(2.f, 2.f);

    if(showDebugInfo) cout << "done.";

    // Load graphics
    {
        if(showDebugInfo) cout << "\nLoading graphics...";

        if(!font.loadFromFile("Tiles/Font.png")) throw("Unable to load font tileset.");
        if(!titleScreen.loadFromFile("Tiles/Title Screen.png")) throw("Unable to load title screen tileset.");
        if(!menu.loadFromFile("Tiles/Menu.png")) throw("Unable to load font tileset.");

        if(showDebugInfo) cout << "done.";
    }

    loadTilemap("Tiles/Title Screen.txt", 0);
    loadTilemap("Tiles/Main Menu.txt", 1);

    while(window.isOpen()) {
        // System window management
        sf::Event event;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::End)) window.close(); // Quick exit
        while(window.pollEvent(event)) {
            if(event.type == sf::Event::Closed) window.close();
        }

        // Toggle debug output
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {
            if(!toggleDebugInfo) {
                showDebugInfo = !showDebugInfo;
                cout << "\n   > Show debug info : " << showDebugInfo;
            }
            toggleDebugInfo = true;
        } else toggleDebugInfo = false;

        // Game logc
        readInput();

        switch(screen) {
        case 0: TitleScreen(); break;
        case 1: MainMenu(); break;

        default:
            buffer.clear(sf::Color::Blue);
            drawText(0, 0, "Error: unrecognized game state.", sf::Color::White);
            drawText(0, 16, "Screen: " + to_string(screen), sf::Color::Cyan);
        }

        // Update graphics
        buffer.display();
        bufferObj.setTexture(buffer.getTexture());
        window.draw(bufferObj);
        window.display();
        window.clear(sf::Color::Black);
    }
}



// Engine functions
void drawTilemapScreen(sf::Texture tex, int layer) {
    sf::Sprite tile;
    tile.setTexture(tex);
    int tileID, tileX, tileY;

    for(int x = 0; x < 16; x++) {
        for(int y = 0; y < 14; y++) {
            tileID = tilemap[layer][x][y];
            tile.setPosition(16.f * x, 16.f * y);

            tileX =(tileID % 16); tileX *= 16;
            tileY =(tileID / 16); tileY *= 16;

            tile.setTextureRect(sf::IntRect(tileX, tileY, 16, 16));

            buffer.draw(tile);
        }
    }
}
void drawText(int x, int y, string txt, sf::Color color) {
    sf::Sprite text;
    text.setTexture(font);
    text.setColor(color);

    int c, cx, cy;

    for(int i = 0; i < txt.length(); i++) {
        c = txt[i] - 32;
        cx = c % 16; cx *= 8;
        cy = c / 16; cy *= 16;

        text.setTextureRect(sf::IntRect(cx, cy, 8, 16));
        text.setPosition((float)x,(float)y);
        buffer.draw(text);
        x += 8;
    }
}
void drawText(int x, int y, string txt) {
    drawText(x, y, txt, sf::Color::Black);
}
void loadTilemap(string filename, int layer) {
    string line = " ";
    stringstream linestream;
    int columns, rows;
    string errorText = "Unable to open tilemap:" + filename;

    fstream file(filename, ios::in);

    if(file.is_open()) {
        // get tilemap width
        getline(file, line, ' ');
        if(line != "tileswide") cout << "\nWarning: Unexpected tilemap format.";
        getline(file, line);
        columns = stoi(line);

        // get tilemap height
        getline(file, line, ' ');
        if(line != "tileshigh") cout << "\nWarning: Unexpected tilemap format.";
        getline(file, line);
        rows = stoi(line);

        // Skip over extra lines
        while(getline(file, line) && line != "layer 0");

        // Read tile ID's
        for(int y = 0; y < rows; y++) {
            for(int x = 0; x < columns; x++) {
                getline(file, line, ',');
                tilemap[layer][x][y] = stoi(line);
            }
        }

    }
    else throw("Unable to open tilemap.");
}
void readInput() {
    // Reset from last frame
    for(int i = 0; i < sizeof(keysPressed); i++) {
        keysPressed[i] = false;
    }

    // Handle multi-frame input blocking
    frameTime = updateFrameTime();
    if(inputTimer > 0) {
        inputTimer -= frameTime;
    }
    else {
        inputTimer = 0;
    }

    // Keyboard
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            keysPressed[up] = true;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            keysPressed[dn] = true;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            keysPressed[lt] = true;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            keysPressed[rt] = true;
    }

    // Controller
    if(sf::Joystick::isConnected(0)) {
        if(sf::Joystick::isButtonPressed(0, 0))
            keysPressed[a] = true;
        if(sf::Joystick::isButtonPressed(0, 1))
            keysPressed[b] = true;
        if(sf::Joystick::isButtonPressed(0, 2))
            keysPressed[x] = true;
        if(sf::Joystick::isButtonPressed(0, 3))
            keysPressed[y] = true;

        if(sf::Joystick::isButtonPressed(0, 4))
            keysPressed[lb] = true;
        if(sf::Joystick::isButtonPressed(0, 5))
            keysPressed[rb] = true;
        if(sf::Joystick::isButtonPressed(0, 6))
            keysPressed[select] = true;
        if(sf::Joystick::isButtonPressed(0, 7))
            keysPressed[start] = true;

        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) < -50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) > 50)
            keysPressed[up] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) > 50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) < -50)
            keysPressed[dn] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) < -50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) > 50)
            keysPressed[lt] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) > 50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) < -50)
            keysPressed[rt] = true;
    }

    // Debug
    if(showDebugInfo) {
        cout << "\n Keys Pressed: ";
        for(int i = 0; i < sizeof(keysPressed); i++) {
            cout << keysPressed [i] << ' ';
        }
        cout << " | Input Timer : " << inputTimer;
    }
}
int updateFrameTime() {
    sf::Time frametime = clk.getElapsedTime();
    clk.restart();

    return frametime.asMilliseconds();
}

// Game States
void TitleScreen() {
    drawTilemapScreen(titleScreen, 0);

    if(keysPressed[start] || keysPressed[a]) {
        screen++;
        inputTimer = 250;
    }
}
void MainMenu() {
    drawTilemapScreen(menu, 1);

    // Load and display text
    string line;
    fstream file("Text/Main Menu.txt", ios::in);
    if(file.is_open()) {
        getline(file, line);
        drawText(128 - 4 *(int)line.length(), 32, line);

        for(int i = 0; i < 4; i++) {
            getline(file, line);
            drawText(100, 32 * i + 64, line);
        }
    }

    // Menu functionality

    cout << " --- " << selection << " --- \n";
 
    if((keysPressed[a] || keysPressed[start]) && inputTimer == 0) {
        switch(selection) {
        case 0: screen++;
            break;
        case 1: screen = -1;
            break;
        case 2: screen = -10;
            break;
        case 3: window.close();
        }
        inputTimer = 250;
    }
    if (keysPressed[up] && inputTimer == 0) {
        selection--;
        if (selection < 0) selection = 3;
        inputTimer = 250;
    }
    if (keysPressed[dn] && inputTimer == 0) {
        selection++;
        if (selection > 3) selection = 0;
        inputTimer = 250;
    }
}
