#include <SpielSpeichern.h>

#include <Spielfigur.h>
#include <Spiel.h>
#include <Spielfeld.h>

Speichern::Speichern(QFile *file) {
	stream = new QDataStream(file);
}

Speichern::~Speichern() {}

//Serializes to a file
void Speichern::serialize(QString filename, QVector<Figur*> pawns, Spielfeld *board, Spiel *game) {
	QFile file(filename);
    file.open(QIODevice::WriteOnly);
    Speichern *save = new Speichern(&file);

    *(save->stream) << pawns.size(); //Save it for later use!
    for(auto p : pawns)
        p->serializeInto(save);
    board->serializeInto(save);
    game->serializeInto(save);

    delete save;
}

//Deserializes from a file
Speichern* Speichern::deserialize(QString filename) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    Speichern *save = new Speichern(&file);

	int pawn_n {};
	*(save->stream) >> pawn_n; //Retrive number of pawns
    for (int i = 0; i < pawn_n; i++)
        save->s_pawns.append(new Figur(save));
    save->s_board = new Spielfeld(save);
    save->s_game = new Spiel(save);

    return save;
}

int Speichern::readInt() {
    int i {};
    *stream >> i;
    return i;
}

void Speichern::writeInt(int i) {
    *stream << i;
}

qreal Speichern::readReal() {
    qreal r;
    *stream >> r;
    return r;
}

void Speichern::writeReal(qreal r) {
    *stream << r;
}

Spiel* Speichern::getGame() {
    return s_game;
}

Spielfeld* Speichern::getBoard() {
    return Speichern::s_board;
}

QVector<Figur*> Speichern::getPawns() {
    return s_pawns;
}
