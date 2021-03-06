// The Backrooms - 1991.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <time.h> // used to seed RNG
#include <stdlib.h> // for rand function

using namespace std;

#include <filesystem>
namespace fs = std::filesystem;
#include <SFML/Graphics.hpp>

// Developer Settings
bool noClip = false;

// Game variables
sf::Vector2f screenPos[4], playerPos;
const sf::Vector2f playerOffset(-8.f, -24.f);
sf::Vector2i chunk;

float speed;

// Map Generation Settings
int mapSettings[3] = {1, 1, 1};
const int solidWallId = 16;
int mapSize = 7; // number of 64x64 chunks per axis. Use odd number for symmetric maps.
int mapDensity = 20; // number of rectangular rooms per chunk
int doorFreq = 15; // percent chance of a door generating at a given position

// Startup settings & defaults
const string title = "The Backrooms: 1991";
bool showDebugInfo = false, toggleDebugInfo = false, wallDensity = 60;

enum keys { up, dn, lt, rt, start, slct, a, b, x, y, lb, rb};
int ctrlMap[] = {-1, -1, 1, -1, 7, 6, 0, 1, 2, 3, 4, 5}; // jst y inv, jst x inv, dpad x inv, dpad y inv, start, slct, a, b, x, y, lb, rb

int scale = 200, aspectRatio = 0, maxFrameRate = 0, frameRateIndex = 0;
bool showScanlines, blur;
const int stdFrameRate[] = { 0, 30, 60, 75, 120, 144, 240, 360, 0}; // 0 = V-Sync

// State data
int screen, textPhase = 1, inputTimer, selection = 0, mappedButtons = 0, frameCount = 0, frUpdateCount = 0;
float frameTime, avgFrameTime, currentFrameRate, frUpdate;
float frameScl; // Normalize for 60 fps
bool pressed[12]; // up, dn, lt, rt, start, select, a, b, x, y, lb, rb

// Global SFML & graphics objects
sf::Clock clk;
sf::RenderTexture buffer;
sf::RenderWindow window(sf::VideoMode(512, 448), title);
sf::Sprite bufferObj;
sf::Sprite scanlineObj;

int tilemap[4][64][64];

// Graphics assets
sf::Texture scanlines;
sf::Texture font;
sf::Texture titleScreen;
sf::Texture menu;
sf::Texture controls;
sf::Texture settings;

sf::Texture walls;
sf::Texture player;

sf::Sprite playerObj;

// Engine functions
void drawTilemapStatic(sf::Texture tex, int layer);
void drawTilemapStatic(sf::Texture tex);
void drawTilemapScroll(sf::Texture tex, int layer);
void drawTilemapScroll(sf::Texture tex);
void drawText(int x, int y, string text, sf::Color color);
void drawText(int x, int y, string text);
void loadTilemap(string filename, int layer);
void loadTilemap(string filename);

void readInput();
void MapControls();
void saveControlMap();
void loadControlMap();
void saveGfxSettings();
void loadGfxSettings();

void movePlayer(float speed, int layer); // layer determines collision detection
void movePlayer(float speed);
void movePlayer();

void updateFrameTime();
void updateScreen();
void update();

// Game Functions
void drawHighlightBox(int x, int y, int width);
void generateMap();
void loadMapChunk(sf::Vector2i chunk);

// Game Screens
void TitleScreen();
void MainMenu();
void Controls();
void GfxSettings();
void gameSettings();
void introText();
void mainGame();

// Screen Effects



int main() {
    // Print startup info to terminal
    cout << "Opening " << title << ". Press TAB to toggle debug information.\n";

    // Create window
    if(showDebugInfo) cout << "Creating window...";

    window.setFramerateLimit(maxFrameRate);
    buffer.create(256, 224);
    bufferObj.setScale(2.f, 2.f);
    window.setMouseCursorVisible(false);

    if(showDebugInfo) cout << "done.";

    // Load graphics
    {
        if (showDebugInfo) cout << "\nLoading graphics...";

        if (!scanlines.loadFromFile("Fullscreen Assets/Scanlines.png")) cout << "\nUnable to load scanline overlay";
        scanlineObj.setTexture(scanlines);
        scanlines.setSmooth(true);

        if (!font.loadFromFile("Tiles/Font.png")) cout << "\nUnable to load font tileset.";
        if (!titleScreen.loadFromFile("Tiles/Title Screen.png")) cout << "\nUnable to load title screen tileset.";
        if (!menu.loadFromFile("Tiles/Menu.png")) cout << "\nUnable to load font tileset.";
        if (!controls.loadFromFile("Tiles/Controls.png")) cout << "\nUnable to load controls tileset.";
        if (!settings.loadFromFile("Tiles/Settings.png")) cout << "\nUnable to load settings tileset.";

        if (!walls.loadFromFile("Tiles/Background.png")) cout << "\nUnable to load background tileset.";
        if (!player.loadFromFile("Sprites/Generic Guy.png")) cout << "\nUnable to load player character.";
        playerObj.setTexture(player);

        loadTilemap("Tiles/Title Screen.txt");
        loadTilemap("Tiles/Main Menu.txt", 1);

        if (showDebugInfo) cout << "done.";
    }

    // Load controls
    loadControlMap();
    loadGfxSettings();

    // Setup RNG
    srand((int)time(NULL));

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

        // Game logic

        switch(screen) {
            // Main menu
        case 0: TitleScreen(); break;
        case 1: MainMenu(); break;

            // Game setup
        case 2: gameSettings(); break;
        case 9: introText(); break;

            // Gameplay
        case 10: mainGame(); break;
            
            // Controls
        case -1: Controls(); break;
        case -2: MapControls(); break;

            // Settings
        case -10: GfxSettings(); break;

            // Unrecognized game state
        default:
            buffer.clear(sf::Color::Blue);
            drawText(0, 0, "Error: unrecognized game state.", sf::Color::White);
            drawText(0, 16, "Screen: " + to_string(screen), sf::Color::Cyan);
        }

        update();
    }

    cout << "\n\n\nThank you for playing!\n\n\n";

    sf::sleep(sf::seconds(5));

    return 0;
}



// Engine functions
void drawTilemapStatic(sf::Texture tex, int layer) {
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
void drawTilemapStatic(sf::Texture tex) {
    drawTilemapStatic(tex, 0);
}
void drawTilemapScroll(sf::Texture tex, int layer) {
    sf::Sprite tile;
    tile.setTexture(tex);
    int tileID, tileX, tileY;
    int dx = screenPos[layer].x, dy = screenPos[layer].y, dxt = dx / 16, dyt = dy / 16;

    for (int x = dxt; x < 17 + dxt; x++) {
        for (int y = dyt; y < 15 + dyt; y++) {
            if (x >= 0 && y >= 0 && x < 64 && y < 64) {
                tileID = tilemap[layer][x][y];
                tileX = (tileID % 16); tileX *= 16;
                tileY = (tileID / 16); tileY *= 16;

                tile.setColor(sf::Color::White);
                tile.setTextureRect(sf::IntRect(tileX, tileY, 16, 16));
            }
            else {
                tile.setTextureRect(sf::IntRect(0, 0, 16, 16));
                tile.setColor(sf::Color::Black);
            }

            tile.setPosition(16.f * x - dx, 16.f * y - dy);

            buffer.draw(tile);
        }
    }

}
void drawTilemapScroll(sf::Texture tex) {
    drawTilemapScroll(tex, 0);
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

    ifstream file(filename);

    //if (file.is_open()) {
     {
        // get tilemap width
        getline(file, line, ' ');
        if (line != "tileswide") cout << "\nWarning: Unexpected tilemap format.";
        getline(file, line);
        columns = stoi(line);

        // get tilemap height
        getline(file, line, ' ');
        if (line != "tileshigh") cout << "\nWarning: Unexpected tilemap format.";
        getline(file, line);
        rows = stoi(line);

        // Skip over extra lines
        while (getline(file, line) && line != "layer 0");

        // Read tile ID's
        for (int y = 0; y < rows; y++) {
            for (int x = 0; x < columns; x++) {
                getline(file, line, ',');
                tilemap[layer][x][y] = stoi(line);
            }
        }

        file.close();
    }
    //else cout << "\nUnable to open tilemap: " << filename;
}
void loadTilemap(string filename) {
    loadTilemap(filename, 0);
}

void readInput() {
    // Reset from last frame
    for(int i = 0; i < sizeof(pressed) / sizeof(pressed[0]); i++) {
        pressed[i] = false;
    }

    // Handle multi-frame input blocking
    if(inputTimer > 0) {
        inputTimer -= frameTime;
    }
    else inputTimer = 0;

    // Keyboard
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            pressed[up] = true;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            pressed[dn] = true;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            pressed[lt] = true;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            pressed[rt] = true;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            pressed[a] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            pressed[b] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
            pressed[x] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
            pressed[y] = true;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
            pressed[start] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            pressed[slct] = true;
    }

    // Controller
    if(sf::Joystick::isConnected(0)) {
        if(sf::Joystick::isButtonPressed(0, ctrlMap[a]))
            pressed[a] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[b]))
            pressed[b] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[x]))
            pressed[x] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[y]))
            pressed[y] = true;

        if(sf::Joystick::isButtonPressed(0, ctrlMap[lb]))
            pressed[lb] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[rb]))
            pressed[rb] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[slct]))
            pressed[slct] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[start]))
            pressed[start] = true;

        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) * ctrlMap[0] > 50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) * ctrlMap[2] > 50)
            pressed[up] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) * ctrlMap[0] < -50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) * ctrlMap[2] < -50)
            pressed[dn] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) * ctrlMap[1] > 50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) * ctrlMap[3] > 50)
            pressed[lt] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) * ctrlMap[1] < -50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) * ctrlMap[3] < -50)
            pressed[rt] = true;
    }
}
void MapControls() {
    if (!sf::Joystick::isConnected(0)) {
        screen = -1;
        if (showDebugInfo) {
            cout << "\n No joystick found to map.";
        }
    }

    buffer.clear(sf::Color::Black);

    string line;
    ifstream file("Text/Map Controls.txt");
    if (file.is_open()) {
        for (int i = 0; i <= mappedButtons; i++) getline(file, line);
        drawText(128 - 4 * (int)line.length(), 16, line, sf::Color::White);

        file.close();
    }

    switch (mappedButtons) {
        // Directional inputs
    case 0:
        if (!sf::Joystick::hasAxis(0, sf::Joystick::Axis::Y)) mappedButtons++;
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) > 50.f) {
            ctrlMap[0] = 1;
            mappedButtons++;
        }
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) < -50.f) {
            ctrlMap[0] = -1;
            mappedButtons++;
        }
        break;
    case 1:
        if (!sf::Joystick::hasAxis(0, sf::Joystick::Axis::X)) mappedButtons++;
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) > 50.f) {
            ctrlMap[1] = -1;
            mappedButtons++;
        }
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) < -50.f) {
            ctrlMap[1] = 1;
            mappedButtons++;
        }
        break;
    case 2:
        if (!sf::Joystick::hasAxis(0, sf::Joystick::Axis::PovY)) mappedButtons++;
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) > 50.f) {
            ctrlMap[2] = 1;
            mappedButtons++;
        }
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) < -50.f) {
            ctrlMap[2] = -1;
            mappedButtons++;
        }
        break;
    case 3:
        if (!sf::Joystick::hasAxis(0, sf::Joystick::Axis::PovX)) mappedButtons++;
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) > 50.f) {
            ctrlMap[3] = -1;
            mappedButtons++;
            inputTimer = 200;
        }
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) < -50.f) {
            ctrlMap[3] = 1;
            mappedButtons++;
            inputTimer = 200;
        }
        break;

        // Cleanup
    case 12:
        screen++;
        saveControlMap();
        selection = 1;
        inputTimer = 200;
        break;

        // Button inputs
    default:
        for (int i = 0; i < sf::Joystick::getButtonCount(0); i++) {
            if (sf::Joystick::isButtonPressed(0, i) && inputTimer == 0) {
                ctrlMap[mappedButtons] = i;
                inputTimer = 250;
                if (mappedButtons < sizeof(ctrlMap) / sizeof(ctrlMap[i])) mappedButtons++;
            }
        }
        break;
    }
}
void saveControlMap() {
    if (showDebugInfo) cout << "\nSaving control map...";

    ofstream file;
    file.open("Controls.dat", ios::out);
    file << "Axis inversion:\n";
    for (int i = 0; i < 4; i++) {
        file << ctrlMap[i] << "\n";
    }
    file << "Button ID's:\n";
    for (int i = 4; i < sizeof(ctrlMap) / sizeof(ctrlMap[i]); i++) {
        file << ctrlMap[i] << "\n";
    }

    file.close();
    if (showDebugInfo) cout << "done.";
}
void loadControlMap() {
    if (showDebugInfo) cout << "\nReading control map...";

    ifstream file ("Controls.dat");
    string line;

    if (file.is_open()) {
        getline(file, line);
        if (line != "Axis inversion:") cout << "\nWarining: controller map may not be formatted correctly (section 1).";
        for (int i = 0; i < 4; i++) {
            getline(file, line);
            ctrlMap[i] = stoi(line);
        }

        getline(file, line);
        if (line != "Button ID's:") cout << "\nWarining: controller map may not be formatted correctly (section 2).";
        for (int i = 4; i < sizeof(ctrlMap) / sizeof(ctrlMap[i]); i++) {
            getline(file, line);
            ctrlMap[i] = stoi(line);
        }

        file.close();
    }

    if (showDebugInfo) cout << "Done.";
}
void saveGfxSettings() {
    if (showDebugInfo) cout << "\nSaving graphics settings...";

    ofstream file;
    file.open("Graphics.dat", ios::out);
    file << "Aspect_Ratio: " << aspectRatio;
    file << "\nScale_Factor: " << scale;
    file << "\nFrame_Rate: " << maxFrameRate;
    file << "\nScanlines: " << showScanlines;
    file << "\nCRT_Blur: " << blur;

    if (showDebugInfo) cout << "done.";
}
void loadGfxSettings() {
    if (showDebugInfo) cout << "\nReading graphics settings...";
    string line;
    string expectedLabels[] = { "Aspect_Ratio:", "Scale_Factor:", "Frame_Rate:", "Scanlines:", "CRT_Blur:" };
    int xSize = 256, ySize = 224;
    int values[5];

    // Parse file
    ifstream file("Graphics.dat");
    if (file.is_open()) {
        for (int i = 0; i < 5; i++) {
            getline(file, line, ' ');
            if (line != expectedLabels[i]) cout << "\nWarining: graphics settings may not be formatted correctly (line " << i + 1 << ").";
            getline(file, line);
            values[i] = stoi(line);
        }

        aspectRatio = values[0];
        scale = values[1];
        maxFrameRate = values[2];
        showScanlines = values[3];
        blur = values[4];

        for (int i = 0; i < sizeof(stdFrameRate) / sizeof(int); i++) {
            if (stdFrameRate[i] == maxFrameRate) frameRateIndex = i;
        }

        file.close();
    }


    // Apply settings
    ySize = 224 * scale / 100;

    if (aspectRatio == 0) xSize = 256 * scale / 100;
    if (aspectRatio == 1) xSize = ySize * 4 / 3;
    if (aspectRatio == 2) xSize = ySize * 16 / 9;

    window.setSize(sf::Vector2u(xSize, ySize));
    buffer.setSmooth(blur);

    window.setFramerateLimit(maxFrameRate);
    window.setVerticalSyncEnabled(maxFrameRate == 0); // Enable V-Sync if frame rate is uncapped

    if (showDebugInfo) cout << "Done.";
}

void movePlayer(float speed, int layer) {
    const int strictness = 5;
    int gridPosX = playerPos.x / 16;
    int gridPosY = playerPos.y / 16;
    bool clrUp = true, clrDn = true, clrLt = true, clrRt = true;

    // check if path is clear
    if (!noClip) {
        clrUp = (tilemap[layer][(int)(playerPos.x + strictness - 1) / 16][(int)(playerPos.y - strictness) / 16] < solidWallId)
            && (tilemap[layer][(int)(playerPos.x - strictness + 1) / 16][(int)(playerPos.y - strictness) / 16] < solidWallId);
        clrDn = (tilemap[layer][(int)(playerPos.x + strictness - 1) / 16][(int)(playerPos.y + strictness) / 16] < solidWallId)
            && (tilemap[layer][(int)(playerPos.x - strictness + 1) / 16][(int)(playerPos.y + strictness) / 16] < solidWallId);
        clrLt = (tilemap[layer][(int)(playerPos.x - strictness) / 16][(int)(playerPos.y + strictness - 1) / 16] < solidWallId)
            && (tilemap[layer][(int)(playerPos.x - strictness) / 16][(int)(playerPos.y - strictness + 1) / 16] < solidWallId);
        clrRt = (tilemap[layer][(int)(playerPos.x + strictness) / 16][(int)(playerPos.y + strictness - 1) / 16] < solidWallId)
            && (tilemap[layer][(int)(playerPos.x + strictness) / 16][(int)(playerPos.y - strictness + 1) / 16] < solidWallId);
    }

    // move character
    if (pressed[up] && clrUp) playerPos.y -= speed * frameScl;
    if (pressed[dn] && clrDn) playerPos.y += speed * frameScl;
    if (pressed[lt] && clrLt) playerPos.x -= speed * frameScl;
    if (pressed[rt] && clrRt) playerPos.x += speed * frameScl;

    // push character out of wall
    if (!noClip) {
        if ((tilemap[layer][(int)(playerPos.x + strictness) / 16][(int)(playerPos.y - 8) / 16] >= solidWallId)
            || (tilemap[layer][(int)(playerPos.x - strictness) / 16][(int)(playerPos.y - 8) / 16] >= solidWallId))
            playerPos.y = gridPosY * 16 + 8;
        if ((tilemap[layer][(int)(playerPos.x + strictness) / 16][(int)(playerPos.y + 8) / 16] >= solidWallId)
            || (tilemap[layer][(int)(playerPos.x - strictness) / 16][(int)(playerPos.y + 8) / 16] >= solidWallId))
            playerPos.y = gridPosY * 16 + 8;
        if ((tilemap[layer][(int)(playerPos.x - 8) / 16][(int)(playerPos.y + strictness) / 16] >= solidWallId)
            || (tilemap[layer][(int)(playerPos.x - 8) / 16][(int)(playerPos.y - strictness) / 16] >= solidWallId))
            playerPos.x = gridPosX * 16 + 8;
        if ((tilemap[layer][(int)(playerPos.x + 8) / 16][(int)(playerPos.y + strictness) / 16] >= solidWallId)
            || (tilemap[layer][(int)(playerPos.x + 8) / 16][(int)(playerPos.y - strictness) / 16] >= solidWallId))
            playerPos.x = gridPosX * 16 + 8;
    }
}
void movePlayer(float speed) {
    movePlayer(speed, 0);
}
void movePlayer() {
    movePlayer(1, 0);
}

void updateFrameTime() {
    sf::Time frametime = clk.getElapsedTime();
    clk.restart();

    frameTime = frametime.asMicroseconds() / 1000.f;
    avgFrameTime += frameTime;
    frameScl = frameTime / 16.6667;
    frameCount++;
    frUpdateCount++;
    frUpdate += frameScl;

    if (frUpdate >= 10) {
        avgFrameTime /= frUpdateCount;
        currentFrameRate = 1000 / avgFrameTime;

        avgFrameTime = 0;
        frUpdate = 0;
        frUpdateCount = 0;
    }
}
void updateScreen() {
    // Framerate counter
    if (showDebugInfo) {
        sf::Color fpsCol;
        sf::RectangleShape fpsBg(sf::Vector2f(92, 16));
        sf::Vector2f fpsStart(207.f - 8 * (currentFrameRate > 99) - 8 * (currentFrameRate > 999), 0.f);

        if (currentFrameRate > 1.05 * maxFrameRate) fpsCol = sf::Color::Cyan;
        else if (currentFrameRate > 0.95 * maxFrameRate) fpsCol = sf::Color::Green;
        else if (currentFrameRate > 0.9 * maxFrameRate) fpsCol = sf::Color::Yellow;
        else if (currentFrameRate > 0.8 * maxFrameRate) fpsCol = sf::Color(255, 127, 0);
        else fpsCol = sf::Color::Red;

        fpsBg.setFillColor(sf::Color(0, 0, 0, 127));
        fpsBg.setPosition(fpsStart);

        buffer.draw(fpsBg);
        drawText(fpsStart.x, fpsStart.y, to_string((int)currentFrameRate) + " FPS", fpsCol);
    }

    // Update graphics
    buffer.display();
    bufferObj.setTexture(buffer.getTexture());
    window.draw(bufferObj);
    if (showScanlines) window.draw(scanlineObj);
    window.display();
    window.clear(sf::Color::Black);
}
void update() {
    readInput();
    updateFrameTime();
    updateScreen();
    buffer.clear();
}

// Game Functions
void drawHighlightBox(int x, int y, int width) {
    sf::Sprite tile;
    tile.setTexture(menu);
    tile.setColor(sf::Color(255, 255, 255, 64));

    for(int row = 0; row < 3; row++) {
        tile.setTextureRect(sf::IntRect(96 + row * 48, 32, 16, 16));
        tile.setPosition(x * 16.f,(y + row) * 16.f);
        buffer.draw(tile);

        for(int i = 1; i < width; i++) {
            tile.setTextureRect(sf::IntRect(112 + row * 48, 32, 16, 16));
            tile.setPosition((x + i) * 16.f,(y + row) * 16.f);
            buffer.draw(tile);
        }

        tile.setTextureRect(sf::IntRect(128 + row * 48, 32, 16, 16));
        tile.setPosition((x + width) * 16.f,(y + row) * 16.f);
        buffer.draw(tile);
    }
}
void generateMap() {
    // Prepare to Display Text From File
    string line;
    ifstream file("Text/MapGen.txt");
    
    // Preparing to generate map
    buffer.clear(sf::Color::Black);
    getline(file, line);
    drawText(128 - line.length() * 4, 16, line, sf::Color::White);
    update();

    vector<vector<int>> walls(mapSize * 64, vector<int>(mapSize * 64, 0));
    
    int x1, y1, x2, y2, tmp;

    // Building walls
    getline(file, line);
    drawText(128 - line.length() * 4, 16, line, sf::Color::White);
    update();
    for (int cx = 0; cx < mapSize; cx ++) {
        for (int cy = 0; cy < mapSize; cy++) {
            if (showDebugInfo) cout << "\n     Chunk (" << cx << ", " << cy << ").";

            // Perimeter Walls
            for (int i = 0; i < 64; i++) {
                walls[i + 64 * cx][64 * cy] = 16;
                walls[i + 64 * cx][63 + 64 * cy] = 16;
                walls[64 * cx][i + 64 * cy] = 16;
                walls[63 + 64 * cx][i + 64 * cy] = 16;
            }

            // Interior Walls
            for (int i = 0; i < mapDensity; i++) {
                x1 = rand() % 22 * 3;
                y1 = rand() % 22 * 3;
                x2 = rand() % 22 * 3;
                y2 = rand() % 22 * 3;

                if (x1 > x2) {
                    tmp = x2;
                    x2 = x1;
                    x1 = tmp;
                }
                if (y1 > y2) {
                    tmp = y2;
                    y2 = y1;
                    y1 = tmp;
                }

                for (int x = x1; x <= x2; x++) {
                    walls[x + 64 * cx][y1 + 64 * cy] = 16;
                    walls[x + 64 * cx][y2 + 64 * cy] = 16;
                }
                for (int y = y1; y <= y2; y++) {
                    walls[x1 + 64 * cx][y + 64 * cy] = 16;
                    walls[x2 + 64 * cx][y + 64 * cy] = 16;
                }
            }
        }
    }

    // Cutting doorways
    getline(file, line);
    drawText(128 - line.length() * 4, 16, line, sf::Color::White);
    update();
    for (int cx = 0; cx <= mapSize; cx++) {
        for (int cy = 0; cy <= mapSize; cy++) {
            if (showDebugInfo) cout << "\n     Chunk (" << cx << ", " << cy << ").";

            // Perimeter Walls
            for (int i = 0; i < 21; i++) {
                // top and bottom
                if (rand() % 100 < doorFreq && cx < mapSize) {
                    if (cy < mapSize) {
                        walls[i * 3 + cx * 64 + 1][cy * 64] = 0;
                        walls[i * 3 + cx * 64 + 2][cy * 64] = 0;
                    }
                    if (cy > 0) {
                        walls[i * 3 + cx * 64 + 1][cy * 64 - 1] = 0;
                        walls[i * 3 + cx * 64 + 2][cy * 64 - 1] = 0;

                    }
                }
                // left and right
                if (rand() % 100 < doorFreq && cy < mapSize) {
                    if (cx < mapSize) {
                        walls[cx * 64][i * 3 + cy * 64 + 1] = 0;
                        walls[cx * 64][i * 3 + cy * 64 + 2] = 0;
                    }
                    if (cx > 0) {
                        walls[cx * 64 - 1][i * 3 + cy * 64 + 1] = 0;
                        walls[cx * 64 - 1][i * 3 + cy * 64 + 2] = 0;
                    }
                }
            }

            // Interior Walls
            if (cx < mapSize && cy < mapSize) {
                for (int i = 1; i < 21; i++) {
                    for (int j = 1; j < 21; j++) {
                        // horizontal
                        if (rand() % 100 < doorFreq && cx < mapSize) {
                            walls[i * 3 + cx * 64 + 1][j * 3 + cy * 64] = 0;
                            walls[i * 3 + cx * 64 + 2][j * 3 + cy * 64] = 0;
                        }
                        // vertical
                        if (rand() % 100 < doorFreq && cx < mapSize) {
                            walls[i * 3 + cx * 64][j * 3 + cy * 64 + 1] = 0;
                            walls[i * 3 + cx * 64][j * 3 + cy * 64 + 2] = 0;
                        }
                    }
                }
            }
        }
    }

    // Saving map
    getline(file, line);
    drawText(128 - line.length() * 4, 16, line, sf::Color::White);
    update();
    fs::create_directory("Map");

    for (int cx = 0; cx < mapSize; cx++) {
        for (int cy = 0; cy < mapSize; cy++) {
            if (showDebugInfo) cout << "\nSaving Chunk (" << cx << ", " << cy << ").";

            stringstream filename;
            filename << "Map/Map_" << cx << "_" << cy << ".txt";

            ofstream file;
            file.open(filename.str());
            file << "tileswide 64\ntileshigh 64\ntilewidth 16\ntileheight 16\n\nlayer 0\n";
            for (int y = 0; y < 64; y++) {
                for (int x = 0; x < 64; x++) {
                    file << walls[64 * cx + x][64 * cy + y] << ",";
                }
                file << "\n";
            }
        }
    }

    file.close();
}
void loadMapChunk(sf::Vector2i chunk) {
    if (showDebugInfo) cout << "\nLoading Chunk: (" << chunk.x << ", " << chunk.y << ")";
    stringstream filename;
    filename << "Map/Map_" << chunk.x << "_" << chunk.y << ".txt";
    loadTilemap(filename.str());
}

// Game Screens
void TitleScreen() {
    drawTilemapStatic(titleScreen);

    if((pressed[start] || pressed[a]) && inputTimer == 0) {
        screen++;
        inputTimer = 250;
    }
}
void MainMenu() {
    drawTilemapStatic(titleScreen, 0);
    drawTilemapStatic(menu, 1);

    // Load and display text
    string line;
    ifstream file("Text/Main Menu.txt");
    if (file.is_open()) {
        getline(file, line);
        drawText(128 - 4 * (int)line.length(), 32, line);

        for (int i = 0; i < 4; i++) {
            getline(file, line);
            drawText(100, 32 * i + 64, line);
        }

        file.close();
    }

    // Menu Visuals
    drawHighlightBox(4, 3 + selection * 2, 7);

    // Menu functionality
    if ((pressed[a] || pressed[start]) && inputTimer == 0) { // slct option
        switch (selection) {
        case 0:
            screen++;
            while (tilemap[0][(int)playerPos.x / 16][(int)playerPos.y / 16] >= solidWallId) {
                playerPos.x += 16;
                playerPos.y += 16;
            }

            break;
        case 1:
            screen = -1;
            loadTilemap("Tiles/Controls.txt");
            loadTilemap("Tiles/Controls Menu.txt", 1);
            break;
        case 2:
            screen = -10;
            loadTilemap("Tiles/Graphics Settings.txt");
            loadTilemap("Tiles/Graphics Settings Bottom.txt", 1);
            break;
        case 3: window.close();
        }
        inputTimer = 250;
        selection = 0;
    }
    if (pressed[up] && inputTimer == 0) { // Move selection down
        selection--;
        if (selection < 0) selection = 3;
        inputTimer = 200;
    }
    if (pressed[dn] && inputTimer == 0) { // Move selection up
        selection++;
        if (selection > 3) selection = 0;
        inputTimer = 200;
    }
}
void Controls() {
    drawTilemapStatic(controls);
    drawTilemapStatic(menu, 1);

    string line;
    ifstream file("Text/Controls.txt");
    if (file.is_open()) {
        // Label buttons
        getline(file, line);
        drawText(16, 16, line, sf::Color::White);
        getline(file, line);
        drawText(112, 16, line, sf::Color::White);
        getline(file, line);
        drawText(192, 16, line, sf::Color::White);
        getline(file, line);
        drawText(128, 160, line, sf::Color::White);
        getline(file, line);
        drawText(208, 160, line, sf::Color::White);

        // Options
        getline(file, line);
        drawText(32, 192, line);
        getline(file, line);
        drawText(175, 192, line);

        file.close();
    }

    //menu display
    drawHighlightBox(selection * 9, 11, 9 - selection * 3);

    // Menu Functionality
    if ((pressed[a] || pressed[start]) && inputTimer == 0) {
        switch (selection) {
        case 0: // Map controller
            screen--;
            mappedButtons = 0;
            break;
        case 1: // Return to main menu
            screen = 0;
            selection = 0;
            inputTimer = 200;
            loadTilemap("Tiles/Title Screen.txt");
            loadTilemap("Tiles/Main Menu.txt", 1);
            break;
        }

        selection = 0;
    }
    if (pressed[lt] && inputTimer == 0 && selection > 0) {
        selection--;
        inputTimer = 200;
    }
    if (pressed[rt] && inputTimer == 0 && selection < 1) {
        selection++;
        inputTimer = 200;
    }
}
void GfxSettings() {
    int xSize = 256, ySize = 224;
    string line, vsync;

    // Draw menu
    drawTilemapStatic(settings);
    drawTilemapStatic(menu, 1);
    ifstream file("Text/Graphics Settings.txt");
    if (file.is_open()) {
        for (int i = 0; i < 5; i++) {
            getline(file, line);
            drawText(40, (i + 1) * 32, line, sf::Color::White);
        }
        getline(file, line);
        drawText(32, 192, line);
        getline(file, line);
        drawText(176, 192, line);
        getline(file, vsync);

        file.close();
    }
    
    // Text values
    drawText(160, 64, to_string(scale) + "%", sf::Color::White);
    if (maxFrameRate == 0) drawText(160, 96, vsync, sf::Color::Green);
    else drawText(160, 96, to_string(maxFrameRate) + " FPS", sf::Color::White);

    // Change Settings
    if (pressed[up] && inputTimer == 0) {
        selection--;
        inputTimer = 200;
    }
    if (pressed[dn] && inputTimer == 0) {
        selection++;
        inputTimer = 200;
    }
    if (pressed[rt] && selection == 5 && inputTimer == 0) {
        selection++;
        inputTimer = 200;
    }
    if (pressed[lt] && selection == 6 && inputTimer == 0) {
        selection--;
        inputTimer = 200;
    }
    switch (selection) {
    case -1: selection = 6; break;
    case 0: // Aspect Ratio
        if (pressed[lt] && inputTimer == 0) {
            aspectRatio--;
            if (aspectRatio < 0) aspectRatio = 2;
            inputTimer = 200;
        }
        if ((pressed[rt] || pressed[a]) && inputTimer == 0) {
            aspectRatio++;
            if (aspectRatio > 2) aspectRatio = 0;
            inputTimer = 200;
        }

        ySize = 224 * scale / 100;

        if (aspectRatio == 0) xSize = 256 * scale / 100;
        if (aspectRatio == 1) xSize = ySize * 4 / 3;
        if (aspectRatio == 2) xSize = ySize * 16 / 9;

        window.setSize(sf::Vector2u(xSize, ySize));
        break;

    case 1: // Scale
        if (pressed[lt] && inputTimer == 0 && scale > 50) {
            scale -= 25;
            inputTimer = 200;
        }
        if (pressed[rt] && inputTimer == 0) {
            scale += 25;
            inputTimer = 200;
        }

        if (scale < 100) blur = true;
        if (scale < 200) showScanlines = false;

        ySize = 224 * scale / 100;

        if (aspectRatio == 0) xSize = 256 * scale / 100;
        if (aspectRatio == 1) xSize = ySize * 4 / 3;
        if (aspectRatio == 2) xSize = ySize * 16 / 9;

        window.setSize(sf::Vector2u(xSize, ySize));
        break;

    case 2: // Frame Rate
        if (pressed[lt] && inputTimer == 0 && frameRateIndex > 0) {
            frameRateIndex--;
            inputTimer = 200;
        }
        if (pressed[rt] && inputTimer == 0 && frameRateIndex < sizeof(stdFrameRate) / sizeof(stdFrameRate[0]) - 1) {
            frameRateIndex++;
            inputTimer = 200;
        }

        maxFrameRate = stdFrameRate[frameRateIndex];
        window.setFramerateLimit(maxFrameRate);
        window.setVerticalSyncEnabled(maxFrameRate == 0); // Enable V-Sync if frame rate is uncapped
        break;

    case 3: // Scanlines
        if ((pressed[rt] || pressed[lt] || pressed[a]) && inputTimer == 0) {
            showScanlines = !showScanlines;
            inputTimer = 200;
        }
        break;

    case 4: // Blur
        if ((pressed[rt] || pressed[lt] || pressed[a]) && inputTimer == 0) {
            blur = !blur;
            if (blur) showScanlines = true;
            inputTimer = 200;
        }
        break;

    case 5: // Save settings
        if ((pressed[a] || pressed[start]) && inputTimer == 0) {
            saveGfxSettings();
            inputTimer = 200;
        }
        break;

    case 6: // Return to main menu
        if ((pressed[a] || pressed[start]) && inputTimer == 0) {
            screen = 0;
            selection = 0;
            inputTimer = 200;
            loadTilemap("Tiles/Title Screen.txt");
            loadTilemap("Tiles/Main Menu.txt", 1);
        }
        break;
    case 7: selection = 0;
    }
    buffer.setSmooth(blur);

    // Indicate selected options
    {
        if (selection < 6) drawHighlightBox(0, 1 + 2 * selection, 15 - 6 * (selection == 5));
        else drawHighlightBox(9, 11, 6);

        sf::RectangleShape rect(sf::Vector2f(20.f, 20.f));
        rect.setPosition(158.f + 32 * aspectRatio, 30.f);
        rect.setOutlineColor(sf::Color::White);
        rect.setFillColor(sf::Color(0, 0, 0, 0));
        rect.setOutlineThickness(1);
        buffer.draw(rect);

        sf::Sprite toggle;
        toggle.setTexture(settings);
        toggle.setTextureRect(sf::IntRect(176, 0, 16, 16));
        toggle.setPosition(160.f + 14 * showScanlines, 128);
        buffer.draw(toggle);
        toggle.setPosition(160.f + 14 * blur, 160);
        buffer.draw(toggle);
    }
}
void gameSettings(){
    string line;
    ifstream file("Text/Game Setup.txt");

    // Check Map Existence
    bool mapExists;
    if (fs::exists("Map/Map_0_0.txt")) {
        mapExists = true;
    }
    else {
        mapExists = false;
    }
    
    // Draw Text
    getline(file, line);
    if (mapExists) {
        drawText(40, 32, line, sf::Color::White);
        getline(file, line);
    } else {
        getline(file, line);
        drawText(40, 32, line, sf::Color::White);
    }

    for (int i = 0; i < 4; i++) {
        getline(file, line);
        drawText(40, 80 + 16 * i, line, sf::Color::White);
        if (i < 3) {
            for (int n = 0; n <= mapSettings[i]; n++) getline(file, line);
            drawText(168, 80 + 16 * i, line, sf::Color::White);
            for (int n = 0; n < 3 - mapSettings[i] - 1; n++) getline(file, line);
        }
    }

    getline(file, line);
    drawText(40, 176, line, sf::Color::White);
     
    // Input
    if (pressed[dn] && inputTimer == 0 && selection < 6) {
        selection++;
        inputTimer = 200;

        if (selection == -2) selection = 0;
        if (selection == 4) selection = 6;
    }
    if (pressed[up] && inputTimer == 0 && selection > 0 - 3 * mapExists) {
        selection--;
        inputTimer = 200;

        if (selection == 5) selection = 3;
        if (selection == -1) selection = -3;
    }

    if (pressed[lt] && inputTimer == 0&& selection >= 0 && selection < 3 && mapSettings[selection] > 0) {
        mapSettings[selection] --;
        inputTimer = 200;
        
        cout << "\n Map Settings:";
        for (int i = 0; i < 3; i++) {
            cout << "\n" << mapSettings[i];
        }
    }
    if (pressed[rt] && inputTimer == 0&& selection >= 0 && selection < 3 && mapSettings[selection] < 2) {
        mapSettings[selection] ++;
        inputTimer = 200;

        cout << "\n Map Settings:";
        for (int i = 0; i < 3; i++) {
            cout << "\n" << mapSettings[i];
        }
    }

    if (pressed[start] || pressed[a]) {
        switch (selection) {
        case -3: break;
        case 6:
            mapSize = 7 + 10 * mapSettings[0];
            mapDensity = 20 + 5 * mapSettings[1];
            doorFreq = 35 - 5 * mapSettings[2];

            playerPos = { 512.f, 512.5 };
            screenPos[0] = { 385, 400 };

            generateMap();
            chunk.x = chunk.y = mapSize / 2;
            loadMapChunk(chunk);
            screen = 9;
            break;
        }
    }

    // User Feedback
    drawHighlightBox(1, 4 + selection, 13);
}
void introText() {
    stringstream filename;
    string line;
    int lineX, lineY;

    filename << "Text/Intro Text p" << textPhase << ".txt";
    ifstream file(filename.str());

    if (textPhase == 1) {
        lineX = 8;
        lineY = 8;
    }
    else {
        lineX = 0;
        lineY = 64;
    }

    while (getline(file, line)) {
        if (textPhase == 2) lineX = 128 - 4 * line.length();
        drawText(lineX, lineY, line, sf::Color::White);
        lineY += 16;
    }

    if ((pressed[start] || pressed[a]) && inputTimer == 0) {
        textPhase++;
        inputTimer = 250;
        if (textPhase == 3) screen++;
    }
}
void mainGame()
{
    // Move player
    if (pressed[b]) speed = 2.5;
    else speed = 1;
    movePlayer(speed);

    // Scroll screen
    if (playerObj.getPosition().x > 192) screenPos[0].x += speed * frameScl;
    if (playerObj.getPosition().x < 64) screenPos[0].x -= speed * frameScl;
    if (playerObj.getPosition().y > 144) screenPos[0].y += speed * frameScl;
    if (playerObj.getPosition().y < 64) screenPos[0].y -= speed * frameScl;

    // Load chunk upon reaching map's edge
    if (playerPos.x <= 8.f) {
        chunk.x--;
        if (chunk.x < 0) {
            cout << "\n\n\nCongratulations! You escaped the Backrooms!";
            window.close();
        }
        else {
            loadMapChunk(chunk);
            playerPos.x = 1015;
            screenPos[0].x += 1015;
        }
    }
    if (playerPos.x >= 1016.f) {
        chunk.x++;
        if (chunk.x >= mapSize) {
            cout << "\n\n\nCongratulations! You escaped the Backrooms!";
            window.close();
        }
        else {
            loadMapChunk(chunk);
            playerPos.x = 9;
            screenPos[0].x -= 1015;
        }
    }
    if (playerPos.y <= 8.f) {
        chunk.y--;
        if (chunk.y < 0) {
            cout << "\n\n\nCongratulations! You escaped the Backrooms!";
            window.close();
        }
        else {
            loadMapChunk(chunk);
            playerPos.y = 1015;
            screenPos[0].y += 1015;
        }
    }
    if (playerPos.y >= 1016.f) {
        chunk.y++;
        if (chunk.y >= mapSize) {
            cout << "\n\n\nCongratulations! You escaped the Backrooms!";
            window.close();
        }
        else {
            loadMapChunk(chunk);
            playerPos.y = 9;
            screenPos[0].y -= 1015;
        }
    }

    // Render graphics
    drawTilemapScroll(walls);
    playerObj.setPosition(playerPos + playerOffset - screenPos[0]);
    buffer.draw(playerObj);
}

// Screen effects

