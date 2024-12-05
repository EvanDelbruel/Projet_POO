#ifndef REGLE_H
#define REGLE_H

class Regle {
public:
    virtual bool application(bool etat, int nombreDeVoisins) const = 0;
};

class RegleSt : public Regle {
public:
    bool application(bool etat, int nombreDeVoisins) const override;
};

#endif // REGLE_H

