#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <ctime>

int rollCellState(double p1)
{
    // Generate a 0 or 1 randomly. 
    // Seed is the current time, p1 is probability of choosing 1.
    double random_variable = static_cast<double>(std::rand()) / RAND_MAX;
    if (random_variable > p1) { return 0; }
    else return 1;
}

struct Conway
{
private:
    int N;
    std::vector<int> grid; //the cell states are registered on this grid
    std::vector<int> neighGrid; //number of living neighbours
    std::vector<int> neighGrid2; //copy of the above table

public:
    Conway(int n, double p1); //initialize grid by specifying the probability of cell being alive
    Conway(int n, std::vector<int>& v); //initialize grid explicitly
    void initNeigh(int y, int x); //initialize neighbour grid
    void printNeigh();
    void increaseNeighbourCount(int y, int x); //add 1 to every neighbouring cell's neigh grid cell
    void decreaseNeighbourCount(int y, int x);
    void oneStep(int k); //evolving one step in time

    int& operator()(int i, int j)
    {
        return grid[N * i + j];
    }

    friend std::ostream& operator<<(std::ostream& o, Conway& A)
    {
        for (int i = 0; i < A.N; i++)
        {
            for (int j = 0; j < A.N; j++)
            {
                o << A.grid[i * A.N + j] << " ";
            }
            o << "\n";
        }
        o << "\n";
        return o;
    }


};


Conway::Conway(int n, double p1)
{
    N = n;
    for (int i = 0; i < N * N; i++)
    {
        grid.push_back(rollCellState(0.5));
    }

    for (int y = 0; y < N; y++)
    {
        for (int x = 0; x < N; x++)
        {
            initNeigh(y, x);
        }
    }
}

Conway::Conway(int n, std::vector<int>& v)
{
    N = n;
    grid = v;
    for (int y = 0; y < N; y++)
    {
        for (int x = 0; x < N; x++)
        {
            initNeigh(y, x);
        }
    }
}

void Conway::initNeigh(int y, int x)
{
    int neighCount = 0;
    //the reason for this section is that the modulo operation in C++ is a mess
    int xleft = (x == 0) ? N - 1 : x - 1;
    int xright = (x == (N - 1)) ? 0 : x + 1;

    int yabove = (y == 0) ? N - 1 : y - 1;
    int ybelow = (y == (N - 1)) ? 0 : y + 1;

    neighCount += (*this)(yabove, xleft);
    neighCount += (*this)(yabove, x);
    neighCount += (*this)(yabove, xright);

    neighCount += (*this)(y, xleft);
    neighCount += (*this)(y, xright);

    neighCount += (*this)(ybelow, xleft);
    neighCount += (*this)(ybelow, x);
    neighCount += (*this)(ybelow, xright);

    neighGrid.push_back(neighCount);
}

void Conway::increaseNeighbourCount(int y, int x)
{
    int xleft = (x == 0) ? N - 1 : x - 1;
    int xright = (x == (N - 1)) ? 0 : x + 1;

    int yabove = (y == 0) ? N * (N - 1) : N * (y - 1);
    int ybelow = (y == (N - 1)) ? 0 : N * (y + 1);

    neighGrid2[xleft + yabove] += 1;
    neighGrid2[x + yabove] += 1;
    neighGrid2[xright + yabove] += 1;

    neighGrid2[xleft + N * y] += 1;
    neighGrid2[xright + N * y] += 1;

    neighGrid2[xleft + ybelow] += 1;
    neighGrid2[x + ybelow] += 1;
    neighGrid2[xright + ybelow] += 1;
}

void Conway::decreaseNeighbourCount(int y, int x)
{
    int xleft = (x == 0) ? N - 1 : x - 1;
    int xright = (x == (N - 1)) ? 0 : x + 1;

    int yabove = (y == 0) ? N - 1 : y - 1;
    int ybelow = (y == (N - 1)) ? 0 : y + 1;

    neighGrid2[xleft + yabove] -= 1;
    neighGrid2[x + yabove] -= 1;
    neighGrid2[xright + yabove] -= 1;

    neighGrid2[xleft + N * y] -= 1;
    neighGrid2[xright + N * y] -= 1;

    neighGrid2[xleft + ybelow] -= 1;
    neighGrid2[x + ybelow] -= 1;
    neighGrid2[xright + ybelow] -= 1;
}

void Conway::oneStep(int k)
{
    neighGrid2 = neighGrid;
    for (int y = 0; y < N; y++)
    {
        for (int x = 0; x < N; x++)
        {
            if ((*this)(y, x) == 0)
            {
                //if the cell is dead and it has k+1 living neighbours, make it alive
                if (neighGrid[y * N + x] == k + 1)

                {
                    (*this)(y, x) = 1;
                    increaseNeighbourCount(y, x);
                    //std::cout << y << " inc " << x << std::endl;
                }
            }
            else
            {
                if ((neighGrid[y * N + x] != k) & (neighGrid[y * N + x] != k + 1))
                {
                    (*this)(y, x) = 0;
                    decreaseNeighbourCount(y, x);
                    //std::cout << y << " dec " << x << std::endl;
                }
            }
        }
    }
    neighGrid = neighGrid2;
}

void Conway::printNeigh()
{
    for (int i = 0; i < N; i++)
    {
        for (int j = 0; j < N; j++)
        {
            std::cout << neighGrid[i * N + j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}


int main()
{
    std::srand(std::time(0)); // set current time as random seed
    //Conway cnw(4, 0.5);
    std::vector<int> v = { 0,0,0,0,0,0,
                           0,1,1,0,0,0,
                           0,1,1,0,0,0,
                           0,0,0,1,1,0,
                           0,0,0,1,1,0,
                           0,0,0,0,0,0 };

    Conway cnw(6, v);
    std::cout << "\n" << cnw;
    cnw.printNeigh();

    cnw.oneStep(2);

    std::cout << "\n" << cnw;
    cnw.printNeigh();

    return 0;
}