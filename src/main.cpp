#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#define WINDOW_H 600
#define WINDOW_W 1200

#define GAME_W 300
#define GAME_H 300

#define pi 3.141592653589f
#define deg pi / 180.0f

using namespace std;

sf::RenderWindow window;
sf::Texture imgBuffer;
sf::Sprite imgSprite;
sf::Clock deltaClock;
sf::Uint8 pixArray[WINDOW_W * WINDOW_H * 4];

vector<sf::Vertex> pVertices;
vector<sf::Vertex> line;

struct keysPrsd {
    bool z;
    bool q;
    bool s;
    bool d;
};
keysPrsd keys;

struct Ray {
    float x;
    float y;
    float ox;
    float oy;
    float a;
    float dist;
};

// ~~ Données du jeu ~~ //

float px(150.0f); // Position X du joueur sur la carte
float py(150.0f); // Position Y du joueur sur la carte
float pa;         // Angle de vue du joueur en radians

float speed(0.05f); // Vitesse
float dTime(0.0f);

int pHeight = 8; // Hauteur du carré
int pWidth = 8;  // Largeur du carré

int mapWall[64] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 1,
                   1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1,
                   1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 1,
                   1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int mapWidth = 8;
int mapHeight = 8;

int wallTex[4] = {1, 0, 1, 0};
int wallTexWidth = 2;
int wallTexHeight = 2;

float wallHeight = 1.0f;

// ~~ Fonctions du jeu ~~ //

void gameLoop();
void gameInput();
void gameUpdate();

void movePlayer();
void drawBlocks();
void castRays();
float normAngle(float a);
float calcDist(float x1, float y1, float x2, float y2);

int main(int argc, char const *argv[]) {
    window.create(sf::VideoMode(WINDOW_W, WINDOW_H), "KitRayCaster");
    imgBuffer.create(WINDOW_W, WINDOW_H);
    imgSprite.setTexture(imgBuffer);

    while (window.isOpen()) {
        gameLoop();
    }

    return 0;
}

void gameLoop() {
    gameInput();

    window.clear(sf::Color::White);
    // gameUpdate();
    window.draw(imgSprite);
    drawBlocks();
    movePlayer();
    castRays();
    window.display();

    dTime = deltaClock.getElapsedTime().asSeconds() * 1000.0f;
    deltaClock.restart();
}

void gameInput() {
    sf::Event e;
    while (window.pollEvent(e)) {
        if (e.type == sf::Event::Closed || (e.type == sf::Event::KeyPressed &&
                                            e.key.code == sf::Keyboard::X)) {
            window.close();
        }

        if (e.type == sf::Event::KeyPressed) {
            if (e.key.code == sf::Keyboard::X) {
                window.close();
            }

            if (e.key.code == sf::Keyboard::Z) {
                keys.z = true;
            }

            if (e.key.code == sf::Keyboard::S) {
                keys.s = true;
            }

            if (e.key.code == sf::Keyboard::D) {
                keys.d = true;
            }

            if (e.key.code == sf::Keyboard::Q) {
                keys.q = true;
            }
        }

        if (e.type == sf::Event::KeyReleased) {
            if (e.key.code == sf::Keyboard::Z) {
                keys.z = false;
            }

            if (e.key.code == sf::Keyboard::S) {
                keys.s = false;
            }

            if (e.key.code == sf::Keyboard::D) {
                keys.d = false;
            }

            if (e.key.code == sf::Keyboard::Q) {
                keys.q = false;
            }
        }
    }
}

void gameUpdate() {
    pVertices.clear();
    pVertices.push_back(sf::Vertex(
        sf::Vector2f(px - pWidth / 2, py - pHeight / 2), sf::Color::Red));
    pVertices.push_back(sf::Vertex(
        sf::Vector2f(px + pWidth / 2, py - pHeight / 2), sf::Color::Red));
    pVertices.push_back(sf::Vertex(
        sf::Vector2f(px + pWidth / 2, py + pHeight / 2), sf::Color::Red));
    pVertices.push_back(sf::Vertex(
        sf::Vector2f(px - pWidth / 2, py + pHeight / 2), sf::Color::Red));

    line.clear();
    line.push_back(sf::Vertex(sf::Vector2f(px, py), sf::Color::Red));
    line.push_back(sf::Vertex(
        sf::Vector2f(px + 20 * cos(pa), py + 20 * sin(pa)), sf::Color::Red));

    window.draw(&pVertices[0], pVertices.size(), sf::Quads);
    window.draw(&line[0], line.size(), sf::LineStrip);
}

void movePlayer() {
    float dX = speed * dTime * cos(pa);
    float dY = speed * dTime * sin(pa);
    float dA = speed * dTime * 0.05f;

    if (keys.z) {
        px += dX;
        py += dY;
    }

    if (keys.s) {
        px -= dX;
        py -= dY;
    }

    if (keys.d) {
        pa += dA;
        pa = normAngle(pa);
    }

    if (keys.q) {
        pa -= dA;
        pa = normAngle(pa);
    }

    // cout << dTime << endl;
}

void drawBlocks() {
    float blockWidth = GAME_W / mapWidth;
    float blockHeight = GAME_H / mapHeight;

    sf::RectangleShape block(sf::Vector2f(blockWidth, blockHeight));

    sf::Color bgGrey(0x55, 0x55, 0x55);

    for (int x = 0; x < mapWidth; x++) {
        for (int y = 0; y < mapHeight; y++) {
            block.setPosition(sf::Vector2f(x * blockWidth, y * blockHeight));

            if (mapWall[y * mapWidth + x] > 0) {
                block.setFillColor(sf::Color::Black);
            } else {
                block.setFillColor(bgGrey);
            }

            window.draw(block);
        }
    }
}

void castRays() {
    Ray ray;
    Ray hRay;
    Ray vRay;

    int mx, my, mp;

    bool hit(false);
    int side; // 0 for NS, 1 for EW
    int r(0);

    int nbOfRay = 90;

    ray.a = normAngle(normAngle(pa) - (nbOfRay / 2) * deg);

    int boxH = GAME_H / mapHeight;
    int boxW = GAME_W / mapWidth;

    sf::Vertex rayLine[2];
    rayLine[0] = sf::Vertex(sf::Vector2f(px, py), sf::Color::Red);

    float shade;
    float pixW = 8;
    float pixH = 8;

    while (r < nbOfRay) {
        // HORIZONTAL
        hit = false;
        float atan = -1 / tan(ray.a);
        if (ray.a > pi) { // Quand on regarde vers le haut
            ray.y = (int)(py / boxH) * boxH - 0.0001;
            ray.x = px + (py - ray.y) * atan;

            ray.oy = -boxH;
            ray.ox = -ray.oy * atan;
        }
        if (ray.a < pi) { // Quand on regarde vers le bas
            ray.y = (int)(py / boxH) * boxH + boxH;
            ray.x = px + (py - ray.y) * atan;

            ray.oy = boxH;
            ray.ox = -ray.oy * atan;
        }
        if (ray.a == 0 || ray.a == 2 * pi) { // Quand on regarde sur les côtés
            ray.y = py;
            ray.x = px;

            ray.oy = 0;
            ray.ox = boxW;
        }
        while (!hit) {
            mx = (int)(ray.x / boxW);
            my = (int)(ray.y / boxH);
            mp = clamp(my * mapWidth + mx, 0, mapWidth * mapHeight);

            if (mapWall[mp] != 0) {
                hit = true;
                hRay.x = ray.x;
                hRay.y = ray.y;
                hRay.dist = calcDist(px, py, hRay.x, hRay.y);
            } else {
                ray.y += ray.oy;
                ray.x += ray.ox;
            }
        }

        // VERTICAL
        hit = false;
        float ntan = -tan(ray.a);
        if (ray.a > pi / 2 &&
            ray.a < 3 * pi / 2) { // Quand on regarde vers la gauche

            ray.x = (int)(px / boxW) * boxW - 0.0001;
            ray.y = py + (px - ray.x) * ntan;

            ray.ox = -boxW;
            ray.oy = -ray.ox * ntan;
        }
        if (ray.a < pi / 2 ||
            ray.a > 3 * pi / 2) { // Quand on regarde vers la droite

            ray.x = (int)(px / boxW) * boxW + boxW;
            ray.y = py + (px - ray.x) * ntan;

            ray.ox = boxW;
            ray.oy = -ray.ox * ntan;
        }
        if (ray.a == pi / 2 ||
            ray.a == 3 * pi / 2) { // Quand on regarde sur les côtés
            ray.y = py;
            ray.x = px;

            ray.oy = boxH;
            ray.ox = 0;
        }
        while (!hit) {
            mx = (int)(ray.x / boxW);
            my = (int)(ray.y / boxH);
            mp = clamp(my * mapWidth + mx, 0, mapWidth * mapHeight);

            if (mapWall[mp] != 0) {
                hit = true;
                vRay.x = ray.x;
                vRay.y = ray.y;
                vRay.dist = calcDist(px, py, vRay.x, vRay.y);
            } else {
                ray.y += ray.oy;
                ray.x += ray.ox;
            }
        }

        if (hRay.dist < vRay.dist) {
            side = 0;
            ray.x = hRay.x;
            ray.y = hRay.y;
            ray.dist = hRay.dist;
        } else {
            side = 1;
            ray.x = vRay.x;
            ray.y = vRay.y;
            ray.dist = vRay.dist;
        }

        rayLine[1] = sf::Vertex(sf::Vector2f(ray.x, ray.y), sf::Color::Green);
        window.draw(&rayLine[0], 2, sf::LineStrip);

        if (side) {
            shade = 0.5f;
        } else {
            shade = 1;
        }

        sf::Color wallColor(0x00, 0x00, 0xFF * shade);

        float ca = normAngle(pa - ray.a);
        ray.dist *= cos(ca);
        float lineH = (WINDOW_H / ray.dist) * wallHeight;

        sf::RectangleShape blockLine(sf::Vector2f(8, WINDOW_H / 2));
        blockLine.setPosition(sf::Vector2f(r * pixW + 400, 0));
        blockLine.setFillColor(sf::Color(0x00, 0xF0, 0xF0));
        window.draw(blockLine);

        blockLine.setPosition(sf::Vector2f(r * pixW + 400, WINDOW_H / 2));
        blockLine.setFillColor(sf::Color(0xBB, 0xBB, 0x00));
        window.draw(blockLine);

        for (int tex_y = 0; tex_y < lineH; tex_y++) {
            /* blockLine.setSize(sf::Vector2f(pixW, pixH));
            blockLine.setPosition(
                sf::Vector2f(pixW * r + 400, (WINDOW_H / 2 - lineH / 2) + tex_y
            * pixH)); blockLine.setFillColor(wallColor); */

            float pixX = pixW * r + 400;
            float pixY = (WINDOW_H / 2 - lineH / 2) + tex_y * pixH;
            pixArray[(int)(pixY * WINDOW_W + pixX)] = wallColor.r;
            pixArray[(int)(pixY * WINDOW_W + pixX) + 1] = wallColor.g;
            pixArray[(int)(pixY * WINDOW_W + pixX) + 2] = wallColor.b;
            pixArray[(int)(pixY * WINDOW_W + pixX) + 3] = wallColor.a;
            imgBuffer.update(pixArray);
        }

        ray.a += deg;
        ray.a = normAngle(ray.a);
        r++;
    }
}

float normAngle(float a) {
    float b = a;
    if (b < 0) {
        b += 2 * pi;
    }
    if (b > 2 * pi) {
        b -= 2 * pi;
    }
    return b;
}

float calcDist(float x1, float y1, float x2, float y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}
