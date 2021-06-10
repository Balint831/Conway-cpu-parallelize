#include <vector>
#include <iostream>
#include <random>
#include <algorithm>
#include <ctime>
#include <thread>
#include <fstream>
#include <time_meas.hpp>


#define WINDOWING 0 //if 0, the grid is not visualized, if 1, a window is created
#define MP 0 //multiprocessing

// Limit loop rate for visibility
#define LIMIT_RATE 0
// Tick-rate in milliseconds (if LIMIT_RATE == 1) 
#define TICK_RATE 50


#if WINDOWING 
#include <windows.h>
#include <SDL.h>

// Cell map dimensions
unsigned int cellmap_width = 1000;
unsigned int cellmap_height = 1000;

// Width and height (in pixels) of a cell i.e. magnification
unsigned int cell_size = 1;

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


struct Conway
{
private:
    int N;
    std::vector<char> grid; //the cell states are registered on this grid
    std::vector<char> grid2;

public:
    Conway(int n, double p1);
    Conway(int n, std::vector<char>& v);
    
    char countNeighs(int y, int x);
    void oneRow(int y); //y - index of row
    void multiRow(int y_start, int y_end);
    void oneStep();
    void printGrid2();
    
    char& operator()(int y, int x)
    {
        return grid[N * y + x];
    }

    friend std::ostream& operator<<(std::ostream& o, Conway& A)
    {
        for (int i = 0; i < A.N; i++)
        {
            for (int j = 0; j < A.N; j++)
            {
                o << static_cast<int>(A.grid[i * A.N + j]) << " ";
            }
            o << "\n";
        }
        o << "\n";
        return o;
    }
};

void Conway::printGrid2()
{
    grid2.swap(grid);
    std::cout << *this;
    grid2.swap(grid);
}


Conway::Conway(int n, double p1)
{
    N = n;
    std::vector<char> g(n * n);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d({ 1- p1, 1 });

    std::generate(g.begin(), g.end(), [&] { return d(gen); });
    
    grid = std::move(g);

    std::copy(grid.begin(), grid.end(), std::back_inserter(grid2) );
}

Conway::Conway(int n, std::vector<char>& v)
{
    N = n;
    grid = v;
    std::copy(grid.begin(), grid.end(), std::back_inserter(grid2));
}

char Conway::countNeighs(int y, int x)
{
    // Loop unrolling
    char neighCount = 0;
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
    return neighCount;
}

void Conway::oneRow(int y)
{
    for (int x = 0; x < N; ++x)
    {
        char neighCount = countNeighs(y, x); //Counting the living neighbours
        //std::cout << static_cast<int>(neighCount) <<" ";

        if ( (*this)(y, x) == 0 && (neighCount == 3)) { grid2[N * y + x] = 1; }

        if ( (*this)(y, x) == 1 && ((neighCount != 3) && (neighCount != 2))) { grid2[N * y + x] = 0; }

    }
}

void Conway::multiRow(int y_start, int y_end)
{
    for (; y_start < y_end; ++y_start)
    {
        oneRow(y_start); 
    }
}


void Conway::oneStep()
{

#if MP
    std::thread threads[8];

    for (int thread_num = 0; thread_num < 8; ++thread_num)  //maximum number of 8 threads on this machine
    {
        int num_of_tasks = std::floor(N / 8);
        int y_start = thread_num * num_of_tasks;
        int y_end = (thread_num  == 7) ? ((thread_num + 1) * num_of_tasks + N % 8) : (thread_num + 1) * num_of_tasks; //the last thread takes on the remainder of the work

        threads[ thread_num ] = std::thread(&Conway::multiRow, this, y_start, y_end);
    }

    for (int y = 0; y < 8; y++)
    {
        threads[y].join();
    }
    
#else
    for (int y = 0; y < N; ++y) 
    {
        oneRow(y);
    }
#endif // MP
    std::cout << std::endl;
    grid = grid2;
}


int main(int argc, char* argv[])
{

    //init grid from a vector
    std::vector<char> v = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };

    //std::ofstream times("0.txt");

    int n = 20;
    //std::vector<int> n_arr = { 30, 50, 100, 150, 200, 300, 400, 500, 600, 700, 800, 1000, 1500, 2000, 2500, 3000, 5000, 10000, 15000, 20000 };
    //for (auto m:  n_arr)
    //{       
    Conway cnw(n, v);

    //std::ofstream ofile("cnw_visu.txt");
        
    //std::cout << cnw;
    //cnw.printGrid2();




    //times << m <<",";

    for (int q = 0; q < 10; ++q)
    {
        auto t1 = tmark();
        cnw.oneStep();
        auto t2 = tmark();
        std::cout << delta_time(t1, t2) << std::endl;

        std::cout << cnw;

        //std::cout << cnw;

        //times << delta_time(t1, t2) << ",";
        //ofile << cnw;
    }
    //times << std::endl;
    //}
    //ofile.close();
    //times.close();
    
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
        auto t1 = tmark();
        cnw.oneStep(2);
        auto t2 = tmark();
        std::cout << delta_time(t1, t2) << std::endl;
        // Update frame buffer
        SDL_UpdateWindowSurface(window);

    
    #endif
    #if LIMIT_RATE
        SDL_Delay(TICK_RATE);
    }
    #endif
    
    #if WINDOWING
    // Destroy window 
    SDL_DestroyWindow(window);
    // Quit SDL subsystems 
    SDL_Quit();

    system("pause");
    #endif


    return 0;
}