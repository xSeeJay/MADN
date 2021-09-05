#ifndef GAME_WINDOW_H
#define GAME_WINDOW_H

#include <QString>
#include <QObject>
#include <QMainWindow>
#include <QWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QVector>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include <QEasingCurve>
#include <QTimer>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>

#include <SpielScreen.h>

class Spiel;
class Spielfeld;
class Figur;
class Dice;
class Speichern;
enum class SpielerFarbe;

class GameWindow : public QMainWindow {
Q_OBJECT

public:
    /* Defines game state.
     * Rolling : dice needs to be rolled.
     * MOVING : player needs to choose a pawn to move
     * ANIMATING : An animation is playing */
    enum GameState {ROLLING, MOVING, ANIMATING};

    //Colors for each player, background and stroke
    static const QColor COLOR_RED;
    static const QColor COLOR_YELLOW;
    static const QColor COLOR_BLUE;
    static const QColor COLOR_GREEN;
    static const QColor BG_COLOR;
    static const QColor STROKE_COLOR;

    //Color for dice background
    static const QColor COLOR_RED_LIGHT;
    static const QColor COLOR_YELLOW_LIGHT;
    static const QColor COLOR_BLUE_LIGHT;
    static const QColor COLOR_GREEN_LIGHT;

    //Space to leave on all sides, apart from board
    static const int BOARD_BOUND = 5;

    //Width and height of each cell
    static const int CELL_SIZE = 40;

    //guider-dot size
    static const int GUIDER_DOT_SIZE = 4;

    //Pawn visual size
    static const int PAWN_SIZE = 25;

    //Dice size
    static const int DICE_SIZE = 50;

    //Stroke size
    static const int STROKE_WIDTH = 2;

    //Time taken for animating single step move of a pawn
    static const int ANIMATION_DURATION = 120;

    //Time taken for dice roll animation
    static const int DICE_ANIMATION_DURATION = 800;

    //Distance the dice is shaken
    static const int DICE_SHAKE_DISTANCE = 25;

    //Constructor and destructor
    explicit GameWindow(unsigned int players_count);

    //Constructs from save state
    explicit GameWindow(Speichern *save);

    ~GameWindow();

    /*Returns a user-friendly string representation of user name*/
    QString getUserName(SpielerFarbe color);

    /*Properly updates the UI according to the value of `state` and current player*/
    void updateUi();

    /* Creates a dialog for saving file
     * Returns: true if game was saved, false otherwise */
    bool saveGame();

signals:
    /* Called when game has exit, after completing all the events on close.
     * Memory held by this window must be freed here */
    void exit();

public slots:
    /* Called when a pawn has clashed and needs to go back home
    * pawn: the pawn which is to be sent home */
    void pawnClashed(Figur *pawn);

    /* Called when a pawn visual is clicked upon and enabled */
    void pawnChosen(Figur *p);

    /* Called when dice visual is clicked */
    void rollDiceClicked();

private slots:

    /* Called when the pawn visual animation is finished
     * p: the Pawn which was animated */
    void pawnAnimationFinished(Figur*);

    /* Called when the dice roll animation is finished */
    void diceAnimationFinished();

    /* Called when user chooses to save the game */
    void saveRequested();

    /* Called when user wants to load a saved game */
    void loadRequested();

    /* Called when user clicks 'about' action from menu bar */
    void aboutRequested();

    /* Called when user asks for exiting game through menu bar */
    void exitRequested();

private:
    unsigned int players_count {}; //No of players
    Spiel *mGame {}; //current game
    Spielfeld *mBoard {};
    GameState state {};

    GameScreen *mScreen {}; //Central widget

    QWidget *footer {}; //Widget for user-interaction
    QVBoxLayout *footerLayout {}; //Layout for footer
    Dice *dice; //Dice control
    QLabel *hintLabel {}; //Hint text
    QSequentialAnimationGroup *animationGroup {}; //Pawn animation
    QParallelAnimationGroup *diceAnimationGroup {}; //Dice Rolling animation

    bool needs_to_save {false}; //User has unsaved changes if this is true

    //Initializes window. Convinence function. Called only in constructors and once
    void init();

    /*Causes the pawn visual to move*/
    void movePawnVisual(Figur *p, int oldpos);

    //Moves the pawn visuals through an animation
    void animateVisualMovement(Figur* p, int startRel);

    //Animates the dice before showing of it's value
    void animateDiceRoll();

protected:
    void closeEvent(QCloseEvent*);
};
#endif //GameWindow.h
