#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
using namespace std;

#define boardSize 6
int board[boardSize][boardSize];
const int TIME_LIMIT = 30;

// function prototypes
void placeBead();
void printBoard();
void createBoard();
int countBeads(int player);
void saveGame(int currPlayer);
bool hasValidMoves(int player);
void loadGame(int &currPlayer);
bool isEmpty(int row, int column);
bool isValid(int row, int column);
bool makeMove(int player, int srcRow, int srcCol, int desRow, int desCol);
bool isEdible(int player, int srcRow, int srcCol, int desRow, int desCol);
bool isMovable(int player, int srcRow, int srcCol, int desRow, int desCol);


int main()
{
    int currPlayer = 1; 
    char option;

    cout << "Do you want to load a previous game?(y/n):";
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
            cout << "Player " << currPlayer << " has no beads left.Player  "
                 << ((currPlayer == 1) ? 2 : 1) << " wins!" << endl;
            break;
        }

        if (!hasValidMoves(currPlayer))
        {
            cout << "Player " << currPlayer << " is blocked . Player "
                 << ((currPlayer == 1) ? 2 : 1) << " wins!" << endl;
            break;
        }

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
                return 0;
            }

            if (makeMove(currPlayer, srcRow, srcCol, desRow, desCol))
            {
                printBoard();
                break; 
            }

          
            auto end = chrono::steady_clock::now();
            auto elapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
            if (elapsed >= TIME_LIMIT)
            {
                cout << "Time's up! Player " << currPlayer << endl;
                break;
            }
        }

        currPlayer = (currPlayer == 1) ? 2 : 1;
    }

    cout << "*Game Over!*" << endl;
    return 0;
}

void saveGame(int currPlayer)
{
    ofstream file("saved_game.txt");
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
    cout << "Game saved successfully!" << endl;
}

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

//loads the game
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

    if (!isValid(midRow, midCol)) {
        return false;
    }

    int opponent = (player == 1) ? 2 : 1;
    if (board[midRow][midCol] != opponent)
    {
        return false;
    } 
    if (abs(srcRow - desRow == 2) && abs(srcCol - desCol == 2)) {
        return true;
    }

    return false;
}

bool isEmpty(int row, int column)
{
    return board[row][column] == 0;
}

bool isValid(int row, int column)
{
    return (row >= 0 && column >= 0 && row < boardSize && column < boardSize);
}

//checks surronding 2 spaces for valid move
bool hasValidMoves(int player)
{
    for (int i = 0; i < boardSize; i++)
    {
        //nested for loop n2
        for (int j = 0; j < boardSize; j++)
        {
            if (board[i][j] == player)
            {
                for (int di = -2; di <= 2; di++)
                {
                    //constant time run
                    for (int dj = -2; dj <= 2; dj++)
                    {
                        if (di == 0 && dj == 0)
                            continue;
                        int newRow = i + di;
                        int newCol = j + dj;
                        if (isMovable(player, i, j, newRow, newCol) || isEdible(player, i, j, newRow, newCol))
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