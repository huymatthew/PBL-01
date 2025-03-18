#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <math.h>
#include "language.cpp"
#include <string.h>

using namespace std;
// Settings
const int SCREEN_WIDTH = 1382;
const int SCREEN_HEIGHT = 777;
const int SCREEN_FPS = 60;
bool isFullScreen = false;
int animationTime = 30; // Animation Observer
int animationSpeed = 12; // Animation Speed
SDL_Color backgroundColor = {40, 50 ,70, 255};

char pin[] = "123456";
char inp[] = "";
int maxInput = 6;
// Database
struct Account {
    int bankID;
    char accountNumber[13];
    char pin[7];
    char name[50];
    double balance;
};
Account accounts[1000];
int countAccount = 0;
Account* currentAccount;
//
SDL_Event e;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* customCursor;
void (*nextSession)(SDL_Renderer*) = nullptr;
bool quit = false;
void toggleFullScreen() {
    if (isFullScreen) {
        SDL_SetWindowFullscreen(window, 0);
        isFullScreen = false;
    } else {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
        isFullScreen = true;
    }
}
bool SetCurrentAccount(char accountNumber[]){
    for (int i = 0; i < countAccount; i++){
        if (strcmp(accounts[i].accountNumber, accountNumber) == 0){
            currentAccount = &accounts[i];
            strcpy(pin, accounts[i].pin);
            return true;
        }
    }
    return false;
}

string exampleString = "123";
// Font
TTF_Font* FONT_HEADER;
TTF_Font* FONT_BODY;
TTF_Font* FONT_TEXT;
TTF_Font* FONT_DETAIL;
TTF_Font* FONT_PINs;
std::map<int, std::string> text = getLanguage(1);
// Buttons
struct Button {
    Button(){}
    Button(SDL_Rect r, int id) : rect(r), isHovered(false), id(id){}
    int id;
    SDL_Rect rect;
    bool isHovered;
};
Button sideButtonsLeft[4];
Button sideButtonsRight[4];
Button keypadButtons[12];
Button buttons[20];
Button *selectedButton = nullptr;
// Function Prototypes
void loadAccounts();
bool pinCheck();

int window_init();
int eventHandler();
void drawMain(SDL_Renderer* renderer, int actions[], std::string bgPath);
void button_init();
bool isMouseInside(SDL_Rect rect, int mouseX, int mouseY);

void drawText(SDL_Renderer* renderer, std::string text, TTF_Font* font, SDL_Rect rect, SDL_Color color);
void drawImage(SDL_Renderer* renderer, std::string path, SDL_Rect rect);
void drawPINs(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect rect, SDL_Color color);
int actionProgess(SDL_Renderer* renderer, int id, int pos, int t);
void drawCursor(SDL_Renderer* renderer);
void continueSession();
void setNextSession(void (*session)(SDL_Renderer*));
void startInput(int length);

void SECTION_login(SDL_Renderer* renderer);
bool SECTION_pins(SDL_Renderer* renderer);
void SECTION_menu(SDL_Renderer* renderer);
void SECTION_language(SDL_Renderer* renderer);
void SECTION_changepins(SDL_Renderer* renderer);
void SECTION_account(SDL_Renderer* renderer);
double SECTION_number(SDL_Renderer* renderer);
double SECTION_money_transfer(SDL_Renderer* renderer);
void SECTION_transfer(SDL_Renderer* renderer);
void SECTION_INITIATION(SDL_Renderer* renderer);
void SECTION_LOADDATA(SDL_Renderer* renderer);

// Function Definitions
void loadAccounts(){
    FILE* file = fopen("database/user.csv", "r");
    if (file == nullptr){
        std::cerr << "Cannot open file user.csv" << std::endl;
        return;
    }
    int i = 0;
    while (!feof(file)){
        if (fscanf(file, "%d %s %s %s %lf", &accounts[i].bankID, accounts[i].accountNumber, accounts[i].pin, accounts[i].name, &accounts[i].balance) == 5) {
            i++;
        } else {
            std::cerr << "Error reading account data" << std::endl;
            break;
        }
    }
    for (int j = 0; j < i; j++){
        std::cout << accounts[j].bankID << " " << accounts[j].accountNumber << " " << accounts[j].pin << " " << accounts[j].name << " " << accounts[j].balance << std::endl;
    }
    countAccount = i;
    fclose(file);
}
void saveAccounts(){
    FILE* file = fopen("database/user.csv", "w");
    if (file == nullptr){
        std::cerr << "Cannot open file user.csv" << std::endl;
        return;
    }
    for (int i = 0; i < 1000; i++){
        if (strlen(accounts[i].pin) == 0) break;
        fprintf(file, "%d %s %s %s %lf\n", accounts[i].bankID, accounts[i].accountNumber, accounts[i].pin, accounts[i].name, accounts[i].balance);
    }
    fclose(file);
}
bool pinCheck(){
    if (strlen(inp) < 6) return false;
    for (int i = 0; i < 6; i++){
        if (inp[i] != pin[i]) return false;
    }
    return true;
}
void startInput(int length){
    memset(inp, 0, strlen(inp));
    maxInput = length;
}
void continueSession(){
    if (nextSession != nullptr){
        animationTime = 30;
        quit = false;
        eventHandler();
        nextSession(renderer);
    }
}
void setNextSession(void (*session)(SDL_Renderer*)){
    nextSession = session;
    quit = true;
}
int actionProgess(SDL_Renderer* renderer, int id, int pos, int t){
    if (id == 0) return 0;
    // Position of ACtion Button (x1, y2, x2, y2 ...)
    int positions[16] = {321, 301, 321, 342, 321, 383, 321, 424, 518, 301, 518, 342, 518, 383, 518, 424};

    int positionX = positions[pos * 2];
    int positionY = positions[pos * 2 + 1];

    // Split Action Button in Left and Righyt
    if (positionX == 321){
        drawImage(renderer, "assets/images/action1.png", {positionX - t, positionY, 200, 20});
    }
    else{
        drawImage(renderer, "assets/images/action2.png", {positionX + t, positionY, 200, 20});
    }
    // Animation is not done yet
    if (t > 0) return 0;
    // Define Type of Action (start with 5: error, 6: success, 7: warning, other normal)
    if (id / 1000 == 5) {
        drawText(renderer, text[id], FONT_BODY, {positionX + 40, positionY + 8, 200, 20}, {255, 0, 0, 255});
        drawImage(renderer, "assets/images/icon1.png", {positionX + 15, positionY + 8, 10, 10});
    }
    else if (id / 1000 == 6) {
        drawText(renderer, text[id], FONT_BODY, {positionX + 40, positionY + 8, 200, 20}, {0, 200, 0, 255});
        drawImage(renderer, "assets/images/icon2.png", {positionX + 15, positionY + 8, 10, 10});
    }
    else if (id / 1000 == 7) {
        drawText(renderer, text[id], FONT_BODY, {positionX + 40, positionY + 8, 200, 20}, {200, 200, 0, 255});
        drawImage(renderer, "assets/images/icon3.png", {positionX + 15, positionY + 8, 10, 10});
    }
    else drawText(renderer, text[id], FONT_BODY, {positionX + 5, positionY + 8, 200, 20}, {0, 0, 0, 255});
    return 0;
}

int min(int a, int b){
    return a < b ? a : b;
}
void drawPINs(SDL_Renderer* renderer, TTF_Font* font, SDL_Rect rect, SDL_Color color = {50, 240, 50, 255}){
    for (int i = 0; i < min(strlen(inp), 6); i++){
        drawText(renderer, "*", font, {rect.x + i * 20, rect.y, rect.w, rect.h}, color);
    }
    for (int i = 0; i < 6; i++){
        drawText(renderer, "_", font, {rect.x + i * 20, rect.y - 8, rect.w, rect.h}, color);
    }
}
void drawImage(SDL_Renderer* renderer, std::string path, SDL_Rect rect) {
    SDL_Surface* imageSurface = IMG_Load(path.c_str());
    if (imageSurface == nullptr) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        quit = true;
        return;
    }
    SDL_Texture* imageTexture = SDL_CreateTextureFromSurface(renderer, imageSurface);
    if (imageTexture == nullptr) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        quit = true;
        return;
    }
    SDL_Rect r = {rect.x, rect.y, imageSurface->w, imageSurface->h};
    SDL_RenderCopy(renderer, imageTexture, NULL, &r);
    SDL_DestroyTexture(imageTexture);
    SDL_FreeSurface(imageSurface);
}
void drawText(SDL_Renderer* renderer, std::string text, TTF_Font* font, SDL_Rect rect, SDL_Color color) {
    if (text == "") return;
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (textSurface == nullptr) {
        std::cerr << "TTF_RenderText_Solid Error: " << TTF_GetError() << std::endl;
        quit = true;
        return;
    }
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        quit = true;
        return;
    }
    SDL_Rect textRect = {rect.x, rect.y, textSurface->w, textSurface->h};
    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_DestroyTexture(textTexture);
}
void drawCursor(SDL_Renderer* renderer){
    int x, y;
    SDL_GetMouseState(&x, &y);
    SDL_Rect cursorRect = {x, y, 47, 51};
    SDL_RenderCopy(renderer, customCursor, NULL, &cursorRect);
}
void drawProgressCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, int width, float progress) {
    int startAngle = (progress - 0.1) * 360;
    int endAngle = progress * 360;

    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
    for (int angle = 0; angle <= 360; angle++) {
        float radians = angle * M_PI / 180.0;
        for (int w = 0; w < width; w++) {
            int x = centerX + (radius + w) * cos(radians);
            int y = centerY + (radius + w) * sin(radians);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    for (int angle = startAngle; angle <= endAngle; angle++) {
        float radians = angle * M_PI / 180.0;
        for (int w = 0; w < width; w++) {
            int x = centerX + (radius + w) * cos(radians);
            int y = centerY + (radius + w) * sin(radians);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}
// Function to check if the mouse is inside a rectangle
bool isMouseInside(SDL_Rect rect, int mouseX, int mouseY) {
    return (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
            mouseY >= rect.y && mouseY <= rect.y + rect.h);
}
void button_init(){
    Button tempSideButtonsLeft[] = {
        Button({264, 304, 42, 30}, 11),
        Button({262, 345, 42, 30}, 12),
        Button({260, 386, 42, 30}, 13),
        Button({258, 427, 42, 30}, 14)
    };

    Button tempSideButtonsRight[] = {
        Button({713, 304, 42, 30}, 15),
        Button({715, 345, 42, 30}, 16),
        Button({717, 386, 42, 30}, 17),
        Button({719, 427, 42, 30}, 18)
    };
    int anchorX = 414;
    int anchorY = 544;
    int sizeX = 60;
    int sizeY = 41;
    int spacingX = 2;
    int spacingY = 1;
    Button tempKeypadButtons[] = {
        Button({anchorX, anchorY, sizeX, sizeY}, 1), // 1
        Button({anchorX + sizeX + spacingX, anchorY, sizeX, sizeY}, 2), // 2
        Button({anchorX + 2 * (sizeX + spacingX), anchorY, sizeX, sizeY}, 3), // 3
        Button({anchorX, anchorY + sizeY + spacingY, sizeX, sizeY}, 4), // 4
        Button({anchorX + sizeX + spacingX, anchorY + sizeY + spacingY, sizeX, sizeY}, 5), // 5
        Button({anchorX + 2 * (sizeX + spacingX), anchorY + sizeY + spacingY, sizeX, sizeY}, 6), // 6
        Button({anchorX, anchorY + 2 * (sizeY + spacingY), sizeX, sizeY}, 7), // 7
        Button({anchorX + sizeX + spacingX, anchorY + 2 * (sizeY + spacingY), sizeX, sizeY}, 8), // 8
        Button({anchorX + 2 * (sizeX + spacingX), anchorY + 2 * (sizeY + spacingY), sizeX, sizeY}, 9), // 9
        Button({anchorX, anchorY + 3 * (sizeY + spacingY), sizeX, sizeY}, 101), // *
        Button({anchorX + sizeX + spacingX, anchorY + 3 * (sizeY + spacingY), sizeX, sizeY}, 10), // 0
        Button({anchorX + 2 * (sizeX + spacingX), anchorY + 3 * (sizeY + spacingY), sizeX, sizeY}, 102)  // #
    };
    int index = 0;

    for (int i = 0; i < sizeof(tempSideButtonsLeft) / sizeof(Button); i++) {
        sideButtonsLeft[i] = tempSideButtonsLeft[i];
        buttons[index++] = sideButtonsLeft[i];
    }

    for (int i = 0; i < sizeof(tempSideButtonsRight) / sizeof(Button); i++) {
        sideButtonsRight[i] = tempSideButtonsRight[i];
        buttons[index++] = sideButtonsRight[i];
    }

    for (int i = 0; i < sizeof(tempKeypadButtons) / sizeof(Button); i++) {
        keypadButtons[i] = tempKeypadButtons[i];
        buttons[index++] = keypadButtons[i];
    }
}
int window_init(){
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    window = SDL_CreateWindow("Automatic Teller Machine", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr) {
        SDL_DestroyWindow(window);
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    FONT_HEADER = TTF_OpenFont("assets/fonts/arialbd.ttf", 24);
    FONT_BODY = TTF_OpenFont("assets/fonts/arial.ttf", 16);
    FONT_TEXT = TTF_OpenFont("assets/fonts/arial.ttf", 8);
    FONT_DETAIL = TTF_OpenFont("assets/fonts/arial.ttf", 12);
    FONT_PINs = TTF_OpenFont("assets/fonts/arial.ttf", 32);

    SDL_Surface* cur = IMG_Load("assets/images/cursor.png");
    if (cur == nullptr) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        quit = true;
        return 1;
    }
    customCursor = SDL_CreateTextureFromSurface(renderer, cur);
    if (customCursor == nullptr) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        quit = true;
        return 1;
    }
    
    SDL_FreeSurface(cur);
    return 0;
}
int eventHandler() {
    int x, y, actionInvoke = -1;
    SDL_GetMouseState(&x, &y);
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
        if (e.type == SDL_MOUSEMOTION) {
            for (int i = 0; i < 20; i++) {
                if (isMouseInside(buttons[i].rect, x, y)) {
                    buttons[i].isHovered = true;
                } else {
                    buttons[i].isHovered = false;
                }
            }
        }
        if (e.type == SDL_MOUSEBUTTONDOWN) {
            for (int i = 0; i < 20; i++) {
                if (isMouseInside(buttons[i].rect, x, y)) {
                    selectedButton = &buttons[i];
                    break;
                }
            }
        }
        if (e.type == SDL_MOUSEBUTTONUP && selectedButton != nullptr) {
            if (isMouseInside(selectedButton->rect, x, y)){
                // Action Input
                if (selectedButton->id >= 11 && selectedButton->id <= 18 && animationTime == 0){
                    actionInvoke = selectedButton->id % 10;
                }
                // Text Input
                if (maxInput > 0 && strlen(inp) < maxInput){
                    if (selectedButton->id == 10){
                        inp[strlen(inp)] = '0';
                    }
                    else if (selectedButton->id > 0 && selectedButton->id < 10){
                        inp[strlen(inp)] = selectedButton->id + '0';
                    }
                }
            }
            selectedButton = nullptr;
        }
        if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_F11:
                    toggleFullScreen();
                    break;
                case SDLK_RETURN:
                    actionInvoke = 8;
                    break;
                case SDLK_BACKSPACE:
                    actionInvoke = 4;
                    break;
                default:
                    break;
            }
            if (e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9) {
                if (maxInput > 0 && strlen(inp) < maxInput) {
                    inp[strlen(inp)] = e.key.keysym.sym;
                }
            }
        }
    }
    return actionInvoke;
}
void drawMain(SDL_Renderer* renderer, int actions[], std::string bgPath = "") {
    nextSession = nullptr;
    //animation
    if (animationTime > 0) animationTime--;
    // Background
    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderClear(renderer);
    if (bgPath != "") {
        drawImage(renderer, bgPath, {322, 183, 383, 286});
    }
    for (int i = 0; i < 8; i++){
        actionProgess(renderer, actions[i], i, animationTime * animationSpeed);
    }

    // Load the background image
    SDL_Surface* bgSurface = IMG_Load("assets/images/atm2.png");
    if (bgSurface == nullptr) {
        std::cerr << "IMG_Load Error: " << IMG_GetError() << std::endl;
        quit = true;
        return;
    }

    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface);
    if (bgTexture == nullptr) {
        std::cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
        quit = true;
        return;
    }

    // Render the background image
    SDL_RenderCopy(renderer, bgTexture, NULL, NULL);
    SDL_DestroyTexture(bgTexture);

    // Render the buttons
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(bgTexture, SDL_BLENDMODE_BLEND);
    for (int i = 0; i < 20; i++) {
        if (buttons[i].isHovered) {
            if (selectedButton != nullptr && selectedButton->id == buttons[i].id) {
                SDL_SetRenderDrawColor(renderer, 50, 50, 50, 100);
            } else {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 100);
            }
        }
        else{
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        }
        SDL_RenderFillRect(renderer, &buttons[i].rect);
    }
    drawCursor(renderer);
}
void SECTION_login(SDL_Renderer* renderer){
    int actions[8] = { 0, 0, 0, 7000, 
                       0, 0, 0, 6002 };
    startInput(12);
    while (!quit){
        drawMain(renderer, actions);
        for (int i = 0; i < strlen(inp); i++) {
            drawText(renderer, std::string(1, inp[i]), FONT_HEADER, {350 + i * 20, 250, 383, 30}, {200, 200, 200, 255});
        }
        for (int i = 0; i < 12; i++){
            drawText(renderer, "_", FONT_HEADER, {350 + i * 20, 250, 383, 30}, {200, 200, 200, 255});
        }
        int action = eventHandler();
        if (action == 4){
            if (strlen(inp) > 0) inp[strlen(inp) - 1] = 0;
        }
        else if (action == 8){
            if (SetCurrentAccount(inp)){
                if (SECTION_pins(renderer)){
                    setNextSession(SECTION_language);
                }
                else{
                    actions[2] = 5100;
                    startInput(12);
                }
            }
            else{
                actions[2] = 5102;
                startInput(12);
            }
        }
        drawText(renderer, text[1001], FONT_HEADER, {350, 215, 383, 30}, {255, 255, 255, 255});
        SDL_RenderPresent(renderer);
    }
    continueSession();
}
void SECTION_language(SDL_Renderer* renderer){
    int actions[8] = { 0, 0, 0, 0, 
                       0, 1, 2, 3 };
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, "Please Select Language", FONT_HEADER, {350, 215, 383, 30}, {255, 255, 255, 255});
        int action = eventHandler();
        if (action >= 6 && action <= 8){
            text = getLanguage(action - 5);
            setNextSession(SECTION_menu);
        }
        SDL_RenderPresent(renderer);
    }
    continueSession();
}
bool SECTION_pins(SDL_Renderer* renderer){
    bool error = false;
    animationTime = 30;
    int actions[8] = { 0, 0, 0, 7000, 
                       0, 0, 5000, 6000 };
    startInput(6);
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, text[1002], FONT_HEADER, {350, 215, 383, 30}, {255, 255, 255, 255});
        drawPINs(renderer, FONT_PINs, {350, 270, 383, 30});
        if (error) drawText(renderer, text[5100], FONT_BODY, {350, 325, 383, 30}, {255, 0, 0, 255});
        // Event Handler
        int action = eventHandler();
        if (action == 4){
            if (strlen(inp) > 0) inp[min(strlen(inp), 6) - 1] = 0;
        }
        else if (action == 7) break; 
        else if (action == 8){
            if (pinCheck()){
                return true;
            }
            else{
                error = true;
                startInput(6);
            }
            
        }
        SDL_RenderPresent(renderer);
    }
    return false;
}
double SECTION_number(SDL_Renderer* renderer){
    double number = 0;
    int actions[8] = { 0, 0, 0, 7000, 
                       0, 0, 5000, 6000 };
    startInput(12);
    while (!quit){
        drawMain(renderer, actions);
        for (int i = 0; i < strlen(inp); i++) {
            drawText(renderer, std::string(1, inp[i]), FONT_HEADER, {350 + i * 20, 250, 383, 30}, {200, 200, 200, 255});
        }
        for (int i = 0; i < 12; i++){
            drawText(renderer, "_", FONT_HEADER, {350 + i * 20, 250, 383, 30}, {200, 200, 200, 255});
        }
        int action = eventHandler();
        if (action == 4){
            if (strlen(inp) > 0) inp[strlen(inp) - 1] = 0;
        }
        else if (action == 8){
            number = atof(inp);
            break;
        }
        SDL_RenderPresent(renderer);
    }
    return number;
}
double SECTION_money_transfer(SDL_Renderer* renderer){
    int actions[8] = { 100, 101, 102, 5001, 
                       103, 104, 105, 1005 };
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, {322, 183, 383, 30}, {255, 255, 255, 255});
        int action = eventHandler();
        if (action == 4) return 0;
        else if (action == 8){
            double amount = SECTION_number(renderer);
            if (amount > 1000) return amount;
        }
        else if (action == 1) return 100000;
        else if (action == 2) return 200000;
        else if (action == 3) return 500000;
        else if (action == 5) return 1000000;
        else if (action == 6) return 20000000;
        else if (action == 7) return 50000000;
        SDL_RenderPresent(renderer);
    }
    return 0;
}
void SECTION_menu(SDL_Renderer* renderer){
    int actions[8] = { 0, 1010, 1020, 5001, 
                       0, 1030, 1040, 0 };
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, {350, 215, 383, 30}, {255, 255, 255, 255});
        drawText(renderer, currentAccount->name, FONT_BODY, {350, 250, 383, 30}, {255, 255, 255, 255});
        int action = eventHandler();
        if (action == 4) setNextSession(SECTION_login);
        if (action == 2) setNextSession(SECTION_account);
        if (action == 7) setNextSession(SECTION_changepins);
        
        SDL_RenderPresent(renderer);
    }
    continueSession();
}
void SECTION_changepins(SDL_Renderer* renderer){
    char newPin[7];
    int currentState = 0;
    int actions[8] = { 0, 0, 0, 7000, 
                        0, 0, 5000, 6000 };
    if (!SECTION_pins(renderer)){
        quit = true;
        setNextSession(SECTION_menu);
    }
    startInput(6);

    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, text[1003 + currentState], FONT_HEADER, {350, 215, 383, 30}, {255, 255, 255, 255});
        drawPINs(renderer, FONT_PINs, {350, 270, 383, 30});

        int action = eventHandler();
        if (action == 4){
            if (strlen(inp) > 0) inp[min(strlen(inp), 6) - 1] = 0;
        }
        else if (action == 7) break; 
        else if (action == 8){
            if (currentState == 0){
                strcpy(newPin, inp);
                startInput(6);
                currentState = 1;
            }
            else{
                if (strcmp(newPin, inp) == 0){
                    strcpy(currentAccount->pin, inp);
                    strcpy(pin, inp);
                    actions[2] = 6001;
                    saveAccounts();
                    setNextSession(SECTION_menu);
                }
                else{
                    actions[2] = 5101;
                    startInput(6);
                }
            }
        }
        SDL_RenderPresent(renderer);
    }
    continueSession();
}
void SECTION_account(SDL_Renderer* renderer){
    int actions[8] = { 0, 1010, 1020, 5001, 
                       0, 1030, 1040, 0 };
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, {322, 183, 383, 30}, {255, 255, 255, 255});
        int action = eventHandler();
        if (action == 4) setNextSession(SECTION_login);
        SDL_RenderPresent(renderer);
    }
    continueSession();
}
void SECTION_transfer(SDL_Renderer* renderer){
    int actions[8] = { 0, 1006, 0, 5001, 
                       0, 0, 0, 6003 };
    double amount = 0;
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, {322, 183, 383, 30}, {255, 255, 255, 255});
        int action = eventHandler();
        if (action == 2) amount = SECTION_money_transfer(renderer);
        if (action == 4) setNextSession(SECTION_login);
        SDL_RenderPresent(renderer);
    }
    continueSession();
}
void SECTION_INITIATION(SDL_Renderer* renderer){
    bool error = false;
    animationTime = 30;
    int actions[8] = { 0, 0, 0, 7000, 
                       0, 0, 5000, 6000 };
    startInput(6);
    int t = 0;
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, "INITIATION ATM STATE", FONT_HEADER, {350, 215, 383, 30}, {255, 255, 255, 255});
        drawText(renderer, "Please Enter The Security Codes", FONT_BODY, {350, 240, 383, 30}, {255, 255, 255, 255});
        drawPINs(renderer, FONT_PINs, {375, 275, 383, 30}, {165, 50, 50, 255});
        if (error) drawText(renderer, text[5100], FONT_BODY, {322, 275, 383, 30}, {255, 0, 0, 255});
        // Event Handler
        int action = eventHandler();
        if (action == 4){
            if (strlen(inp) > 0) inp[min(strlen(inp), 6) - 1] = 0;
        }
        else if (action == 7) break; 
        else if (action == 8){
            if (pinCheck()){
                setNextSession(SECTION_LOADDATA);
            }
            else{
                error = true;
                startInput(6);
            }
        }
        continueSession();
        SDL_RenderPresent(renderer);
    }
}
void SECTION_LOADDATA(SDL_Renderer* renderer){
    int actions[8] = { 0, 0, 10, 0, 
                       0, 0, 11, 12};
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, "Administrator Screen", FONT_HEADER, {350, 215, 383, 30}, {255, 255, 255, 255});
        int action = eventHandler();
        if (action == 7) saveAccounts();
        if (action == 3) loadAccounts();
        if (action == 8) setNextSession(SECTION_login);
        SDL_RenderPresent(renderer);
    }
    continueSession();
}
/*void SECTION_menu(SDL_Renderer* renderer){
    int actions[8] = { 0, 1010, 1020, 5001, 
                       0, 1030, 1040, 0 };
    while (!quit){
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, {322, 183, 383, 30}, {255, 255, 255, 255});
        int action = eventHandler();
        if (action == 4) setNextSession(SECTION_login);
        SDL_RenderPresent(renderer);
    }
    continueSession();
}*/


int main(int argc, char* argv[]) {
    std::cout << "----------------------------------------" << std::endl;
    if (window_init() == 1) return 1;
    button_init();
    SDL_ShowCursor(SDL_DISABLE);
    SECTION_INITIATION(renderer);
    printf("-- ATM STOP --");
    SDL_DestroyTexture(customCursor);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    SDL_Quit();

    return 0;
}