#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <cmath>
#include <vector>
using namespace std;

#define boardSize 4
int board[boardSize][boardSize];
const int TIME_LIMIT = 15;

// Prototypes for the new functions
void chooseGameMode();
void callOriginalGame();
void playerVsComputer();
void playerVsPlayer();

// Existing function prototypes (unchanged)
void placeBead();
void printBoard();
void createBoard();
int countBeads(int player);
void saveGame(int currPlayer);
bool computerMove();
bool hasValidMoves(int player);
void loadGame(int &currPlayer);
bool isEmpty(int row, int column);
bool isValid(int row, int column);
bool makeMove(int player, int srcRow, int srcCol, int desRow, int desCol);
bool isEdible(int player, int srcRow, int srcCol, int desRow, int desCol);
bool isMovable(int player, int srcRow, int srcCol, int desRow, int desCol);

// New Function Implementations
void chooseGameMode()
{
    char choice;
    cout << "Choose game mode:\n";
    cout << "1. Player vs Player (Original Game)\n";
    cout << "2. Player vs Computer\n";
    cout << "Enter your choice (1 or 2): ";
    cin >> choice;

    if (choice == '1')
    {
        callOriginalGame(); // Call original game logic
    }
    else if (choice == '2')
    {
        playerVsComputer(); // Call Player vs Computer game mode
    }
    else
    {
        cout << "Invalid choice. Defaulting to Player vs Player mode.\n";
        callOriginalGame();
    }
}

void callOriginalGame()
{
    playerVsPlayer(); // Call the extracted Player vs Player game logic
}

void playerVsComputer()
{
    int currPlayer = 1; // Player 1 is human, Player 2 is the computer
    char option;

    cout << "Do you want to load a previous game? (y/n): ";
    cin >> option;
    if (option == 'y' || option == 'Y')
    {
        loadGame(currPlayer);
        printBoard();
    }
    else
    {
        createBoard();
        printBoard();
    }

    while (true)
    {
        cout << "Player " << currPlayer << "'s turn." << endl;

        if (countBeads(currPlayer) == 0)
        {
            cout << "Player " << currPlayer << " has no beads left. Player "
                 << ((currPlayer == 1) ? 2 : 1) << " wins!" << endl;
            break;
        }

        if (!hasValidMoves(currPlayer))
        {
            cout << "Player " << currPlayer << " is blocked. Player "
                 << ((currPlayer == 1) ? 2 : 1) << " wins!" << endl;
            break;
        }

        if (currPlayer == 1)
        {
            // Human Player's Turn
            auto start = chrono::steady_clock::now();

            while (true)
            {
                cout << "Enter source(row,col) and destination(row,col) (or -1 -1 -1 -1 to quit): ";
                int srcRow, srcCol, desRow, desCol;
                cin >> srcRow >> srcCol >> desRow >> desCol;

                if (srcRow == -1 && srcCol == -1 && desRow == -1 && desCol == -1)
                {
                    cout << "Do you want to save the game before quitting? (y/n): ";
                    cin >> option;
                    if (option == 'y' || option == 'Y')
                    {
                        saveGame(currPlayer);
                    }
                    cout << "Player " << currPlayer << " has quit the game." << endl;
                    return;
                }

                auto end = chrono::steady_clock::now();
                auto elapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
                if (elapsed >= TIME_LIMIT)
                {
                    cout << "Time's up! Player " << currPlayer << "'s turn is over." << endl;
                    break;
                }

                if (makeMove(currPlayer, srcRow, srcCol, desRow, desCol))
                {
                    printBoard();
                    break;
                }
                else
                {
                    cout << "Invalid move, try again." << endl;
                }
            }
        }
        else
        {
            // Computer's Turn
            cout << "Computer is thinking...\n";
            this_thread::sleep_for(chrono::seconds(1));

            if (!computerMove())
            {
                cout << "Computer has no valid moves. Skipping turn...\n";
            }

            printBoard();
        }

        currPlayer = (currPlayer == 1) ? 2 : 1;
    }

    cout << "Game Over!" << endl;
}

bool computerMove()
{
    vector<pair<int, int>> possibleMoves;
    pair<int, int> bestMove;
    pair<int, int> bestCapture;

    for (int srcRow = 0; srcRow < boardSize; ++srcRow)
    {
        for (int srcCol = 0; srcCol < boardSize; ++srcCol)
        {
            if (board[srcRow][srcCol] == 2)
            {
                for (int desRow = 0; desRow < boardSize; ++desRow)
                {
                    for (int desCol = 0; desCol < boardSize; ++desCol)
                    {
                        if (isEdible(2, srcRow, srcCol, desRow, desCol))
                        {
                            bestCapture = {srcRow, srcCol};
                            if (makeMove(2, srcRow, srcCol, desRow, desCol))
                            {
                                return true;
                            }
                        }
                        else if (isMovable(2, srcRow, srcCol, desRow, desCol))
                        {
                            possibleMoves.push_back({srcRow, srcCol});
                        }
                    }
                }
            }
        }
    }

    if (!possibleMoves.empty())
    {
        bestMove = possibleMoves[rand() % possibleMoves.size()];
        for (int desRow = 0; desRow < boardSize; ++desRow)
        {
            for (int desCol = 0; desCol < boardSize; ++desCol)
            {
                if (isMovable(2, bestMove.first, bestMove.second, desRow, desCol))
                {
                    return makeMove(2, bestMove.first, bestMove.second, desRow, desCol);
                }
            }
        }
    }

    return false;
}

// Extracted Game Logic (from original main)
void playerVsPlayer()
{
    int currPlayer = 1;
    char option;

    cout << "Do you want to load a previous game? (y/n): ";
    cin >> option;
    if (option == 'y' || option == 'Y')
    {
        loadGame(currPlayer);
        printBoard();
    }
    else
    {
        createBoard();
        printBoard();
    }

    while (true)
    {
        cout << "Player " << currPlayer << "'s turn." << endl;

        if (countBeads(currPlayer) == 0)
        {
            cout << "Player " << currPlayer << " has no beads left. Player "
                 << ((currPlayer == 1) ? 2 : 1) << " wins!" << endl;
            break;
        }

        if (!hasValidMoves(currPlayer))
        {
            cout << "Player " << currPlayer << " is blocked. Player "
                 << ((currPlayer == 1) ? 2 : 1) << " wins!" << endl;
            break;
        }

        auto start = chrono::steady_clock::now(); // Start the timer

        while (true)
        {
            cout << "Enter source(row,col) and destination(row,col) (or -1 -1 -1 -1 to quit): ";
            int srcRow, srcCol, desRow, desCol;
            cin >> srcRow >> srcCol >> desRow >> desCol;

            // Check for quitting the game
            if (srcRow == -1 && srcCol == -1 && desRow == -1 && desCol == -1)
            {
                cout << "Do you want to save the game before quitting? (y/n): ";
                cin >> option;
                if (option == 'y' || option == 'Y')
                {
                    saveGame(currPlayer);
                }
                cout << "Player " << currPlayer << " has quit the game." << endl;
                return;
            }

            // Check elapsed time
            auto end = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
            if (elapsed >= TIME_LIMIT)
            {
                cout << "Time's up! Player " << currPlayer << "'s turn is over." << endl;
                break;
            }

            // If valid move, update the board and break out of the loop
            if (makeMove(currPlayer, srcRow, srcCol, desRow, desCol))
            {
                printBoard();
                break; // Exit the input loop if a valid move was made
            }
            else
            {
                cout << "Invalid move, try again." << endl;
            }
        }

        currPlayer = (currPlayer == 1) ? 2 : 1; // Switch player
    }

    cout << "****Game Over!****" << endl;
}

// Original main function
int main()
{
    chooseGameMode(); // Entry point now goes to chooseGameMode
    return 0;
}

// FIXME:
bool makeMove(int player, int srcRow, int srcCol, int desRow, int desCol)
{
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol))
    {
        cout << "Invalid move. Out of board bounds." << endl;
        return false;
    }

    if (board[srcRow][srcCol] != player)
    {
        cout << "Invalid move. Source does not contain your bead." << endl;
        return false;
    }

    if (!isEmpty(desRow, desCol))
    {
        cout << "Invalid move. Destination not empty!" << endl;
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
        // Jump ,eat opponent bead
        int midRow = (srcRow + desRow) / 2;
        int midCol = (srcCol + desCol) / 2;

        // FIXME:
        board[midRow][midCol] = 0;
        board[desRow][desCol] = board[srcRow][srcCol];
        board[srcRow][srcCol] = 0;

        return true;
    }
    else
    {
        cout << "Invalid move " << endl;
        cout << "Neither simple nor valid jump." << endl;
        return false;
    }
}

int calculateDistance(int srcRow, int srcCol, int desRow, int desCol)
{
    return sqrt(pow((srcRow - desRow), 2) + pow((srcCol - desCol), 2));
}

// FIXME:
bool isMovable(int player, int srcRow, int srcCol, int desRow, int desCol)
{
    // if the source and destination is valid
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol))
    {
        return false;
    }

    // if the source is not empty
    if (isEmpty(srcRow, srcCol) || board[srcRow][srcCol] != player)
    {
        return false;
    }

    // if the destination is not empty
    if (!isEmpty(desRow, desCol))
    {
        return false;
    }

    // FIXME: replace with the calculate  function
    if (calculateDistance(srcRow, srcCol, desRow, desCol) != 1)
    {
        return false;
    }
    return true;
}

// FIXME:
bool isEdible(int player, int srcRow, int srcCol, int desRow, int desCol)
{
    // if the source and the destination is valid
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol))
    {
        return false;
    }

    if (isEmpty(srcRow, srcCol) || board[srcRow][srcCol] != player)
    {
        return false;
    }
    if (!isEmpty(desRow, desCol))
    {
        return false;
    }

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

// FIXME:
// checks surrounding 2 spaces for valid move
bool hasValidMoves(int player)
{
    for (int i = 0; i < boardSize; i++)
    {
        // nested for loop n2
        for (int j = 0; j < boardSize; j++)
        {
            if (board[i][j] == player)
            {
                for (int di = -2; di <= 2; di++)
                {
                    // constant time run
                    for (int dj = -2; dj <= 2; dj++)
                    {
                        if (di == 0 && dj == 0)
                            continue;
                        int newRow = i + di;
                        int newCol = j + dj;
                        if (isEdible(player, i, j, newRow, newCol))
                        {
                            return true;
                        }
                        else if (isMovable(player, i, j, newRow, newCol))
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

void createBoard()
{
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            board[i][j] = 0;
        }
    }
    placeBead();
}

void placeBead()
{
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            if (i < boardSize / 3)
            {
                board[i][j] = 1;
            }
            else if (i >= (boardSize - boardSize / 3))
            {
                board[i][j] = 2;
            }
        }
    }
}

void printBoard()
{
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            if (board[i][j] == 0)
            {
                cout << ". ";
            }
            else
            {
                cout << board[i][j] << " ";
            }
        }
        cout << endl;
    }
}

int countBeads(int player)
{
    int count = 0;
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            if (board[i][j] == player)
            {
                count++;
            }
        }
    }
    return count;
}

bool isEmpty(int row, int column)
{
    return board[row][column] == 0;
}

bool isValid(int row, int column)
{
    return (row >= 0 && column >= 0 && row < boardSize && column < boardSize);
}

// loads the game
void loadGame(int &currPlayer)
{
    ifstream file("saved_game.txt");
    if (!file)
    {
        cout << "No game found." << endl;
        cout << "Starting new game!" << endl;
        createBoard();
        return;
    }

    file >> currPlayer;
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            file >> board[i][j];
        }
    }

    file.close();
    cout << "##Game loaded successfully!" << endl;
}

void saveGame(int currPlayer)
{
    ofstream file("savedG.txt");
    if (!file)
    {
        cout << "Error saving the game!" << endl;
        return;
    }

    file << currPlayer << endl;
    for (int i = 0; i < boardSize; i++)
    {
        for (int j = 0; j < boardSize; j++)
        {
            file << board[i][j] << " ";
        }
        file << endl;
    }

    file.close();
    cout << "***Game saved successfully!***" << endl;
}
