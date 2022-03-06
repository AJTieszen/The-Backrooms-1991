// The Backrooms - 1991.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;

#include <SFML/Graphics.hpp>

// Startup settings
const string title = "The Backrooms: 1991";
bool showDebugInfo = true, toggleDebugInfo = false;
int frameRate = 60;

// Global SFML & graphics objects
sf::RenderTexture buffer;
sf::Sprite bufferObj;

int tilemap[4][64][64];

// Graphics assets
sf::Texture titleScreen;

// Function definitions
void drawTilemapScreen(sf::Texture, int layer);

int main() {
    // Game state
    int screen = 0;

    // Create window
    sf::RenderWindow window(sf::VideoMode(512, 448), title);
    window.setFramerateLimit(frameRate);

    // Print startup info to terminal
    cout << "Opening " << title << ". Press TAB to toggle debug information.\n";

    // Load graphics
    {
        if(!titleScreen.loadFromFile("Tiles/Title Screen.png")) throw("Unable to load file.");
    }

    // Initialize framebuffer
    buffer.create(256,224);
    bufferObj.setScale(2.f, 2.f);

    while(window.isOpen()) {
        // System window management
        sf::Event event;
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
        drawTilemapScreen(titleScreen, 0);

        // Update graphics
        window.clear(sf::Color::Black);
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

    for(int x = 0; x < 14; x++) {
        for(int y = 0; y < 14; y++) {
            tileID = tilemap[layer][x][y];
            tile.setPosition(16.f * x, 16.f * y);

            tileX = (tileID % 16); tileX *= 16;
            tileY = (tileID % 16); tileY *= 16;

            tile.setTextureRect(sf::IntRect(tileX, tileY, 16, 16));

            buffer.draw(tile);
        }
    }
}