
// Lab08.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFPhysics.h>
#include <vector>
#include <SFML/Audio.hpp>

using namespace std;
using namespace sf;
using namespace sfp;

const float KB_SPEED = 0.2;

void LoadTex(Texture& tex, string filename) {
    if (!tex.loadFromFile(filename)) {
        cout << "Could not load " << filename << endl;
    }
}

void MoveCrossbow(PhysicsSprite& crossbow, int elapsedMS) {
    if (Keyboard::isKeyPressed(Keyboard::Right)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x + (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
    if (Keyboard::isKeyPressed(Keyboard::Left)) {
        Vector2f newPos(crossbow.getCenter());
        newPos.x = newPos.x - (KB_SPEED * elapsedMS);
        crossbow.setCenter(newPos);
    }
}

Vector2f GetTextSize(Text text) {
    FloatRect r = text.getGlobalBounds();
    return Vector2f(r.width, r.height);
}

int main()
{
    RenderWindow window(VideoMode(800, 600), "Duck Hunter");
    World world(Vector2f(0, 0));
    int score(0);
    int arrows(5); // Player starts with 5 arrows

    SoundBuffer popBuffer;
    if (!popBuffer.loadFromFile("balloonpop.ogg")) {
        cout << "coul not load balloonpop.ogg" << endl;
        exit(5);
    }
    Sound popSound;
    popSound.setBuffer(popBuffer);

    Music music;
    if (!music.openFromFile("circus.ogg")) {
        cout << "Failed to load circus.ogg ";
        exit(6);
    }
    music.play();

    PhysicsSprite& crossBow = *new PhysicsSprite();
    Texture cbowTex;
    LoadTex(cbowTex, "images/crossbow.png");
    crossBow.setTexture(cbowTex);
    Vector2f sz = crossBow.getSize();
    crossBow.setCenter(Vector2f(400,
        600 - (sz.y / 2)));

    PhysicsSprite arrow;
    Texture arrowTex;
    LoadTex(arrowTex, "images/arrow.png");
    arrow.setTexture(arrowTex);
    bool drawingArrow(false);

    PhysicsRectangle top;
    top.setSize(Vector2f(800, 10));
    top.setCenter(Vector2f(400, 5));
    top.setStatic(true);
    world.AddPhysicsBody(top);

    // Right boundary removes ducks that leave the screen
    PhysicsRectangle right;
    right.setSize(Vector2f(10, 600));
    right.setCenter(Vector2f(795, 300));
    right.setStatic(true);
    world.AddPhysicsBody(right);

    Texture duckTex;
    LoadTex(duckTex, "images/duck.png");
    PhysicsShapeList<PhysicsSprite> ducks;

    top.onCollision = [&drawingArrow, &world, &arrow]
    (PhysicsBodyCollisionResult result) {
        if (result.object2 == arrow && drawingArrow) {
            drawingArrow = false;
            world.RemovePhysicsBody(arrow);
        }
        };

    Font fnt;
    if (!fnt.loadFromFile("arial.ttf")) {
        cout << "Could not load font." << endl;
        exit(3);
    }

    Clock clock;
    Time lastTime(clock.getElapsedTime());
    Time currentTime(lastTime);

    long duckMS(0); // Timer for spawning ducks
    bool spacePressed(false);

    while ((arrows > 0) || drawingArrow) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                return 0;
            }
        }

        currentTime = clock.getElapsedTime();
        Time deltaTime = currentTime - lastTime;
        long deltaMS = deltaTime.asMilliseconds();

        if (deltaMS > 9) {
            lastTime = currentTime;
            duckMS = duckMS + deltaMS;

            // Spawn a duck every 1.5 seconds
            if (duckMS > 1500) {
                duckMS = 0;

                PhysicsSprite& duck = ducks.Create();
                duck.setTexture(duckTex);
                Vector2f sz = duck.getSize();
                duck.setCenter(Vector2f(-sz.x / 2, 20 + (sz.y / 2)));
                duck.setVelocity(Vector2f(0.25, 0));
                world.AddPhysicsBody(duck);

                duck.onCollision =
                    [&drawingArrow, &world, &arrow, &duck, &ducks, &score, &right, &popSound]
                    (PhysicsBodyCollisionResult result) {
                    if (result.object2 == arrow && drawingArrow) {
                        popSound.play();
                        drawingArrow = false;
                        world.RemovePhysicsBody(arrow);
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                        score += 10;
                    }
                    else if (result.object2 == right) {
                        world.RemovePhysicsBody(duck);
                        ducks.QueueRemove(duck);
                    }
                    };
            }

            world.UpdatePhysics(deltaMS);
            MoveCrossbow(crossBow, deltaMS);

            if (Keyboard::isKeyPressed(Keyboard::Space) &&
                !drawingArrow && arrows > 0 && !spacePressed) {
                drawingArrow = true;
                arrows = arrows - 1;
                arrow.setCenter(crossBow.getCenter());
                arrow.setVelocity(Vector2f(0, -1));
                world.AddPhysicsBody(arrow);
            }

            // Prevent holding space from shooting repeatedly
            spacePressed = Keyboard::isKeyPressed(Keyboard::Space);

            window.clear();
            if (drawingArrow) {
                window.draw(arrow);
            }

            ducks.DoRemovals();
            for (PhysicsShape& duck : ducks) {
                window.draw((PhysicsSprite&)duck);
            }

            window.draw(crossBow);

            Text scoreText;
            scoreText.setString(to_string(score));
            scoreText.setFont(fnt);
            scoreText.setPosition(Vector2f(790 - GetTextSize(scoreText).x, 550));
            window.draw(scoreText);

            Text arrowCountText;
            arrowCountText.setString(to_string(arrows));
            arrowCountText.setFont(fnt);
            arrowCountText.setPosition(Vector2f(10, 550));
            window.draw(arrowCountText);

            //world.VisualizeAllBounds(window);

            window.display();
        }
    }

    Text gameOverText;
    gameOverText.setString("GAME OVER");
    gameOverText.setFont(fnt);
    sz = GetTextSize(gameOverText);
    gameOverText.setPosition(400 - (sz.x / 2), 300 - (sz.y / 2));
    window.draw(gameOverText);
    window.display();

    // Keep GAME OVER on screen until space is pressed again
    bool spaceReleased(false);

    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }

        if (!Keyboard::isKeyPressed(Keyboard::Space)) {
            spaceReleased = true;
        }

        if (spaceReleased && Keyboard::isKeyPressed(Keyboard::Space)) {
            window.close();
        }
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu
