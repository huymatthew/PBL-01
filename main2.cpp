#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <cmath>
#include "language.cpp"

namespace ATM {
    // Constants and Settings
    constexpr int SCREEN_WIDTH = 1382;
    constexpr int SCREEN_HEIGHT = 777;
    constexpr int SCREEN_FPS = 60;
    bool isFullScreen = false;
    int animationTime = 30;
    int animationSpeed = 12;
    SDL_Color backgroundColor = {40, 50, 70, 255};

    // Data Structures
    struct Account {
        int bankID;
        char accountNumber[13];
        char pin[7];
        char name[50];
        double balance;
    };

    struct Button {
        SDL_Rect rect;
        int id;
        bool isHovered;

        Button() : id(0), isHovered(false) {}
        Button(SDL_Rect r, int i) : rect(r), id(i), isHovered(false) {}
    };

    // Global Variables
    char pin[] = "123456";
    char inp[32] = "";
    int maxInput = 6;
    Account accounts[1000];
    int countAccount = 0;
    Account* currentAccount = nullptr;
    SDL_Event e;
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* customCursor = nullptr;
    void (*nextSession)(SDL_Renderer*) = nullptr;
    bool quit = false;
    std::map<int, std::string> text = getLanguage(1);
    Button sideButtonsLeft[4];
    Button sideButtonsRight[4];
    Button keypadButtons[12];
    Button buttons[20];
    Button* selectedButton = nullptr;
    TTF_Font* FONT_HEADER = nullptr;
    TTF_Font* FONT_MEDIUM = nullptr;
    TTF_Font* FONT_BODY = nullptr;
    TTF_Font* FONT_TEXT = nullptr;
    TTF_Font* FONT_DETAIL = nullptr;
    TTF_Font* FONT_PINs = nullptr;

    //
    // Helper function for drawing the main ATM interface
    void drawMain(SDL_Renderer* renderer, const int actions[8], 
        const std::string& bgPath = "") {
        if (animationTime > 0) animationTime--;

        SDL_SetRenderDrawColor(renderer, 
                            backgroundColor.r, 
                            backgroundColor.g, 
                            backgroundColor.b, 
                            backgroundColor.a);
        SDL_RenderClear(renderer);

        if (!bgPath.empty()) {
        drawImage(renderer, bgPath, {322, 183, 383, 286});
        }

        for (int i = 0; i < 8; i++) {
            actionProgess(renderer, actions[i], i, animationTime * animationSpeed);
        }

        drawImage(renderer, "assets/images/atm2.png", {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT});

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (int i = 0; i < 20; i++) {
        SDL_SetRenderDrawColor(renderer, 
                                buttons[i].isHovered ? 255 : 0,
                                buttons[i].isHovered ? 255 : 0,
                                buttons[i].isHovered ? 255 : 0,
                                buttons[i].isHovered ? 100 : 0);
        if (selectedButton && selectedButton->id == buttons[i].id) {
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 100);
        }
        SDL_RenderFillRect(renderer, &buttons[i].rect);
        }

        drawCursor(renderer);
        nextSession = nullptr;
        }

        void SECTION_login(SDL_Renderer* renderer) {
        int actions[8] = {0, 0, 0, 7000, 0, 0, 0, 6002};
        startInput(12);

        while (!quit) {
        drawMain(renderer, actions);
        
        // Draw account number input
        for (int i = 0; i < 12; i++) {
            drawText(renderer, i < strlen(inp) ? std::string(1, inp[i]) : "_",
                    FONT_HEADER, {350 + i * 20, 250, 383, 30}, 
                    {200, 200, 200, 255});
        }
        drawText(renderer, text[1001], FONT_HEADER, 
                {350, 215, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        switch (action) {
            case 4: // Backspace
                if (strlen(inp) > 0) inp[strlen(inp) - 1] = '\0';
                break;
            case 8: // Enter
                if (setCurrentAccount(inp)) {
                    if (SECTION_pins(renderer)) {
                        setNextSession(SECTION_language);
                    } else {
                        actions[2] = 5100;
                        startInput(12);
                    }
                } else {
                    actions[2] = 5102;
                    startInput(12);
                }
                break;
        }
        SDL_RenderPresent(renderer);
        }
        continueSession();
        }

        bool SECTION_pins(SDL_Renderer* renderer) {
        bool error = false;
        animationTime = 30;
        int actions[8] = {0, 0, 0, 7000, 0, 0, 5000, 6000};
        startInput(6);

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, text[1002], FONT_HEADER, 
                {350, 215, 383, 30}, {255, 255, 255, 255});
        drawPINs(renderer, FONT_PINs, {350, 270, 383, 30});
        if (error) {
            drawText(renderer, text[5100], FONT_BODY, 
                    {350, 325, 383, 30}, {255, 0, 0, 255});
        }

        int action = handleEvents();
        switch (action) {
            case 4: // Backspace
                if (strlen(inp) > 0) inp[min(strlen(inp), maxInput) - 1] = '\0';
                break;
            case 7: // Cancel
                return false;
            case 8: // Enter
                if (pinCheck()) return true;
                error = true;
                startInput(6);
                break;
        }
        SDL_RenderPresent(renderer);
        }
        return false;
        }

        void SECTION_language(SDL_Renderer* renderer) {
        int actions[8] = {0, 0, 0, 0, 0, 1, 2, 3};

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, "Please Select Language", FONT_HEADER, 
                {350, 215, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        if (action >= 6 && action <= 8) {
            text = getLanguage(action - 5);
            setNextSession(SECTION_menu);
        }
        SDL_RenderPresent(renderer);
        }
        continueSession();
        }

        void SECTION_menu(SDL_Renderer* renderer) {
        int actions[8] = {0, 1010, 1020, 5001, 0, 1030, 1040, 0};

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, 
                {350, 215, 383, 30}, {255, 255, 255, 255});
        drawText(renderer, currentAccount->name, FONT_BODY, 
                {350, 250, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        switch (action) {
            case 4: setNextSession(SECTION_login); break;
            case 2: setNextSession(SECTION_account); break;
            case 3: setNextSession(SECTION_transfer); break;
            case 7: setNextSession(SECTION_changepins); break;
        }
        SDL_RenderPresent(renderer);
        }
        continueSession();
        }

        double SECTION_number(SDL_Renderer* renderer) {
        double number = 0;
        int actions[8] = {0, 0, 0, 7000, 0, 0, 5000, 6000};
        startInput(12);

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, text[1006], FONT_HEADER, 
                {350, 215, 383, 30}, {255, 255, 255, 255});
        drawText(renderer, numberFormat(inp), FONT_HEADER, 
                {350, 250, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        switch (action) {
            case 4:
                if (strlen(inp) > 0) inp[strlen(inp) - 1] = '\0';
                break;
            case 8:
                number = atof(inp);
                return number;
        }
        SDL_RenderPresent(renderer);
        }
        return number;
        }

        double SECTION_money_transfer(SDL_Renderer* renderer) {
        int actions[8] = {100, 101, 102, 5001, 103, 104, 105, 1005};

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, 
                {322, 183, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        switch (action) {
            case 4: return 0;
            case 8: {
                double amount = SECTION_number(renderer);
                if (amount > 1000) return amount;
                break;
            }
            case 1: return 100000;
            case 2: return 200000;
            case 3: return 500000;
            case 5: return 1000000;
            case 6: return 2000000;
            case 7: return 5000000;
        }
        SDL_RenderPresent(renderer);
        }
        return 0;
        }

        void SECTION_changepins(SDL_Renderer* renderer) {
        char newPin[7];
        int currentState = 0;
        int actions[8] = {0, 0, 0, 7000, 0, 0, 5000, 6000};

        if (!SECTION_pins(renderer)) {
        setNextSession(SECTION_menu);
        return;
        }
        startInput(6);

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, text[1003 + currentState], FONT_HEADER, 
                {350, 215, 383, 30}, {255, 255, 255, 255});
        drawPINs(renderer, FONT_PINs, {350, 270, 383, 30});

        int action = handleEvents();
        switch (action) {
            case 4:
                if (strlen(inp) > 0) inp[min(strlen(inp), 6) - 1] = '\0';
                break;
            case 7:
                return;
            case 8:
                if (currentState == 0) {
                    strcpy(newPin, inp);
                    startInput(6);
                    currentState = 1;
                } else {
                    if (strcmp(newPin, inp) == 0) {
                        strcpy(currentAccount->pin, inp);
                        strcpy(pin, inp);
                        actions[2] = 6001;
                        saveAccounts();
                        setNextSession(SECTION_menu);
                    } else {
                        actions[2] = 5101;
                        startInput(6);
                    }
                }
                break;
        }
        SDL_RenderPresent(renderer);
        }
        continueSession();
        }

        int SECTION_bankselect(SDL_Renderer* renderer) {
        int page = 0;
        int actions[8] = {500, 501, 502, 5001, 503, 504, 505, 7001};

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, 
                {322, 183, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        switch (action) {
            case 4: return -1;
            case 8:
                page = (page + 1) % 3;
                for (int i = 0; i < 8; i++) {
                    if (i != 3 && i != 7) {
                        actions[i] = 500 + page * 6 + (i > 3 ? i - 1 : i);
                    }
                }
                break;
            case 1: case 2: case 3:
            case 5: case 6: case 7:
                return actions[action - 1] - 500;
        }
        SDL_RenderPresent(renderer);
        }
        continueSession();
        return -1;
        }

        void SECTION_account(SDL_Renderer* renderer) {
        int actions[8] = {0, 1010, 1020, 5001, 0, 1030, 1040, 0};

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, text[1001], FONT_HEADER, 
                {322, 183, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        if (action == 4) setNextSession(SECTION_login);
        SDL_RenderPresent(renderer);
        }
        continueSession();
        }

        void SECTION_transfer(SDL_Renderer* renderer) {
        int actions[8] = {1006, 1007, 0, 7000, 0, 0, 5000, 6003};
        double amount = 0;
        int bankID = 0;
        startInput(12);

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, text[1008], FONT_MEDIUM, 
                {335, 200, 383, 30}, {255, 255, 255, 255});
        
        for (int i = 0; i < 12; i++) {
            drawText(renderer, i < strlen(inp) ? std::string(1, inp[i]) : "_",
                    FONT_HEADER, {460 + i * 16, 200, 383, 30}, 
                    {200, 200, 200, 255});
        }

        if (strlen(inp) == 12) {
            Account* target = findAccount(inp);
            if (target && target->bankID == bankID) {
                drawText(renderer, target->name, FONT_HEADER, 
                        {515, 285, 383, 30}, {255, 255, 255, 255});
            } else {
                drawText(renderer, text[5102], FONT_HEADER, 
                        {515, 285, 383, 30}, {255, 0, 0, 255});
            }
        }

        char buffer[100];
        sprintf(buffer, "%.0lf", amount);
        drawText(renderer, numberFormat(buffer), FONT_HEADER, 
                {515, 315, 383, 30}, {255, 255, 255, 255});
        drawText(renderer, text[bankID + 500], FONT_HEADER, 
                {515, 345, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        switch (action) {
            case 1: amount = SECTION_money_transfer(renderer); break;
            case 2: bankID = SECTION_bankselect(renderer); break;
            case 4: if (strlen(inp) > 0) inp[min(strlen(inp), maxInput) - 1] = '\0'; break;
            case 7: setNextSession(SECTION_login); break;
        }
        SDL_RenderPresent(renderer);
        }
        continueSession();
        }

        void SECTION_INITIATION(SDL_Renderer* renderer) {
        bool error = false;
        animationTime = 30;
        int actions[8] = {0, 0, 0, 7000, 0, 0, 5000, 6000};
        startInput(6);

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, "INITIATION ATM STATE", FONT_HEADER, 
                {350, 215, 383, 30}, {255, 255, 255, 255});
        drawText(renderer, "Please Enter The Security Codes", FONT_BODY, 
                {350, 240, 383, 30}, {255, 255, 255, 255});
        drawPINs(renderer, FONT_PINs, {375, 275, 383, 30}, 
                {165, 50, 50, 255});
        if (error) {
            drawText(renderer, text[5100], FONT_BODY, 
                    {322, 275, 383, 30}, {255, 0, 0, 255});
        }

        int action = handleEvents();
        switch (action) {
            case 4:
                if (strlen(inp) > 0) inp[min(strlen(inp), 6) - 1] = '\0';
                break;
            case 7:
                return;
            case 8:
                if (pinCheck()) {
                    setNextSession(SECTION_LOADDATA);
                } else {
                    error = true;
                    startInput(6);
                }
                break;
        }
        SDL_RenderPresent(renderer);
        }
        continueSession();
        }

        void SECTION_LOADDATA(SDL_Renderer* renderer) {
        int actions[8] = {0, 0, 10, 0, 0, 0, 11, 12};

        while (!quit) {
        drawMain(renderer, actions);
        drawText(renderer, "Administrator Screen", FONT_HEADER, 
                {350, 215, 383, 30}, {255, 255, 255, 255});

        int action = handleEvents();
        switch (action) {
            case 3: loadAccounts(); break;
            case 7: saveAccounts(); break;
            case 8: setNextSession(SECTION_login); break;
        }
        SDL_RenderPresent(renderer);
        }
        continueSession();
        }

    // Utility Functions
    bool checkDuplicateID(const char* userID, int bankID) {
        for (int i = 0; i < countAccount; i++) {
            if (strcmp(accounts[i].accountNumber, userID) == 0 && 
                accounts[i].bankID == bankID) {
                return true;
            }
        }
        return false;
    }

    void toggleFullScreen() {
        isFullScreen = !isFullScreen;
        SDL_SetWindowFullscreen(window, 
            isFullScreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    }

    Account* findAccount(const char* accountNumber) {
        for (int i = 0; i < countAccount; i++) {
            if (strcmp(accounts[i].accountNumber, accountNumber) == 0) {
                return &accounts[i];
            }
        }
        return nullptr;
    }

    bool setCurrentAccount(const char* accountNumber) {
        currentAccount = findAccount(accountNumber);
        if (currentAccount) {
            strcpy(pin, currentAccount->pin);
            return true;
        }
        return false;
    }

    std::string numberFormat(const char* str) {
        std::string buffer;
        int len = strlen(str);
        for (int i = 0; i < len; i++) {
            if (i > 0 && i % 3 == 0) buffer.insert(0, ".");
            buffer.insert(0, 1, str[len - i - 1]);
        }
        return buffer + "VND";
    }

    int min(int a, int b) {
        return (a < b) ? a : b;
    }

    // Resource Management
    class ResourceManager {
    public:
        static bool init() {
            if (SDL_Init(SDL_INIT_VIDEO) != 0 || 
                TTF_Init() != 0 || 
                IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
                logError("Initialization failed");
                return false;
            }

            window = SDL_CreateWindow("Automatic Teller Machine",
                                    100, 100, SCREEN_WIDTH, SCREEN_HEIGHT,
                                    SDL_WINDOW_SHOWN);
            if (!window) {
                logError("Window creation failed");
                return false;
            }

            renderer = SDL_CreateRenderer(window, -1, 
                                        SDL_RENDERER_ACCELERATED | 
                                        SDL_RENDERER_PRESENTVSYNC);
            if (!renderer) {
                logError("Renderer creation failed");
                return false;
            }

            return loadFonts() && loadCursor();
        }

        static void cleanup() {
            if (customCursor) SDL_DestroyTexture(customCursor);
            if (renderer) SDL_DestroyRenderer(renderer);
            if (window) SDL_DestroyWindow(window);
            TTF_Quit();
            IMG_Quit();
            SDL_Quit();
        }

    private:
        static void logError(const std::string& msg) {
            std::cerr << msg << ": " << SDL_GetError() << std::endl;
        }

        static bool loadFonts() {
            FONT_HEADER = TTF_OpenFont("assets/fonts/arialbd.ttf", 24);
            FONT_MEDIUM = TTF_OpenFont("assets/fonts/arialbd.ttf", 20);
            FONT_BODY = TTF_OpenFont("assets/fonts/arial.ttf", 16);
            FONT_TEXT = TTF_OpenFont("assets/fonts/arial.ttf", 8);
            FONT_DETAIL = TTF_OpenFont("assets/fonts/arial.ttf", 12);
            FONT_PINs = TTF_OpenFont("assets/fonts/arial.ttf", 32);

            return FONT_HEADER && FONT_MEDIUM && FONT_BODY && 
                   FONT_TEXT && FONT_DETAIL && FONT_PINs;
        }

        static bool loadCursor() {
            SDL_Surface* cur = IMG_Load("assets/images/cursor.png");
            if (!cur) {
                logError("Cursor loading failed");
                return false;
            }
            customCursor = SDL_CreateTextureFromSurface(renderer, cur);
            SDL_FreeSurface(cur);
            return customCursor != nullptr;
        }
    };

    // Drawing Functions
    void drawText(SDL_Renderer* renderer, const std::string& text, 
                 TTF_Font* font, SDL_Rect rect, SDL_Color color) {
        if (text.empty()) return;

        SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
        if (!surface) {
            std::cerr << "Text rendering failed: " << TTF_GetError() << std::endl;
            quit = true;
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect textRect = {rect.x, rect.y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &textRect);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    void drawImage(SDL_Renderer* renderer, const std::string& path, 
                  SDL_Rect rect) {
        SDL_Surface* surface = IMG_Load(path.c_str());
        if (!surface) {
            std::cerr << "Image loading failed: " << IMG_GetError() << std::endl;
            quit = true;
            return;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_Rect r = {rect.x, rect.y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &r);

        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    void drawPINs(SDL_Renderer* renderer, TTF_Font* font, 
                 SDL_Rect rect, SDL_Color color = {50, 240, 50, 255}) {
        int len = min(strlen(inp), 6);
        for (int i = 0; i < len; i++) {
            drawText(renderer, "*", font, 
                    {rect.x + i * 20, rect.y, rect.w, rect.h}, color);
        }
        for (int i = 0; i < 6; i++) {
            drawText(renderer, "_", font, 
                    {rect.x + i * 20, rect.y - 8, rect.w, rect.h}, color);
        }
    }

    // Input Handling
    int handleEvents() {
        int mouseX, mouseY, actionInvoke = -1;
        SDL_GetMouseState(&mouseX, &mouseY);

        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_MOUSEMOTION:
                    updateButtonHover(mouseX, mouseY);
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    selectButton(mouseX, mouseY);
                    break;
                case SDL_MOUSEBUTTONUP:
                    actionInvoke = processButtonRelease(mouseX, mouseY);
                    break;
                case SDL_KEYDOWN:
                    actionInvoke = processKeyDown(e.key.keysym.sym);
                    break;
            }
        }
        return actionInvoke;
    }

    void updateButtonHover(int x, int y) {
        for (int i = 0; i < 20; i++) {
            buttons[i].isHovered = isMouseInside(buttons[i].rect, x, y);
        }
    }

    void selectButton(int x, int y) {
        for (int i = 0; i < 20; i++) {
            if (isMouseInside(buttons[i].rect, x, y)) {
                selectedButton = &buttons[i];
                break;
            }
        }
    }

    int processButtonRelease(int x, int y) {
        if (!selectedButton || !isMouseInside(selectedButton->rect, x, y)) {
            selectedButton = nullptr;
            return -1;
        }

        int id = selectedButton->id;
        selectedButton = nullptr;

        if (id >= 11 && id <= 18 && animationTime == 0) {
            return id % 10;
        }

        if (maxInput > 0 && strlen(inp) < maxInput) {
            if (id == 10) inp[strlen(inp)] = '0';
            else if (id > 0 && id < 10) inp[strlen(inp)] = id + '0';
        }
        return -1;
    }

    int processKeyDown(SDL_Keycode key) {
        switch (key) {
            case SDLK_ESCAPE:
                quit = true;
                break;
            case SDLK_F11:
                toggleFullScreen();
                break;
            case SDLK_RETURN:
                return 8;
            case SDLK_BACKSPACE:
                return 4;
            default:
                if (key >= SDLK_0 && key <= SDLK_9 && 
                    maxInput > 0 && strlen(inp) < maxInput) {
                    inp[strlen(inp)] = key;
                }
                break;
        }
        return -1;
    }

    bool isMouseInside(SDL_Rect rect, int mouseX, int mouseY) {
        return (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
                mouseY >= rect.y && mouseY <= rect.y + rect.h);
    }

    // Database Management
    void loadAccounts() {
        FILE* file = fopen("database/user.csv", "r");
        if (!file) {
            std::cerr << "Cannot open file user.csv" << std::endl;
            return;
        }

        int i = 0;
        while (!feof(file) && i < 1000) {
            if (fscanf(file, "%d %12s %6s %49s %lf", 
                      &accounts[i].bankID, 
                      accounts[i].accountNumber, 
                      accounts[i].pin, 
                      accounts[i].name, 
                      &accounts[i].balance) == 5) {
                if (!checkDuplicateID(accounts[i].accountNumber, 
                                    accounts[i].bankID)) {
                    i++;
                } else {
                    std::cerr << "Duplicate Account ID: " 
                             << accounts[i].accountNumber << std::endl;
                }
            }
        }
        
        countAccount = i;
        for (int j = 0; j < i; j++) {
            std::cout << text[500 + accounts[j].bankID] << " "
                     << accounts[j].accountNumber << " "
                     << accounts[j].pin << " "
                     << accounts[j].name << " "
                     << accounts[j].balance << std::endl;
        }
        fclose(file);
    }

    void saveAccounts() {
        FILE* file = fopen("database/user.csv", "w");
        if (!file) {
            std::cerr << "Cannot open file user.csv" << std::endl;
            return;
        }

        for (int i = 0; i < countAccount; i++) {
            fprintf(file, "%d %s %s %s %.0lf\n",
                   accounts[i].bankID, 
                   accounts[i].accountNumber, 
                   accounts[i].pin, 
                   accounts[i].name, 
                   accounts[i].balance);
        }
        fclose(file);
    }

    // Session Management
    void startInput(int length) {
        memset(inp, 0, sizeof(inp));
        maxInput = length;
    }

    void setNextSession(void (*session)(SDL_Renderer*)) {
        nextSession = session;
        quit = true;
    }

    void continueSession() {
        if (nextSession) {
            animationTime = 30;
            quit = false;
            handleEvents();
            nextSession(renderer);
        }
    }

    // Main Function
    int main() {
        std::cout << "----------------------------------------" << std::endl;
        if (!ResourceManager::init()) return 1;

        button_init();  // Assume this function is defined elsewhere
        SDL_ShowCursor(SDL_DISABLE);
        SECTION_INITIATION(renderer);  // Assume this function is defined elsewhere

        std::cout << "-- ATM STOP --" << std::endl;
        ResourceManager::cleanup();
        return 0;
    }
}
