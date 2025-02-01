#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <vector>
#include <random>
#include <ctime>

// Settings
constexpr Uint32 WindowWidth{1600};
constexpr Uint32 WindowHeight{900};

constexpr Uint8 CellSize{2};
constexpr Uint32 Columns{WindowWidth / CellSize};
constexpr Uint32 Rows{WindowHeight / CellSize};

constexpr Uint8 FPS{12};
constexpr Uint32 FrameDuration{1000 / FPS};

// Application holds the window, renderer, and running condition
struct Application
{
	SDL_Window *window{nullptr};
	SDL_Renderer *renderer{nullptr};
	SDL_AppResult status{SDL_APP_CONTINUE};
	bool runSimulation{true};
};

// Grid for cells (initialized based on settings values)
std::vector<std::vector<Uint8>> Grid{Rows, std::vector<Uint8>(Columns, 0)};

// Stores the new grid values.
std::vector<std::vector<Uint8>> NewGrid{Rows, std::vector<Uint8>(Columns, 0)};

// Sets up a random grid of alive/dead cells. (0 == dead, 1 == alive)
void initGrid(std::vector<std::vector<Uint8>> &grid)
{
	for (int r = 0; r < Rows; ++r)
	{
		for (int c = 0; c < Columns; ++c)
		{
			grid[r][c] = ((std::rand() % 100) < 10) ? 1 : 0;
		}
	}
}

// Draws the cells in the grid
void drawGrid(SDL_Renderer *renderer, const std::vector<std::vector<Uint8>> &grid)
{
	// Green cells
	SDL_SetRenderDrawColor(renderer, 0, 200, 0, SDL_ALPHA_OPAQUE);
	SDL_FRect cell;

	for (int r = 0; r < Rows; ++r)
	{
		for (int c = 0; c < Columns; ++c)
		{
			// If the cell is alive, draw it
			if (grid[r][c])
			{
				cell.x = c * CellSize;
				cell.y = r * CellSize;
				cell.w = CellSize;
				cell.h = CellSize;
				SDL_RenderFillRect(renderer, &cell);
			}
		}
	}
}

// Returns the number of neighboring ALIVE cells
Uint8 getNeighborCount(const std::vector<std::vector<Uint8>> &grid, int row, int column)
{
	Uint8 count = 0;
	
	// Check all neighbors
	for (int r = -1; r <= 1; ++r)
	{
		for (int c = -1; c <= 1; ++c)
		{
			// Dont check self
			if (r == 0 && c == 0)
			{
				continue;
			}
			else
			{
				// Rows/Colums wrap (toroidal)
				int destRow = (row + r + Rows) % Rows;
				int destColumn = (column + c + Columns) % Columns;

				if (grid[destRow][destColumn])
				{
					++count;
				}
			}
		}
	}
	return count;
}

// Check all cell neighbors and each cell accordingly
void updateGrid(std::vector<std::vector<Uint8>> &grid, std::vector<std::vector<Uint8>> &newGrid)
{
	for (int r = 0; r < Rows; ++r)
	{
		for (int c = 0; c < Columns; ++c)
		{
			int neighborCount = getNeighborCount(grid, r, c);

			if (grid[r][c])
			{
				// Rule 1 and 2: Death on Underpopulation (less than 2 neigbors)/Overpopulation (more than 3 neighbors)
				if (neighborCount < 2 || neighborCount > 3)
				{
					newGrid[r][c] = 0;
				}
				else // Survive otherwise
				{
					newGrid[r][c] = 1;
				}
			}
			else
			{
				// Rule 3: Reproduce if 3 neighbors
				if (neighborCount == 3)
				{
					newGrid[r][c] = 1;
				}
				else // Death otherwise
				{
					newGrid[r][c] = 0;
				}
			}
		}
	}
	grid = newGrid;
}

// Runs once at startup
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
	// Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_Log("Failed to initialize SDL with error: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Create Window with Renderer
	SDL_Window *window;
	SDL_Renderer *renderer;

	if (!SDL_CreateWindowAndRenderer("SDL3 Game Of Life", WindowWidth, WindowHeight, 0, &window, &renderer))
	{
		SDL_Log("Failed to initialize SDL Window or Renderer with error: %s", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	// Set up application data
	*appstate = new Application{
		.window = window,
		.renderer = renderer,
		};

	// Random seed for initializing grid cells
	std::srand(std::time(nullptr));

	// Randomize the grid a bit
	initGrid(Grid);

	return SDL_APP_CONTINUE;
}

// Runs when a new event (keyboard/mouse input, controllers, etc.)
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
	Application *app = (Application *)appstate;

	// Window "X" event shuts down the app
	if (event->type == SDL_EVENT_QUIT)
	{
		app->status = SDL_APP_SUCCESS;
	}
	else if (event->type == SDL_EVENT_KEY_DOWN)
	{
		if (event->key.key == SDLK_SPACE)
		{
			app->runSimulation = !app->runSimulation;
		} 
		else if (event->key.key == SDLK_R)
		{
			initGrid(Grid);
		}
	}

	return SDL_APP_CONTINUE;
}

// Runs once per frame while application is running
SDL_AppResult SDL_AppIterate(void *appstate)
{
	Uint64 startTime = SDL_GetTicks();

	Application *app = (Application *)appstate;

	// Set the renderer drawing color
	SDL_SetRenderDrawColorFloat(app->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

	// Clear the window to draw
	SDL_RenderClear(app->renderer);

	drawGrid(app->renderer, Grid);

	if (app->runSimulation)
	{
		updateGrid(Grid, NewGrid);
	}

	// Draw renderer to screen
	SDL_RenderPresent(app->renderer);

	// Limit FPS
	Uint64 elapsedTime = SDL_GetTicks() - startTime;
	if (elapsedTime < FrameDuration)
	{
		SDL_Delay(FrameDuration - elapsedTime);
	}
	
	return app->status;
}

// Exit the application and cleanup SDL.
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
	Application *app = (Application *)appstate;

	if (app)
	{
		SDL_DestroyRenderer(app->renderer);
		SDL_DestroyWindow(app->window);

		delete app;
	}

	SDL_Quit();
}
