#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <fstream>
#include <chrono>
#include <sstream>

const int GRID_SIZE = 6;
const int CELL_SIZE = 100;
const int BOARD_SIZE = GRID_SIZE * CELL_SIZE;
const int WINDOW_HEIGHT = BOARD_SIZE + 150; // Extra space for buttons
const int WINDOW_WIDTH = BOARD_SIZE;

int board[GRID_SIZE][GRID_SIZE] = {0};
int currentPlayer = 1; // 1 for Red, 2 for Blue
std::chrono::time_point<std::chrono::steady_clock> startTime;
const int TURN_TIME_LIMIT = 30; // 30 seconds per turn

// Function to check valid position
bool isValid(int row, int col) {
    return row >= 0 && col >= 0 && row < GRID_SIZE && col < GRID_SIZE;
}

// Function to check if a cell is empty
bool isEmpty(int row, int col) {
    return board[row][col] == 0;
}

// Calculate distance between two points
int calculateDistance(int srcRow, int srcCol, int desRow, int desCol) {
    if ((std::pow(srcRow - desRow, 2) + std::pow(srcCol - desCol, 2)) == 5 ) {
        return 3;//don't change this value
    }
    return std::sqrt(std::pow(srcRow - desRow, 2) + std::pow(srcCol - desCol, 2));
    //don't change this value
}

// Check if a bead can move
bool isMovable(int player, int srcRow, int srcCol, int desRow, int desCol) {
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

bool isEdible(int player, int srcRow, int srcCol, int desRow, int desCol) {
    // Ensure source and destination are valid
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol)) {
        return false;
    }

    // Ensure the source contains the player's bead
    if (isEmpty(srcRow, srcCol) || board[srcRow][srcCol] != player) {
        return false;
    }

    // Ensure the destination is empty
    if (!isEmpty(desRow, desCol)) {
        return false;
    }

    // Calculate the midpoint between source and destination
    int midRow = (srcRow + desRow) / 2;
    int midCol = (srcCol + desCol) / 2;

    if (!isValid(midRow, midCol)) {
        return false;
    }

    int opponent = (player == 1) ? 2 : 1;
    if (board[midRow][midCol] != opponent) {
        return false;
    }
    if (calculateDistance(srcRow, srcCol, desRow, desCol) == 2) {
        return true;
    }

    return false;
}

bool hasValidMoves(int player) {
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] == player) {
                for (int di = -2; di <= 2; di++) {
                    for (int dj = -2; dj <= 2; dj++) {
                        if (di == 0 && dj == 0)
                            continue;
                        int newRow = i + di;
                        int newCol = j + dj;
                        if (isEdible(player, i, j, newRow, newCol) || isMovable(player, i, j, newRow, newCol)) {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool makeMove(int player, int srcRow, int srcCol, int desRow, int desCol) {
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol)) {
        return false;
    }

    if (board[srcRow][srcCol] != player) {
        return false;
    }

    if (!isEmpty(desRow, desCol)) {
        return false;
    }

    if (isMovable(player, srcRow, srcCol, desRow, desCol)) {
        // Simple move
        board[desRow][desCol] = board[srcRow][srcCol];
        board[srcRow][srcCol] = 0;
        return true;
    } else if (isEdible(player, srcRow, srcCol, desRow, desCol)) {
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
void saveBoard() {
    std::ofstream file("board_save.txt");
    for (int i = 0; i < GRID_SIZE; ++i) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            file << board[i][j] << " ";
        }
        file << "\n";
    }
    file.close();
}

// Load game state
void loadBoard() {
    std::ifstream file("board_save.txt");
    if (file.is_open()) {
        for (int i = 0; i < GRID_SIZE; ++i) {
            for (int j = 0; j < GRID_SIZE; ++j) {
                file >> board[i][j];
            }
        }
        file.close();
    }
}

// Switch player and reset the timer
void switchPlayer() {
    currentPlayer = (currentPlayer == 1) ? 2 : 1;
    startTime = std::chrono::steady_clock::now();
}

// Get remaining time before auto-switch
int getTimeRemaining() {
    auto currentTime = std::chrono::steady_clock::now();
    int elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
    return TURN_TIME_LIMIT - elapsedTime;
}

bool checkWinCondition(sf::Text &winText) {
    int player1Beads = 0, player2Beads = 0;

    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            if (board[i][j] == 1) {
                player1Beads++;
            } else if (board[i][j] == 2) {
                player2Beads++;
            }
        }
    }

    if (player1Beads == 0) {
        winText.setString("Player 2 Wins! Congratulations!");
        return true;
    }

    if (player2Beads == 0) {
        winText.setString("Player 1 Wins! Congratulations!");
        return true;
    }

    return false;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "6x6 Bead Grid");

    // Initialize beads
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < GRID_SIZE; j++)
            board[i][j] = 1; // Red beads for Player 1

    for (int i = 4; i < GRID_SIZE; i++)
        for (int j = 0; j < GRID_SIZE; j++)
            board[i][j] = 2; // Blue beads for Player 2

    sf::Font font;
    font.loadFromFile("arial.ttf");

    sf::Text saveButton("  Save", font, 30);
    saveButton.setPosition(50, BOARD_SIZE + 20);
    saveButton.setFillColor(sf::Color::Black);

    sf::Text loadButton("  Load", font, 30);
    loadButton.setPosition(200, BOARD_SIZE + 20);
    loadButton.setFillColor(sf::Color::Black);

    sf::Text timerText("", font, 30);
    timerText.setPosition(350, BOARD_SIZE + 20);
    timerText.setFillColor(sf::Color::Black);

    sf::RectangleShape saveButtonBg(sf::Vector2f(100, 50));
    saveButtonBg.setPosition(50, BOARD_SIZE + 20);
    saveButtonBg.setFillColor(sf::Color::Cyan);

    sf::RectangleShape loadButtonBg(sf::Vector2f(100, 50));
    loadButtonBg.setPosition(200, BOARD_SIZE + 20);
    loadButtonBg.setFillColor(sf::Color::Cyan);

    sf::Text winText("", font, 40);
    winText.setPosition(50, BOARD_SIZE / 2 - 20);
    winText.setFillColor(sf::Color::Black);

    startTime = std::chrono::steady_clock::now();

    int selectedRow = -1, selectedCol = -1;
    std::vector<std::pair<int, int>> possibleMoves;

    bool gameWon = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            else if (!gameWon && event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    int x = event.mouseButton.x;
                    int y = event.mouseButton.y;

                    if (y > BOARD_SIZE) {
                        if (saveButton.getGlobalBounds().contains(x, y)) {
                            saveBoard();
                        } else if (loadButton.getGlobalBounds().contains(x, y)) {
                            loadBoard();
                        }
                    } else {
                        int row = y / CELL_SIZE;
                        int col = x / CELL_SIZE;

                        if (isValid(row, col)) {
                            if (selectedRow == -1 && selectedCol == -1 && !isEmpty(row, col) && board[row][col] == currentPlayer) {
                                selectedRow = row;
                                selectedCol = col;
                                possibleMoves.clear();
                                for (int i = 0; i < GRID_SIZE; i++)
                                    for (int j = 0; j < GRID_SIZE; j++)
                                        if (isMovable(currentPlayer, selectedRow, selectedCol, i, j) || isEdible(currentPlayer, selectedRow, selectedCol, i, j))
                                            possibleMoves.push_back({i, j});
                            } else if (selectedRow != -1 && selectedCol != -1) {
                                bool moved = makeMove(currentPlayer, selectedRow, selectedCol, row, col);
                                if (moved) {
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

        // Check if time expired
        if (!gameWon) {
            int timeRemaining = getTimeRemaining();
            if (timeRemaining <= 0) {
                switchPlayer();
            }

            // Check if a player has won
            if (checkWinCondition(winText)) {
                gameWon = true;
            }

            // Update timer display
            std::stringstream ss;
            ss << "Time Left: " << timeRemaining << "s";
            timerText.setString(ss.str());
        }

        window.clear(sf::Color::White);

        // Draw grid
        for (int i = 0; i <= GRID_SIZE; i++) {
            sf::Vertex horizontalLine[] = {
                sf::Vertex(sf::Vector2f(0, i * CELL_SIZE), sf::Color::Black),
                sf::Vertex(sf::Vector2f(BOARD_SIZE, i * CELL_SIZE), sf::Color::Black)
            };
            window.draw(horizontalLine, 2, sf::Lines);

            sf::Vertex verticalLine[] = {
                sf::Vertex(sf::Vector2f(i * CELL_SIZE, 0), sf::Color::Black),
                sf::Vertex(sf::Vector2f(i * CELL_SIZE, BOARD_SIZE), sf::Color::Black)
            };
            window.draw(verticalLine, 2, sf::Lines);
        }

        // Draw beads
        for (int i = 0; i < GRID_SIZE; i++)
            for (int j = 0; j < GRID_SIZE; j++)
                if (board[i][j] != 0) {
                    sf::CircleShape bead(CELL_SIZE / 3);
                    bead.setFillColor(board[i][j] == 1 ? sf::Color::Red : sf::Color::Blue);
                    bead.setPosition(j * CELL_SIZE + CELL_SIZE / 6, i * CELL_SIZE + CELL_SIZE / 6);
                    window.draw(bead);
                }

        // Highlight valid moves
        for (auto &move : possibleMoves) {
            sf::RectangleShape cell(sf::Vector2f(CELL_SIZE, CELL_SIZE));
            cell.setPosition(move.second * CELL_SIZE, move.first * CELL_SIZE);
            cell.setFillColor(sf::Color(0, 255, 0, 128));
            window.draw(cell);
        }

        // Draw buttons and timer
        window.draw(saveButtonBg);
        window.draw(loadButtonBg);
        window.draw(saveButton);
        window.draw(loadButton);
        window.draw(timerText);

        // Draw win message if game is won
        if (gameWon) {
            window.draw(winText);
        }

        window.display();
    }

    return 0;
}
