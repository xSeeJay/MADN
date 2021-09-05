#include <Spielfeld.h>

#include <QString>
#include <QDebug>

#include <Spielfigur.h>
#include <SpielerFarbe.h>
#include <Path.h>
#include <SpielSpeichern.h>
#include <ValueError.h>

Spielfeld::Spielfeld(unsigned int players) :
players_count(players) {

    /* Initialize the player vector */
    #pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
    switch(players) {
        case 4:
            mPawns.append(new Figur(SpielerFarbe::GREEN, 0));
            mPawns.append(new Figur(SpielerFarbe::GREEN, 1));
            mPawns.append(new Figur(SpielerFarbe::GREEN, 2));
            mPawns.append(new Figur(SpielerFarbe::GREEN, 3));
        case 3:
            mPawns.append(new Figur(SpielerFarbe::YELLOW, 4));
            mPawns.append(new Figur(SpielerFarbe::YELLOW, 5));
            mPawns.append(new Figur(SpielerFarbe::YELLOW, 6));
            mPawns.append(new Figur(SpielerFarbe::YELLOW, 7));
        case 2:
            mPawns.append(new Figur(SpielerFarbe::BLUE, 8));
            mPawns.append(new Figur(SpielerFarbe::BLUE, 9));
            mPawns.append(new Figur(SpielerFarbe::BLUE, 10));
            mPawns.append(new Figur(SpielerFarbe::BLUE, 11));

            mPawns.append(new Figur(SpielerFarbe::RED, 12));
            mPawns.append(new Figur(SpielerFarbe::RED, 13));
            mPawns.append(new Figur(SpielerFarbe::RED, 14));
            mPawns.append(new Figur(SpielerFarbe::RED, 15));
            break;

        default:
            ValueError::raise_new(QString("Invalid number of players. Expected value \
            between 2 to 4 but got %1").arg(players));

        #pragma GCC diagnostic warning "-Wimplicit-fallthrough"
    }
}

Spielfeld::Spielfeld(Speichern *save) {
    players_count = save->readInt();
    mPawns = save->getPawns();
}

void Spielfeld::serializeInto(Speichern *save) {
    save->writeInt(players_count);
}

/* Destory all the resources used by the class */
Spielfeld::~Spielfeld() {
    for (auto p : mPawns)
        delete p;
}

//Returns the number of playing game
unsigned int Spielfeld::getSpielerAnzahl() {
    return players_count;
}

/* Returns the coordinates of a pawn if it's color and relpos were same as given
 * This should not be called when the pawn is at home (relpos == -1) */
QPoint Spielfeld::getSpielfigurKoordinaten(SpielerFarbe color, unsigned int relpos) {
    qInfo() << "Board::getPawnCoordinates(PlayerColor, int) : relpos == " << relpos;

    if (relpos > Path::MAX_REL)
        ValueError::raise_new(QString("Board::getPawnCoordinates(PlayerColor, unsigned int) \
        : Invalid value for relpos == %1").arg(relpos));

    switch (color) {
        case SpielerFarbe::RED:
            return Path::getGenaueKoordinaten(relpos);
        case SpielerFarbe::YELLOW:
            return Path::rotatePointToRight(Path::getGenaueKoordinaten(relpos));
        case SpielerFarbe::BLUE:
            return Path::rotatePointToRight(Path::getGenaueKoordinaten(relpos), 2);
        case SpielerFarbe::GREEN:
            return Path::rotatePointToRight(Path::getGenaueKoordinaten(relpos), 3);

        //This will never happen
        default:
            ValueError::raise_new(QString("Board::getPawnCoordinates(PlayerColor, int) : Invalid PlayerColor"));
        }
    #pragma GCC diagnostic ignored "-Wreturn-type"
}
#pragma GCC diagnostic warning "-Wreturn-type"

/* Returns the coordinates of the pawn passed by pointer. Pawn is not required to be on board
 * This should not be called when the pawn is at home */
QPoint Spielfeld::getSpielfigurKoordinaten(Figur* p) {
    return getSpielfigurKoordinaten(p->getColor(), p->getRelPosition());
}

//Returns a vector, which holds all the Pawns on the board, including the ones won or at home
QVector<Figur*> Spielfeld::getAlleSpielfiguren() {
    return mPawns;
}

//Returns a vector holding all the Pawns located at the given point on the board
QVector<Figur*> Spielfeld::getSpielfigurAt(QPoint point) {
    qInfo() << "Board::getPawnsAt(QPoint)";

    QVector<Figur*> result {};
    for (auto pawn : mPawns) {
        if(pawn->isAtHome()) //Skip this one
            continue;
        if(getSpielfigurKoordinaten(pawn).x() == point.x() && getSpielfigurKoordinaten(pawn).y() == point.y())
            result.append(pawn);
    }

    return result;
}

/* Returns a pointer to the pawn with the passed id. Ids are unique to all pawns
 * on a board. Returned pawn is supposed to be on the board */
Figur* Spielfeld::getSpielfigurById(unsigned int id) {
    return mPawns[id];
}


/* Returns a vector of pawn pointers matching the given color */
QVector<Figur*> Spielfeld::getAlleSpielfigurenByFarbe(SpielerFarbe color) {
    if(color == SpielerFarbe::YELLOW && players_count == 2)
        ValueError::raise_new(QString("Board::getAllPawnsByColor : Current game does not have YELLOW color"));
    if(color == SpielerFarbe::GREEN && players_count != 4)
        ValueError::raise_new(QString("Board::getAllPawnsByColor : Current game does not have GREEN color"));

    QVector<Figur*> result {};
    for (auto p : mPawns) {
        if(p->getColor() == color)
            result.append(p);
    }

    return result;
}

/* Returns a pointer to the Pawn identified by it's color and index (1~4)
 * Returns the nullptr if the pawn does not exist */
Figur* Spielfeld::getPawn(SpielerFarbe color, unsigned int which) {
    if(color == SpielerFarbe::YELLOW && players_count == 2)
        return nullptr;
    if(color == SpielerFarbe::GREEN && players_count != 4)
        return nullptr;
    if(which > 4)
        return nullptr;

    int id {};
    switch (color) {
        case SpielerFarbe::GREEN:
            id = 0;
            break;
        case SpielerFarbe::YELLOW:
            id = 4;
            break;
        case SpielerFarbe::BLUE:
            id = 8;
            break;
        case SpielerFarbe::RED:
            id = 12;
            break;
    }

    return mPawns[id+which-1];
}
