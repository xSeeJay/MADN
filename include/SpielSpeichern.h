#ifndef SAVE_GAME_ENGINE_H
#define SAVE_GAME_ENGINE_H

#include <QtGlobal>
#include <QDataStream>
#include <QFile>
#include <QVector>
#include <QString>
#include <QIODevice>

class Spiel;
class Spielfeld;
class Figur;

/* This class provides a mechanism to save/load game from/to a file(s)
 * It releys on QDataStream for serialization */
class Speichern {

public:
    virtual ~Speichern();

    //Serializes to a file
    static void serialize(QString filename, QVector<Figur*> pawns, Spielfeld *board, Spiel *game);

    /* Deserializes from a file and returns the state handle constructed on heap
     * NOTE: This class/function does not holds the ownership of the returned
     * object */
    static Speichern* deserialize(QString filename);

    //Reads an integer from the stream
    int readInt();

    //Writes an integer to the stream
    void writeInt(int i);

    //Reads a qreal from the stream
    qreal readReal();

    //Writes a qreal to the stream
    void writeReal(qreal r);

    Spiel* getGame();
    Spielfeld* getBoard();
    QVector<Figur*> getPawns();

private:
    explicit Speichern(QFile *file);

    QDataStream *stream {};

    Spiel *s_game {}; //Serialized game
    Spielfeld *s_board {}; //Serialized board
    QVector<Figur*> s_pawns {}; //Serialized pawns
};

#endif //SAVE_GAME_ENGINE_H
