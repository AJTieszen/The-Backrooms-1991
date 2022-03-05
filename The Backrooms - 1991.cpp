// The Backrooms - 1991.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;

#include <SFML/Graphics.hpp>

// Startup Settings
const string title = "The Backrooms: 1991";
int frameRate = 60;

// Global SFML Objects
sf::RenderTexture buffer;


int main()
{
    sf::RenderWindow window (sf::VideoMode(512, 448), title);
    window.setFramerateLimit (frameRate);

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent (event)) {
            if (event.type == sf::Event::Closed) window.close();
        }

        window.clear (sf::Color::Black);
        window.display();
    }
}
