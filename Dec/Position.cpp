#include "Position.h"

Position::Position(int x, int y) : x(x), y(y) {}

int Position::getX() const { return x; }
int Position::getY() const { return y; }
void Position::setX(int newX) { x = newX; }
void Position::setY(int newY) { y = newY; }

bool Position::operator<(const Position& other) const {
    return std::tie(x, y) < std::tie(other.x, other.y);
}

bool Position::operator==(const Position& other) const {
    return x == other.x && y == other.y;
}

