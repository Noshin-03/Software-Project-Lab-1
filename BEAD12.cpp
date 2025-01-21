#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
using namespace std;

#define BOARD_SIZE 4
int board[BOARD_SIZE][BOARD_SIZE];

void createBoard();
void placeBead();
void printBoard();
bool isEmpty(int row, int column);
bool isValid(int row, int column);
bool makeMove(int player, int srcRow, int srcCol, int desRow, int desCol);
bool isMovable(int player, int srcRow, int srcCol, int desRow, int desCol);
bool isEdible(int player, int srcRow, int srcCol, int desRow, int desCol);
bool hasValidMoves(int player);
int countBeads(int player);
void saveGame(int currentPlayer);
void loadGame(int &currentPlayer);

const int TIME_LIMIT = 30; // Time limit for each player's turn in seconds

int main()
{
    int currentPlayer = 1; // Player 1 starts
    char option;

    cout << "Do you want to load a previous game? (y/n): ";
    cin >> option;
    if (option == 'y' || option == 'Y')
    {
        loadGame(currentPlayer);
        printBoard();
    }
    else
    {
        createBoard();
        printBoard();
    }

    while (true)
    {
        cout << "Player " << currentPlayer << "'s turn." << endl;

        // Check if the current player has any beads left
        if (countBeads(currentPlayer) == 0)
        {
            cout << "Player " << currentPlayer << " has no beads left. Player "
                 << ((currentPlayer == 1) ? 2 : 1) << " wins!" << endl;
            break;
        }

        // Check if the player is blocked
        if (!hasValidMoves(currentPlayer))
        {
            cout << "Player " << currentPlayer << " is blocked. Player "
                 << ((currentPlayer == 1) ? 2 : 1) << " wins!" << endl;
            break;
        }

        auto start = chrono::steady_clock::now(); // Start timer

        while (true)
        {
            cout << "Enter source(row, col) and destination(row, col) (or -1 -1 -1 -1 to quit): ";
            int srcRow, srcCol, desRow, desCol;
            cin >> srcRow >> srcCol >> desRow >> desCol;

            if (srcRow == -1 && srcCol == -1 && desRow == -1 && desCol == -1)
            {
                cout << "Do you want to save the game before quitting? (y/n): ";
                cin >> option;
                if (option == 'y' || option == 'Y')
                {
                    saveGame(currentPlayer);
                }
                cout << "Player " << currentPlayer << " has quit the game." << endl;
                return 0;
            }

            if (makeMove(currentPlayer, srcRow, srcCol, desRow, desCol))
            {
                printBoard();
                break; 
            }

          
            auto end = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
            if (elapsed >= TIME_LIMIT)
            {
                cout << "Time's up! Player " << currentPlayer << " has run out of time." << endl;
                break;
            }
        }

        // Switch to the other player
        currentPlayer = (currentPlayer == 1) ? 2 : 1;
    }

    cout << "Game Over!" << endl;
    return 0;
}

void saveGame(int currentPlayer)
{
    ofstream file("saved_game.txt");
    if (!file)
    {
        cout << "Error saving the game!" << endl;
        return;
    }

    file << currentPlayer << endl;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            file << board[i][j] << " ";
        }
        file << endl;
    }

    file.close();
    cout << "Game saved successfully!" << endl;
}

bool makeMove(int player, int srcRow, int srcCol, int desRow, int desCol)
{
    if (!isValid(srcRow, srcCol) || !isValid(desRow, desCol))
    {
        cout << "Invalid move: Out of board bounds." << endl;
        return false;
    }

    if (board[srcRow][srcCol] != player)
    {
        cout << "Invalid move: The selected source does not contain your bead." << endl;
        return false;
    }

    if (!isEmpty(desRow, desCol))
    {
        cout << "Invalid move: The destination is not empty." << endl;
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
        // Jump and eat opponent's bead
        int midRow = (srcRow + desRow) / 2;
        int midCol = (srcCol + desCol) / 2;

        board[midRow][midCol] = 0; // Remove opponent's bead
        board[desRow][desCol] = board[srcRow][srcCol];
        board[srcRow][srcCol] = 0;

        return true;
    }
    else
    {
        cout << "Invalid move: The move is neither simple nor a valid jump." << endl;
        return false;
    }
}

void loadGame(int &currentPlayer)
{
    ifstream file("saved_game.txt");
    if (!file)
    {
        cout << "No saved game found. Starting a new game!" << endl;
        createBoard();
        return;
    }

    file >> currentPlayer;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            file >> board[i][j];
        }
    }

    file.close();
    cout << "Game loaded successfully!" << endl;
}

bool isEmpty(int row, int column)
{
    return board[row][column] == 0;
}

bool isValid(int row, int column)
{
    return (row >= 0 && column >= 0 && row < BOARD_SIZE && column < BOARD_SIZE);
}

bool isMovable(int player, int srcRow, int srcCol, int desRow, int desCol)
{
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
    if (abs(srcRow - desRow) > 1 || abs(srcCol - desCol) > 1)
    {
        return false;
    }
    return true;
}

bool isEdible(int player, int srcRow, int srcCol, int desRow, int desCol)
{
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

    int opponent = (player == 1) ? 2 : 1;
    if (board[midRow][midCol] == opponent &&
        abs(srcRow - desRow) == 2 && abs(srcCol - desCol) == 2)
    {
        return true;
    }
    return false;
}

bool hasValidMoves(int player)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
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
                        if (isMovable(player, i, j, newRow, newCol) ||
                            isEdible(player, i, j, newRow, newCol))
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
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = 0;
        }
    }
    placeBead();
}

void placeBead()
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (i < BOARD_SIZE / 3)
            {
                board[i][j] = 1;
            }
            else if (i >= (BOARD_SIZE - BOARD_SIZE / 3))
            {
                board[i][j] = 2;
            }
        }
    }
}

void printBoard()
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
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
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            if (board[i][j] == player)
            {
                count++;
            }
        }
    }
    return count;
}