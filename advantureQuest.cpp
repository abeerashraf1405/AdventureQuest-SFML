/*
 * -----------------------------------------------------------------------------
 * Project: Adventure Quest - Spiral Path (GUI Edition)
 * Author: Abeer
 * University: FAST National University of Computer and Emerging Sciences
 * Date: February 2026
 * * Copyright (c) 2026 Abeer. All rights reserved.
 * * This software is provided for educational and portfolio purposes. 
 * Unauthorized copying of this file, via any medium, is strictly prohibited.
 * -----------------------------------------------------------------------------
 */

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <cmath>
#include <functional>

using namespace std;

// ─────────────────────────────────────────────────────────────────────────────
// Constants
// ─────────────────────────────────────────────────────────────────────────────
const int MIN_BOARD_SIZE   = 5;
const int MAX_BOARD_SIZE   = 11;
const int MAX_ITEMS        = 20;
const int MAX_PATH_LENGTH  = 121;
const int MAX_INVENTORY    = 5;
const int WIN_W            = 1100;
const int WIN_H            = 720;

// ─────────────────────────────────────────────────────────────────────────────
// Forward declarations
// ─────────────────────────────────────────────────────────────────────────────
class Player;
class GameItem;
class HelperObject;
class Sword;
class GameBoard;
class PadLock;      

// ─────────────────────────────────────────────────────────────────────────────
// GameItem — abstract base
// ─────────────────────────────────────────────────────────────────────────────
class GameItem {
protected:
    string name;
    int x, y;
public:
    GameItem(string n, int xPos, int yPos) : name(n), x(xPos), y(yPos) {}
    virtual ~GameItem() {}
    virtual GameItem* clone() const = 0;
    string getName() const { return name; }
    virtual void interact(Player* player) = 0;
    int getX() const { return x; }
    int getY() const { return y; }
    void setPosition(int nx, int ny) { x = nx; y = ny; }
};

// ─────────────────────────────────────────────────────────────────────────────
// HelperObject base
// ─────────────────────────────────────────────────────────────────────────────
class HelperObject : public GameItem {
protected:
    int pointsCost;
    bool used;
public:
    HelperObject(string n, int xPos, int yPos, int cost)
        : GameItem(n, xPos, yPos), pointsCost(cost), used(false) {}
    virtual ~HelperObject() {}
    int getPointsCost() const { return pointsCost; }
    bool isUsed() const  { return used; }
    void setUsed()       { used = true; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Sword
// ─────────────────────────────────────────────────────────────────────────────
class Sword : public HelperObject {
private:
    int usesLeft;
public:
    Sword(int xPos, int yPos) : HelperObject("Sword", xPos, yPos, 40), usesLeft(2) {}
    GameItem* clone() const override { return new Sword(*this); }
    void interact(Player* player) override;
    int getUsesLeft() const { return usesLeft; }
    void use() { if (usesLeft > 0) usesLeft--; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Shield
// ─────────────────────────────────────────────────────────────────────────────
class Shield : public HelperObject {
public:
    Shield(int xPos, int yPos) : HelperObject("Shield", xPos, yPos, 30) {}
    GameItem* clone() const override { return new Shield(*this); }
    void interact(Player* player) override;
};

// ─────────────────────────────────────────────────────────────────────────────
// Water
// ─────────────────────────────────────────────────────────────────────────────
class Water : public HelperObject {
public:
    Water(int xPos, int yPos) : HelperObject("Water", xPos, yPos, 50) {}
    GameItem* clone() const override { return new Water(*this); }
    void interact(Player* player) override;
};

// ─────────────────────────────────────────────────────────────────────────────
// Key
// ─────────────────────────────────────────────────────────────────────────────
class Key : public HelperObject {
public:
    Key(int xPos, int yPos) : HelperObject("Key", xPos, yPos, 70) {}
    GameItem* clone() const override { return new Key(*this); }
    void interact(Player* player) override;
};

// ─────────────────────────────────────────────────────────────────────────────
// Player
// ─────────────────────────────────────────────────────────────────────────────
class Player {
private:
    string name;
    int x, y;
    int pathIndex;
    int pathX[MAX_PATH_LENGTH], pathY[MAX_PATH_LENGTH];
    int pathLength;
    int goldCoins, silverCoins, points;
    int skipTurns;
    HelperObject* inventory[MAX_INVENTORY];
    int inventorySize;

public:
    Player(string n, int startX, int startY, int gold, int silver)
        : name(n), x(startX), y(startY), pathIndex(0), pathLength(0),
          goldCoins(gold), silverCoins(silver), points(0), skipTurns(0), inventorySize(0) {
        for (int i = 0; i < MAX_INVENTORY; i++) inventory[i] = nullptr;
    }

    Player(const Player& o)
        : name(o.name), x(o.x), y(o.y), pathIndex(o.pathIndex),
          pathLength(o.pathLength), goldCoins(o.goldCoins),
          silverCoins(o.silverCoins), points(o.points),
          skipTurns(o.skipTurns), inventorySize(o.inventorySize) {
        for (int i = 0; i < pathLength; i++) {
            pathX[i] = o.pathX[i];
            pathY[i] = o.pathY[i];
        }
        for (int i = 0; i < MAX_INVENTORY; i++) {
            inventory[i] = o.inventory[i]
                ? dynamic_cast<HelperObject*>(o.inventory[i]->clone())
                : nullptr;
        }
    }

    ~Player() {
        for (int i = 0; i < MAX_INVENTORY; i++) delete inventory[i];
    }

    void setPath(int px[], int py[], int length) {
        pathLength = length;
        for (int i = 0; i < length; i++) { pathX[i] = px[i]; pathY[i] = py[i]; }
    }

    bool moveForward(int /*boardSize*/) {
        if (pathIndex + 1 < pathLength) {
            pathIndex++;
            x = pathX[pathIndex];
            y = pathY[pathIndex];
            return true;
        }
        return false;
    }

    void moveBackward(int steps, int /*boardSize*/) {
        pathIndex = (pathIndex - steps >= 0) ? pathIndex - steps : 0;
        x = pathX[pathIndex];
        y = pathY[pathIndex];
    }

    void addPoints(int p)         { points += p; }
    void deductPoints(int p)      { points = (points - p >= 0) ? points - p : 0; }
    void addGold(int a)           { goldCoins += a; }
    void addSilver(int a)         { silverCoins += a; }
    void deductGold(int a)        { goldCoins   = (goldCoins   - a >= 0) ? goldCoins   - a : 0; }
    void deductSilver(int a)      { silverCoins = (silverCoins - a >= 0) ? silverCoins - a : 0; }
    void setSkipTurns(int t)      { skipTurns = t; }
    void decrementSkipTurns()     { if (skipTurns > 0) skipTurns--; }

    bool addToInventory(HelperObject* obj) {
        if (inventorySize < MAX_INVENTORY) { inventory[inventorySize++] = obj; return true; }
        return false;
    }

    int     getX()             const { return x; }
    int     getY()             const { return y; }
    int     getPathIndex()     const { return pathIndex; }
    int     getPoints()        const { return points; }
    int     getGold()          const { return goldCoins; }
    int     getSilver()        const { return silverCoins; }
    int     getSkipTurns()     const { return skipTurns; }
    string  getName()          const { return name; }
    int     getInventorySize() const { return inventorySize; }
    HelperObject* getInventoryItem(int idx) const { return inventory[idx]; }

    void useHelperObject(int idx) {
        if (idx >= 0 && idx < inventorySize) {
            inventory[idx]->setUsed();
            if (Sword* sw = dynamic_cast<Sword*>(inventory[idx])) sw->use();
        }
    }

    void clearUsedItems() {
        for (int i = 0; i < inventorySize; i++) {
            if (inventory[i] && inventory[i]->isUsed()) {
                delete inventory[i];
                for (int j = i; j < inventorySize - 1; j++) inventory[j] = inventory[j + 1];
                inventory[inventorySize - 1] = nullptr;
                inventorySize--;
                i--;
            }
        }
    }

    GameItem* moveAndGetItem(int boardSize, GameBoard* board);
};

// ─────────────────────────────────────────────────────────────────────────────
// Coin
// ─────────────────────────────────────────────────────────────────────────────
class Coin : public GameItem {
private:
    int value;
    bool isGold;
public:
    Coin(string n, int xPos, int yPos, int val, bool gold)
        : GameItem(n, xPos, yPos), value(val), isGold(gold) {}
    GameItem* clone() const override { return new Coin(*this); }
    void interact(Player* player) override;
    int  getValue()      const { return value; }
    bool isGoldCoin()    const { return isGold; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Hurdle base
// ─────────────────────────────────────────────────────────────────────────────
class Hurdle : public GameItem {
protected:
    int blockTurns;
    int pointsCost;
public:
    Hurdle(string n, int xPos, int yPos, int turns, int cost)
        : GameItem(n, xPos, yPos), blockTurns(turns), pointsCost(cost) {}
    GameItem* clone() const override { return new Hurdle(*this); }
    int getBlockTurns() const { return blockTurns; }
    int getPointsCost() const { return pointsCost; }
    virtual void interact(Player* player) override {
        player->setSkipTurns(blockTurns);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Snake
// ─────────────────────────────────────────────────────────────────────────────
class Snake : public Hurdle {
public:
    Snake(int xPos, int yPos) : Hurdle("Snake", xPos, yPos, 3, 30) {}
    GameItem* clone() const override { return new Snake(*this); }
    void interact(Player* player) override;
};

// ─────────────────────────────────────────────────────────────────────────────
// Lion
// ─────────────────────────────────────────────────────────────────────────────
class Lion : public Hurdle {
public:
    Lion(int xPos, int yPos) : Hurdle("Lion", xPos, yPos, 4, 50) {}
    GameItem* clone() const override { return new Lion(*this); }
    void interact(Player* player) override { Hurdle::interact(player); }
};

// ─────────────────────────────────────────────────────────────────────────────
// Fire
// ─────────────────────────────────────────────────────────────────────────────
class Fire : public Hurdle {
public:
    Fire(int xPos, int yPos) : Hurdle("Fire", xPos, yPos, 2, 50) {}
    GameItem* clone() const override { return new Fire(*this); }
    void interact(Player* player) override;
};

// ─────────────────────────────────────────────────────────────────────────────
// Ghost
// ─────────────────────────────────────────────────────────────────────────────
class Ghost : public Hurdle {
public:
    Ghost(int xPos, int yPos) : Hurdle("Ghost", xPos, yPos, 1, 20) {}
    GameItem* clone() const override { return new Ghost(*this); }
    void interact(Player* player) override { Hurdle::interact(player); }
};

// ─────────────────────────────────────────────────────────────────────────────
// PadLock  (was "Lock" — renamed to avoid conflict with sf::Lock)
// ─────────────────────────────────────────────────────────────────────────────
class PadLock : public Hurdle {
public:
    PadLock(int xPos, int yPos) : Hurdle("Lock", xPos, yPos, 999, 60) {}
    GameItem* clone() const override { return new PadLock(*this); }
    void interact(Player* player) override;
};

// ─────────────────────────────────────────────────────────────────────────────
// GameBoard
// ─────────────────────────────────────────────────────────────────────────────
class GameBoard {
private:
    GameItem* items[MAX_ITEMS];
    int itemCount;
    GameItem*** board;
    int size;

public:
    GameBoard(int s) : size(s), itemCount(0) {
        board = new GameItem**[size];
        for (int i = 0; i < size; i++) {
            board[i] = new GameItem*[size];
            for (int j = 0; j < size; j++) board[i][j] = nullptr;
        }
        for (int i = 0; i < MAX_ITEMS; i++) items[i] = nullptr;
    }

    GameBoard(const GameBoard& o) : size(o.size), itemCount(o.itemCount) {
        board = new GameItem**[size];
        for (int i = 0; i < size; i++) {
            board[i] = new GameItem*[size];
            for (int j = 0; j < size; j++) {
                board[i][j] = o.board[i][j] ? o.board[i][j]->clone() : nullptr;
            }
        }
        for (int i = 0; i < itemCount; i++)
            items[i] = o.items[i] ? o.items[i]->clone() : nullptr;
    }

    ~GameBoard() {
        for (int i = 0; i < itemCount; i++) delete items[i];
        for (int i = 0; i < size; i++) delete[] board[i];
        delete[] board;
    }

    void placeItem(GameItem* item) {
        if (itemCount >= MAX_ITEMS) { delete item; return; }
        int x = item->getX(), y = item->getY();
        if (x >= 0 && x < size && y >= 0 && y < size && !board[x][y]) {
            board[x][y] = item;
            items[itemCount++] = item;
        } else {
            delete item;
        }
    }

    GameItem* getItemAt(int x, int y) const {
        return (x >= 0 && x < size && y >= 0 && y < size) ? board[x][y] : nullptr;
    }

    void removeItemAt(int x, int y) {
        if (x < 0 || x >= size || y < 0 || y >= size) return;
        for (int i = 0; i < itemCount; i++) {
            if (items[i] == board[x][y]) {
                for (int j = i; j < itemCount - 1; j++) items[j] = items[j + 1];
                items[itemCount - 1] = nullptr;
                itemCount--;
                break;
            }
        }
        delete board[x][y];
        board[x][y] = nullptr;
    }

    int getSize() const { return size; }

    // ═══════════════════════════════════════════════════════════
    // Cute item drawing helpers
    // ═══════════════════════════════════════════════════════════

    // Draw a rounded-looking card behind an icon
    void drawCellBg(sf::RenderWindow& w, float px, float py, float cs,
                    sf::Color bg, sf::Color border, float pad=4.f) const {
        sf::RectangleShape c(sf::Vector2f(cs-pad, cs-pad));
        c.setPosition(px+pad/2, py+pad/2);
        c.setFillColor(bg);
        c.setOutlineColor(border);
        c.setOutlineThickness(2.f);
        w.draw(c);
    }

    // ── Snake: cute wiggle body + round head + tongue ─────────────────────
    void drawSnake(sf::RenderWindow& w, float cx, float cy, float cs, float t) const {
        float s = cs * 0.38f;
        // Body segments
        sf::Color body(80, 200, 80);
        sf::Color belly(160, 240, 140);
        float wag = sinf(t*4.f)*3.f;
        for (int seg=3; seg>=0; seg--) {
            float bx = cx + (seg-1.5f)*s*0.45f + (seg%2==0?wag:-wag)*0.4f;
            float by = cy + seg*s*0.18f;
            float r  = s*0.30f - seg*s*0.02f;
            sf::CircleShape b(r);
            b.setOrigin(r,r); b.setPosition(bx,by);
            b.setFillColor(body); w.draw(b);
        }
        // Head
        float hx = cx + wag*0.5f, hy = cy - s*0.35f;
        float hr = s*0.36f;
        sf::CircleShape head(hr);
        head.setOrigin(hr,hr); head.setPosition(hx,hy);
        head.setFillColor(sf::Color(60,180,60));
        head.setOutlineColor(sf::Color(30,120,30)); head.setOutlineThickness(1.5f);
        w.draw(head);
        // Eyes
        for (int e=0; e<2; e++) {
            float ex=hx+(e?1:-1)*hr*0.45f, ey=hy-hr*0.25f;
            sf::CircleShape eye(hr*0.22f); eye.setOrigin(hr*0.22f,hr*0.22f);
            eye.setPosition(ex,ey); eye.setFillColor(sf::Color::White); w.draw(eye);
            sf::CircleShape pupil(hr*0.12f); pupil.setOrigin(hr*0.12f,hr*0.12f);
            pupil.setPosition(ex+hr*0.04f,ey+hr*0.04f);
            pupil.setFillColor(sf::Color(20,20,20)); w.draw(pupil);
        }
        // Tongue
        sf::RectangleShape tongue(sf::Vector2f(hr*0.12f, hr*0.55f));
        tongue.setOrigin(hr*0.06f, 0); tongue.setPosition(hx, hy+hr*0.7f);
        tongue.setFillColor(sf::Color(240,60,100)); w.draw(tongue);
        // Forked tips
        for (int f=0; f<2; f++) {
            sf::RectangleShape tip(sf::Vector2f(hr*0.1f, hr*0.28f));
            tip.setOrigin(hr*0.05f,0);
            tip.setPosition(hx+(f?1:-1)*hr*0.12f, hy+hr*1.22f);
            tip.setRotation(f?15.f:-15.f);
            tip.setFillColor(sf::Color(240,60,100)); w.draw(tip);
        }
    }

    // ── Fire: flickering flame shape ──────────────────────────────────────
    void drawFire(sf::RenderWindow& w, float cx, float cy, float cs, float t) const {
        float s = cs * 0.40f;
        float flicker = sinf(t*7.f)*0.1f;
        // Outer flame (orange)
        sf::CircleShape outer(s*(0.85f+flicker), 8);
        outer.setOrigin(s*(0.85f+flicker), s*(0.85f+flicker));
        outer.setPosition(cx, cy+s*0.15f);
        outer.setFillColor(sf::Color(255,100,20,220)); w.draw(outer);
        // Mid flame (yellow)
        float ms = s*(0.62f+flicker*0.5f);
        sf::CircleShape mid(ms,8); mid.setOrigin(ms,ms);
        mid.setPosition(cx, cy-s*0.05f);
        mid.setFillColor(sf::Color(255,210,30,230)); w.draw(mid);
        // Inner glow (white-hot)
        float is = s*0.32f;
        sf::CircleShape inner(is,8); inner.setOrigin(is,is);
        inner.setPosition(cx, cy-s*0.18f);
        inner.setFillColor(sf::Color(255,255,200,240)); w.draw(inner);
        // Teardrop top spike
        sf::ConvexShape spike(3);
        spike.setPoint(0, sf::Vector2f(-s*0.18f, 0));
        spike.setPoint(1, sf::Vector2f(s*0.18f, 0));
        spike.setPoint(2, sf::Vector2f(0, -s*(1.1f+flicker*0.3f)));
        spike.setPosition(cx, cy-s*0.3f);
        spike.setFillColor(sf::Color(255,180,20,200)); w.draw(spike);
    }

    // ── Lion: cute round face + mane ─────────────────────────────────────
    void drawLion(sf::RenderWindow& w, float cx, float cy, float cs, float t) const {
        float s = cs * 0.38f;
        float bob = sinf(t*2.5f)*1.5f;
        // Mane (spiky circle)
        sf::CircleShape mane(s*1.05f, 12);
        mane.setOrigin(s*1.05f, s*1.05f);
        mane.setPosition(cx, cy+bob);
        mane.setFillColor(sf::Color(200,120,20)); w.draw(mane);
        // Face
        sf::CircleShape face(s*0.75f);
        face.setOrigin(s*0.75f,s*0.75f);
        face.setPosition(cx, cy+bob);
        face.setFillColor(sf::Color(255,200,100));
        face.setOutlineColor(sf::Color(200,150,50)); face.setOutlineThickness(1.5f);
        w.draw(face);
        // Ears
        for (int e=0; e<2; e++) {
            float ex=cx+(e?1:-1)*s*0.62f, ey=cy-s*0.55f+bob;
            sf::CircleShape ear(s*0.25f); ear.setOrigin(s*0.25f,s*0.25f);
            ear.setPosition(ex,ey); ear.setFillColor(sf::Color(200,120,20)); w.draw(ear);
            sf::CircleShape inner(s*0.14f); inner.setOrigin(s*0.14f,s*0.14f);
            inner.setPosition(ex,ey); inner.setFillColor(sf::Color(255,180,180)); w.draw(inner);
        }
        // Eyes
        for (int e=0; e<2; e++) {
            float ex=cx+(e?1:-1)*s*0.32f, ey=cy-s*0.18f+bob;
            sf::CircleShape eye(s*0.18f); eye.setOrigin(s*0.18f,s*0.18f);
            eye.setPosition(ex,ey); eye.setFillColor(sf::Color(255,230,100)); w.draw(eye);
            sf::CircleShape pupil(s*0.10f); pupil.setOrigin(s*0.10f,s*0.10f);
            pupil.setPosition(ex,ey); pupil.setFillColor(sf::Color(30,20,10)); w.draw(pupil);
        }
        // Nose
        sf::CircleShape nose(s*0.13f,3); nose.setOrigin(s*0.13f,s*0.13f);
        nose.setPosition(cx, cy+s*0.10f+bob);
        nose.setFillColor(sf::Color(210,100,80)); w.draw(nose);
        // Whisker lines
        for (int wh=0; wh<3; wh++) {
            for (int side=0; side<2; side++) {
                sf::RectangleShape wl(sf::Vector2f(s*0.45f, s*0.04f));
                wl.setOrigin(side==0?s*0.45f:0.f, s*0.02f);
                wl.setPosition(cx+(side?s*0.08f:-s*0.08f), cy+s*(0.18f+wh*0.12f)+bob);
                wl.setRotation(side==0?(-10.f+wh*8.f):(10.f-wh*8.f));
                wl.setFillColor(sf::Color(180,130,60,180)); w.draw(wl);
            }
        }
    }

    // ── Ghost: floating wiggly shape ─────────────────────────────────────
    void drawGhost(sf::RenderWindow& w, float cx, float cy, float cs, float t) const {
        float s = cs * 0.38f;
        float bob = sinf(t*2.0f)*3.f;
        // Body (ellipse approximated)
        sf::CircleShape body(s*0.82f); body.setOrigin(s*0.82f,s*0.82f);
        body.setPosition(cx, cy-s*0.1f+bob);
        body.setFillColor(sf::Color(180,140,255,210));
        body.setOutlineColor(sf::Color(130,80,220,150)); body.setOutlineThickness(1.5f);
        w.draw(body);
        // Wavy skirt
        int bumps=5;
        for (int b=0; b<bumps; b++) {
            float bx=cx-s*0.7f + b*(s*1.4f/(bumps-1));
            float by=cy+s*0.65f+bob;
            float br=s*0.22f;
            sf::CircleShape bump(br); bump.setOrigin(br,br);
            bump.setPosition(bx,by); bump.setFillColor(sf::Color(180,140,255,210));
            w.draw(bump);
        }
        // Cover bottom gap
        sf::RectangleShape skirtCover(sf::Vector2f(s*1.5f, s*0.3f));
        skirtCover.setOrigin(s*0.75f, 0);
        skirtCover.setPosition(cx, cy+s*0.45f+bob);
        skirtCover.setFillColor(sf::Color(180,140,255,210)); w.draw(skirtCover);
        // Eyes (hollow O eyes)
        for (int e=0; e<2; e++) {
            float ex=cx+(e?1:-1)*s*0.30f, ey=cy-s*0.12f+bob;
            sf::CircleShape eye(s*0.20f); eye.setOrigin(s*0.20f,s*0.20f);
            eye.setPosition(ex,ey); eye.setFillColor(sf::Color(60,20,130,240)); w.draw(eye);
            sf::CircleShape shine(s*0.08f); shine.setOrigin(s*0.08f,s*0.08f);
            shine.setPosition(ex-s*0.05f,ey-s*0.05f);
            shine.setFillColor(sf::Color(255,255,255,200)); w.draw(shine);
        }
        // Cute blush marks
        for (int e=0; e<2; e++) {
            sf::CircleShape blush(s*0.17f); blush.setOrigin(s*0.17f,s*0.17f);
            blush.setPosition(cx+(e?1:-1)*s*0.50f, cy+s*0.08f+bob);
            blush.setFillColor(sf::Color(120,60,220,80)); w.draw(blush);
        }
    }

    // ── Gold coin ──────────────────────────────────────────────────────
    void drawGoldCoin(sf::RenderWindow& w, float cx, float cy, float cs, float t) const {
        float s = cs * 0.32f;
        float spin = sinf(t*3.f)*s*0.15f; // perspective squish
        sf::CircleShape outer(s); outer.setOrigin(s,s);
        outer.setPosition(cx, cy);
        outer.setFillColor(sf::Color(255,215,0));
        outer.setOutlineColor(sf::Color(200,160,0)); outer.setOutlineThickness(2.f);
        w.draw(outer);
        sf::CircleShape inner(s*0.62f); inner.setOrigin(s*0.62f,s*0.62f);
        inner.setPosition(cx,cy); inner.setFillColor(sf::Color(255,235,80)); w.draw(inner);
        // $ sign
        sf::Text lbl; lbl.setString("$"); // no font here — just circles
        // Star instead
        sf::CircleShape star(s*0.22f,5); star.setOrigin(s*0.22f,s*0.22f);
        star.setPosition(cx,cy); star.setFillColor(sf::Color(200,140,0)); w.draw(star);
    }

    // ── Silver coin ────────────────────────────────────────────────────
    void drawSilverCoin(sf::RenderWindow& w, float cx, float cy, float cs, float t) const {
        float s = cs * 0.30f;
        sf::CircleShape outer(s); outer.setOrigin(s,s);
        outer.setPosition(cx,cy);
        outer.setFillColor(sf::Color(210,210,220));
        outer.setOutlineColor(sf::Color(150,150,170)); outer.setOutlineThickness(2.f);
        w.draw(outer);
        sf::CircleShape inner(s*0.60f); inner.setOrigin(s*0.60f,s*0.60f);
        inner.setPosition(cx,cy); inner.setFillColor(sf::Color(240,240,255)); w.draw(inner);
        sf::CircleShape star(s*0.20f,6); star.setOrigin(s*0.20f,s*0.20f);
        star.setPosition(cx,cy); star.setFillColor(sf::Color(150,150,180)); w.draw(star);
    }

    // ── Sword ──────────────────────────────────────────────────────────
    void drawSword(sf::RenderWindow& w, float cx, float cy, float cs) const {
        float s = cs * 0.38f;
        // Blade
        sf::RectangleShape blade(sf::Vector2f(s*0.18f, s*1.55f));
        blade.setOrigin(s*0.09f, s*1.55f);
        blade.setPosition(cx, cy+s*0.6f);
        blade.setFillColor(sf::Color(200,230,255));
        blade.setOutlineColor(sf::Color(120,170,220)); blade.setOutlineThickness(1.f);
        w.draw(blade);
        // Guard
        sf::RectangleShape guard(sf::Vector2f(s*0.85f, s*0.18f));
        guard.setOrigin(s*0.425f, s*0.09f);
        guard.setPosition(cx, cy+s*0.12f);
        guard.setFillColor(sf::Color(220,180,60)); w.draw(guard);
        // Pommel
        sf::CircleShape pommel(s*0.18f); pommel.setOrigin(s*0.18f,s*0.18f);
        pommel.setPosition(cx, cy+s*0.6f);
        pommel.setFillColor(sf::Color(180,140,40)); w.draw(pommel);
        // Tip
        sf::ConvexShape tip(3);
        tip.setPoint(0,sf::Vector2f(-s*0.09f,0));
        tip.setPoint(1,sf::Vector2f(s*0.09f,0));
        tip.setPoint(2,sf::Vector2f(0,-s*0.28f));
        tip.setPosition(cx, cy-s*1.1f);
        tip.setFillColor(sf::Color(200,230,255)); w.draw(tip);
    }

    // ── Shield ─────────────────────────────────────────────────────────
    void drawShield(sf::RenderWindow& w, float cx, float cy, float cs) const {
        float s = cs * 0.36f;
        sf::ConvexShape sh(5);
        sh.setPoint(0,sf::Vector2f(-s*0.75f,-s*0.9f));
        sh.setPoint(1,sf::Vector2f(s*0.75f,-s*0.9f));
        sh.setPoint(2,sf::Vector2f(s*0.75f,s*0.2f));
        sh.setPoint(3,sf::Vector2f(0,s*1.0f));
        sh.setPoint(4,sf::Vector2f(-s*0.75f,s*0.2f));
        sh.setPosition(cx,cy);
        sh.setFillColor(sf::Color(60,120,220));
        sh.setOutlineColor(sf::Color(30,60,160)); sh.setOutlineThickness(2.f);
        w.draw(sh);
        // Cross
        sf::RectangleShape hBar(sf::Vector2f(s*0.9f,s*0.18f));
        hBar.setOrigin(s*0.45f,s*0.09f); hBar.setPosition(cx,cy-s*0.2f);
        hBar.setFillColor(sf::Color(180,200,255)); w.draw(hBar);
        sf::RectangleShape vBar(sf::Vector2f(s*0.18f,s*0.9f));
        vBar.setOrigin(s*0.09f,s*0.45f); vBar.setPosition(cx,cy-s*0.15f);
        vBar.setFillColor(sf::Color(180,200,255)); w.draw(vBar);
    }

    // ── Water drop ─────────────────────────────────────────────────────
    void drawWater(sf::RenderWindow& w, float cx, float cy, float cs, float t) const {
        float s = cs * 0.35f;
        float drip = sinf(t*3.f)*2.f;
        // Drop body
        sf::CircleShape body(s*0.72f); body.setOrigin(s*0.72f,s*0.72f);
        body.setPosition(cx, cy+s*0.22f+drip);
        body.setFillColor(sf::Color(60,180,255,230)); w.draw(body);
        // Teardrop top
        sf::ConvexShape top(3);
        top.setPoint(0,sf::Vector2f(-s*0.40f,0));
        top.setPoint(1,sf::Vector2f(s*0.40f,0));
        top.setPoint(2,sf::Vector2f(0,-s*0.90f));
        top.setPosition(cx, cy-s*0.35f+drip);
        top.setFillColor(sf::Color(60,180,255,230)); w.draw(top);
        // Shine
        sf::CircleShape shine(s*0.20f); shine.setOrigin(s*0.20f,s*0.20f);
        shine.setPosition(cx-s*0.22f, cy-s*0.05f+drip);
        shine.setFillColor(sf::Color(255,255,255,160)); w.draw(shine);
    }

    // ── Key ────────────────────────────────────────────────────────────
    void drawKey(sf::RenderWindow& w, float cx, float cy, float cs) const {
        float s = cs * 0.36f;
        // Ring
        sf::CircleShape ring(s*0.42f); ring.setOrigin(s*0.42f,s*0.42f);
        ring.setPosition(cx, cy-s*0.28f);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineColor(sf::Color(255,220,50)); ring.setOutlineThickness(s*0.18f);
        w.draw(ring);
        // Hole
        sf::CircleShape hole(s*0.16f); hole.setOrigin(s*0.16f,s*0.16f);
        hole.setPosition(cx, cy-s*0.28f);
        hole.setFillColor(sf::Color(20,20,40)); w.draw(hole);
        // Shaft
        sf::RectangleShape shaft(sf::Vector2f(s*0.20f, s*0.95f));
        shaft.setOrigin(s*0.10f,0);
        shaft.setPosition(cx, cy+s*0.12f);
        shaft.setFillColor(sf::Color(255,220,50)); w.draw(shaft);
        // Teeth
        for (int t2=0; t2<2; t2++) {
            sf::RectangleShape tooth(sf::Vector2f(s*0.22f, s*0.16f));
            tooth.setOrigin(0,s*0.08f);
            tooth.setPosition(cx+s*0.10f, cy+s*(0.45f+t2*0.28f));
            tooth.setFillColor(sf::Color(255,220,50)); w.draw(tooth);
        }
    }

    // ── PadLock ────────────────────────────────────────────────────────
    void drawLock(sf::RenderWindow& w, float cx, float cy, float cs) const {
        float s = cs * 0.35f;
        // Shackle
        sf::CircleShape shackle(s*0.55f); shackle.setOrigin(s*0.55f,s*0.55f);
        shackle.setPosition(cx, cy-s*0.28f);
        shackle.setFillColor(sf::Color::Transparent);
        shackle.setOutlineColor(sf::Color(180,130,50)); shackle.setOutlineThickness(s*0.22f);
        w.draw(shackle);
        // Cover lower half of shackle
        sf::RectangleShape cover(sf::Vector2f(s*1.4f, s*0.8f));
        cover.setOrigin(s*0.7f,0); cover.setPosition(cx, cy-s*0.25f);
        cover.setFillColor(sf::Color(20,15,35)); w.draw(cover);
        // Body
        sf::RectangleShape body(sf::Vector2f(s*1.20f, s*0.95f));
        body.setOrigin(s*0.60f,0); body.setPosition(cx, cy+s*0.05f);
        body.setFillColor(sf::Color(180,130,50));
        body.setOutlineColor(sf::Color(120,80,20)); body.setOutlineThickness(1.5f);
        w.draw(body);
        // Keyhole
        sf::CircleShape khole(s*0.18f); khole.setOrigin(s*0.18f,s*0.18f);
        khole.setPosition(cx, cy+s*0.38f);
        khole.setFillColor(sf::Color(50,30,10)); w.draw(khole);
        sf::RectangleShape slot(sf::Vector2f(s*0.14f,s*0.28f));
        slot.setOrigin(s*0.07f,0); slot.setPosition(cx, cy+s*0.52f);
        slot.setFillColor(sf::Color(50,30,10)); w.draw(slot);
    }

    // ── GUI draw (main board draw call) ──────────────────────────────────────
    void draw(sf::RenderWindow& window,
              const Player& p1, const Player& p2,
              float offsetX, float offsetY,
              float cellSize, float t,
              const sf::Font& font) const
    {
        int goal = size / 2;
        float pulse = (sinf(t * 3.f) + 1.f) / 2.f;

        // ── Board border shadow
        sf::RectangleShape boardShadow(sf::Vector2f(size*cellSize+12, size*cellSize+12));
        boardShadow.setPosition(offsetX-4, offsetY-4);
        boardShadow.setFillColor(sf::Color(0,0,0,120));
        window.draw(boardShadow);

        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                float px = offsetX + j * cellSize;
                float py = offsetY + i * cellSize;
                float pad = 4.f;

                // ── Pastel checker cells
                sf::Color bgA(22, 10, 50);   // deep purple
                sf::Color bgB(12, 30, 45);   // deep teal
                bool isGoalCell = (i==goal && j==goal);
                sf::Color cellColor = (i+j)%2==0 ? bgA : bgB;
                if (isGoalCell) cellColor = sf::Color(0, 50, 30); // emerald base

                sf::RectangleShape cell(sf::Vector2f(cellSize-pad, cellSize-pad));
                cell.setPosition(px+pad/2, py+pad/2);
                cell.setFillColor(cellColor);
                // Cute rounded-look border
                sf::Color borderCol = (i+j)%2==0
                    ? sf::Color(60,30,110,200)
                    : sf::Color(20,70,80,200);
                if (isGoalCell) borderCol = sf::Color(0,230,150,220);
                cell.setOutlineColor(borderCol); cell.setOutlineThickness(2.f);
                window.draw(cell);

                // Goal cell star glow
                if (isGoalCell) {
                    float gA = (sf::Uint8)(140 + 80*pulse);
                    sf::RectangleShape glow(sf::Vector2f(cellSize-pad+4, cellSize-pad+4));
                    glow.setPosition(px+pad/2-2, py+pad/2-2);
                    glow.setFillColor(sf::Color(0,0,0,0));
                    glow.setOutlineColor(sf::Color(0,240,160,(sf::Uint8)gA));
                    glow.setOutlineThickness(3.f);
                    window.draw(glow);
                    // Star text
                    sf::Text gt; gt.setFont(font);
                    gt.setString("GOAL"); gt.setCharacterSize((unsigned)(cellSize*0.18f));
                    gt.setFillColor(sf::Color(0,240,160,220));
                    sf::FloatRect gb=gt.getLocalBounds();
                    gt.setOrigin(gb.width/2, gb.height/2);
                    gt.setPosition(px+cellSize/2, py+cellSize*0.82f);
                    window.draw(gt);
                }

                // ── Item drawing ─────────────────────────────────────────────
                GameItem* itm = getItemAt(i, j);
                float cx = px + cellSize/2.f;
                float cy = py + cellSize/2.f;

                if (itm && !isGoalCell) {
                    const string& n = itm->getName();
                    if      (n=="Snake")  drawSnake(window,cx,cy,cellSize,t);
                    else if (n=="Fire")   drawFire(window,cx,cy,cellSize,t);
                    else if (n=="Lion")   drawLion(window,cx,cy,cellSize,t);
                    else if (n=="Ghost")  drawGhost(window,cx,cy,cellSize,t);
                    else if (n=="Gold")   drawGoldCoin(window,cx,cy,cellSize,t);
                    else if (n=="Silver") drawSilverCoin(window,cx,cy,cellSize,t);
                    else if (n=="Sword")  drawSword(window,cx,cy,cellSize);
                    else if (n=="Shield") drawShield(window,cx,cy,cellSize);
                    else if (n=="Water")  drawWater(window,cx,cy,cellSize,t);
                    else if (n=="Key")    drawKey(window,cx,cy,cellSize);
                    else if (n=="Lock")   drawLock(window,cx,cy,cellSize);
                    else {
                        // Fallback pill
                        float r=cellSize*0.28f;
                        sf::CircleShape ic(r); ic.setOrigin(r,r); ic.setPosition(cx,cy);
                        ic.setFillColor(sf::Color(200,200,200)); window.draw(ic);
                    }
                }
            }
        }

        // ── Draw players ─────────────────────────────────────────────────────
        auto drawPlayer = [&](const Player& pl, sf::Color body, sf::Color hat, bool isP1) {
            float px = offsetX + pl.getY() * cellSize + cellSize/2.f;
            float py = offsetY + pl.getX() * cellSize + cellSize/2.f;
            float s  = cellSize * 0.30f;
            float bob = sinf(t*3.5f + (isP1?0.f:1.57f))*2.f;

            // Shadow
            sf::CircleShape shd(s*0.80f,20); shd.setOrigin(s*0.80f,s*0.80f);
            shd.setPosition(px+3, py+s*1.2f+bob);
            shd.setScale(1.5f,0.4f);
            shd.setFillColor(sf::Color(0,0,0,50)); window.draw(shd);

            // Body (cute rounded square person)
            sf::RectangleShape body2(sf::Vector2f(s*1.3f,s*1.4f));
            body2.setOrigin(s*0.65f,0); body2.setPosition(px, py+s*0.25f+bob);
            body2.setFillColor(body); body2.setOutlineColor(sf::Color(255,255,255,120));
            body2.setOutlineThickness(1.5f); window.draw(body2);

            // Head
            sf::CircleShape head(s*0.52f); head.setOrigin(s*0.52f,s*0.52f);
            head.setPosition(px, py-s*0.28f+bob);
            head.setFillColor(sf::Color(255,220,180));
            head.setOutlineColor(sf::Color(200,160,120)); head.setOutlineThickness(1.5f);
            window.draw(head);

            // Hat
            sf::RectangleShape hatB(sf::Vector2f(s*1.1f,s*0.35f));
            hatB.setOrigin(s*0.55f,s*0.35f); hatB.setPosition(px,py-s*0.72f+bob);
            hatB.setFillColor(hat); window.draw(hatB);
            sf::RectangleShape hatT(sf::Vector2f(s*0.70f,s*0.55f));
            hatT.setOrigin(s*0.35f,s*0.55f); hatT.setPosition(px,py-s*1.05f+bob);
            hatT.setFillColor(hat); window.draw(hatT);

            // Eyes
            for (int e=0;e<2;e++) {
                sf::CircleShape eye(s*0.10f); eye.setOrigin(s*0.10f,s*0.10f);
                eye.setPosition(px+(e?1:-1)*s*0.22f, py-s*0.30f+bob);
                eye.setFillColor(sf::Color(40,40,40)); window.draw(eye);
            }
            // Smile
            sf::CircleShape smile(s*0.18f); smile.setOrigin(s*0.18f,0);
            smile.setPosition(px-s*0.18f, py-s*0.08f+bob);
            smile.setFillColor(sf::Color::Transparent);
            smile.setOutlineColor(sf::Color(180,80,80)); smile.setOutlineThickness(s*0.09f);
            // just a dot smile
            sf::CircleShape smileDot(s*0.06f); smileDot.setOrigin(s*0.06f,s*0.06f);
            smileDot.setPosition(px, py-s*0.06f+bob);
            smileDot.setFillColor(sf::Color(200,80,80)); window.draw(smileDot);

            // Player number badge
            sf::CircleShape badge(s*0.28f); badge.setOrigin(s*0.28f,s*0.28f);
            badge.setPosition(px+s*0.55f, py-s*0.65f+bob);
            badge.setFillColor(hat);
            badge.setOutlineColor(sf::Color::White); badge.setOutlineThickness(1.5f);
            window.draw(badge);
            sf::Text num; num.setFont(font);
            num.setString(isP1?"1":"2"); num.setCharacterSize((unsigned)(s*0.30f));
            num.setFillColor(sf::Color::White);
            sf::FloatRect nb=num.getLocalBounds();
            num.setOrigin(nb.width/2,nb.height/2+nb.top*0.5f);
            num.setPosition(px+s*0.55f, py-s*0.67f+bob);
            window.draw(num);
        };

        drawPlayer(p1, sf::Color(0,200,130), sf::Color(0,120,80), true);
        drawPlayer(p2, sf::Color(150,70,255), sf::Color(90,20,180), false);
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// Player::moveAndGetItem  (needs GameBoard fully defined)
// ─────────────────────────────────────────────────────────────────────────────
GameItem* Player::moveAndGetItem(int boardSize, GameBoard* board) {
    if (moveForward(boardSize)) return board->getItemAt(x, y);
    return nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// interact implementations
// ─────────────────────────────────────────────────────────────────────────────
void Sword::interact(Player*)  { /* picked up — message displayed by caller */ }
void Shield::interact(Player*) { }
void Water::interact(Player*)  { }
void Key::interact(Player*)    { }

void Coin::interact(Player* player) {
    player->addPoints(value);
    if (isGold) player->addGold(1); else player->addSilver(1);
}

void Snake::interact(Player* player) {
    for (int i = 0; i < player->getInventorySize(); i++) {
        if (Sword* sw = dynamic_cast<Sword*>(player->getInventoryItem(i))) {
            if (sw->getUsesLeft() > 0) {
                player->useHelperObject(i);
                return;
            }
        }
    }
    Hurdle::interact(player);
    player->moveBackward(3, player->getPathIndex() + 3);
}

void Fire::interact(Player* player) {
    for (int i = 0; i < player->getInventorySize(); i++) {
        if (dynamic_cast<Water*>(player->getInventoryItem(i))) {
            player->useHelperObject(i);
            return;
        }
    }
    Hurdle::interact(player);
}

// FIX 3 — was bare Hurdle::interact(player) without object; use this->
void PadLock::interact(Player* player) {
    for (int i = 0; i < player->getInventorySize(); i++) {
        if (dynamic_cast<Key*>(player->getInventoryItem(i))) {
            player->useHelperObject(i);
            return;
        }
    }
    this->Hurdle::interact(player);  // ← FIX: explicit this
}

// ─────────────────────────────────────────────────────────────────────────────
// AdventureQuest — game controller
// ─────────────────────────────────────────────────────────────────────────────
class AdventureQuest {
public:
    GameBoard* board;
    Player player1, player2;
    Player* currentPlayer;
    bool gameOver;
    int level, boardSize;

    GameBoard* getBoard()     { return board; }
    Player& getPlayer1()      { return player1; }
    Player& getPlayer2()      { return player2; }

    void initializeBoard() {
        int minI = (boardSize == 5) ? 4 : 10;
        int maxI = (boardSize == 5) ? 10 : 20;
        int num  = minI + rand() % (maxI - minI + 1);
        int forb[3][2] = {
            {boardSize/2, boardSize/2},
            {player1.getX(), player1.getY()},
            {player2.getX(), player2.getY()}
        };
        for (int i = 0; i < num; i++) {
            int x, y; bool ok = false;
            while (!ok) {
                x = rand() % boardSize; y = rand() % boardSize;
                ok = true;
                for (auto& f : forb)
                    if (x == f[0] && y == f[1]) { ok = false; break; }
                if (ok && board->getItemAt(x, y)) ok = false;
            }
            int t = rand() % 9;
            if      (t == 0) board->placeItem(new Coin("Gold",  x, y, 70,  true));
            else if (t == 1) board->placeItem(new Coin("Silver",x, y, 50, false));
            else if (t == 2) board->placeItem(new Fire(x, y));
            else if (t == 3) board->placeItem(new Snake(x, y));
            else if (t == 4) board->placeItem(new Ghost(x, y));
            else if (t == 5) board->placeItem(new Lion(x, y));
            else if (t == 6) board->placeItem(new PadLock(x, y));  // FIX 1 & 2
            else if (t == 7) board->placeItem(new Sword(x, y));
            else             board->placeItem(new Water(x, y));
        }
    }

    void definePaths() {
        int p1X[MAX_PATH_LENGTH], p1Y[MAX_PATH_LENGTH];
        int p2X[MAX_PATH_LENGTH], p2Y[MAX_PATH_LENGTH];
        int p1L = 0, p2L = 0, idx = 1;

        for (int i = boardSize - 1; i >= 0; i--) {
            if (i % 2 == 0) {
                for (int j = 0; j < boardSize && idx <= boardSize*boardSize; j++, idx++)
                    { p1X[p1L] = i; p1Y[p1L] = j; p1L++; }
            } else {
                for (int j = boardSize-1; j >= 0 && idx <= boardSize*boardSize; j--, idx++)
                    { p1X[p1L] = i; p1Y[p1L] = j; p1L++; }
            }
        }

        int top=0, bot=boardSize-1, lft=0, rgt=boardSize-1;
        idx = 1;
        while (top <= bot && lft <= rgt) {
            for (int i = rgt; i >= lft && idx<=boardSize*boardSize; i--, idx++)
                { p2X[p2L]=top; p2Y[p2L]=i; p2L++; }
            top++;
            for (int i = top; i <= bot && idx<=boardSize*boardSize; i++, idx++)
                { p2X[p2L]=i; p2Y[p2L]=lft; p2L++; }
            lft++;
            for (int i = lft; i <= rgt && idx<=boardSize*boardSize; i++, idx++)
                { p2X[p2L]=bot; p2Y[p2L]=i; p2L++; }
            bot--;
            for (int i = bot; i >= top && idx<=boardSize*boardSize; i--, idx++)
                { p2X[p2L]=i; p2Y[p2L]=rgt; p2L++; }
            rgt--;
        }
        player1.setPath(p1X, p1Y, p1L);
        player2.setPath(p2X, p2Y, p2L);
    }

    void switchPlayer() {
        currentPlayer = (currentPlayer == &player1) ? &player2 : &player1;
    }

    bool isGoal(int x, int y) const { return x == boardSize/2 && y == boardSize/2; }

    bool isOpponentAt(int x, int y) {
        Player* o = (currentPlayer == &player1) ? &player2 : &player1;
        return o->getX() == x && o->getY() == y;
    }

    AdventureQuest(int sz = MIN_BOARD_SIZE)
        : player1("Player 1", sz-1, 0, 10, 20),
          player2("Player 2", 0, sz-1, 10, 20),
          gameOver(false), level(1), boardSize(sz)
    {
        board = new GameBoard(boardSize);
        currentPlayer = &player1;
        srand((unsigned)time(nullptr));
        definePaths();
        initializeBoard();
    }

    ~AdventureQuest() { delete board; }

    // Returns a human-readable event description for the HUD log
    string doMove() {
        if (gameOver) return "";
        Player& cur = *currentPlayer;
        if (cur.getSkipTurns() > 0) {
            string msg = cur.getName() + " is blocked ("
                       + to_string(cur.getSkipTurns()) + " turns left).";
            cur.decrementSkipTurns();
            switchPlayer();
            return msg;
        }
        if (!cur.moveForward(boardSize))
            return cur.getName() + " cannot move further!";

        if (isOpponentAt(cur.getX(), cur.getY())) {
            cur.moveBackward(1, boardSize);
            return cur.getName() + " bumped into opponent — wait.";
        }

        GameItem* itm = board->getItemAt(cur.getX(), cur.getY());
        string msg = cur.getName() + " moved to ("
                   + to_string(cur.getX()) + "," + to_string(cur.getY()) + ").";

        if (itm) {
            string n = itm->getName();
            msg += " Hit: " + n + "!";
            itm->interact(&cur);
            if (dynamic_cast<Coin*>(itm) || dynamic_cast<HelperObject*>(itm)) {
                board->removeItemAt(cur.getX(), cur.getY());
            }
        }
        cur.clearUsedItems();

        if (isGoal(cur.getX(), cur.getY())) {
            gameOver = true;
            msg += "  ★ GOAL REACHED!";
        } else {
            switchPlayer();
        }
        return msg;
    }

    bool isGameOver() const { return gameOver; }
    string getWinner() const {
        if (isGoal(player1.getX(), player1.getY()) && !isGoal(player2.getX(), player2.getY()))
            return player1.getName();
        if (isGoal(player2.getX(), player2.getY()) && !isGoal(player1.getX(), player1.getY()))
            return player2.getName();
        return (player1.getPoints() >= player2.getPoints()) ? player1.getName() : player2.getName();
    }

    void nextLevel() {
        if (boardSize >= MAX_BOARD_SIZE) return;
        boardSize += 2; level++;
        gameOver = false;
        delete board; board = new GameBoard(boardSize);
        player1 = Player("Player 1", boardSize-1, 0, player1.getGold(), player1.getSilver());
        player2 = Player("Player 2", 0, boardSize-1, player2.getGold(), player2.getSilver());
        currentPlayer = &player1;
        definePaths(); initializeBoard();
    }
};


// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
sf::RectangleShape panel(float x, float y, float w, float h,
                         sf::Color fill, sf::Color outline=sf::Color::Transparent,
                         float thick=1.5f) {
    sf::RectangleShape r(sf::Vector2f(w,h));
    r.setPosition(x,y); r.setFillColor(fill);
    r.setOutlineColor(outline); r.setOutlineThickness(thick);
    return r;
}

sf::Text makeText(const sf::Font& font, const string& str, unsigned size,
                  sf::Color col, float x, float y) {
    sf::Text t; t.setFont(font); t.setString(str); t.setCharacterSize(size);
    t.setFillColor(col); t.setPosition(x,y); return t;
}

void drawRoundLabel(sf::RenderWindow& w, const sf::Font& font,
                    const string& s, unsigned sz, sf::Color textCol,
                    sf::Color bgCol, float x, float y, float padX=8.f, float padY=4.f) {
    sf::Text t = makeText(font,s,sz,textCol,0,0);
    sf::FloatRect tb = t.getLocalBounds();
    sf::RectangleShape bg(sf::Vector2f(tb.width+padX*2, sz+padY*2));
    bg.setPosition(x,y); bg.setFillColor(bgCol);
    bg.setOutlineColor(sf::Color(255,255,255,60)); bg.setOutlineThickness(1.f);
    w.draw(bg);
    t.setPosition(x+padX, y+padY-tb.top*0.5f); w.draw(t);
}

// ─────────────────────────────────────────────────────────────────────────────
// main
// ─────────────────────────────────────────────────────────────────────────────
int main() {
    AdventureQuest game;

    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H),
        "Adventure Quest ✦ Emerald Edition",
        sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;
    bool fontOk = font.loadFromFile("arial.ttf");
    if (!fontOk) fontOk = font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf");
    if (!fontOk) fontOk = font.loadFromFile("C:\\Windows\\Fonts\\segoeui.ttf");
    if (!fontOk) fontOk = font.loadFromFile("C:\\Windows\\Fonts\\calibri.ttf");
    if (!fontOk) fontOk = font.loadFromFile("C:\\Windows\\Fonts\\tahoma.ttf");
    if (!fontOk) fontOk = font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    vector<string> log;
    log.push_back("Welcome to Adventure Quest! ^-^");
    log.push_back("Press M to move  ~  Q to quit");

    sf::Clock clock;
    bool showWinScreen = false;

    while (window.isOpen()) {
        float t = clock.getElapsedTime().asSeconds();
        float pulse = (sinf(t*3.f)+1.f)/2.f;

        sf::Event ev;
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) window.close();

            if (ev.type == sf::Event::KeyPressed) {
                string msg;
                if (showWinScreen) {
                    if (ev.key.code==sf::Keyboard::N && game.getBoard()->getSize()<MAX_BOARD_SIZE) {
                        game.nextLevel(); showWinScreen=false;
                        log.clear(); log.push_back("Level "+to_string(game.level)+" - Let's go! ^o^");
                    } else if (ev.key.code==sf::Keyboard::Q) window.close();
                } else {
                    switch (ev.key.code) {
                        case sf::Keyboard::M:
                            msg = game.doMove();
                            if (game.isGameOver()) showWinScreen = true;
                            break;
                        case sf::Keyboard::Q: window.close(); break;
                        default: break;
                    }
                }
                if (!msg.empty()) {
                    log.push_back(msg);
                    if (log.size()>8) log.erase(log.begin());
                }
            }
        }

        // Layout
        const float HUD_W = 310.f;
        const float BOARD_W = WIN_W - HUD_W;
        const int   bs = game.getBoard()->getSize();
        const float maxCS = std::min(BOARD_W/bs, (float)(WIN_H-60)/bs) - 2.f;
        const float cellSz = std::min(maxCS, 80.f);
        const float bW = cellSz*bs, bH = cellSz*bs;
        const float offX = (BOARD_W-bW)/2.f;
        const float offY = 56.f + ((WIN_H-56.f)-bH)/2.f;

        // ── Cute pastel background ────────────────────────────────────────────
        // Gradient from warm pink-lavender top to soft mint bottom
        for (int yi=0; yi<WIN_H; yi+=1) {
            float f = (float)yi/WIN_H;
            sf::Uint8 r2=(sf::Uint8)(18+f*10);
            sf::Uint8 g2=(sf::Uint8)(8+f*30);
            sf::Uint8 b2=(sf::Uint8)(40+f*20);
            sf::RectangleShape strip(sf::Vector2f((float)WIN_W,1.f));
            strip.setPosition(0,(float)yi); strip.setFillColor(sf::Color(r2,g2,b2));
            window.draw(strip);
        }

        // Decorative polka-dot background
        for (int di=0; di<12; di++) {
            float dx = fmod(di*137.f, BOARD_W-40)+20;
            float dy = fmod(di*89.f+30.f, (float)(WIN_H-40))+20;
            float dr = 6.f + (di%4)*4.f;
            sf::CircleShape dot(dr); dot.setOrigin(dr,dr); dot.setPosition(dx,dy);
            dot.setFillColor(sf::Color(0,210,140,30)); window.draw(dot);
        }

        // ── Top header bar ─────────────────────────────────────────────────────
        sf::RectangleShape hdr(sf::Vector2f(WIN_W,54.f));
        hdr.setFillColor(sf::Color(30,10,65,245)); window.draw(hdr);
        // Underline
        sf::RectangleShape hdrLine(sf::Vector2f(WIN_W,3.f));
        hdrLine.setPosition(0,51.f); hdrLine.setFillColor(sf::Color(0,210,140,200));
        window.draw(hdrLine);

        // Title hearts and text
        window.draw(makeText(font,"✦  ADVENTURE QUEST  ✦",22,sf::Color(0,230,155),14.f,13.f));
        string lvlStr = "Level "+to_string(game.level)+"   "+to_string(bs)+"x"+to_string(bs)+" Board";
        window.draw(makeText(font,lvlStr,14,sf::Color(180,130,255),(float)WIN_W/2-70,18.f));
        window.draw(makeText(font,"[M] Move  [Q] Quit",12,sf::Color(100,200,170),(float)WIN_W-180.f,18.f));

        // ── Board ────────────────────────────────────────────────────────────────
        game.getBoard()->draw(window, game.getPlayer1(), game.getPlayer2(),
                              offX, offY, cellSz, t, font);

        // ── HUD ──────────────────────────────────────────────────────────────────
        float hudX = BOARD_W;
        // HUD background
        sf::RectangleShape hudBg(sf::Vector2f(HUD_W,WIN_H));
        hudBg.setPosition(hudX,0);
        hudBg.setFillColor(sf::Color(20,8,50,245));
        hudBg.setOutlineColor(sf::Color(0,200,130,160)); hudBg.setOutlineThickness(2.f);
        window.draw(hudBg);

        // Cute HUD title
        window.draw(makeText(font,"✦ PLAYERS ✦",15,sf::Color(0,230,155),hudX+90.f,10.f));

        float hy = 36.f;
        bool p1Active = (game.currentPlayer == &game.getPlayer1());

        auto drawPlayerCard = [&](const Player& pl, sf::Color accent, sf::Color cardBg,
                                  bool isActive, bool isP1) {
            // Card shadow
            sf::RectangleShape shadow(sf::Vector2f(HUD_W-14, 148.f));
            shadow.setPosition(hudX+9, hy+4);
            shadow.setFillColor(sf::Color(0,0,0,60)); window.draw(shadow);

            // Card
            sf::RectangleShape card(sf::Vector2f(HUD_W-14,148.f));
            card.setPosition(hudX+7, hy);
            card.setFillColor(isActive ? cardBg : sf::Color(28,12,58));
            card.setOutlineColor(accent); card.setOutlineThickness(isActive?3.f:1.5f);
            window.draw(card);

            // Active bouncing marker
            if (isActive) {
                float gy = hy + 6.f + sinf(t*4.f)*3.f;
                sf::Text arrow; arrow.setFont(font); arrow.setString(">");
                arrow.setCharacterSize(18); arrow.setFillColor(accent);
                arrow.setPosition(hudX+HUD_W-22.f, gy+8); window.draw(arrow);
            }

            // Player avatar blob
            float avX=hudX+26.f, avY=hy+14.f;
            sf::CircleShape av(18.f); av.setOrigin(18.f,18.f);
            av.setPosition(avX+18.f,avY+18.f);
            av.setFillColor(accent); av.setOutlineColor(sf::Color::White);
            av.setOutlineThickness(2.5f); window.draw(av);
            window.draw(makeText(font,isP1?"1":"2",16,sf::Color::White,avX+12.f,avY+8.f));

            // Name
            window.draw(makeText(font,pl.getName()+(isActive?" <-- YOU":""),14,
                                 accent,hudX+56.f,hy+8.f));

            // Stats with cute icons
            window.draw(makeText(font,"★ "+to_string(pl.getPoints())+" pts",13,
                                 sf::Color(200,160,255),hudX+16.f,hy+42.f));
            window.draw(makeText(font,"◆ "+to_string(pl.getGold())+" gold",13,
                                 sf::Color(255,215,60),hudX+16.f,hy+60.f));
            window.draw(makeText(font,"◆ "+to_string(pl.getSilver())+" silver",13,
                                 sf::Color(160,230,210),hudX+16.f,hy+78.f));

            // Inventory
            string inv="Bag: ";
            for (int i2=0;i2<pl.getInventorySize();i2++) {
                if(i2) inv+=", ";
                inv+=pl.getInventoryItem(i2)->getName();
            }
            if (pl.getInventorySize()==0) inv+="(empty)";
            window.draw(makeText(font,inv,11,sf::Color(140,200,180),hudX+16.f,hy+98.f));

            int sk=pl.getSkipTurns();
            if (sk>0) {
                drawRoundLabel(window,font,"BLOCKED "+to_string(sk)+" turn(s)",11,
                               sf::Color::White,sf::Color(240,80,80,220),hudX+16.f,hy+116.f);
            }

            hy += 160.f;
        };

        drawPlayerCard(game.getPlayer1(),sf::Color(0,220,148),sf::Color(18,38,55),p1Active,true);
        drawPlayerCard(game.getPlayer2(),sf::Color(160,80,255),sf::Color(35,12,60),!p1Active,false);

        // ── Legend ────────────────────────────────────────────────────────────
        hy += 4.f;
        window.draw(makeText(font,"✦ GUIDE ✦",13,sf::Color(0,220,148),hudX+100.f,hy));
        hy += 20.f;
        struct LegEntry { const char* emoji; const char* label; sf::Color col; };
        LegEntry entries[] = {
            {"$","Gold coin",    sf::Color(255,215,60)},
            {"o","Silver coin",  sf::Color(160,230,210)},
            {"~","Snake",        sf::Color(50,220,120)},
            {"*","Fire",         sf::Color(255,100,30)},
            {"@","Lion",         sf::Color(255,180,40)},
            {"o","Ghost",        sf::Color(180,140,255)},
            {"#","Lock",         sf::Color(200,150,60)},
            {"/","Sword",        sf::Color(80,220,255)},
            {"]","Shield",       sf::Color(100,80,255)},
            {"~","Water",        sf::Color(0,200,255)},
            {"!","Key",          sf::Color(255,230,80)},
            {"*","Goal",         sf::Color(0,240,180)},
        };
        for (auto& e : entries) {
            if (hy > WIN_H-32.f) break;
            sf::CircleShape dot(5.f); dot.setFillColor(e.col);
            dot.setPosition(hudX+16.f, hy+3.f); window.draw(dot);
            window.draw(makeText(font,e.label,11,sf::Color(180,220,200),hudX+28.f,hy));
            hy += 15.f;
        }

        // ── Action log (bottom) ───────────────────────────────────────────────
        float logY = WIN_H-110.f;
        sf::RectangleShape logBg(sf::Vector2f(BOARD_W,110.f));
        logBg.setPosition(0,logY);
        logBg.setFillColor(sf::Color(15,6,38,230));
        logBg.setOutlineColor(sf::Color(0,200,140,140)); logBg.setOutlineThickness(2.f);
        window.draw(logBg);
        window.draw(makeText(font,"✦ Action Log ✦",11,sf::Color(0,220,148),8,logY+4));
        int logStart=(int)log.size()-6; if(logStart<0)logStart=0;
        for (int i2=logStart;i2<(int)log.size();i2++) {
            float alpha=80+(float)(i2-logStart)/6.f*175.f;
            window.draw(makeText(font,"> "+log[i2],12,
                                 sf::Color(160,240,200,(sf::Uint8)alpha),
                                 8.f,logY+18.f+(i2-logStart)*15.f));
        }

        // ── Win screen ────────────────────────────────────────────────────────
        if (showWinScreen) {
            sf::RectangleShape ov(sf::Vector2f(WIN_W,WIN_H));
            ov.setFillColor(sf::Color(15,5,40,210)); window.draw(ov);

            string wMsg = "★  "+game.getWinner()+" WINS!  ★";
            sf::Text wt; wt.setFont(font); wt.setString(wMsg); wt.setCharacterSize(44);
            wt.setFillColor(sf::Color(0,240,170));
            sf::FloatRect wb=wt.getLocalBounds(); wt.setOrigin(wb.width/2,wb.height/2);
            wt.setPosition(WIN_W/2.f, WIN_H/2.f-50.f); window.draw(wt);

            bool canCont=(game.getBoard()->getSize()<MAX_BOARD_SIZE);
            string sub = canCont?"Press N — Next Level   |   Q — Quit":"Press Q to Quit";
            sf::Text st; st.setFont(font); st.setString(sub); st.setCharacterSize(20);
            st.setFillColor(sf::Color(180,130,255));
            sf::FloatRect sb=st.getLocalBounds(); st.setOrigin(sb.width/2,sb.height/2);
            st.setPosition(WIN_W/2.f, WIN_H/2.f+20.f); window.draw(st);

            // Confetti dots
            for (int d=0;d<20;d++) {
                float dx=fmod(d*173.f+t*30.f,(float)WIN_W);
                float dy=fmod(d*97.f+t*60.f*(float)(1+d%3),(float)WIN_H);
                sf::Color cc[]={sf::Color(0,230,155),sf::Color(160,80,255),
                                sf::Color(255,210,60),sf::Color(0,200,255)};
                sf::CircleShape conf(5.f+d%5); conf.setOrigin(5.f,5.f);
                conf.setPosition(dx,dy); conf.setFillColor(cc[d%4]); window.draw(conf);
            }
        }

        window.display();
    }
    return 0;
}
