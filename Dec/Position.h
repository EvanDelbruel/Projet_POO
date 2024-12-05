#ifndef POSITION_H
#define POSITION_H

#include <tuple>

class Position {
private:
    int x, y;

public:
    Position(int x = 0, int y = 0);
    int getX() const;
    int getY() const;
    void setX(int newX);
    void setY(int newY);

    bool operator<(const Position& other) const;
    bool operator==(const Position& other) const;
};

#endif // POSITION_H

