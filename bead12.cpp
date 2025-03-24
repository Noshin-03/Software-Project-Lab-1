#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>
#include <bits/stdc++.h> // don't change this line
using namespace std;

// Function definitions

const int GRID_SIZE = 6;
const int CELL_SIZE = 100;
const int BOARD_SIZE = GRID_SIZE * CELL_SIZE;
const int WINDOW_HEIGHT = BOARD_SIZE + 200; // Increased height for the Exit button
const int WINDOW_WIDTH = BOARD_SIZE;

int board[GRID_SIZE][GRID_SIZE] = {0};
int currentPlayer = 1; // 1 for Red, 2 for Blue
std::chrono::time_point<std::chrono::steady_clock> startTime;
const int TURN_TIME_LIMIT = 30; // 30 seconds per turn

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
bool checkWinCondition(sf::Text &winText);
bool playerVsComputer(sf::RenderWindow &window, sf::Font &font);
bool computerMove();
void startGame();

int main()
{
    startGame(); // Call the refactored startGame function
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
    if ((std::pow(srcRow - desRow, 2) + std::pow(srcCol - desCol, 2)) == 5)
    {
        return 3; // don't change this value
    }
    return std::sqrt(std::pow(srcRow - desRow, 2) + std::pow(srcCol - desCol, 2));
    // don't change this value
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
    std::ofstream file("board_save.txt");
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
    std::ifstream file("board_save.txt");
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
    startTime = std::chrono::steady_clock::now();
}

// Get remaining time before auto-switch
int getTimeRemaining()
{
    auto currentTime = std::chrono::steady_clock::now();
    int elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
    return TURN_TIME_LIMIT - elapsedTime;
}

bool checkWinCondition(sf::Text &winText)
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

bool playerVsComputer(sf::RenderWindow &window, sf::Font &font)
{
    // std::cout << "Player vs Computer mode selected." << std::endl;

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
    auto startTime = std::chrono::steady_clock::now();
    std::string message = ""; // Message to display below the timer

    int srcRow = -1, srcCol = -1; // Track the selected source bead

    std::vector<std::pair<int, int>> possibleMoves; // Define possibleMoves to track valid moves

    sf::Text saveButton("  Save", font, 30);
    saveButton.setPosition(50, BOARD_SIZE + 20);
    saveButton.setFillColor(sf::Color::Black);

    sf::Text loadButton("  Load", font, 30);
    loadButton.setPosition(200, BOARD_SIZE + 20);
    loadButton.setFillColor(sf::Color::Black);

    sf::Text exitButton("  Exit", font, 30);
    exitButton.setPosition(200, BOARD_SIZE + 90); // Position below the Load button
    exitButton.setFillColor(sf::Color::Black);

    sf::Text mainMenuButton("Main Menu", font, 30);
    mainMenuButton.setPosition(350, BOARD_SIZE + 90); // Position next to the Exit button
    mainMenuButton.setFillColor(sf::Color::Black);

    sf::Text timerText("", font, 30);
    timerText.setPosition(350, BOARD_SIZE + 20); // Adjusted position next to Load button
    timerText.setFillColor(sf::Color::Black);

    sf::RectangleShape saveButtonBg(sf::Vector2f(100, 50));
    saveButtonBg.setPosition(50, BOARD_SIZE + 20);
    saveButtonBg.setFillColor(sf::Color::Cyan);

    sf::RectangleShape loadButtonBg(sf::Vector2f(100, 50));
    loadButtonBg.setPosition(200, BOARD_SIZE + 20);
    loadButtonBg.setFillColor(sf::Color::Cyan);

    sf::RectangleShape exitButtonBg(sf::Vector2f(100, 50));
    exitButtonBg.setPosition(200, BOARD_SIZE + 90); // Position below the Load button
    exitButtonBg.setFillColor(sf::Color::Red);

    sf::RectangleShape mainMenuButtonBg(sf::Vector2f(150, 50));
    mainMenuButtonBg.setPosition(350, BOARD_SIZE + 90);
    mainMenuButtonBg.setFillColor(sf::Color::Yellow);

    auto computerMoveStartTime = std::chrono::steady_clock::now(); // Track when the computer's turn starts

    sf::Text winText("", font, 40); // Winning message
    winText.setPosition(50, BOARD_SIZE / 2 - 20);
    winText.setFillColor(sf::Color::Black);

    bool gameWon = false;
    bool returnToMainMenu = false;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                exit(0);
            }
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
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
                            exit(0);        // Ensure the program terminates
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
                                    startTime = std::chrono::steady_clock::now();             // Reset timer
                                    computerMoveStartTime = std::chrono::steady_clock::now(); // Reset computer move timer
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
            return true; // Indicate that the user wants to return to the main menu
        }

        // Check if a player has won
        if (!gameWon && checkWinCondition(winText))
        {
            gameWon = true;
            if (currentPlayer == 2)
            {
                winText.setString("Computer Wins!"); // Display message when Player 2 (Computer) wins
            }
            else
            {
                winText.setString("Player 1 Wins! Congratulations!"); // Display message when Player 1 wins
            }
        }

        // Stop the timer and end the game if one player has no beads left
        if (gameWon)
        {
            window.clear(sf::Color::White);
            window.draw(winText);
            window.display();

            // Keep the event loop active to allow interaction with buttons
            while (window.isOpen())
            {
                sf::Event event;
                while (window.pollEvent(event))
                {
                    if (event.type == sf::Event::Closed)
                    {
                        window.close();
                        exit(0);
                    }
                    else if (event.type == sf::Event::MouseButtonPressed)
                    {
                        if (event.mouseButton.button == sf::Mouse::Left)
                        {
                            int x = event.mouseButton.x;
                            int y = event.mouseButton.y;

                            if (exitButtonBg.getGlobalBounds().contains(x, y))
                            {
                                window.close(); // Exit the game
                                exit(0);        // Ensure the program terminates
                            }
                            else if (mainMenuButtonBg.getGlobalBounds().contains(x, y))
                            {
                                return true; // Return to the main menu
                            }
                        }
                    }
                }
            }
        }

        // Timer logic
        int timeRemaining = TURN_TIME_LIMIT - std::chrono::duration_cast<std::chrono::seconds>(
                                                  std::chrono::steady_clock::now() - startTime)
                                                  .count();
        if (timeRemaining <= 0 && !gameWon)
        {
            currentPlayer = (currentPlayer == 1) ? 2 : 1; // Switch player
            startTime = std::chrono::steady_clock::now(); // Reset timer
            if (currentPlayer == 2)
            {
                computerMoveStartTime = std::chrono::steady_clock::now(); // Reset computer move timer
            }
        }

        // Computer's move
        if (currentPlayer == 2 && !gameWon)
        {
            auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(
                                   std::chrono::steady_clock::now() - computerMoveStartTime)
                                   .count();
            if (elapsedTime >= 2)
            { // Wait for 2 seconds before making a move
                if (computerMove())
                {
                    currentPlayer = 1;                            // Switch back to player
                    startTime = std::chrono::steady_clock::now(); // Reset timer
                }
            }
        }

        window.clear(sf::Color::White);

        // Draw grid and beads
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                cell.setPosition(j * CELL_SIZE, i * CELL_SIZE);
                cell.setFillColor(sf::Color::White);
                cell.setOutlineThickness(1);
                cell.setOutlineColor(sf::Color::Black);
                window.draw(cell);

                if (board[i][j] == 1)
                {
                    sf::CircleShape bead(CELL_SIZE / 3);
                    bead.setFillColor(sf::Color::Red);
                    bead.setPosition(j * CELL_SIZE + CELL_SIZE / 6, i * CELL_SIZE + CELL_SIZE / 6);
                    window.draw(bead);
                }
                else if (board[i][j] == 2)
                {
                    sf::CircleShape bead(CELL_SIZE / 3);
                    bead.setFillColor(sf::Color::Blue);
                    bead.setPosition(j * CELL_SIZE + CELL_SIZE / 6, i * CELL_SIZE + CELL_SIZE / 6);
                    window.draw(bead);
                }
            }
        }

        // Highlight valid moves
        for (auto &move : possibleMoves)
        {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(move.second * CELL_SIZE, move.first * CELL_SIZE);
            cell.setFillColor(sf::Color(0, 255, 0, 128));
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

        std::stringstream ss;
        ss << "Time: " << timeRemaining << "s";
        timerText.setString(ss.str());
        window.draw(timerText);

        // Draw win message if game is won
        if (gameWon)
        {
            window.draw(winText);
        }

        window.display();
    }

    return false; // Indicate that the game ended without returning to the main menu
}

bool computerMove()
{
    std::vector<std::tuple<int, int, int, int>> captureMoves; // Store capturing moves
    std::vector<std::pair<int, int>> possibleMoves;           // Store simple moves

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
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "6x6 Bead Grid");

    sf::Font font;
    font.loadFromFile("arial.ttf");

    // Option buttons
    sf::Text pvpButton("Player vs Player", font, 30);
    pvpButton.setPosition(100, BOARD_SIZE / 2 - 50);
    pvpButton.setFillColor(sf::Color::Black);

    sf::Text pvcButton("Player vs Computer", font, 30);
    pvcButton.setPosition(100, BOARD_SIZE / 2 + 10);
    pvcButton.setFillColor(sf::Color::Black);

    sf::RectangleShape pvpButtonBg(sf::Vector2f(300, 50));
    pvpButtonBg.setPosition(90, BOARD_SIZE / 2 - 55);
    pvpButtonBg.setFillColor(sf::Color::Cyan);

    sf::RectangleShape pvcButtonBg(sf::Vector2f(300, 50));
    pvcButtonBg.setPosition(90, BOARD_SIZE / 2 + 5);
    pvcButtonBg.setFillColor(sf::Color::Cyan);

    bool gameStarted = false;
    bool isPlayerVsComputer = false;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (!gameStarted && event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
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

        window.clear(sf::Color::White);

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
                if (playerVsComputer(window, font))
                {
                    gameStarted = false; // Reset to show the menu after returning to the main menu
                }
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

                sf::Text saveButton("  Save", font, 30);
                saveButton.setPosition(50, BOARD_SIZE + 20);
                saveButton.setFillColor(sf::Color::Black);

                sf::Text loadButton("  Load", font, 30);
                loadButton.setPosition(200, BOARD_SIZE + 20);
                loadButton.setFillColor(sf::Color::Black);

                sf::Text exitButton("  Exit", font, 30);
                exitButton.setPosition(200, BOARD_SIZE + 90); // Position below the Load button
                exitButton.setFillColor(sf::Color::Black);

                sf::Text mainMenuButton("Main Menu", font, 30);
                mainMenuButton.setPosition(350, BOARD_SIZE + 90); // Position next to the Exit button
                mainMenuButton.setFillColor(sf::Color::Black);

                sf::Text timerText("", font, 30);
                timerText.setPosition(350, BOARD_SIZE + 20); // Adjusted position next to Load button
                timerText.setFillColor(sf::Color::Black);

                sf::RectangleShape saveButtonBg(sf::Vector2f(100, 50));
                saveButtonBg.setPosition(50, BOARD_SIZE + 20);
                saveButtonBg.setFillColor(sf::Color::Cyan);

                sf::RectangleShape loadButtonBg(sf::Vector2f(100, 50));
                loadButtonBg.setPosition(200, BOARD_SIZE + 20);
                loadButtonBg.setFillColor(sf::Color::Cyan);

                sf::RectangleShape exitButtonBg(sf::Vector2f(100, 50));
                exitButtonBg.setPosition(200, BOARD_SIZE + 90); // Position below the Load button
                exitButtonBg.setFillColor(sf::Color::Red);

                sf::RectangleShape mainMenuButtonBg(sf::Vector2f(150, 50));
                mainMenuButtonBg.setPosition(350, BOARD_SIZE + 90);
                mainMenuButtonBg.setFillColor(sf::Color::Yellow);

                sf::Text winText("", font, 40);
                winText.setPosition(50, BOARD_SIZE / 2 - 20);
                winText.setFillColor(sf::Color::Black);

                startTime = std::chrono::steady_clock::now();

                int selectedRow = -1, selectedCol = -1;
                std::vector<std::pair<int, int>> possibleMoves;

                bool gameWon = false;
                bool returnToMainMenu = false;

                while (window.isOpen())
                {
                    sf::Event event;
                    while (window.pollEvent(event))
                    {
                        if (event.type == sf::Event::Closed)
                            window.close();
                        else if (!gameWon && event.type == sf::Event::MouseButtonPressed)
                        {
                            if (event.mouseButton.button == sf::Mouse::Left)
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
                                        exit(0);        // Ensure the program terminates
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
                        std::stringstream ss;
                        ss << "Time: " << timeRemaining << "s";
                        timerText.setString(ss.str());
                    }

                    window.clear(sf::Color::White);

                    // Draw grid
                    for (int i = 0; i <= GRID_SIZE; i++)
                    {
                        sf::Vertex horizontalLine[] = {
                            sf::Vertex(sf::Vector2f(0, i * CELL_SIZE), sf::Color::Black),
                            sf::Vertex(sf::Vector2f(BOARD_SIZE, i * CELL_SIZE), sf::Color::Black)};
                        window.draw(horizontalLine, 2, sf::Lines);

                        sf::Vertex verticalLine[] = {
                            sf::Vertex(sf::Vector2f(i * CELL_SIZE, 0), sf::Color::Black),
                            sf::Vertex(sf::Vector2f(i * CELL_SIZE, BOARD_SIZE), sf::Color::Black)};
                        window.draw(verticalLine, 2, sf::Lines);
                    }

                    // Draw beads
                    for (int i = 0; i < GRID_SIZE; i++)
                        for (int j = 0; j < GRID_SIZE; j++)
                            if (board[i][j] != 0)
                            {
                                sf::CircleShape bead(CELL_SIZE / 3);
                                bead.setFillColor(board[i][j] == 1 ? sf::Color::Red : sf::Color::Blue);
                                bead.setPosition(j * CELL_SIZE + CELL_SIZE / 6, i * CELL_SIZE + CELL_SIZE / 6);
                                window.draw(bead);
                            }

                    // Highlight valid moves
                    for (auto &move : possibleMoves)
                    {
                        sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                        cell.setPosition(move.second * CELL_SIZE, move.first * CELL_SIZE);
                        cell.setFillColor(sf::Color(0, 255, 0, 128));
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
                        window.draw(winText);

                        // Keep the event loop active to allow interaction with buttons
                        while (window.isOpen())
                        {
                            sf::Event event;
                            while (window.pollEvent(event))
                            {
                                if (event.type == sf::Event::Closed)
                                {
                                    window.close();
                                    exit(0);
                                }
                                else if (event.type == sf::Event::MouseButtonPressed)
                                {
                                    if (event.mouseButton.button == sf::Mouse::Left)
                                    {
                                        int x = event.mouseButton.x;
                                        int y = event.mouseButton.y;

                                        if (exitButtonBg.getGlobalBounds().contains(x, y))
                                        {
                                            window.close(); // Exit the game
                                            exit(0);        // Ensure the program terminates
                                        }
                                        else if (mainMenuButtonBg.getGlobalBounds().contains(x, y))
                                        {
                                            returnToMainMenu = true; // Return to the main menu
                                            return;                  // Exit the loop and reset the game
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
