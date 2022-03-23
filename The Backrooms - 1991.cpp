// The Backrooms - 1991.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
using namespace std;

#include <SFML/Graphics.hpp>

// Startup settings & defaults
const string title = "The Backrooms: 1991";
bool showDebugInfo = true, toggleDebugInfo = false;
int maxFrameRate = 60;

enum keys { up, dn, lt, rt, start, slct, a, b, x, y, lb, rb};
int ctrlMap[] = {-1, -1, 1, -1, 7, 6, 0, 1, 2, 3, 4, 5}; // jst y inv, jst x inv, dpad x inv, dpad y inv, start, slct, a, b, x, y, lb, rb

int scale = 200, aspectRatio = 0, frameRateIndex = 2;
bool showScanlines, blur;
const int stdFrameRate[] = { 0, 30, 60, 75, 120, 144, 240, 360, 0 }; // 0 = V-Sync

// State data
int screen, inputTimer, frameTime, slction = 0;
bool keysPressed[12]; // up, dn, lt, rt, start, select, a, b, x, y, lb, rb

// Game state
int mappedButtons = 0;

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

// Engine functions
void drawTilemapScreen(sf::Texture, int layer);
void drawText(int x, int y, string text, sf::Color color);
void drawText(int x, int y, string text);
void loadTilemap(string filename, int layer);
void readInput();
void MapControls();
void saveControlMap();
void loadControlMap();
void saveGfxSettings();
void loadGfxSettings();

int updateFrameTime();

// Game Functions
void drawHighlightBox(int x, int y, int width);

// Game Screens
void TitleScreen();
void MainMenu();
void Controls();
void GfxSettings();

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

        if (!scanlines.loadFromFile("Scanlines.png")) cout << "\nUnable to load scanline overlay";
        scanlineObj.setTexture(scanlines);
        scanlines.setSmooth(true);

        if (!font.loadFromFile("Tiles/Font.png")) cout << "\nUnable to load font tileset.";
        if (!titleScreen.loadFromFile("Tiles/Title Screen.png")) cout << "\nUnable to load title screen tileset.";
        if (!menu.loadFromFile("Tiles/Menu.png")) cout << "\nUnable to load font tileset.";
        if (!controls.loadFromFile("Tiles/Controls.png")) cout << "\nUnable to load controls tileset.";
        if (!settings.loadFromFile("Tiles/Settings.png")) cout << "\nUnable to load settings tileset.";

        loadTilemap("Tiles/Title Screen.txt", 0);
        loadTilemap("Tiles/Main Menu.txt", 1);

        if (showDebugInfo) cout << "done.";
    }

    loadTilemap("Tiles/Title Screen.txt", 0);
    loadTilemap("Tiles/Main Menu.txt", 1);

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
            
            // Gameplay
            
            // Controls
        case -1: Controls(); break;
        case -2: MapControls(); break;
            // Settings
        case -10: GfxSettings(); break;

        default:
            buffer.clear(sf::Color::Blue);
            drawText(0, 0, "Error: unrecognized game state.", sf::Color::White);
            drawText(0, 16, "Screen: " + to_string(screen), sf::Color::Cyan);
        }

        // Update graphics
        buffer.display();
        bufferObj.setTexture(buffer.getTexture());
        window.draw(bufferObj);
        if (showScanlines) window.draw(scanlineObj);
        window.display();
        window.clear(sf::Color::Black);
    }

    cout << "\n\n\nThank you for playing!\n\n\n";
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
void readInput() {
    // Reset from last frame
    for(int i = 0; i < sizeof(keysPressed) / sizeof(keysPressed[0]); i++) {
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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            keysPressed[a] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            keysPressed[x] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R))
            keysPressed[b] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::F))
            keysPressed[y] = true;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter))
            keysPressed[start] = true;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            keysPressed[slct] = true;
    }

    // Controller
    if(sf::Joystick::isConnected(0)) {
        if(sf::Joystick::isButtonPressed(0, ctrlMap[a]))
            keysPressed[a] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[b]))
            keysPressed[b] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[x]))
            keysPressed[x] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[y]))
            keysPressed[y] = true;

        if(sf::Joystick::isButtonPressed(0, ctrlMap[lb]))
            keysPressed[lb] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[rb]))
            keysPressed[rb] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[slct]))
            keysPressed[slct] = true;
        if(sf::Joystick::isButtonPressed(0, ctrlMap[start]))
            keysPressed[start] = true;

        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) * ctrlMap[0] > 50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) * ctrlMap[2] > 50)
            keysPressed[up] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::Y) * ctrlMap[0] < -50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovY) * ctrlMap[2] < -50)
            keysPressed[dn] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) * ctrlMap[1] > 50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) * ctrlMap[3] > 50)
            keysPressed[lt] = true;
        if(sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) * ctrlMap[1] < -50 || sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) * ctrlMap[3] < -50)
            keysPressed[rt] = true;
    }
}
void MapControls() {
    if (!sf::Joystick::isConnected(0)) {
        screen = -1;
        if (showDebugInfo) {
            cout << "\n No joystick found to map.";
        }
    }

    if (showDebugInfo) cout << "\nMapped buttons:" << mappedButtons;

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
        }
        if (sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::PovX) < -50.f) {
            ctrlMap[3] = 1;
            mappedButtons++;
        }
        break;

        // Cleanup
    case 12:
        screen++;
        saveControlMap();
        slction = 1;
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

int updateFrameTime() {
    sf::Time frametime = clk.getElapsedTime();
    clk.restart();

    return frametime.asMilliseconds();
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
    drawTilemapScreen(titleScreen, 0);

    if((keysPressed[start] || keysPressed[a]) && inputTimer == 0) {
        screen++;
        inputTimer = 250;
    }
}
void MainMenu() {
    drawTilemapScreen(menu, 1);

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
    drawHighlightBox(4, 3 + slction * 2, 7);

    // Menu functionality
    if ((keysPressed[a] || keysPressed[start]) && inputTimer == 0) { // slct option
        switch (slction) {
        case 0: screen++;
            break;
        case 1:
            screen = -1;
            loadTilemap("Tiles/Controls.txt", 0);
            loadTilemap("Tiles/Controls Menu.txt", 1);
            break;
        case 2:
            screen = -10;
            loadTilemap("Tiles/Graphics Settings.txt", 0);
            loadTilemap("Tiles/Graphics Settings Bottom.txt", 1);
            break;
        case 3: window.close();
        }
        inputTimer = 250;
        slction = 0;
    }
    if (keysPressed[up] && inputTimer == 0) { // Move slction down
        slction--;
        if (slction < 0) slction = 3;
        inputTimer = 200;
    }
    if (keysPressed[dn] && inputTimer == 0) { // Move slction up
        slction++;
        if (slction > 3) slction = 0;
        inputTimer = 200;
    }
}
void Controls() {
    drawTilemapScreen(controls, 0);
    drawTilemapScreen(menu, 1);

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
    drawHighlightBox(slction * 9, 11, 9 - slction * 3);

    // Menu Functionality
    if ((keysPressed[a] || keysPressed[start]) && inputTimer == 0) {
        switch (slction) {
        case 0: // Map controller
            screen--;
            mappedButtons = 0;
            break;
        case 1: // Return to main menu
            screen = 0;
            slction = 0;
            inputTimer = 200;
            loadTilemap("Tiles/Title Screen.txt", 0);
            loadTilemap("Tiles/Main Menu.txt", 1);
            break;
        }

        slction = 0;
    }
    if (keysPressed[lt] && inputTimer == 0 && slction > 0) {
        slction--;
        inputTimer = 200;
    }
    if (keysPressed[rt] && inputTimer == 0 && slction < 1) {
        slction++;
        inputTimer = 200;
    }
}
void GfxSettings() {
    int xSize = 256, ySize = 224;
    string line, vsync;

    // Draw menu
    drawTilemapScreen(settings, 0);
    drawTilemapScreen(menu, 1);
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
    if (keysPressed[up] && inputTimer == 0) {
        slction--;
        inputTimer = 200;
    }
    if (keysPressed[dn] && inputTimer == 0) {
        slction++;
        inputTimer = 200;
    }
    if (keysPressed[rt] && slction == 5 && inputTimer == 0) {
        slction++;
        inputTimer = 200;
    }
    if (keysPressed[lt] && slction == 6 && inputTimer == 0) {
        slction--;
        inputTimer = 200;
    }
    switch (slction) {
    case -1: slction = 6; break;
    case 0: // Aspect Ratio
        if (keysPressed[lt] && inputTimer == 0) {
            aspectRatio--;
            if (aspectRatio < 0) aspectRatio = 2;
            inputTimer = 200;
        }
        if ((keysPressed[rt] || keysPressed[a]) && inputTimer == 0) {
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
        if (keysPressed[lt] && inputTimer == 0 && scale > 50) {
            scale -= 25;
            inputTimer = 200;
        }
        if (keysPressed[rt] && inputTimer == 0) {
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
        if (keysPressed[lt] && inputTimer == 0 && frameRateIndex > 0) {
            frameRateIndex--;
            inputTimer = 200;
        }
        if (keysPressed[rt] && inputTimer == 0 && frameRateIndex < sizeof(stdFrameRate) / sizeof(stdFrameRate[0] - 1)) {
            frameRateIndex++;
            inputTimer = 200;
        }

        maxFrameRate = stdFrameRate[frameRateIndex];
        window.setFramerateLimit(maxFrameRate);
        window.setVerticalSyncEnabled(maxFrameRate == 0); // Enable V-Sync if frame rate is uncapped
        break;

    case 3: // Scanlines
        if ((keysPressed[rt] || keysPressed[lt] || keysPressed[a]) && inputTimer == 0) {
            showScanlines = !showScanlines;
            inputTimer = 200;
        }
        break;

    case 4: // Blur
        if ((keysPressed[rt] || keysPressed[lt] || keysPressed[a]) && inputTimer == 0) {
            blur = !blur;
            if (blur) showScanlines = true;
            inputTimer = 200;
        }
        break;

    case 5: // Save settings
        if ((keysPressed[a] || keysPressed[start]) && inputTimer == 0) {
            saveGfxSettings();
            inputTimer = 200;
        }
        break;

    case 6: // Return to main menu
        if ((keysPressed[a] || keysPressed[start]) && inputTimer == 0) {
            screen = 0;
            slction = 0;
            inputTimer = 200;
            loadTilemap("Tiles/Title Screen.txt", 0);
            loadTilemap("Tiles/Main Menu.txt", 1);
        }
        break;
    case 7: slction = 0;
    }
    buffer.setSmooth(blur);

    // Indicate slcted options
    {
        if (slction < 6) drawHighlightBox(0, 1 + 2 * slction, 15 - 6 * (slction == 5));
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