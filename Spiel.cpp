#include <Spiel.h>

#include <QDebug>

#include <Spielfeld.h>
#include <Spielfigur.h>
#include <SpielerFarbe.h>
#include <SpielSpeichern.h>
#include <ValueError.h>

Spiel::Spiel(unsigned int players) :
    players_count(players), mBoard(new Spielfeld {players}),
    currentSequence(new QVector<SpielerFarbe> {}), current (0),
    random (QRandomGenerator::securelySeeded()) {

    switch(players) {
        case 2:
            currentSequence->push_back(SpielerFarbe::RED);
            currentSequence->push_back(SpielerFarbe::BLUE); //So that pawns are at opposite corners
            break;
        case 3:
            currentSequence->push_back(SpielerFarbe::RED);
            currentSequence->push_back(SpielerFarbe::YELLOW);
            currentSequence->push_back(SpielerFarbe::BLUE);
            break;
        case 4:
            currentSequence->push_back(SpielerFarbe::RED);
            currentSequence->push_back(SpielerFarbe::YELLOW);
            currentSequence->push_back(SpielerFarbe::BLUE);
            currentSequence->push_back(SpielerFarbe::GREEN);
            break;
    }
}

Spiel::Spiel(Speichern *save) {
    this->players_count = save->readInt();
    
    this->currentSequence = new QVector<SpielerFarbe> {};
    for(unsigned int i = 0; i < players_count; i++)
        this->currentSequence->append(static_cast<SpielerFarbe>(save->readInt()));
    
    this->current = save->readInt();
    this->lastDiceValue = save->readInt();
    
    this->mBoard = save->getBoard();
    
    this->random = QRandomGenerator::securelySeeded();
}

void Spiel::serializeInto(Speichern *save) {
    save->writeInt(this->players_count);
    
    for(auto p : *(this->currentSequence))
        save->writeInt(static_cast<int>(p));
    
    save->writeInt(this->current);
    save->writeInt(this->lastDiceValue);
}

Spiel::~Spiel() {
    delete currentSequence;
    delete mBoard;
}

int Spiel::wuerfelWuerfel() {
    //TODO:: Generate a random number between 1 to 6

    lastDiceValue = random.bounded(1, 7);
    return lastDiceValue;
}

SpielerFarbe Spiel::getCurrentPlayer() {
    return (*currentSequence)[current];
}

Spielfeld* Spiel::getSpielfeld() {
    return mBoard;
}

QVector<Figur*> Spiel::getSpielbareSpielfiguren(int diceFace) {
    if(diceFace < 1 || diceFace > 6)
        ValueError::raise_new(QString("Invalid dice value : %1").arg(diceFace));

    QVector<Figur*> result;
    QVector<Figur*> pawns = mBoard->getAlleSpielfiguren();

    for(auto p : pawns) {
        if(p->getColor() != getCurrentPlayer())
            continue;
        if(p->isAtHome() && diceFace != 6 && SIX_FOR_HOME)
            continue;
        if(p->hasReachedDestination())
            continue;
        if(p->getRelPosition() + diceFace > Figur::DEST)
            continue;

        result.append(p);
    }

    return result;
}

unsigned int Spiel::predictRel(Figur* pawn, unsigned int diceFace) {
    qInfo() << "Game::predictRel(Pawn*, int)";

    if(pawn->isAtHome() && diceFace != 6 && SIX_FOR_HOME)
        ValueError::raise_new(QString("Invalid move"));
    if(pawn->getRelPosition() + diceFace > Figur::DEST)
        ValueError::raise_new(QString("Invalid move"));

    if(pawn->isAtHome()) //Just get out of home by one step
        return pawn->getRelPosition() + 1;
    else
        return pawn->getRelPosition() + diceFace;
}

bool Spiel::playMove(Figur* pawn, int diceFace) {
    qInfo() << "Game::playMove(Pawn*, int)";

    //Set it to the number by which we're gonna move
    unsigned int futureRel {Spiel::predictRel(pawn, diceFace)};

    //Will the player get a turn again?
    bool re_turn = diceFace == 6;

    //Pawn which was hit by this one while moving
    Figur* toClash {nullptr};

    //This pawn couldn't have been clashed with anybody, because it has reached destination
    if(futureRel != Figur::DEST) {

        //If there is only one pawn at this new location, send it back home
        //This should be done before moving the current pawn
        QVector<Figur*> pawnsThere = mBoard->getSpielfigurAt(
            mBoard->getSpielfigurKoordinaten(
                pawn->getColor(),
                futureRel //Our future position
            )
        );

        if(pawnsThere.size() == 1 && pawnsThere[0]->getColor() != pawn->getColor()) {
            toClash = pawnsThere[0];
            re_turn = true;
        }

    } else {
        //But we do get a turn again,..
        re_turn = true;
    }

    //We can now move the pawn
    pawn->changePosition(futureRel);

    //And send the clashed one home
    if(toClash != nullptr)
        toClash->goBackHome();

    return re_turn;
}

void Spiel::changeCurrentPlayer() {
    if(current >= (unsigned int)(currentSequence->size())-1) {
        current = 0;
        return;
    }

    current++;
}

QVector<SpielerFarbe> Spiel::getCurrentPlayerSequence() {
    return *currentSequence;
}

unsigned int Spiel::getLastDiceValue() {
    return lastDiceValue;
}

bool Spiel::playerHasFinished(SpielerFarbe c) {
    for(auto p : mBoard->getAlleSpielfigurenByFarbe(c))
        if(!p->hasReachedDestination())
            return false;

    return false;
}

bool Spiel::isFinished() {
    unsigned int i = 0;
    for (auto c : *currentSequence)
        if(playerHasFinished(c))
            i++;

    return i >= players_count - 1;
}
