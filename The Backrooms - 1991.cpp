// The Backrooms - 1991.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;

#include <SFML/Graphics.hpp>

// Startup settings
const string title = "The Backrooms: 1991";
bool showDebugInfo = false, toggleDebugInfo = false;
int frameRate = 60;

// Global SFML objects
sf::RenderTexture buffer;


int main() {
    // Create window
    sf::RenderWindow window (sf::VideoMode(512, 448), title);
    window.setFramerateLimit (frameRate);

    // Print startup info to terminal
    cout << "Opening " << title << ". Press TAB to toggle debug information.\n";

    while (window.isOpen()) {
        // System window management
        sf::Event event;
        while (window.pollEvent (event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        // Toggle debug output
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Tab)) {
            if (!toggleDebugInfo) {
                showDebugInfo = !showDebugInfo;
                cout << "\n   > Show debug info : " << showDebugInfo;
            }
            toggleDebugInfo = true;
        } else toggleDebugInfo = false;

        // Game logc

        // Update graphics
        window.clear (sf::Color::Black);
        window.display();
    }
}
