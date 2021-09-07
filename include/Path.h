#ifndef PATH_H
#define PATH_H

#include <string>

#include <QPoint>
#include <QVector>

/* Diese Klasse zeigt wo genau auf dem Spielfeld eine Spielfigur sein sollte.
 * Es wandelt die relative Position einer Spielfigur auf dem Spielfeld um.
 * Die Klasse besitzt nur alle Koordinaten. */
class Path {
public:
    static const unsigned int MAX_REL; //Der maximale Wert, den eine Spielfigur haben kann

    //Gibt die Spielfigurkoordinaten auf dem Spielfeld zurück mithilfe der relativen Position
    static QPoint getGenaueKoordinaten(unsigned int rel);
    static QPoint rotatePointToRight(QPoint p);
    static QPoint rotatePointToRight(QPoint p, int times);
private:
    static QVector<QPoint> path; //Hier werden die Koordinaten gespeichert
    static const int TOP = 14; //Insgesamte Anzahl von Felder in einer Reihe
};

#endif //Path.h
