#include <Spielfigur.h>

#include <QDebug>

#include <Spielfeld.h>
#include <SpielerFarbe.h>
#include <SpielWindow.h>
#include <SpielSpeichern.h>
#include <ValueError.h>
#include <ZeichnungPosition.h>

const qreal Figur::GLOW_SCALE = 0.25;

Figur::Figur(SpielerFarbe color, int id) :
mColor(color), mId(id), mPos(-1) {

    //This is not the drawing size (refer to paintEvent())
    //this->setFixedSize(GameWindow::CELL_SIZE, GameWindow::CELL_SIZE);

    this->setGeometry(painthelp::getPawnHomePosGeometry(color, this->getIndex()));
}

Figur::Figur(Speichern *save) {
    this->mColor = static_cast<SpielerFarbe>(save->readInt());
    this->mId = save->readInt();
    this->mPos = save->readInt();
    this->mGlow = save->readReal();

    if(this->isAtHome())
        this->setGeometry(painthelp::getPawnHomePosGeometry(mColor, this->getIndex()));
    else
        this->setGeometry(painthelp::getPawnGeometry(this));
}

void Figur::serializeInto(Speichern *save) {
    save->writeInt(static_cast<int>(this->mColor));
    save->writeInt(this->mId);
    save->writeInt(this->mPos);
    save->writeReal(this->mGlow);
}

Figur::~Figur() {}

void Figur::attatchWindow(GameWindow *w) {
    this->setParent(w->centralWidget());
    this->setVisible(true);
    QObject::connect(this, &Figur::visualClicked, w, &GameWindow::pawnChosen);
    QObject::connect(this, &Figur::clashed, w, &GameWindow::pawnClashed);
}

int Figur::getRelPosition() {
    return mPos;
}

SpielerFarbe Figur::getColor() {
    return mColor;
}

unsigned int Figur::getId() {
    return mId;
}

unsigned int Figur::getIndex() {
    //index is extracted using id of the pawn, as the ids are sure to be in sequence
    return ((this->getId()+1) % 4) + 1;
}

QPoint Figur::getPositionOnBoard() {
    return Spielfeld::getSpielfigurKoordinaten(this);
}

bool Figur::isAtHome() {
    return mPos == HOME;
}

bool Figur::hasReachedDestination() {
    return mPos == DEST;
}

void Figur::changePosition(int rel) {
    if(rel > DEST || rel < HOME)
        ValueError::raise_new(QString("Pawn::goto(int) : Invalid move! rel == ").arg(rel));

    this->mPos = rel;
}

void Figur::moveForward(int steps) {
    if(this->mPos + steps > DEST || steps < 1 || steps > 6)
        ValueError::raise_new(QString("Pawn::moveForward(int) : Invalid move! step == ")
            .arg(steps));
    this->changePosition(this->mPos + steps);
}

void Figur::goBackHome(bool clash) {
    qInfo() << "Pawn::goBackHome()";
    if(clash)
        emit clashed(this);
    else
        this->changePosition(HOME);
}

void Figur::setGlow(qreal glow)  {
    if(glow < 0) {
        mGlow = 0;
    } else if(glow > 1) {
        mGlow = 1;
    } else {
        mGlow = glow;
    }

    this->update();
}

qreal Figur::getGlow() {
    return mGlow;
}

void Figur::setEnabled(bool state) {
    QWidget::setEnabled(state);

    if(glowEffect != nullptr) {
        delete glowEffect;
        glowEffect = nullptr;
    }

    if(state) { //Play animation
        glowEffect = new QPropertyAnimation(this, "glow", this);
        glowEffect->setStartValue(0);
        glowEffect->setKeyValueAt(0.5, 1);
        glowEffect->setEndValue(0);
        glowEffect->setLoopCount(-1);
        glowEffect->setDuration(GLOW_DURATION);
        glowEffect->start();

    } else
        setGlow(0);
}

void Figur::paintEvent(QPaintEvent*) {

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(
        GameWindow::STROKE_COLOR,
        GameWindow::STROKE_WIDTH
    ));

    QColor color {};
    switch (mColor) {
        case SpielerFarbe::RED:
            color = GameWindow::COLOR_RED;
            break;
        case SpielerFarbe::YELLOW:
            color = GameWindow::COLOR_YELLOW;
            break;
        case SpielerFarbe::BLUE:
            color = GameWindow::COLOR_BLUE;
            break;
        case SpielerFarbe::GREEN:
            color = GameWindow::COLOR_GREEN;
            break;
    }

    painter.setBrush(color);
    QRect r = this->rect();

    qreal scale = ((qreal) GameWindow::PAWN_SIZE / (qreal) GameWindow::CELL_SIZE) + (getGlow() * GLOW_SCALE);

    painter.translate(
        ((GameWindow::CELL_SIZE) - (r.width() * scale)) / 2,
        ((GameWindow::CELL_SIZE) - (r.width() * scale)) / 2
    );

    painter.drawEllipse(
        0,
        0,
        r.width() * scale,
        r.width() * scale
    );
}

void Figur::mousePressEvent(QMouseEvent*) {
    emit visualClicked(this);
}
