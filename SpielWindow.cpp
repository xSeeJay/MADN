#include <SpielWindow.h>

#include <QDebug>

#include <Spiel.h>
#include <Spielfeld.h>
#include <Spielfigur.h>
#include <SpielerFarbe.h>
#include <Wuerfel.h>
#include <ZeichnungPosition.h>
#include <ValueError.h>
#include <UeberUns.h>
#include <SpielSpeichern.h>

const QColor GameWindow::COLOR_RED = QColor (249,94,95);
const QColor GameWindow::COLOR_YELLOW = QColor (255,238,103);
const QColor GameWindow::COLOR_BLUE = QColor (3,175,212);
const QColor GameWindow::COLOR_GREEN = QColor (121,189,154);
const QColor GameWindow::BG_COLOR = QColor (251,248,248);
const QColor GameWindow::STROKE_COLOR = QColor (128,128,128);

const QColor GameWindow::COLOR_RED_LIGHT = QColor (255,134,134);
const QColor GameWindow::COLOR_YELLOW_LIGHT = QColor (255,252,151);
const QColor GameWindow::COLOR_BLUE_LIGHT = QColor (102,212,224);
const QColor GameWindow::COLOR_GREEN_LIGHT = QColor (153,235,148);

GameWindow::GameWindow(unsigned int players_count) :
mGame {new Spiel(players_count)},
mBoard {mGame->getSpielfeld()},
state {ROLLING},
mScreen (new GameScreen(this)),
footer {new QWidget(this)},
footerLayout {new QVBoxLayout()},
dice {new Dice(nullptr, 6)},
hintLabel {new QLabel()} {

    init();
}

GameWindow::GameWindow(Speichern *save) :
    state {ROLLING},
    mScreen (new GameScreen(this)),
    footer {new QWidget(this)},
    footerLayout {new QVBoxLayout()},
    dice {new Dice(nullptr, 6)},
    hintLabel {new QLabel()} {

    this->mBoard = save->getBoard();
    this->mGame = save->getGame();

    init();
}

void GameWindow::init() {

    this->setCentralWidget(this->mScreen);
    this->setWindowTitle("Ludo Z+");

    //Setup menu bar
    QMenuBar *menuBar = this->menuBar();
    QMenu *gameMenu = menuBar->addMenu("&Game");
    QAction *saveAction = gameMenu->addAction(QIcon::fromTheme("file-save"), "&Save");
    QAction *loadAction = gameMenu->addAction(QIcon::fromTheme("file-save"), "&Load");
    gameMenu->addSeparator();
    QAction *aboutAction = gameMenu->addAction(QIcon::fromTheme("file-save"), "&About");
    QAction *exitAction = gameMenu->addAction(QIcon::fromTheme("file-save"), "&Exit");
    connect(saveAction, &QAction::triggered, this, &GameWindow::saveRequested);
    connect(loadAction, &QAction::triggered, this, &GameWindow::loadRequested);
    connect(aboutAction, &QAction::triggered, this, &GameWindow::aboutRequested);
    connect(exitAction, &QAction::triggered, this, &GameWindow::exitRequested);

    dice->setVisualSize(DICE_SIZE);
    hintLabel->setFixedHeight(CELL_SIZE);

    footer->setFixedSize(CELL_SIZE * 5, dice->height()+hintLabel->height());
    footer->move((CELL_SIZE * 15),
        BOARD_BOUND + (this->height() / 2)); //To center-right

    hintLabel->setWordWrap(true);
    dice->setColor(COLOR_RED_LIGHT);
    hintLabel->setFixedWidth(footer->geometry().width());
    hintLabel->setAlignment(Qt::AlignCenter);

    footerLayout->addWidget(dice, 0, Qt::AlignCenter);
    footerLayout->addWidget(hintLabel, 0, Qt::AlignCenter);
    footer->setLayout(footerLayout);

    this->mScreen->setFixedSize(
        (BOARD_BOUND * 2) + (CELL_SIZE * 15) + footer->width(), //Extra space for interaction widget,
        (BOARD_BOUND * 2) + (CELL_SIZE * 15)
    );

    QObject::connect(dice, SIGNAL(clicked()), this, SLOT(rollDiceClicked()));

    //Attach all pawns with the game window
    for(auto pawn : mBoard->getAlleSpielfiguren()) {
        pawn->attatchWindow(this);
        pawn->setEnabled(false); //Disable all the pawn visuals (buttons)
    }

    updateUi();
}

GameWindow::~GameWindow() {
    delete dice;
    delete hintLabel;
    delete footerLayout;
    delete footer;
    delete mGame;

    delete mScreen;
}

QString GameWindow::getUserName(SpielerFarbe color) {
    //TODO: Replace this temporary code with something more sensible
    switch (color) {
        case SpielerFarbe::RED:
            return "Player 1";
        case SpielerFarbe::YELLOW:
            return "Player 2";
        case SpielerFarbe::BLUE:
            return "Player 3";
        case SpielerFarbe::GREEN:
            return "Player 4";

        //This will never happen
        default:
            ValueError::raise_new(QString("GameWindow::getUserName(PlayerColor) : Invalid value of PlayerColor!"));
    }
    #pragma GCC diagnostic ignored "-Wreturn-type"
}
#pragma GCC diagnostic warning "-Wreturn-type"

void GameWindow::updateUi() {
    qInfo() << "GameWindow::updateUi()";

    //Set appropriate color for dice
    switch (mGame->getCurrentPlayer()) {
        case SpielerFarbe::RED:
            dice->setColor(COLOR_RED_LIGHT);
            break;
        case SpielerFarbe::YELLOW:
            dice->setColor(COLOR_YELLOW_LIGHT);
            break;
        case SpielerFarbe::BLUE:
            dice->setColor(COLOR_BLUE_LIGHT);
            break;
        case SpielerFarbe::GREEN:
            dice->setColor(COLOR_GREEN_LIGHT);
            break;
    }

    if (state == ROLLING) {
        hintLabel->setText(
            QString("%1: Roll the dice!")
                .arg(getUserName(mGame->getCurrentPlayer()))
            );
        for (auto p : mBoard->getAlleSpielfiguren())
            p->setEnabled(false);
        dice->setEnabled(true);

    } else if (state == MOVING) {
        hintLabel->setText(
            QString("%1: Choose a pawn to move!")
                .arg(getUserName(mGame->getCurrentPlayer()))
            );
        dice->setEnabled(false);

    } else if (state == ANIMATING) {
        hintLabel->setText(QString(""));
        for (auto p : mBoard->getAlleSpielfiguren())
            p->setEnabled(false);
        dice->setEnabled(false);

    } else
        ValueError::raise_new(QString("GameWindow::updateUi() : Invalid value for GameWindow::state"));
}

void GameWindow::rollDiceClicked() {

    hintLabel->setText(QString(""));
    animateDiceRoll();
}

void GameWindow::animateDiceRoll() {
    state = ANIMATING;
    updateUi();

    if(diceAnimationGroup != 0)
        delete diceAnimationGroup;

    diceAnimationGroup = new QParallelAnimationGroup {};

    QPropertyAnimation *rollanimation = new QPropertyAnimation {dice, "rotation", diceAnimationGroup};
    rollanimation->setStartValue(0);
    rollanimation->setEndValue(360*8);
    rollanimation->setDuration(DICE_ANIMATION_DURATION);
    rollanimation->setEasingCurve(QEasingCurve::InOutQuad);

    QSequentialAnimationGroup *shake = new QSequentialAnimationGroup {diceAnimationGroup};

    for (int i = 0; i < 5; i++) {
        QPropertyAnimation *s = new QPropertyAnimation {dice, "pos", shake};
        s->setStartValue(QPoint(dice->x(), dice->y()));
        s->setKeyValueAt(0.25, QPoint(dice->x() - DICE_SHAKE_DISTANCE, dice->y()));
        s->setKeyValueAt(0.50, QPoint(dice->x(), dice->y()));
        s->setKeyValueAt(0.75, QPoint(dice->x() + DICE_SHAKE_DISTANCE, dice->y()));
        s->setEndValue(QPoint(dice->x(), dice->y()));
        s->setDuration(DICE_ANIMATION_DURATION / 5);
        s->setEasingCurve(QEasingCurve::InOutQuad);
        shake->addAnimation(s);
    }

    diceAnimationGroup->addAnimation(rollanimation);
    diceAnimationGroup->addAnimation(shake);
    diceAnimationGroup->start();

    connect(diceAnimationGroup, &QParallelAnimationGroup::finished,
        this,&GameWindow::diceAnimationFinished);
}

void GameWindow::diceAnimationFinished() {
    qDebug() << "GameWindow::diceAnimationFinished()";

    dice->setValue(mGame->wuerfelWuerfel());

    QVector<Figur*> playables = mGame->getSpielbareSpielfiguren(mGame->getLastDiceValue());

    if(playables.size() == 0) {
        mGame->changeCurrentPlayer(); //We got no pawns worth moving
        //But we need to wait for a second!

        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [timer, this](){
            this->updateUi();
            delete timer; //No need to stop it, just delete it
        });

        timer->start(700);
        this->state = ROLLING;
        return; //It makes it look like we're still animating, but it's just a small pause between dice rollings

    } else if(playables.size() == 1) { //Only a singal move available, rather not ask for it
        pawnChosen(playables[0]); //Auto choosen

    } else {
        for (auto p : playables)
            p->setEnabled(true);

        state = MOVING;
    }

    updateUi();
}

void GameWindow::pawnChosen(Figur *p) {
    qInfo() << "GameWindow::pawnChosen()";

    qDebug() << "mGame->getLastDiceValue()" << mGame->getLastDiceValue();
    qDebug() << "Pawn::getRelPosition() == " << p->getRelPosition();
    needs_to_save = true; //Changes made to game
    p->raise(); //So that it is visible on top
    movePawnVisual(p, mGame->predictRel(p, mGame->getLastDiceValue()));
}

void GameWindow::movePawnVisual(Figur *p, int newrel) {
    qInfo() << "GameWindow::movePawnVisual()";

    if(p->isAtHome()) {
        QRect geom = painthelp::getPawnGeometry(Spielfeld::getSpielfigurKoordinaten(
            p->getColor(), newrel
        ));
        p->setGeometry(geom);
        pawnAnimationFinished(p); //No animation, so no wait
        state = ROLLING;

    } else {
        //Our magic spell.. Ha..ha..
        state = ANIMATING;
        animateVisualMovement(p, newrel);
    }

    updateUi();
}

void GameWindow::animateVisualMovement(Figur* p, int endRel) {
    qInfo() << "GameWindow::animateVisualMovement() endRel == " << endRel;

    if(animationGroup != 0)
        delete animationGroup;

    animationGroup = new QSequentialAnimationGroup();
    int startRel = p->getRelPosition();

    for (int i = startRel + 1; i <= endRel; i++) {
        QRect iniCell = painthelp::getPawnGeometry(mBoard->getSpielfigurKoordinaten(p->getColor(), i-1));
        QRect destCell;

        if(i == Figur::DEST)
            //It's stepping into destination, so...
            destCell = painthelp::getPawnDestGeometry(p);
        else
            destCell = painthelp::getPawnGeometry(mBoard->getSpielfigurKoordinaten(p->getColor(), i));

        QPropertyAnimation *animation = new QPropertyAnimation (p, "geometry");
        animation->setDuration(ANIMATION_DURATION);
        animation->setStartValue(iniCell);
        animation->setEndValue(destCell);
        animationGroup->addAnimation(animation);

        animationGroup->addPause(100); //This feels smooth
    }

    connect(animationGroup, &QSequentialAnimationGroup::finished, this, [this, p](){this->pawnAnimationFinished(p);});
    animationGroup->start();
}

void GameWindow::pawnAnimationFinished(Figur *p) {
    if (!mGame->playMove(p, mGame->getLastDiceValue()))
        mGame->changeCurrentPlayer();

    state = ROLLING;
    updateUi();
}

void GameWindow::pawnClashed(Figur *p) {
    p->changePosition(Figur::HOME);
    //This line is used in the the Constructor of Pawn and has details in the comment there
    p->setGeometry(painthelp::getPawnHomePosGeometry(p->getColor(), ((p->getId()+1) % 4) + 1));
}

void GameWindow::saveRequested() {
    saveGame();
}

bool GameWindow::saveGame() {
    QString filename = QFileDialog::getSaveFileName(this,
        "Save Game",
        "",
        "Ludo Z+ game save (*.lzs)");
    if(filename == "") //Canceled
        return false;
    Speichern::serialize(filename, mBoard->getAlleSpielfiguren(), mBoard, mGame);
    return true;
}

void GameWindow::loadRequested() {
    qDebug() << "Load game requested";

    if(this->needs_to_save) {

        QMessageBox dialog {};
        dialog.setIcon(QMessageBox::Question);
        dialog.setText("Confirm load");
        dialog.setInformativeText("Your current game has some unsaved changes. \
        Are you sure to discard those and load a new game?");

        dialog.addButton(QMessageBox::Yes);
        dialog.addButton(QMessageBox::No);

        int ret = dialog.exec();

        switch (ret) {
            case QMessageBox::Yes:
                break; //Let this game be discarded
            case QMessageBox::No:
                return; //Skip loading
        }
    }

    QString filename = QFileDialog::getOpenFileName(this,
        "Load Game",
        "",
        "Ludo Z+ game save (*.lzs)");
    if(filename == "")
        return; //Canceled
    Speichern *saveState = Speichern::deserialize(filename);

    delete this->mBoard;

    this->mGame = saveState->getGame();
    this->mBoard = saveState->getBoard();
    this->state = ROLLING;

    for(auto p : this->mBoard->getAlleSpielfiguren())
        p->attatchWindow(this);

    needs_to_save = false;
    updateUi();
}

void GameWindow::aboutRequested() {
    UeberUnsDialog::show();
}

void GameWindow::exitRequested() {
    this->close(); //It automatically goes to GameWindow::closeEvent()
}

void GameWindow::closeEvent(QCloseEvent* event) {
    if(!needs_to_save)
        return;

    QMessageBox dialog {};
    dialog.setIcon(QMessageBox::Question);
    dialog.setText("Confirm exit");
    dialog.setInformativeText("Would you like to save the game before exiting?");

    dialog.addButton(QMessageBox::Save);
    dialog.addButton(QMessageBox::Discard);
    dialog.addButton(QMessageBox::Cancel);

    int ret = dialog.exec();

    switch (ret) {
        case QMessageBox::Cancel:
            event->ignore(); //Do nothing
            break;
        case QMessageBox::Save:
            //TODO: Open save dialog here
            event->accept();
            emit exit();
            break;
        case QMessageBox::Discard:
            event->accept();
            break;
    }
}
