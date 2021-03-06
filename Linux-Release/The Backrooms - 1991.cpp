// The Backrooms - 1991.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
using namespace std;

#include <SFML/Graphics.hpp>

// Game variables
sf::Vector2f screenPos[4], playerPos;
const sf::Vector2f playerOffset(-8.f, -24.f);

float speed;

// Startup settings & defaults
const string title = "The Backrooms: 1991";
bool showDebugInfo = true, toggleDebugInfo = false;
const int solidWallId = 16;

enum keys { up, dn, lt, rt, start, slct, a, b, x, y, lb, rb};
int ctrlMap[] = {-1, -1, 1, -1, 7, 6, 0, 1, 2, 3, 4, 5}; // jst y inv, jst x inv, dpad x inv, dpad y inv, start, slct, a, b, x, y, lb, rb

int scale = 200, aspectRatio = 0, maxFrameRate = 0, frameRateIndex = 0, frameCount = 0;
bool showScanlines, blur;
const int stdFrameRate[] = { 0, 30, 60, 75, 120, 144, 240, 360, 0 }; // 0 = V-Sync

// State data
int screen, inputTimer, selection = 0, mappedButtons = 0;
float frameScl, frameTime, currentFrameRate; // Normalize for 60 fps
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

// Game Functions
void drawHighlightBox(int x, int y, int width);

// Game Screens
void TitleScreen();
void MainMenu();
void Controls();
void GfxSettings();
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
            // Main menu
        case 0: TitleScreen(); break;
        case 1: MainMenu(); break;

            // Game setup
        case 2: screen = 10; break;

            // Gameplay
        case 10:
            mainGame(); break;
            
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
        buffer.clear();
    }

    cout << "\n\n\nThank you for playing!\n\n\n";
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

    if (file.is_open()) {
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
    else cout << "\nUnable to open tilemap: " << filename;
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
    updateFrameTime();
    if(inputTimer > 0) {
        inputTimer -= frameTime;
    }
    else {
        inputTimer = 0;
    }

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
            pressed[x] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
            pressed[b] = true;
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
    bool clrUp, clrDn, clrLt, clrRt;

    // check if path is clear
    {
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
    {
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
    frameScl = frameTime / 16.6667;

    frameCount++;

    if ((int)(frameCount * frameScl) % 10 == 0) currentFrameRate = 1000 / frameTime;
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
            loadTilemap("Tiles/Collision Test.txt");
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
void mainGame()
{
    if (pressed[b]) speed = 2.5;
    else speed = 1;
    movePlayer(speed);

    drawTilemapScroll(walls);
    playerObj.setPosition(playerPos + playerOffset - screenPos[0]);
    buffer.draw(playerObj);

    if (playerObj.getPosition().x > 192) screenPos[0].x += speed * frameScl;
    if (playerObj.getPosition().x < 64) screenPos[0].x -= speed * frameScl;
    if (playerObj.getPosition().y > 144) screenPos[0].y += speed * frameScl;
    if (playerObj.getPosition().y < 64) screenPos[0].y -= speed * frameScl;
}

// Screen effects

