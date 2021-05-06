#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <ctime>
#include <windows.h>
#include <SDL.h>
#include <thread>

#define WINDOWING 1

#if WINDOWING
// Limit loop rate for visibility
#define LIMIT_RATE 1
// Tick-rate in milliseconds (if LIMIT_RATE == 1) 
#define TICK_RATE 50

// Cell map dimensions
unsigned int cellmap_width = 200;
unsigned int cellmap_height = 200;

// Width and height (in pixels) of a cell i.e. magnification
unsigned int cell_size = 3;

SDL_Window *window = NULL;
SDL_Surface* surface = NULL;
unsigned int s_width = cellmap_width * cell_size;
unsigned int s_height = cellmap_height * cell_size;


void DrawCell(int x, int y, unsigned int color)
{
    Uint8* pixel_ptr = (Uint8*)surface->pixels + (y * cell_size * s_width + x * cell_size) * 4;

    for (unsigned int i = 0; i < cell_size; i++)
    {
        for (unsigned int j = 0; j < cell_size; j++)
        {
            *(pixel_ptr + j * 4) = color;
            *(pixel_ptr + j * 4 + 1) = color;
            *(pixel_ptr + j * 4 + 2) = color;
        }
        pixel_ptr += s_width * 4;
    }
}
#endif //endif of windowing


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
    std::vector<int> neighGrid;
    std::vector<int> neighGrid2;

public:
    Conway(int n, double p1);
    Conway(int n, std::vector<int>& v);
    void initNeigh(int y, int x);
    void printNeigh();
    void increaseNeighbourCount(int y, int x);
    void decreaseNeighbourCount(int y, int x);
    void oneStep(int k);

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
    int xleft, xright, yabove, ybelow;
    //the reason for this section is that the modulo operation in C++ is a mess
    xleft = (x == 0) ? N - 1 : x - 1;
    xright = (x == (N - 1)) ? 0 : x + 1;

    yabove = (y == 0) ? N - 1 : y - 1;
    ybelow = (y == (N - 1)) ? 0 : y + 1;

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
    int xleft, xright, yabove, ybelow;

    xleft = (x == 0) ? N - 1 : x - 1;
    xright = (x == (N - 1)) ? 0 : x + 1;

    yabove = (y == 0) ? N - 1 : y - 1;
    ybelow = (y == (N - 1)) ? 0 : y + 1;

    neighGrid2[xleft + N * yabove] += 1;
    neighGrid2[x + N * yabove] += 1;
    neighGrid2[xright + N * yabove] += 1;

    neighGrid2[xleft + N * y] += 1;
    neighGrid2[xright + N * y] += 1;

    neighGrid2[xleft + N * ybelow] += 1;
    neighGrid2[x + N * ybelow] += 1;
    neighGrid2[xright + N * ybelow] += 1;
}

void Conway::decreaseNeighbourCount(int y, int x)
{
    int xleft, xright, yabove, ybelow;

    xleft = (x == 0) ? N - 1 : x - 1;
    xright = (x == (N - 1)) ? 0 : x + 1;

    yabove = (y == 0) ? N - 1 : y - 1;
    ybelow = (y == (N - 1)) ? 0 : y + 1;

    neighGrid2[xleft + N * yabove] -= 1;
    neighGrid2[x + N * yabove] -= 1;
    neighGrid2[xright + N * yabove] -= 1;

    neighGrid2[xleft + N * y] -= 1;
    neighGrid2[xright + N * y] -= 1;

    neighGrid2[xleft + N * ybelow] -= 1;
    neighGrid2[x + N * ybelow] -= 1;
    neighGrid2[xright + N * ybelow] -= 1;
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
                    #if WINDOWING
                    DrawCell(y, x, 0xFF); //color the cell on the canvas to white
                    #endif
                    //std::cout << y << " inc " << x << std::endl;
                }
            }
            else
            {
                if ((neighGrid[y * N + x] != k) && (neighGrid[y * N + x] != k + 1))
                {
                    (*this)(y, x) = 0;
                    decreaseNeighbourCount(y, x);
                    #if WINDOWING
                    DrawCell(y, x, 0x00); //color the cell on the canvas to black
                    #endif          
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


int main(int argc, char* argv[])
{

    //init grid from vector
    std::vector<int> v = { 0,0,0,0,0,0,
                           0,1,1,0,0,0,
                           0,1,1,0,0,0,
                           0,0,0,1,1,0,
                           0,0,0,1,1,0,
                           0,0,0,0,0,0 };

    Conway cnw(200, 0.5);

    #if WINDOWING
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Conway's Game of Life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, s_width, s_height, SDL_WINDOW_SHOWN);
    surface = SDL_GetWindowSurface(window);
    SDL_Event e;


    bool quit = false;
    while (!quit)
    {
        while (SDL_PollEvent(&e) != 0)
            if (e.type == SDL_QUIT) quit = true;

        // Recalculate and draw next generation
        cnw.oneStep(2);
        // Update frame buffer
        SDL_UpdateWindowSurface(window);

    #endif
    #if LIMIT_RATE
        SDL_Delay(TICK_RATE);
    #endif
    }
    #if WINDOWING
    // Destroy window 
    SDL_DestroyWindow(window);
    // Quit SDL subsystems 
    SDL_Quit();

    system("pause");
    #endif


    return 0;
}