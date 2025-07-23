#include <iostream>

#include <SFML/Graphics.hpp>
#include <fstream>
#include <sstream>

     
#include <SFML/Audio.hpp>

using namespace std;

enum GameState { Menu, InGame, WinScreen, HighScores };
enum PlayState { ServePlayerOne, ServePlayerTwo, Playing };
enum ButtonState { UP, DOWN, HOVER };
enum NameEntryState { NoEntry, EnteringP1Name, EnteringP2Name };

class Vector2D { 
public:
    float x, y;
    Vector2D(float x = 0, float y = 0) : x(x), y(y) {}
};

class RectangleShapeData {
public:
    float x, y, width, height;
    RectangleShapeData(float x = 0, float y = 0, float w = 0, float h = 0)
        : x(x), y(y), width(w), height(h) {
    }
};

class Button {
private:
    string m_text;
    RectangleShapeData m_positionAndSize;
    sf::Color m_colorUp, m_colorDown, m_colorHover;
    ButtonState m_status;

public:
    Button(const string& text, const RectangleShapeData& posAndSize,
        sf::Color up, sf::Color down, sf::Color hover)
        : m_text(text), m_positionAndSize(posAndSize),
        m_colorUp(up), m_colorDown(down), m_colorHover(hover), m_status(UP) {
    }

    bool HandleInput(Vector2D mousePos, bool mousePressed) {
        bool isInside = mousePos.x >= m_positionAndSize.x &&
            mousePos.x <= m_positionAndSize.x + m_positionAndSize.width &&
            mousePos.y >= m_positionAndSize.y &&
            mousePos.y <= m_positionAndSize.y + m_positionAndSize.height;

        if (isInside) {
            m_status = mousePressed ? DOWN : HOVER;
            return mousePressed;
        }
        else {
            m_status = UP;
            return false;
        }
    }

    void Draw(sf::RenderWindow& window, sf::Font& font) {
        sf::RectangleShape shape(sf::Vector2f(m_positionAndSize.width, m_positionAndSize.height));
        shape.setPosition(m_positionAndSize.x, m_positionAndSize.y);
        shape.setFillColor(m_status == DOWN ? m_colorDown : (m_status == HOVER ? m_colorHover : m_colorUp));

        sf::Text text(m_text, font, 20);
        text.setFillColor(sf::Color::White);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.width / 2, bounds.height / 2);
        text.setPosition(m_positionAndSize.x + m_positionAndSize.width / 2,
            m_positionAndSize.y + m_positionAndSize.height / 2);

        window.draw(shape);
        window.draw(text);
    }

    string getText() const { return m_text; }
};

class GameObject {
public:
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void update() {}
    virtual ~GameObject() {}
};

class Paddle : public GameObject {
public:
    sf::RectangleShape rect;
    Paddle(float x, float y, float width, float height, sf::Color color) {
        rect.setPosition(x, y);
        rect.setSize(sf::Vector2f(width, height));
        rect.setFillColor(color);
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(rect);
    }

    void update() override {
    }
};

class Ball : public GameObject {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;

    Ball(float radius) {
        shape.setRadius(radius);
        shape.setFillColor(sf::Color::White);
        shape.setPosition(400, 300);
        velocity = sf::Vector2f(0, 0);
    }

    void update() override {
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& window) override {
        window.draw(shape);
    }

    void setPosition(float x, float y) {
        shape.setPosition(x, y);
    }

    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

    void reset() {
        setPosition(400, 300);
        velocity = sf::Vector2f(0, 0);
    }
};

class Court {
public:
    void draw(sf::RenderWindow& window) {
        sf::RectangleShape background(sf::Vector2f(800, 600));
        background.setFillColor(sf::Color::Black);
        window.draw(background);

        sf::RectangleShape centerLine(sf::Vector2f(2, 600));
        centerLine.setFillColor(sf::Color::White);
        centerLine.setPosition(399, 0);
        window.draw(centerLine);

        sf::CircleShape centerCircle(60);
        centerCircle.setOrigin(60, 60);
        centerCircle.setPosition(400, 300);
        centerCircle.setFillColor(sf::Color::Transparent);
        centerCircle.setOutlineThickness(2);
        centerCircle.setOutlineColor(sf::Color::White);
        window.draw(centerCircle);
    }
};

class ScoreBoard {
private:
    sf::Text p1Text, p2Text;
    int* p1Score;
    int* p2Score;

public:
    ScoreBoard(sf::Font& font, int* s1, int* s2) : p1Score(s1), p2Score(s2) {
        p1Text.setFont(font);
        p1Text.setCharacterSize(30);
        p1Text.setPosition(100, 20);
        p1Text.setFillColor(sf::Color::White);

        p2Text.setFont(font);
        p2Text.setCharacterSize(30);
        p2Text.setPosition(600, 20);
        p2Text.setFillColor(sf::Color::White);
    }

    void draw(sf::RenderWindow& window) {
        p1Text.setString("P1: " + to_string(*p1Score));
        p2Text.setString("P2: " + to_string(*p2Score));
        window.draw(p1Text);
        window.draw(p2Text);
    }
};

class PlayerScore {
public:
    string name;
    int score;

    PlayerScore(const string& n = "", int s = 0) : name(n), score(s) {}
};

class PongMenu {

    
    Button botButton;
    Button pvpButton;
    Button highScoreButton;
    Button quitButton;

    
    sf::Text titleText;

public:
    PongMenu()
        : botButton("Player vs Bot", RectangleShapeData(300, 150, 200, 60), sf::Color(100, 100, 255), sf::Color::Blue, sf::Color(150, 150, 255)),
        pvpButton("Player vs Player", RectangleShapeData(300, 230, 200, 60), sf::Color(255, 100, 100), sf::Color::Red, sf::Color(255, 150, 150)),
        highScoreButton("High Scores", RectangleShapeData(300, 310, 200, 60), sf::Color(0, 200, 0), sf::Color::Green, sf::Color(100, 255, 100)),
        quitButton("Quit", RectangleShapeData(300, 390, 200, 60), sf::Color(200, 200, 0), sf::Color::Yellow, sf::Color(255, 255, 100)) {

      
        titleText.setString("PONG GAME");
        titleText.setCharacterSize(70);  
        titleText.setFillColor(sf::Color::White);

       
        titleText.setPosition(185, 40);  
    }

    void draw(sf::RenderWindow& window, sf::Font& font) {
       
        titleText.setFont(font);

       
        window.draw(titleText);

        
        botButton.Draw(window, font);
        pvpButton.Draw(window, font);
        highScoreButton.Draw(window, font);
        quitButton.Draw(window, font);
    }


    void handle(Vector2D mousePos, bool clicked, GameState& state, bool& vsBot) {
        if (botButton.HandleInput(mousePos, clicked)) {
            state = InGame;
            vsBot = true;
        }
        if (pvpButton.HandleInput(mousePos, clicked)) {
            state = InGame;
            vsBot = false;
        }
        if (highScoreButton.HandleInput(mousePos, clicked)) {
            state = HighScores;
        }
        if (quitButton.HandleInput(mousePos, clicked)) {
            exit(0);
        }
    }
};

class PongGame {

    GameState state = Menu;
    PlayState playState = ServePlayerOne;
    NameEntryState nameEntryState = NoEntry;
    bool vsBot = false;
    int p1Score = 0, p2Score = 0;
    Paddle p1 = Paddle(50, 250, 10, 100, sf::Color::Red);
    Paddle p2 = Paddle(740, 250, 10, 100, sf::Color::Blue);
    Ball ball = Ball(10.f);
    Court court;
    sf::Font font;
    ScoreBoard scoreboard = ScoreBoard(font, &p1Score, &p2Score);
    PongMenu menu;
    Vector2D mousePos;
    bool mouseClicked = false;
    sf::Text serveText, winText;


   
    sf::SoundBuffer wallHitBuffer;
    sf::SoundBuffer paddleHitBuffer;
    sf::SoundBuffer scoreBuffer;
    sf::SoundBuffer victoryBuffer;
    sf::Sound wallHitSound;
    sf::Sound paddleHitSound;
    sf::Sound scoreSound;
    sf::Sound victorySound;

    
    sf::Music menuMusic;




    Button continueButton;
    Button returnButton;

    GameObject** gameObjects;
    int gameObjectCount;

    PlayerScore* highScores;
    int highScoreCount;
    int highScoreCapacity;

    sf::Text highScoreTitle;
    sf::Text* highScoreTexts;
    int highScoreTextCount;
    sf::Text backToMenuText;

    string player1Name = "";
    string player2Name = "";
    string currentInputName = "";

    sf::Text namePrompt;
    sf::Text currentNameText;
    sf::Text instructionText;
    bool newGameStarting = false;

public:
    PongGame()
        : continueButton("Continue Game", RectangleShapeData(300, 320, 200, 60), sf::Color::Green, sf::Color(0, 180, 0), sf::Color(100, 255, 100)),
        returnButton("Return to Menu", RectangleShapeData(300, 400, 200, 60), sf::Color::Cyan, sf::Color::Blue, sf::Color(150, 255, 255)) {

        if (!font.loadFromFile("Arial.ttf")) {
            cerr << "Failed to load font!" << endl;
        }

        serveText.setFont(font);
        serveText.setCharacterSize(20);
        serveText.setString("Press SPACE to serve!");
        serveText.setPosition(280, 550);
        serveText.setFillColor(sf::Color::White);

        winText.setFont(font);
        winText.setCharacterSize(40);
        winText.setFillColor(sf::Color::White);

        gameObjectCount = 3;
        gameObjects = new GameObject * [gameObjectCount];
        gameObjects[0] = &p1;
        gameObjects[1] = &p2;
        gameObjects[2] = &ball;

        highScoreCapacity = 10;
        highScores = new PlayerScore[highScoreCapacity];
        highScoreCount = 0;

        highScoreTexts = new sf::Text[highScoreCapacity];
        highScoreTextCount = 0;

        initHighScores();


       
        if (!wallHitBuffer.loadFromFile("assets/sounds/wall.wav")) {
            cerr << "Failed to load wall hit sound!" << endl;
        }
        if (!paddleHitBuffer.loadFromFile("assets/sounds/hitpaddle.wav")) {
            cerr << "Failed to load paddle hit sound!" << endl;
        }
        if (!scoreBuffer.loadFromFile("assets/sounds/score.wav")) {
            cerr << "Failed to load score sound!" << endl;
        }
        if (!victoryBuffer.loadFromFile("assets/sounds/victory.wav")) {
            cerr << "Failed to load victory sound!" << endl;
        }

       
        wallHitSound.setBuffer(wallHitBuffer);
        paddleHitSound.setBuffer(paddleHitBuffer);
        scoreSound.setBuffer(scoreBuffer);
        victorySound.setBuffer(victoryBuffer);

       
        if (!menuMusic.openFromFile("assets/sounds/menu-music.wav")) {
            cerr << "Failed to load menu music!" << endl;
        }
        menuMusic.setLoop(true);
    }

    ~PongGame() {
        delete[] gameObjects;
        delete[] highScores;
        delete[] highScoreTexts;
        menuMusic.stop(); 
    }
    void resetScores() {
        p1Score = 0;
        p2Score = 0;
        
    }

    void startMenuMusic() {
        menuMusic.play();
    }

    void stopMenuMusic() {
        menuMusic.stop();
    }

    void resetBall(PlayState nextState) {
        ball.setPosition(390, 300);
        ball.velocity = { 0, 0 };
        playState = nextState;
    }

    void initHighScores() {
        highScoreTitle.setFont(font);
        highScoreTitle.setString("HIGH SCORES");
        highScoreTitle.setCharacterSize(40);
        highScoreTitle.setFillColor(sf::Color::Magenta);
        highScoreTitle.setPosition(280, 50);

        backToMenuText.setFont(font);
        backToMenuText.setString("Press ESC to return to menu");
        backToMenuText.setCharacterSize(20);
        backToMenuText.setFillColor(sf::Color::White);
        backToMenuText.setPosition(270, 550);

        namePrompt.setFont(font);
        namePrompt.setString("Player 1 Name:");
        namePrompt.setCharacterSize(30);
        namePrompt.setFillColor(sf::Color::Yellow);
        namePrompt.setPosition(200, 280);

        currentNameText.setFont(font);
        currentNameText.setString("_");
        currentNameText.setCharacterSize(30);
        currentNameText.setFillColor(sf::Color::White);
        currentNameText.setPosition(400, 280);

        instructionText.setFont(font);
        instructionText.setString("Enter name and press ENTER");
        instructionText.setCharacterSize(20);
        instructionText.setFillColor(sf::Color::White);
        instructionText.setPosition(280, 340);

        loadHighScores();
        updateHighScoreDisplay();
    }

    void loadHighScores() {
      
        highScoreCount = 0;

        ifstream file("highscores.txt");
        if (file.is_open()) {
            string line;
            while (getline(file, line) && highScoreCount < highScoreCapacity) {
                istringstream iss(line);
                string name;
                int score;

                if (iss >> name >> score) {
                    highScores[highScoreCount] = PlayerScore(name, score);
                    highScoreCount++;
                }
            }
            file.close();

           
            sortHighScores();
        }
    }

    void sortHighScores() {
        
        for (int i = 0; i < highScoreCount - 1; i++) {
            int maxIndex = i;
            for (int j = i + 1; j < highScoreCount; j++) {
                if (highScores[j].score > highScores[maxIndex].score) {
                    maxIndex = j;
                }
            }
            if (maxIndex != i) {
                PlayerScore temp = highScores[i];
                highScores[i] = highScores[maxIndex];
                highScores[maxIndex] = temp;
            }
        }
    }

    void saveHighScores() {
        ofstream file("highscores.txt");

        if (file.is_open()) {
            for (int i = 0; i < highScoreCount; i++) {
                file << highScores[i].name << "  " << highScores[i].score << endl;
            }
            file.close();
        }
    }

    void updateHighScoreDisplay() {
        highScoreTextCount = highScoreCount;
        delete[] highScoreTexts;
        highScoreTexts = new sf::Text[highScoreCapacity];

        int height = 100;
        int space = 40;

        for (int i = 0; i < highScoreCount; i++) {
            highScoreTexts[i].setFont(font);
            stringstream ss;
            ss << (i + 1) << ". " << highScores[i].name << " - " << highScores[i].score;
            highScoreTexts[i].setString(ss.str());
            highScoreTexts[i].setCharacterSize(24);
            highScoreTexts[i].setFillColor(sf::Color::White);
            highScoreTexts[i].setPosition(100, height + i * space);
        }
    }

    bool isHighScore(int score) {
        if (highScoreCount < highScoreCapacity) {
            return true;
        }

        return score > highScores[highScoreCount - 1].score;
    }

    void addHighScore(const string& name, int score) {
        
        if (highScoreCount < highScoreCapacity) {
            highScores[highScoreCount] = PlayerScore(name, score);
            highScoreCount++;
        }
        else {
            
            if (score > highScores[highScoreCount - 1].score) {
                highScores[highScoreCount - 1] = PlayerScore(name, score);
            }
        }

        
        sortHighScores();
        saveHighScores();
        updateHighScoreDisplay();
    }

    void handleWin() {
      
        player1Name = "";
        player2Name = "";
        currentInputName = "";

        bool player1Win = p1Score >= 10;
        int winnerScore = player1Win ? p1Score : p2Score;

        if (isHighScore(winnerScore)) {
            if (vsBot) {
               
                nameEntryState = EnteringP1Name;
                namePrompt.setString("Player Name:");
                namePrompt.setPosition(230, 280);
                currentNameText.setString("_");
                currentNameText.setPosition(420, 280);
            }
            else {
               
                if (player1Win) {
                    nameEntryState = EnteringP1Name;
                    namePrompt.setString("Player 1 Name:");
                }
                else {
                    nameEntryState = EnteringP2Name;
                    namePrompt.setString("Player 2 Name:");
                }
                namePrompt.setPosition(200, 280);
                currentNameText.setString("_");
                currentNameText.setPosition(420, 280);
            }
        }
    }

    void handleNameEntry(sf::Event event) {
        if (event.type == sf::Event::TextEntered) {
            if (event.text.unicode == 13) { 
                if (!currentInputName.empty()) {
                    if (nameEntryState == EnteringP1Name) {
                        player1Name = currentInputName;

                        if (!vsBot && p2Score >= 10) {
                            

                            nameEntryState = EnteringP2Name;
                            namePrompt.setString("Player 2 Name:");
                            currentInputName = "";
                            currentNameText.setString("_");
                        }
                        else if (!vsBot && p1Score >= 10) {
                            

                            nameEntryState = EnteringP2Name;
                            namePrompt.setString("Player 2 Name:");
                            currentInputName = "";
                            currentNameText.setString("_");
                        }
                        else {
                            

                            addHighScore(player1Name, p1Score);
                            nameEntryState = NoEntry;
                            state = HighScores;
                        }
                    }
                    else if (nameEntryState == EnteringP2Name) {
                        player2Name = currentInputName;

                        
                        addHighScore(player1Name, p1Score);
                        addHighScore(player2Name, p2Score);

                        nameEntryState = NoEntry;
                        state = HighScores;
                    }
                }
            }
            else if (event.text.unicode == 8) { 
                if (!currentInputName.empty()) {
                    currentInputName.pop_back();
                    currentNameText.setString(currentInputName + "_");
                }
            }
            else if (event.text.unicode >= 32 && event.text.unicode < 128) {
                if (currentInputName.length() < 15) {
                    currentInputName += static_cast<char>(event.text.unicode);
                    currentNameText.setString(currentInputName + "_");
                }
            }
        }
    }

    void handleHighScoreEvents(sf::Event event) {
        if (nameEntryState != NoEntry) {
            handleNameEntry(event);
        }
        else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            state = Menu;
        }
    }



    void update() {
        if (state == Menu) {
            if (menuMusic.getStatus() != sf::Music::Playing) {
                startMenuMusic();
            }
            menu.handle(mousePos, mouseClicked, state, vsBot);
            if (state != Menu) {
                stopMenuMusic();  
                newGameStarting = true; 
            }

            mouseClicked = false;
            return;
        }

        
        if (newGameStarting && state == InGame) {
            resetScores();
            newGameStarting = false;
        }


        if (state == WinScreen) {
            if (nameEntryState != NoEntry) {
                return;
            }

            if (continueButton.HandleInput(mousePos, mouseClicked)) {
                resetScores();
                resetBall(ServePlayerOne);
                state = InGame;
            }
            if (returnButton.HandleInput(mousePos, mouseClicked)) {
                resetScores();
                state = Menu;
            }
            mouseClicked = false;
            return;
        }

        if (state == HighScores) {
            return;
        }

        if ((playState == ServePlayerOne || playState == ServePlayerTwo) &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            ball.velocity = playState == ServePlayerOne ? sf::Vector2f(3, 3) : sf::Vector2f(-3, 3);
            playState = Playing;
        }

        if (playState == Playing)
            ball.update();

        auto bounds = ball.getBounds();
        if (bounds.top <= 0 || bounds.top + bounds.height >= 600) {
            ball.velocity.y *= -1;
            wallHitSound.play();  
        }

        if (ball.getBounds().intersects(p1.rect.getGlobalBounds())) {
            ball.velocity.x = abs(ball.velocity.x);
            paddleHitSound.play();  
        }

        if (ball.getBounds().intersects(p2.rect.getGlobalBounds())) {
            ball.velocity.x = -abs(ball.velocity.x);
            paddleHitSound.play(); 
        }

        if (bounds.left <= 0) {
            p2Score++;
            scoreSound.play();  
            checkWin();
            resetBall(ServePlayerOne);
        }
        else if (bounds.left + bounds.width >= 800) {
            p1Score++;
            scoreSound.play();  
            checkWin();
            resetBall(ServePlayerTwo);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && p1.rect.getPosition().y > 0)
            p1.rect.move(0, -5);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) &&
            p1.rect.getPosition().y + p1.rect.getSize().y < 600)
            p1.rect.move(0, 5);

        if (vsBot)
            botAI();
        else {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) && p2.rect.getPosition().y > 0)
                p2.rect.move(0, -5);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) &&
                p2.rect.getPosition().y + p2.rect.getSize().y < 600)
                p2.rect.move(0, 5);
        }

        mouseClicked = false;
    }

    void checkWin() {
        if (p1Score >= 10 || p2Score >= 10) {
            victorySound.play();  
            state = WinScreen;
            winText.setString(vsBot ? (p1Score >= 10 ? "Player wins!" : "Bot wins!") :
                (p1Score >= 10 ? "Player 1 wins!" : "Player 2 wins!"));
            winText.setPosition(400 - winText.getLocalBounds().width / 2, 200);

            handleWin();
        }
    }

    void botAI() {
        float ballY = ball.shape.getPosition().y;
        float botY = p2.rect.getPosition().y + p2.rect.getSize().y / 2;
        if (ballY < botY - 8)
            p2.rect.move(0, -2);
        else if (ballY > botY + 8)
            p2.rect.move(0, 2);
    }

    void draw(sf::RenderWindow& window) {
        window.clear();

        if (state == Menu) {
            menu.draw(window, font);
            return;
        }

        if (state == WinScreen) {
            window.draw(winText);

            if (nameEntryState != NoEntry) {
                window.draw(namePrompt);
                window.draw(currentNameText);
                window.draw(instructionText);
            }
            else {
                continueButton.Draw(window, font);
                returnButton.Draw(window, font);
            }
            return;
        }

        if (state == HighScores) {
            window.draw(highScoreTitle);
            for (int i = 0; i < highScoreTextCount; i++) {
                window.draw(highScoreTexts[i]);
            }
            window.draw(backToMenuText);
            return;
        }

        court.draw(window);
        for (int i = 0; i < gameObjectCount; i++)
            gameObjects[i]->draw(window);
        scoreboard.draw(window);

        if (playState == ServePlayerOne || playState == ServePlayerTwo)
            window.draw(serveText);
    }

    void handleEvent(sf::Event& event) {
        if (event.type == sf::Event::MouseMoved)
            mousePos = Vector2D(static_cast<float>(event.mouseMove.x), static_cast<float>(event.mouseMove.y));
        if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            mouseClicked = true;

        if (state == WinScreen && nameEntryState != NoEntry) {
            handleHighScoreEvents(event);
        }
        else if (state == HighScores) {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
                state = Menu;
            }
        }
    }
};

int main() {
    
    sf::RenderWindow window(sf::VideoMode(800, 600), "Pong Game");
    PongGame game;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            game.handleEvent(event);
        }

        game.update();
        game.draw(window);
        window.display();
    }

    return 0;
}
