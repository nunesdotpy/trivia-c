#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdbool.h>

bool init();
void close();

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;
TTF_Font* gFont = NULL;
SDL_Renderer* renderer = NULL;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() == -1) {
        printf("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    // Use OpenGL 3.3 core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    gWindow = SDL_CreateWindow("Quiz Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (gWindow == NULL) {
        printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    gContext = SDL_GL_CreateContext(gWindow);
    if (gContext == NULL) {
        printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (glewInit() != GLEW_OK) {
        printf("Error initializing GLEW!\n");
        return false;
    }

    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Cria o renderizador SDL
    renderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    gFont = TTF_OpenFont("font.ttf", 28);  // Caminho atualizado para a fonte
    if (gFont == NULL) {
        printf("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError());
        return false;
    }

    return true;
}

void close() {
    TTF_CloseFont(gFont);
    gFont = NULL;

    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    SDL_DestroyWindow(gWindow);
    gWindow = NULL;

    SDL_Quit();
    TTF_Quit();
}

void renderText(const char* textureText, int x, int y) {
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText, textColor);
    if (textSurface == NULL) {
        printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == NULL) {
        printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
        SDL_FreeSurface(textSurface);
        return;
    }

    int textWidth = textSurface->w;
    int textHeight = textSurface->h;
    SDL_FreeSurface(textSurface);

    SDL_Rect renderQuad = {x, y, textWidth, textHeight};
    SDL_RenderCopy(renderer, textTexture, NULL, &renderQuad);
    SDL_DestroyTexture(textTexture);
}

typedef struct {
    char* question;
    char* options[4];
    int correctOption;
} Question;

Question questions[] = {
    {"What is the capital of France?", {"Paris", "London", "Berlin", "Madrid"}, 0},
    {"What is 2 + 2?", {"3", "4", "5", "6"}, 1},
    // Adicione mais perguntas aqui
};

int totalQuestions = sizeof(questions) / sizeof(questions[0]);
int currentQuestion = 0;
bool answerSelected = false;
int score = 0;

void renderQuestion(Question q) {
    renderText(q.question, 50, 50);
    for (int i = 0; i < 4; i++) {
        renderText(q.options[i], 50, 100 + i * 50);
    }
}

void handleKeyPress(SDL_Keycode key) {
    if (!answerSelected) {
        if (key >= SDLK_1 && key <= SDLK_4) {
            int selectedOption = key - SDLK_1;
            if (selectedOption == questions[currentQuestion].correctOption) {
                printf("Correct!\n");
                score += 100;
            } else {
                printf("Wrong!\n");
            }
            answerSelected = true;
        }
    } else {
        currentQuestion++;
        if (currentQuestion >= totalQuestions) {
            printf("Quiz completed! Final score: %d\n", score);
            currentQuestion = 0;
            score = 0;
        }
        answerSelected = false;
    }
}

void handleMouseClick(int x, int y) {
    if (!answerSelected) {
        for (int i = 0; i < 4; i++) {
            if (x > 50 && x < 250 && y > 100 + i * 50 && y < 100 + (i + 1) * 50) {
                if (i == questions[currentQuestion].correctOption) {
                    printf("Correct!\n");
                    score += 100;
                } else {
                    printf("Wrong!\n");
                }
                answerSelected = true;
                break;
            }
        }
    } else {
        currentQuestion++;
        if (currentQuestion >= totalQuestions) {
            printf("Quiz completed! Final score: %d\n", score);
            currentQuestion = 0;
            score = 0;
        }
        answerSelected = false;
    }
}

int main(int argc, char* args[]) {
    if (!init()) {
        printf("Failed to initialize!\n");
        return -1;
    }

    bool quit = false;
    SDL_Event e;

    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_KEYDOWN) {
                handleKeyPress(e.key.keysym.sym);
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                handleMouseClick(x, y);
            }
        }

        // Limpar a tela
        SDL_SetRenderDrawColor(renderer, 0, 0, 0,
        255);
        SDL_RenderClear(renderer);

    renderQuestion(questions[currentQuestion]);

    // Atualizar a tela
    SDL_RenderPresent(renderer);
}
        return 0;
    }
