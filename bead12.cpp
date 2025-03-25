#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iostream>
using namespace std;
using namespace sf;


// Function prototypes
bool isValid(int row, int col);
bool isEmpty(int row, int col);
int calculateDistance(int srcRow, int srcCol, int desRow, int desCol);
bool isMovable(int player, int srcRow, int srcCol, int desRow, int desCol);
bool isEdible(int player, int srcRow, int srcCol, int desRow, int desCol);
bool hasValidMoves(int player);
bool makeMove(int player, int srcRow, int srcCol, int desRow, int desCol);
void saveBoard();
void loadBoard();
void switchPlayer();
int getTimeRemaining();
bool checkWinCondition(Text &winText);
void playerVsComputer(RenderWindow &window, Font &font);
bool computerMove();
void startGame();

const int GRID_SIZE = 6;
const int CELL_SIZE = 100;
const int BOARD_SIZE = GRID_SIZE * CELL_SIZE;
const int WINDOW_HEIGHT = BOARD_SIZE + 200; // Increased height for the Exit button
const int WINDOW_WIDTH = BOARD_SIZE;

int board[GRID_SIZE][GRID_SIZE] = {0};
int currentPlayer = 1; // 1 for Red, 2 for Blue
chrono::time_point<chrono::steady_clock> startTime;
const int TURN_TIME_LIMIT = 30; // 30 seconds per turn

int main()
{
    startGame();

    return 0;
}

// Function to check valid position
bool isValid(int row, int col)
{
    return row >= 0 && col >= 0 && row < GRID_SIZE && col < GRID_SIZE;
}

// Function to check if a cell is empty
bool isEmpty(int row, int col)
{
    return board[row][col] == 0;
}

// Calculate distance between two points
int calculateDistance(int srcRow, int srcCol, int desRow, int desCol)
{
    if ((pow(srcRow - desRow, 2) + pow(srcCol - desCol, 2)) == 5)
    {
        return 3; 
    }

    return sqrt(pow(srcRow - desRow, 2) + pow(srcCol - desCol, 2));
}

// Check if a bead can move
bool isMovable(int player, int srcRow, int srcCol, int desRow, int desCol)
{
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol))
        return false;
    if (isEmpty(srcRow, srcCol) || board[srcRow][srcCol] != player)
        return false;
    if (!isEmpty(desRow, desCol))
        return false;
    if (calculateDistance(srcRow, srcCol, desRow, desCol) != 1)
        return false;
    return true;
}

bool isEdible(int player, int srcRow, int srcCol, int desRow, int desCol)
{
    // Ensure source and destination are valid
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol))
    {
        return false;
    }

    // Ensure the source contains the player's bead
    if (isEmpty(srcRow, srcCol) || board[srcRow][srcCol] != player)
    {
        return false;
    }

    // Ensure the destination is empty
    if (!isEmpty(desRow, desCol))
    {
        return false;
    }

    // Calculate the midpoint between source and destination
    int midRow = (srcRow + desRow) / 2;
    int midCol = (srcCol + desCol) / 2;

    if (!isValid(midRow, midCol))
    {
        return false;
    }

    int opponent = (player == 1) ? 2 : 1;
    if (board[midRow][midCol] != opponent)
    {
        return false;
    }
    if (calculateDistance(srcRow, srcCol, desRow, desCol) == 2)
    {
        return true;
    }

    return false;
}

bool hasValidMoves(int player)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (board[i][j] == player)
            {
                for (int di = -2; di <= 2; di++)
                {
                    for (int dj = -2; dj <= 2; dj++)
                    {
                        if (di == 0 && dj == 0)
                            continue;
                        int newRow = i + di;
                        int newCol = j + dj;
                        if (isEdible(player, i, j, newRow, newCol) || isMovable(player, i, j, newRow, newCol))
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool makeMove(int player, int srcRow, int srcCol, int desRow, int desCol)
{
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol))
    {
        return false;
    }

    if (board[srcRow][srcCol] != player)
    {
        return false;
    }

    if (!isEmpty(desRow, desCol))
    {
        return false;
    }

    if (isMovable(player, srcRow, srcCol, desRow, desCol))
    {
        // Simple move
        board[desRow][desCol] = board[srcRow][srcCol];
        board[srcRow][srcCol] = 0;
        return true;
    }
    else if (isEdible(player, srcRow, srcCol, desRow, desCol))
    {
        // Jump, eat opponent bead
        int midRow = (srcRow + desRow) / 2;
        int midCol = (srcCol + desCol) / 2;
        board[midRow][midCol] = 0;
        board[desRow][desCol] = board[srcRow][srcCol];
        board[srcRow][srcCol] = 0;
        return true;
    }

    return false;
}

// Save game state
void saveBoard()
{
    ofstream file("board_save.txt");
    for (int i = 0; i < GRID_SIZE; ++i)
    {
        for (int j = 0; j < GRID_SIZE; ++j)
        {
            file << board[i][j] << " ";
        }
        file << "\n";
    }
    file.close();
}

// Load game state
void loadBoard()
{
    ifstream file("board_save.txt");
    if (file.is_open())
    {
        for (int i = 0; i < GRID_SIZE; ++i)
        {
            for (int j = 0; j < GRID_SIZE; ++j)
            {
                file >> board[i][j];
            }
        }
        file.close();
    }
}

// Switch player and reset the timer
void switchPlayer()
{
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    startTime = chrono::steady_clock::now();
}

// Get remaining time before auto-switch
int getTimeRemaining()
{
    auto currentTime = chrono::steady_clock::now();
    int elapsedTime = chrono::duration_cast<chrono::seconds>(currentTime - startTime).count();
    return TURN_TIME_LIMIT - elapsedTime;
}

bool checkWinCondition(Text &winText)
{
    int player1Beads = 0, player2Beads = 0;

    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (board[i][j] == 1)
            {
                player1Beads++;
            }
            else if (board[i][j] == 2)
            {
                player2Beads++;
            }
        }
    }

    if (player1Beads == 0)
    {
        winText.setString("Player 2 Wins! Congratulations!");
        return true;
    }

    if (player2Beads == 0)
    {
        winText.setString("Player 1 Wins! Congratulations!");
        return true;
    }

    return false;
}

void playerVsComputer(RenderWindow &window, Font &font)
{

    // Initialize the board with default positions for Player 1 and Player 2
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (i < 2)
            {
                board[i][j] = 1; // Player 1's beads
            }
            else if (i >= GRID_SIZE - 2)
            {
                board[i][j] = 2; // Player 2's beads
            }
            else
            {
                board[i][j] = 0; // Empty cells
            }
        }
    }

    int currentPlayer = 1;          // Player 1 starts
    int timeLeft = TURN_TIME_LIMIT; // 30 seconds for each turn
    auto startTime = chrono::steady_clock::now();
    string message = ""; // Message to display below the timer

    int srcRow = -1, srcCol = -1; // Track the selected source bead

    vector<pair<int, int>> possibleMoves; // Define possibleMoves to track valid moves

    Text saveButton(" Save", font, 30);
    saveButton.setPosition(50, BOARD_SIZE + 20);
    saveButton.setFillColor(Color::Black);

    Text loadButton(" Load", font, 30);
    loadButton.setPosition(200, BOARD_SIZE + 20);
    loadButton.setFillColor(Color::Black);

    Text exitButton(" Exit", font, 30);
    exitButton.setPosition(200, BOARD_SIZE + 90); // Position below the Load button
    exitButton.setFillColor(Color::Black);

    Text mainMenuButton("Main Menu", font, 30);
    mainMenuButton.setPosition(350, BOARD_SIZE + 90); // Position next to the Exit button
    mainMenuButton.setFillColor(Color::Black);

    Text timerText("", font, 30);
    timerText.setPosition(350, BOARD_SIZE + 20); // Adjusted position next to Load button
    timerText.setFillColor(Color::Black);

    RectangleShape saveButtonBg(Vector2f(100, 50));
    saveButtonBg.setPosition(50, BOARD_SIZE + 20);
    saveButtonBg.setFillColor(Color::Cyan);

    RectangleShape loadButtonBg(Vector2f(100, 50));
    loadButtonBg.setPosition(200, BOARD_SIZE + 20);
    loadButtonBg.setFillColor(Color::Cyan);

    RectangleShape exitButtonBg(Vector2f(100, 50));
    exitButtonBg.setPosition(200, BOARD_SIZE + 90); // Position below the Load button
    exitButtonBg.setFillColor(Color::Red);

    RectangleShape mainMenuButtonBg(Vector2f(150, 50));
    mainMenuButtonBg.setPosition(350, BOARD_SIZE + 90);
    mainMenuButtonBg.setFillColor(Color::Yellow);

    auto computerMoveStartTime = chrono::steady_clock::now(); // Track when the computer's turn starts

    Text winText("", font, 40); // Winning message
    winText.setPosition(50, BOARD_SIZE / 2 - 20);
    winText.setFillColor(Color::Black);

    bool gameWon = false;
    bool returnToMainMenu = false;

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
            {
                window.close();
                exit(0);
            }
            else if (event.type == Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == Mouse::Left)
                {
                    int x = event.mouseButton.x;
                    int y = event.mouseButton.y;

                    if (gameWon)
                    {
                        if (exitButtonBg.getGlobalBounds().contains(x, y))
                        {
                            window.close(); // Exit the game
                        }
                    }

                    if (y > BOARD_SIZE)
                    {
                        if (saveButton.getGlobalBounds().contains(x, y))
                        {
                            saveBoard();
                        }
                        else if (loadButton.getGlobalBounds().contains(x, y))
                        {
                            loadBoard();
                        }
                        else if (exitButtonBg.getGlobalBounds().contains(x, y))
                        {
                            window.close(); // Exit the game
                        }
                        else if (mainMenuButtonBg.getGlobalBounds().contains(x, y))
                        {
                            returnToMainMenu = true; // Return to the main menu
                        }
                    }
                    else if (currentPlayer == 1 && !gameWon)
                    { // Player's turn
                        int row = y / CELL_SIZE;
                        int col = x / CELL_SIZE;

                        if (isValid(row, col))
                        {
                            if (srcRow == -1 && srcCol == -1 && !isEmpty(row, col) && board[row][col] == currentPlayer)
                            {
                                srcRow = row;
                                srcCol = col;
                                possibleMoves.clear();
                                for (int i = 0; i < GRID_SIZE; i++)
                                {
                                    for (int j = 0; j < GRID_SIZE; j++)
                                    {
                                        if (isMovable(currentPlayer, srcRow, srcCol, i, j) || isEdible(currentPlayer, srcRow, srcCol, i, j))
                                        {
                                            possibleMoves.push_back({i, j});
                                        }
                                    }
                                }
                            }
                            else if (srcRow != -1 && srcCol != -1)
                            {
                                if (makeMove(currentPlayer, srcRow, srcCol, row, col))
                                {
                                    currentPlayer = 2;                                        // Switch to computer
                                    startTime = chrono::steady_clock::now();             // Reset timer
                                    computerMoveStartTime = chrono::steady_clock::now(); // Reset computer move timer
                                }
                                srcRow = -1;
                                srcCol = -1;
                                possibleMoves.clear();
                            }
                        }
                    }
                }
            }
        }

        // Return to the main menu if the button is clicked
        if (returnToMainMenu)
        {
            return; // Exit the function to go back to the main menu
        }

        // Check if a player has won
        if (!gameWon && checkWinCondition(winText))
        {
            gameWon = true;
        }

        // Stop the timer and end the game if one player has no beads left
        if (gameWon)
        {
            Text mainMenuButton("Main Menu", font, 30);
            mainMenuButton.setPosition(200, BOARD_SIZE + 50);
            mainMenuButton.setFillColor(Color::Black);

            RectangleShape mainMenuButtonBg(Vector2f(150, 50));
            mainMenuButtonBg.setPosition(190, BOARD_SIZE + 50);
            mainMenuButtonBg.setFillColor(Color::Yellow);

            while (window.isOpen())
            {
                window.clear(Color::White); // Clear the window with white color
                window.draw(winText);
                window.draw(mainMenuButtonBg);
                window.draw(mainMenuButton);
                window.display();

                Event event;
                while (window.pollEvent(event))
                {
                    if (event.type == Event::Closed)
                    {
                        window.close();
                        exit(0);
                    }
                    else if (event.type == Event::MouseButtonPressed)
                    {
                        if (event.mouseButton.button == Mouse::Left)
                        {
                            int x = event.mouseButton.x;
                            int y = event.mouseButton.y;

                            if (mainMenuButtonBg.getGlobalBounds().contains(x, y))
                            {
                                // Reset the game state and return to the main menu
                                for (int i = 0; i < GRID_SIZE; ++i)
                                {
                                    for (int j = 0; j < GRID_SIZE; ++j)
                                    {
                                        board[i][j] = 0;
                                    }
                                }
                                currentPlayer = 1; // Reset to Player 1
                                return; // Exit the loop and show the main menu
                            }
                        }
                    }
                }
            }
        }

        // Timer logic
        int timeRemaining = TURN_TIME_LIMIT - chrono::duration_cast<chrono::seconds>(
                                                  chrono::steady_clock::now() - startTime)
                                                  .count();
        if (timeRemaining <= 0 && !gameWon)
        {
            currentPlayer = (currentPlayer == 1) ? 2 : 1; // Switch player
            startTime = chrono::steady_clock::now(); // Reset timer
            if (currentPlayer == 2)
            {
                computerMoveStartTime = chrono::steady_clock::now(); // Reset computer move timer
            }
        }

        // Computer's move
        if (currentPlayer == 2 && !gameWon)
        {
            auto elapsedTime = chrono::duration_cast<chrono::seconds>(
                                   chrono::steady_clock::now() - computerMoveStartTime)
                                   .count();
            if (elapsedTime >= 1)
            { 
                if (computerMove())
                {
                    currentPlayer = 1;                            // Switch back to player
                    startTime = chrono::steady_clock::now(); // Reset timer
                }
            }
        }

        window.clear(Color::White);

        // Draw grid and beads
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                RectangleShape cell(Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);
                cell.setFillColor(Color::White);
                cell.setOutlineThickness(1);
                cell.setOutlineColor(Color::Black);
                window.draw(cell);

                if (board[i][j] == 1)
                {
                    CircleShape bead(CELL_SIZE / 3);
                    bead.setFillColor(Color::Red);
                    bead.setPosition(j * CELL_SIZE + CELL_SIZE / 6, i * CELL_SIZE + CELL_SIZE / 6);
                    window.draw(bead);
                }
                else if (board[i][j] == 2)
                {
                    CircleShape bead(CELL_SIZE / 3);
                    bead.setFillColor(Color::Blue);
                    bead.setPosition(j * CELL_SIZE + CELL_SIZE / 6, i * CELL_SIZE + CELL_SIZE / 6);
                    window.draw(bead);
                }
            }
        }

        // Highlight valid moves
        for (auto &move : possibleMoves)
        {
            RectangleShape cell(Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(move.second * CELL_SIZE, move.first * CELL_SIZE);
            cell.setFillColor(Color(0, 255, 0, 128));
            window.draw(cell);
        }

        // Draw buttons and timer
        window.draw(saveButtonBg);
        window.draw(loadButtonBg);
        window.draw(exitButtonBg);
        window.draw(mainMenuButtonBg);
        window.draw(saveButton);
        window.draw(loadButton);
        window.draw(exitButton);
        window.draw(mainMenuButton);

        stringstream ss;
        ss << "Time: " << timeRemaining << "s";
        timerText.setString(ss.str());
        window.draw(timerText);

        // Draw win message if game is won
        if (gameWon)
        {
            window.clear(Color::White); // Clear the window with white color
            window.draw(winText); // Only display the win message
            window.display();
            continue; // Skip the rest of the loop
        }

        window.display();
    }
}

bool computerMove()
{
    vector<tuple<int, int, int, int>> captureMoves; // Store capturing moves
    vector<pair<int, int>> possibleMoves;           // Store simple moves

    for (int srcRow = 0; srcRow < GRID_SIZE; ++srcRow)
    {
        for (int srcCol = 0; srcCol < GRID_SIZE; ++srcCol)
        {
            if (board[srcRow][srcCol] == 2)
            { // Computer's beads
                for (int desRow = 0; desRow < GRID_SIZE; ++desRow)
                {
                    for (int desCol = 0; desCol < GRID_SIZE; ++desCol)
                    {
                        if (isEdible(2, srcRow, srcCol, desRow, desCol))
                        {
                            // Store capturing moves with source and destination
                            captureMoves.emplace_back(srcRow, srcCol, desRow, desCol);
                        }
                        else if (isMovable(2, srcRow, srcCol, desRow, desCol))
                        {
                            // Store simple moves
                            possibleMoves.emplace_back(srcRow, srcCol);
                        }
                    }
                }
            }
        }
    }

    // Prioritize capturing moves
    if (!captureMoves.empty())
    {
        auto [srcRow, srcCol, desRow, desCol] = captureMoves[rand() % captureMoves.size()];
        return makeMove(2, srcRow, srcCol, desRow, desCol);
    }

    // If no capturing moves, perform a simple move
    if (!possibleMoves.empty())
    {
        auto [srcRow, srcCol] = possibleMoves[rand() % possibleMoves.size()];
        for (int desRow = 0; desRow < GRID_SIZE; ++desRow)
        {
            for (int desCol = 0; desCol < GRID_SIZE; ++desCol)
            {
                if (isMovable(2, srcRow, srcCol, desRow, desCol))
                {
                    return makeMove(2, srcRow, srcCol, desRow, desCol);
                }
            }
        }
    }

    return false; // No valid moves
}

void startGame()
{
    RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "6x6 Bead Grid");

    Font font;
    font.loadFromFile("arial.ttf");

    // Option buttons
    Text pvpButton("Player vs Player", font, 30);
    pvpButton.setPosition(100, BOARD_SIZE / 2 - 50);
    pvpButton.setFillColor(Color::Black);

    Text pvcButton("Player vs Computer", font, 30);
    pvcButton.setPosition(100, BOARD_SIZE / 2 + 10);
    pvcButton.setFillColor(Color::Black);

    RectangleShape pvpButtonBg(Vector2f(300, 50));
    pvpButtonBg.setPosition(90, BOARD_SIZE / 2 - 55);
    pvpButtonBg.setFillColor(Color::Cyan);

    RectangleShape pvcButtonBg(Vector2f(300, 50));
    pvcButtonBg.setPosition(90, BOARD_SIZE / 2 + 5);
    pvcButtonBg.setFillColor(Color::Cyan);

    bool gameStarted = false;
    bool isPlayerVsComputer = false;

    while (window.isOpen())
    {
        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed)
                window.close();
            else if (!gameStarted && event.type == Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == Mouse::Left)
                {
                    int x = event.mouseButton.x;
                    int y = event.mouseButton.y;

                    if (pvpButtonBg.getGlobalBounds().contains(x, y))
                    {
                        gameStarted = true; // Start Player vs Player game
                        isPlayerVsComputer = false;
                    }
                    else if (pvcButtonBg.getGlobalBounds().contains(x, y))
                    {
                        gameStarted = true; // Start Player vs Computer game
                        isPlayerVsComputer = true;
                    }
                }
            }
        }

        window.clear(Color::White);

        if (!gameStarted)
        {
            // Draw option buttons
            window.draw(pvpButtonBg);
            window.draw(pvcButtonBg);
            window.draw(pvpButton);
            window.draw(pvcButton);
        }
        else
        {
            if (isPlayerVsComputer)
            {
                // Start Player vs Computer game
                playerVsComputer(window, font);
                gameStarted = false; // Reset to show the menu after the game ends or "Main Menu" is clicked
            }
            else
            {
                // Initialize beads
                for (int i = 0; i < 2; i++)
                    for (int j = 0; j < GRID_SIZE; j++)
                        board[i][j] = 1; // Red beads for Player 1

                for (int i = 4; i < GRID_SIZE; i++)
                    for (int j = 0; j < GRID_SIZE; j++)
                        board[i][j] = 2; // Blue beads for Player 2

                Text saveButton(" Save", font, 30);
                saveButton.setPosition(50, BOARD_SIZE + 20);
                saveButton.setFillColor(Color::Black);

                Text loadButton(" Load", font, 30);
                loadButton.setPosition(200, BOARD_SIZE + 20);
                loadButton.setFillColor(Color::Black);

                Text exitButton(" Exit", font, 30);
                exitButton.setPosition(200, BOARD_SIZE + 90); // Position below the Load button
                exitButton.setFillColor(Color::Black);

                Text mainMenuButton("Main Menu", font, 30);
                mainMenuButton.setPosition(350, BOARD_SIZE + 90); // Position next to the Exit button
                mainMenuButton.setFillColor(Color::Black);

                Text timerText("", font, 30);
                timerText.setPosition(350, BOARD_SIZE + 20); // Adjusted position next to Load button
                timerText.setFillColor(Color::Black);

                RectangleShape saveButtonBg(Vector2f(100, 50));
                saveButtonBg.setPosition(50, BOARD_SIZE + 20);
                saveButtonBg.setFillColor(Color::Cyan);

                RectangleShape loadButtonBg(Vector2f(100, 50));
                loadButtonBg.setPosition(200, BOARD_SIZE + 20);
                loadButtonBg.setFillColor(Color::Cyan);

                RectangleShape exitButtonBg(Vector2f(100, 50));
                exitButtonBg.setPosition(200, BOARD_SIZE + 90); // Position below the Load button
                exitButtonBg.setFillColor(Color::Red);

                RectangleShape mainMenuButtonBg(Vector2f(150, 50));
                mainMenuButtonBg.setPosition(350, BOARD_SIZE + 90);
                mainMenuButtonBg.setFillColor(Color::Yellow);

                Text winText("", font, 40);
                winText.setPosition(50, BOARD_SIZE / 2 - 20);
                winText.setFillColor(Color::Black);

                startTime = chrono::steady_clock::now();

                int selectedRow = -1, selectedCol = -1;
                vector<pair<int, int>> possibleMoves;

                bool gameWon = false;
                bool returnToMainMenu = false;

                while (window.isOpen())
                {
                    Event event;
                    while (window.pollEvent(event))
                    {
                        if (event.type == Event::Closed)
                            window.close();
                        else if (!gameWon && event.type == Event::MouseButtonPressed)
                        {
                            if (event.mouseButton.button == Mouse::Left)
                            {
                                int x = event.mouseButton.x;
                                int y = event.mouseButton.y;

                                if (y > BOARD_SIZE)
                                {
                                    if (saveButton.getGlobalBounds().contains(x, y))
                                    {
                                        saveBoard();
                                    }
                                    else if (loadButton.getGlobalBounds().contains(x, y))
                                    {
                                        loadBoard();
                                    }
                                    else if (exitButtonBg.getGlobalBounds().contains(x, y))
                                    {
                                        window.close(); // Exit the game
                                    }
                                    else if (mainMenuButtonBg.getGlobalBounds().contains(x, y))
                                    {
                                        returnToMainMenu = true; // Return to the main menu
                                    }
                                }
                                else
                                {
                                    int row = y / CELL_SIZE;
                                    int col = x / CELL_SIZE;

                                    if (isValid(row, col))
                                    {
                                        if (selectedRow == -1 && selectedCol == -1 && !isEmpty(row, col) && board[row][col] == currentPlayer)
                                        {
                                            selectedRow = row;
                                            selectedCol = col;
                                            possibleMoves.clear();
                                            for (int i = 0; i < GRID_SIZE; i++)
                                                for (int j = 0; j < GRID_SIZE; j++)
                                                    if (isMovable(currentPlayer, selectedRow, selectedCol, i, j) || isEdible(currentPlayer, selectedRow, selectedCol, i, j))
                                                        possibleMoves.push_back({i, j});
                                        }
                                        else if (selectedRow != -1 && selectedCol != -1)
                                        {
                                            bool moved = makeMove(currentPlayer, selectedRow, selectedCol, row, col);
                                            if (moved)
                                            {
                                                switchPlayer();
                                            }
                                            selectedRow = -1;
                                            selectedCol = -1;
                                            possibleMoves.clear();
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Return to the main menu if the button is clicked
                    if (returnToMainMenu)
                    {
                        gameStarted = false; // Reset to show the menu
                        break;
                    }

                    // Check if time expired
                    if (!gameWon)
                    {
                        int timeRemaining = getTimeRemaining();
                        if (timeRemaining <= 0)
                        {
                            switchPlayer();
                        }

                        // Check if a player has won
                        if (checkWinCondition(winText))
                        {
                            gameWon = true;
                        }

                        // Update timer display
                        stringstream ss;
                        ss << "Time: " << timeRemaining << "s";
                        timerText.setString(ss.str());
                    }

                    window.clear(Color::White);

                    // Draw grid
                    for (int i = 0; i <= GRID_SIZE; i++)
                    {
                        Vertex horizontalLine[] = {
                            Vertex(Vector2f(0, i * CELL_SIZE), Color::Black),
                            Vertex(Vector2f(BOARD_SIZE, i * CELL_SIZE), Color::Black)};
                        window.draw(horizontalLine, 2, Lines);

                        Vertex verticalLine[] = {
                            Vertex(Vector2f(i * CELL_SIZE, 0), Color::Black),
                            Vertex(Vector2f(i * CELL_SIZE, BOARD_SIZE), Color::Black)};
                        window.draw(verticalLine, 2, Lines);
                    }

                    // Draw beads
                    for (int i = 0; i < GRID_SIZE; i++)
                        for (int j = 0; j < GRID_SIZE; j++)
                            if (board[i][j] != 0)
                            {
                                CircleShape bead(CELL_SIZE / 3);
                                bead.setFillColor(board[i][j] == 1 ? Color::Red : Color::Blue);
                                bead.setPosition(j * CELL_SIZE + CELL_SIZE / 6, i * CELL_SIZE + CELL_SIZE / 6);
                                window.draw(bead);
                            }

                    // Highlight valid moves
                    for (auto &move : possibleMoves)
                    {
                        RectangleShape cell(Vector2f(CELL_SIZE, CELL_SIZE));
                        cell.setPosition(move.second * CELL_SIZE, move.first * CELL_SIZE);
                        cell.setFillColor(Color(0, 255, 0, 128));
                        window.draw(cell);
                    }

                    // Draw buttons and timer
                    window.draw(saveButtonBg);
                    window.draw(loadButtonBg);
                    window.draw(exitButtonBg);
                    window.draw(mainMenuButtonBg);
                    window.draw(saveButton);
                    window.draw(loadButton);
                    window.draw(exitButton);
                    window.draw(mainMenuButton);
                    window.draw(timerText); // Timer is now next to Load button

                    // Draw win message if game is won
                    if (gameWon)
                    {
                        Text mainMenuButton("Exit", font, 30);
                        mainMenuButton.setPosition(200, BOARD_SIZE + 50);
                        mainMenuButton.setFillColor(Color::Black);

                        RectangleShape mainMenuButtonBg(Vector2f(150, 50));
                        mainMenuButtonBg.setPosition(190, BOARD_SIZE + 50);
                        mainMenuButtonBg.setFillColor(Color::Red);

                        while (window.isOpen())
                        {
                            window.clear(Color::White); // Clear the window with white color
                            window.draw(winText); // Only display the win message
                            window.draw(mainMenuButtonBg);
                            window.draw(mainMenuButton);
                            window.display();

                            Event event;
                            while (window.pollEvent(event))
                            {
                                if (event.type == Event::Closed)
                                {
                                    window.close();
                                    exit(0);
                                }
                                else if (event.type == Event::MouseButtonPressed)
                                {
                                    if (event.mouseButton.button == Mouse::Left)
                                    {
                                        int x = event.mouseButton.x;
                                        int y = event.mouseButton.y;

                                        if (mainMenuButtonBg.getGlobalBounds().contains(x, y))
                                        {
                                            // Reset the game state and return to the main menu
                                            for (int i = 0; i < GRID_SIZE; ++i)
                                            {
                                                for (int j = 0; j < GRID_SIZE; ++j)
                                                {
                                                    board[i][j] = 0;
                                                }
                                            }
                                            currentPlayer = 1; // Reset to Player 1
                                            return; // Exit the loop and show the main menu
                                        }
                                    }
                                }
                            }
                        }
                    }

                    window.display();
                }
            }
        }

        window.display();
    }
}

