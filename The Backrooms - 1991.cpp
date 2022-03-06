// The Backrooms - 1991.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <sstream>
using namespace std;

#include <SFML/Graphics.hpp>

// Startup settings / defaults
const string title = "The Backrooms: 1991";
bool showDebugInfo = false, toggleDebugInfo = false;
int maxFrameRate = 60;

// Global SFML & graphics objects
sf::RenderTexture buffer;
sf::Sprite bufferObj;

int tilemap[4][64][64];

// Graphics assets
sf::Texture titleScreen;
sf::Texture font;

// Function declarations
void drawTilemapScreen(sf::Texture, int layer);
void drawText(int x, int y, string text, sf::Color color);
void drawText(int x, int y, string text);
void loadTilemap(string filename, int layer);



int main() {
    // Game state
    int screen = 0;

    // Print startup info to terminal
    cout << "Opening " << title << ". Press TAB to toggle debug information.\n";

    // Create window
    if(showDebugInfo) cout << "Creating window...";

    sf::RenderWindow window(sf::VideoMode(512, 448), title);
    window.setFramerateLimit(maxFrameRate);
    buffer.create(256, 224);
    bufferObj.setScale(2.f, 2.f);

    if(showDebugInfo) cout << "done.";

    // Load graphics
    {
        if(showDebugInfo) cout << "\nLoading graphics...";

        if (!font.loadFromFile("Tiles/Font.png")) throw("Unable to load font tileset.");
        if (!titleScreen.loadFromFile("Tiles/Title Screen.png")) throw("Unable to load title screen tileset.");

        if(showDebugInfo) cout << "done.";
    }

    loadTilemap("Tiles/Title Screen.txt", 0);

    while(window.isOpen()) {
        // System window management
        window.clear(sf::Color::Black);
        sf::Event event;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::End)) window.close();
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
        if(screen == 0) {
            drawTilemapScreen(titleScreen, 0);
            drawText(32, 192, "Text rendering works!", sf::Color::Cyan);
        }

        // Update graphics
        buffer.display();
        bufferObj.setTexture(buffer.getTexture());
        window.draw(bufferObj);
        window.display();
    }
}



// Function definitions
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

    for (int i = 0; i < txt.length(); i++) {
        int c = txt[i] - 32;
        cx = c % 16; cx *= 8;
        cy = c / 16; cy *= 16;

        text.setTextureRect(sf::IntRect(cx, cy, 8, 16));
        text.setPosition(x, y);
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

    } else throw("Unable to open tilemap.");
}