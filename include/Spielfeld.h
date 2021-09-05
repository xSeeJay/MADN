#ifndef SPIELFELD_H
#define SPIELFELD_H

#include <string>

#include <QVector>
#include <QPoint>

class Figur;
class Speichern;
enum class SpielerFarbe;

class Spielfeld {
public:

    //Constructor. The players - no. of players playing the game
    Spielfeld(unsigned int players);

    //Creates a new instance and coppies the data from save state
    Spielfeld(Speichern *speichern);

    //Copies the data to save state
    void serializeInto(Speichern *speichern);

    //Virtual destructor
    virtual ~Spielfeld();

    //Returns the number of playing game
    unsigned int getSpielerAnzahl();

    //Returns the coordinates of a pawn if it's color and relpos were same as given
    static QPoint getSpielfigurKoordinaten(SpielerFarbe color, unsigned int relpos);

    //Returns the coordinates of the pawn passed by pointer. Pawn is not required to be on board
    static QPoint getSpielfigurKoordinaten(Figur* p);

    //Returns a vector, which holds all the Pawns on the board, including the ones won or at home
    QVector<Figur*> getAlleSpielfiguren();

    //Returns a vector holding all the Pawns located at the given point on the board
    QVector<Figur*> getSpielfigurAt(QPoint point);

    //Returns a pointer to the pawn with the passed id. Ids are unique to all pawns
    //on a board. Returned pawn is supposed to be on the board
    Figur* getSpielfigurById(unsigned int id);

    //Returns a vector of pawn pointers matching the given color
    QVector<Figur*> getAlleSpielfigurenByFarbe(SpielerFarbe farbe);

    //Returns a pointer to the Pawn identified by it's color and index (1~4)
    //Returns the nullptr if the pawn does not exist
    Figur* getPawn(SpielerFarbe color, unsigned int which);

private:
    unsigned int players_count {}; //Count of players
    QVector<Figur*> mPawns {}; //Vector holding pawn handles
};

#endif //Board.h
