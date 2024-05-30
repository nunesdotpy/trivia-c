#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>
#include <stdio.h>
#include <stdbool.h>

bool init();
void close();
void handleMouseClick(int x, int y);

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
    {"Qual e a capital do Brasil?", {"Brasilia", "Rio de Janeiro", "Sao Paulo", "Belo Horizonte"}, 0},
    {"Qual e o maior oceano do mundo?", {"Oceano Atlantico", "Oceano indico", "Oceano Pacifico", "Mar Mediterraneo"}, 2},
    {"Quem escreveu 'Dom Quixote'?", {"Miguel de Cervantes", "William Shakespeare", "Franz Kafka", "Leo Tolstoy"}, 0},
    {"Qual e o pais com a maior area territorial do mundo?", {"Estados Unidos", "China", "Russia", "Brasil"}, 2},
    {"Qual e o elemento mais abundante na crosta terrestre?", {"Oxigênio", "Silicio", "Ferro", "Aluminio"}, 1},
    {"Qual e a montanha mais alta do mundo?", {"Monte Kilimanjaro", "Monte Everest", "Monte McKinley", "Monte Aconcágua"}, 1},
    {"Qual e o nome do satelite natural da Terra?", {"Europa", "Tita", "Lua", "Ganimedes"}, 2},
    {"Quem pintou a 'Mona Lisa'?", {"Vincent van Gogh", "Leonardo da Vinci", "Pablo Picasso", "Michelangelo"}, 1},
    {"Qual e a capital do Canada?", {"Toronto", "Ottawa", "Montreal", "Vancouver"}, 1},
    {"Quem e conhecido como o 'Rei do Pop'?", {"Elvis Presley", "Michael Jackson", "Prince", "Madonna"}, 1},
};

int totalQuestions = sizeof(questions) / sizeof(questions[0]);
int currentQuestion = 0;
bool answerSelected = false;
int score = 0;
bool showFeedback = false; // Variavel para controlar se o feedback deve ser mostrado
bool correctAnswer = false; // Variavel para armazenar se a resposta selecionada pelo jogador foi correta

void renderQuestion(Question q) {
    renderText(q.question, 50, 50);
    for (int i = 0; i < 4; i++) {
        renderText(q.options[i], 50, 100 + i * 50);
    }
}

// Adicione estas variaveis globais para definir as dimens�es e a posi��o do bot�o
const int BUTTON_WIDTH = 200;
const int BUTTON_HEIGHT = 50;
const int BUTTON_X = 300;
const int BUTTON_Y = 500;

void renderButton() {
    // Desenha o botao na tela
    SDL_Rect buttonRect = {BUTTON_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &buttonRect);

    // Renderiza o texto "Next" no centro do bot�o
    renderText("Next", BUTTON_X + BUTTON_WIDTH / 2 - 20, BUTTON_Y + BUTTON_HEIGHT / 2 - 10);
}

bool isInsideButton(int x, int y) {
    // Verifica se as coordenadas (x, y) est�o dentro dos limites do bot�o
    return (x >= BUTTON_X && x <= BUTTON_X + BUTTON_WIDTH && y >= BUTTON_Y && y <= BUTTON_Y + BUTTON_HEIGHT);
}

void handleMouseClick(int x, int y) {
    if (!answerSelected) {
        for (int i = 0; i < 4; i++) {
            if (x > 50 && x < 250 && y > 100 + i * 50 && y < 100 + (i + 1) * 50) {
                if (i == questions[currentQuestion].correctOption) {
                    printf("Correct!\n");
                    score += 100;
                    showFeedback = true; // Mostrar feedback apenas se uma resposta foi selecionada
                    correctAnswer = true; // Indica que a resposta foi correta
                } else {
                    printf("Wrong!\n");
                    showFeedback = true; // Mostrar feedback apenas se uma resposta foi selecionada
                    correctAnswer = false; // Indica que a resposta foi incorreta
                }
                answerSelected = true;
                break;
            }
        }
    } else {
        if (isInsideButton(x, y)) {
            currentQuestion++;
            if (currentQuestion >= totalQuestions) {
                printf("Quiz completed! Final score: %d\n", score);
                currentQuestion = 0;
                score = 0;
            }
            answerSelected = false;
            showFeedback = false; // Reinicia a vari�vel de controle do feedback
        }
    }
}

void renderFeedback() {
    if (showFeedback) {
        if (correctAnswer) {
            renderText("Correct!", 50, 300);
        } else {
            renderText("Wrong!", 50, 300);
        }
    }
}

void renderScore() {
    char scoreText[50];
    sprintf(scoreText, "Score: %d", score);
    renderText(scoreText, 50, 20);
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
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                handleMouseClick(x, y);
            }
        }

        // Limpar a tela
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        renderScore();
        renderQuestion(questions[currentQuestion]);
        renderButton();
        renderFeedback(); // Renderiza o feedback se showFeedback for verdadeiro

        // Atualizar a tela
        SDL_RenderPresent(renderer);
    }

    close();
    return 0;
}
