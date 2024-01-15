#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <AudioToolbox/AudioToolbox.h>      // all above for hangman
// gcc -o compiled compiled.c -framework AudioToolbox -framework CoreFoundation
//gcc -o compiled compiled.c -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2 -lSDL2_mixer -framework AudioToolbox -framework CoreFoundation

#include <signal.h>     // So i can interrupt with control + c or else music loop blocks it
#include <SDL2/SDL.h>       // for bg music
#include <SDL2/SDL_mixer.h>     // for bg music // these ones for tictactoe

// each of the text files contains 1000 words
#define LISTSIZE 1000
#define TERMINAL_WIDTH 120
#define BRIGHT_GREEN "\033[38;5;46m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[34m"
#define RED "\033[31m"
#define RESET_COLOR "\033[0m"
void printRed(const char *format, ...);
void printGreen(const char *format, ...);
void printBlue(const char *format, ...);
void printYellow(const char *format, ...);
void printYellowWoSpaces(const char *format, ...);
void audio_shorten(const char *file_path);
void play_correct_audio(void);
void play_ghalat_hai_boss_audio(void);
void play_win_audio(void);
void play_fail_audio(void);
void completionCallback(SystemSoundID  soundID, void *clientData);
void current_output(int wordsize, char correct_input[]);
void basicman(void);
void printman(int counter);

int run_again(int *play_again);
void print_table(char table[3][3]);
void checkrow(char table[3][3], char play1, char play2, int *win);
void checkcol(char table[3][3], char play1, char play2, int *win);      // for tictactoe
Mix_Music *music;

void playBackgroundMusic()
{
    music = Mix_LoadMUS("tictactoe.mp3");  // Load the music file

    if (music == NULL)
    {
        printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }
    else
    {
        Mix_PlayMusic(music, -1);  // Play the music
    }
}

volatile sig_atomic_t quit = 0;

// Signal handler
void handle_signal(int signal) {
    if (signal == SIGINT) {
        // Cleanup and close SDL_mixer
        Mix_FreeMusic(music);
        Mix_CloseAudio();
        Mix_Quit();

        // Close SDL
        SDL_Quit();

        // Exit the program
        exit(0);
    }
}       // for tictactoe

        // FOR SCRABBLE START
// Points assigned to each letter of the alphabet
int POINTS[] = {1, 3, 3, 2, 1, 4, 2, 4, 1, 8, 5, 1, 3, 1, 1, 3, 10, 1, 1, 1, 1, 4, 4, 8, 4, 10};
int compute_score(char word[50]);
void clear_input_buffer();
        // FOR SCRABBLE END

int hangman(void);     // all above for hangman
int scrabble(int POINTS[]);
int tictactoe(void);

int main(void)
{
    system("clear");
    int selection;
    printYellow("Welcome to Miniclip cheap version!\n\n\n");
    printYellow("We have multiple games in store for you!\n");
    printYellow("1. Tictactoe\n");
    printYellow("2. Scrabble\n");
    printYellow("3. Hangman\n");
    printf("\n\n");
    printYellow("Enter the number for whichever game you'd like to play: ");
    scanf(" %i", &selection);

    if (selection == 1)
    {
        return tictactoe();
    }
    else if(selection == 2)
    {
        return scrabble(POINTS);
    }
    else if(selection == 3)
    {
        return hangman();
    }
    else
    {
        printf("Wrong selection\n");
        return 0;
    }
}



int hangman(void)
{
    system("clear");
    // Asking the user for the number of letters the word must be
    int wordsize, runitback, winner;
    printYellow("Welcome to Hangman!\n");
    printYellow("You are allowed 7 mistakes or the man shall hang!\n");
    do
    {
        if (runitback == 1)
        {
            system("clear");
        }
        wordsize = 0, runitback = 0, winner = 0;
        printYellow("How many letters should the word contain (5-8): ");
        scanf("%i", &wordsize);

        while (wordsize < 5 || wordsize > 8)
        {
            printYellow("Error: wordsize must be either 5, 6, 7, or 8\n");
            printYellow("How many letters should the word contain: ");
            scanf("%i", &wordsize);
            printYellow("Understood\n");
        }

        // Printing the basic hangman
        printYellow("\n");
        basicman();
        
        // Making an array for correct user input and making it empty
        char correct_input[wordsize + 1];   // 6    0 1 2 3 4 5
        for (int i = 0; i < wordsize; i++)
        {
            correct_input[i] = ' ';
        }
        correct_input[wordsize] = '\0';
        current_output(wordsize, correct_input);

        // open correct file, each file has exactly LISTSIZE words
        char wl_filename[10];
        sprintf(wl_filename, "%i.txt", wordsize);   // stores 5.txt or 6.txt whatever in wl_filename
        FILE *wordlist = fopen(wl_filename, "r");   // uses the name from wl_filename to write from that file into a file called wordlist
        if (wordlist == NULL)
        {
            printYellow("Error opening file %s.\n", wl_filename);
            return 1;
        }

        // load word file into an array options of size LISTSIZE
        char options[LISTSIZE][wordsize + 1];

        for (int i = 0; i < LISTSIZE; i++)
        {
            fscanf(wordlist, "%s", options[i]);
        }
        fclose(wordlist);

        // pseudorandomly select a word for this game
        srand(time(NULL));
        char choice[wordsize + 1];
        int random = rand() % LISTSIZE; // generates a random number which I then use as index for options
        for (int i = 0; i < wordsize; i++)
        {
            choice[i] = options[random][i];
        }
        choice[wordsize] = '\0';

        // Declaring variables and asking user for guess
        char guess;
        const int mistakes = 7;
        int excluded[26], included[26];
        int included_counter = 0, excluded_counter = 0;
        int present = 0, counter = 0;
        printYellow("%s\n", choice);
        printYellow("Enter a letter: ");
        scanf(" %c", &guess);

        // checking if it is alphabet or not
        if (isalpha(guess) == 0)
        {
            printYellow("The character you entered is not a letter\n");
        }

        while(isalpha(guess) == 0)
        {
            printYellow("Enter a letter: ");
            scanf(" %c", &guess);
        }
        printYellow("\n");
        guess = tolower(guess);

        while (counter < 7)
        {
            present = 0; // Reset present for each new guess

            for (int i = 0; i < wordsize; i++)  
            {
                if (guess == choice[i])
                {
                    present = 1;    // Indicates that the guessed char is in the choice
                    correct_input[i] = guess; // Store it in the correct position
                }
            }

            int compare = strcmp(choice, correct_input);
            if (compare == 0)
            {
                current_output(wordsize, correct_input);
                printGreen("CONGRATULATIONS!!!\n");
                printGreen("YOU HAVE WON YOU CHEEKY MAN!!!\n");
                winner = 1;
                play_win_audio();
                printf("\n");
                printYellow("Enter 1 to play again, else enter 0: ");
                scanf("%i", &runitback);
                if (runitback == 1)
                {
                    break;
                }
                else
                {
                    printf("\n\n\n");
                    printYellow("Thanks for playing\n");
                    sleep(3);
                    system("clear");
                    return 0;
                }
            }

            if (present == 1)
            {
                included[included_counter++] = guess;
                play_correct_audio();
                current_output(wordsize, correct_input);
            }
            else
            {
                excluded[excluded_counter++] = guess;
                counter++;

                // Print excluded letters
                for (int j = 0; j < excluded_counter; j++)
                {
                    if (j == 0)
                    {
                        printYellowWoSpaces("%c", excluded[j]);
                    } else
                    {
                        printYellowWoSpaces(", %c", excluded[j]);
                    }
                }
                printYellowWoSpaces(" is excluded\n\n");
            }
    
            // output
            if (present == 1)
            {
                included_counter++;
            }

            if (present == 0)
            {
                play_ghalat_hai_boss_audio();
                printman(counter);
            }

            // Taking user input every time after the first time
            int previous = 0;
            while (counter < 7 || previous == 1)
            {
                previous = 0;
                printYellow("\n");
                printYellow("Enter another letter: ");
                scanf(" %c", &guess);
                guess = tolower(guess);
                printYellow("\n");
                if (isalpha(guess) == 0)
                {
                    printYellow("The character you entered is not a letter\n");
                    while(isalpha(guess) == 0)
                    {
                        printYellow("Enter a letter: ");
                        scanf(" %c", &guess);
                    }
                }

                for (int c = 0; c < excluded_counter; c++)
                {
                    if (guess == excluded[c])
                    {
                        previous = 1;
                        printYellow("Please enter a new letter\n");
                        break;
                    }
                }
                if (previous == 1)
                {
                    continue;
                }
                for (int c = 0; c < included_counter; c++)
                {
                    if (guess == included[c])
                    {
                        previous = 1;
                        printYellow("Please enter a new letter\n");
                        break;
                    }
                }
                if (previous == 0)
                {
                    break;
                }
            }
        }
        if (winner == 0)
        {
            printRed("You have lost\n");
            printRed("You are indeed a loser\n\n");
            printYellow("The word was \"%s\"\n", choice);
            play_fail_audio();
            printf("\n\n");
            printYellow("Enter 1 to play again, else enter 0: ");
            scanf("%i", &runitback);
        }
    }
    while (runitback == 1);
    printf("\n\n\n");
    printYellow("Thanks for playing\n");
    sleep(3);
    system("clear");    
    return 0;
}
void basicman(void)
{
    printYellow("  +---+\n");
    printYellow("  |   |\n");
    printYellow("      |\n");
    printYellow("      |\n");
    printYellow("      |\n");
    printYellow("      |\n");
    printYellow("=========\n\n");
}
void current_output(int wordsize, char correct_input[])
{
    char line1[1024] = {0}; // Buffer for the first line
    char line2[1024] = {0}; // Buffer for the second line

    // Building the first line
    for (int z = 0; z < wordsize; z++) {
        char temp[4];
        snprintf(temp, sizeof(temp), " %c ", correct_input[z]);
        strcat(line1, temp);
    }

    // Building the second line
    for (int z = 0; z < wordsize; z++) {
        strcat(line2, "__ ");
    }

    // Printing the lines
    printYellow(line1);
    printYellow("\n");
    printYellow(line2);
    printYellow("\n\n");
}
void printman(int counter)
{
    if (counter == 1)
    {
        printRed("  +---+\n");
        printRed("  |   |\n");
        printRed("  0   |\n");
        printRed("      |\n");
        printRed("      |\n");
        printRed("      |\n");
        printRed("=========\n");
    }
    else if (counter == 2)
    {
        printRed("  +---+\n");
        printRed("  |   |\n");
        printRed("  0   |\n");
        printRed("  |   |\n");
        printRed("      |\n");
        printRed("      |\n");
        printRed("=========\n");
    }
    else if (counter == 3)
    {
        printRed("  +---+\n");
        printRed("  |   |\n");
        printRed("  0   |\n");
        printRed(" \\|   |\n");
        printRed("      |\n");
        printRed("      |\n");
        printRed("=========\n");
    }
    else if (counter == 4)
    {
        printRed("  +---+\n");
        printRed("  |   |\n");
        printRed("  0   |\n");
        printRed(" \\|/  |\n");
        printRed("      |\n");
        printRed("      |\n");
        printRed("=========\n");
    }
    else if (counter == 5)
    {
        printRed("  +---+\n");
        printRed("  |   |\n");
        printRed("  0   |\n");
        printRed(" \\|/  |\n");
        printRed("  |   |\n");
        printRed("      |\n");
        printRed("=========\n");
    }
    else if (counter == 6)
    {
        printRed("  +---+\n");
        printRed("  |   |\n");
        printRed("  0   |\n");
        printRed(" \\|/  |\n");
        printRed("  |   |\n");
        printRed("   \\  |\n");
            printRed("=========\n");
    }
    else if (counter == 7)
    {
        printRed("  +---+\n");
        printRed("  |   |\n");
        printRed("  0   |\n");
        printRed(" \\|/  |\n");
        printRed("  |   |\n");
        printRed(" / \\  |\n");
        printRed("=========\n");
    }

}
void completionCallback(SystemSoundID  soundID, void *clientData)
{
    CFRunLoopStop(CFRunLoopGetCurrent());
}

void play_fail_audio(void)
{
    const char *file_path = "fail.wav"; // Replace this with the path to your WAV file
    audio_shorten(file_path);
}
void play_correct_audio(void)
{
    const char *file_path = "correct.wav"; // Replace this with the path to your WAV file
    audio_shorten(file_path);
}
void play_ghalat_hai_boss_audio(void)
{
    const char *file_path = "ghalat.wav"; // Replace this with the path to your WAV file
    audio_shorten(file_path);
}
void *playAudio(void *arg) {
    SystemSoundID soundID = *((SystemSoundID*)arg);

    AudioServicesPlaySystemSound(soundID);
    return NULL;
}
void play_win_audio(void) {
    const char *file_path = "win.wav"; // Replace with your WAV file path

    CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)file_path, strlen(file_path), false);
    SystemSoundID soundID;
    OSStatus status = AudioServicesCreateSystemSoundID(fileURL, &soundID);
    
    if (status != kAudioServicesNoError) {
        printYellow("Error occurred while loading sound file\n");
        return;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, playAudio, &soundID);

    usleep(9000000); // 7 seconds in microseconds

    AudioServicesDisposeSystemSoundID(soundID); // Stop the sound
    CFRelease(fileURL);

    pthread_cancel(thread_id); // Cancel the thread if it's still running
}
void audio_shorten(const char *file_path)
{
    CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *)file_path, strlen(file_path), false);
    SystemSoundID soundID;
    OSStatus status = AudioServicesCreateSystemSoundID(fileURL, &soundID);
    
    if (status != kAudioServicesNoError) {
        printYellow("Error occurred while loading sound file\n");
        //return 1;
    }

    AudioServicesAddSystemSoundCompletion(soundID, NULL, NULL, completionCallback, NULL);
    
    AudioServicesPlaySystemSound(soundID);
    
    // Run the loop to wait for the sound to finish playing
    CFRunLoopRun();

    // Clean up
    AudioServicesDisposeSystemSoundID(soundID);
    CFRelease(fileURL);
}
void printYellow(const char *format, ...)
{
    char buffer[1024]; // Large buffer to hold the formatted string
    va_list args;
    
    // Using vsnprintf to format the string
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    int len = strlen(buffer);
    int spaces = (TERMINAL_WIDTH - len) / 2;

    // Print leading spaces
    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }

    // Print text in yellow
    printf("%s%s%s", YELLOW, buffer, RESET_COLOR); // Removed '\n' from here
}
void printBlue(const char *format, ...)
{
    char buffer[1024]; // Large buffer to hold the formatted string
    va_list args;
    
    // Using vsnprintf to format the string
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    int len = strlen(buffer);
    int spaces = (TERMINAL_WIDTH - len) / 2;

    // Print leading spaces
    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }

    // Print text in blue
    printf("%s%s%s", BLUE, buffer, RESET_COLOR); // Removed '\n' from here
}
void printGreen(const char *format, ...)
{
    char buffer[1024]; // Large buffer to hold the formatted string
    va_list args;
    
    // Using vsnprintf to format the string
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    int len = strlen(buffer);
    int spaces = (TERMINAL_WIDTH - len) / 2;

    // Print leading spaces
    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }

    // Print text in green
    printf("%s%s%s", BRIGHT_GREEN, buffer, RESET_COLOR); // Removed '\n' from here
}
void printRed(const char *format, ...)
{
    char buffer[1024]; // Large buffer to hold the formatted string
    va_list args;
    
    // Using vsnprintf to format the string
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    int len = strlen(buffer);
    int spaces = (TERMINAL_WIDTH - len) / 2;

    // Print leading spaces
    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }

    // Print text in green
    printf("%s%s%s", RED, buffer, RESET_COLOR); // Removed '\n' from here
}
void printYellowWoSpaces(const char *format, ...)
{
    char buffer[1024]; // Large buffer to hold the formatted string
    va_list args;
    
    // Using vsnprintf to format the string
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    
    int len = strlen(buffer);

    // Print text in yellow
    printf("%s%s%s", YELLOW, buffer, RESET_COLOR); // Removed '\n' from here
}

// FOR TICTACTOE
int run_again(int *play_again)      // passing by pointer because if i dont, main ke andar wali val affect nhi hogi
{
    printYellow("\nWould you like to play again?\n");
    printYellow("\nEnter 1 to play again, 0 to end: ");
    scanf("%i", play_again);
    return *play_again;
}
void print_table(char table[3][3])
{
    printYellow(" %c | %c | %c \n", table[0][0], table[0][1], table[0][2]);
    printYellow("-----------\n");
    printYellow(" %c | %c | %c \n", table[1][0], table[1][1], table[1][2]);
    printYellow("-----------\n");
    printYellow("  %c | %c | %c \n\n", table[2][0], table[2][1], table[2][2]);
}
void checkrow(char table[3][3], char play1, char play2, int *win)
{
    for (int row = 0; row < 3; row++)
    {
        if (table[row][0] == table[row][1] && table[row][1] == table[row][2] && table[row][0] != ' ')
        {
            if (table[row][0] == play1)
            {
                printGreen("Player 1 is the Winner!\n");
                *win = 1;
                break;
                
            }
            else
            {
                printGreen("Player 2 is the Winner!\n");
                *win = 1;
            }
        }
    }
}
void checkcol(char table[3][3], char play1, char play2, int *win)
{
    for (int col = 0; col < 3; col++)
    {
        if (table[0][col] == table[1][col] && table[1][col] == table[2][col] && table[0][col] != ' ')
        {
            if (table[0][col] == play1)
            {
                printGreen("Player 1 is the Winner!\n");
                *win = 1;
                break;
            }
            else
            {
                printGreen("Player 2 is the Winner!\n");
                *win = 1;
            }
        }
    }
}
int tictactoe(void)
{
    // Register signal handler
    signal(SIGINT, handle_signal);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
        return 1;
    }

    // Play background music
    playBackgroundMusic();
    int play_again;
    system("clear");
    printYellow("\nWelcome to the game of tictactoe!\nThis is a 2 player game\n\n");
    do
    {
        play_again = 0;
        int win = 0;
        int x, y;
        char play1, play2, table[3][3];
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                table[i][j] = ' ';
            }
        }
        print_table(table);
        printYellow("Player 1, enter your symbol please: ");
        scanf(" %c", &play1);
        printYellow("Player 2, enter your symbol please: ");
        scanf(" %c", &play2);
        while (play2 == play1)
        {
            printYellow("Player 2, enter a different symbol please: ");
            scanf(" %c", &play2);
        }
        printYellow("\n");
        printYellow("Enter the coordinate you would like to pick in a 0,0 pattern\n");

        for (int i = 0; i < 9; i++)
        {
            if (i % 2 == 0)
            {
                printYellow("Player 1's turn. Enter the coordinate you would like to pick: ");
                scanf("%i,%i", &x, &y);
                while (table[x][y] == play1 || table[x][y] == play2)
                {
                    printYellow("Spot already taken. Please enter a different coordinate: ");
                    scanf("%i,%i", &x, &y);
                }
                table[x][y] = play1;
            }
            else
            {
                printYellow("Player 2's turn. Enter the coordinate you would like to pick: ");
                scanf("%i,%i", &x, &y);
                while (table[x][y] == play1 || table[x][y] == play2)
                {
                    printYellow("Spot already taken. Please enter a different coordinate: ");
                    scanf("%i,%i", &x, &y);
                }
                table[x][y] = play2;
            }

            system("clear"); // This command clears the terminal screen on Unix-based systems like macOS
            print_table(table);
            if (i >= 4)
            {
                // for rows
                checkrow(table, play1, play2, &win);
                // for columns
                checkcol(table, play1, play2, &win);
                // for diagonal
                if ((table[0][0] == table[1][1] && table[1][1] == table[2][2]) ||
                    (table[0][2] == table[1][1] && table[1][1] == table[2][0]))
                {
                    if (table[1][1] == play1)
                    {
                        printGreen("Player 1 is the Winner!\n");
                        win = 1;
                        break;
                    }
                    else if (table[1][1] == play2)
                    {
                        printGreen("Player 2 is the Winner!\n");
                        win = 1;
                        break;
                    }
                }
            }
            if (win == 1)
            {
                break;
            }
        }
        if (win == 0)
        {
            printBlue("Tie!\n");
        }
        run_again(&play_again);     // taakey wo address par jaakar dekhy kia maamlat hain or wahi pass karrey
        if (play_again == 1)
        {
            system("clear");
        }
    }
    while (play_again == 1);

    if (play_again == 0)
    {
        printYellow("Thanks for playing!\n\n\n");
        // Cleanup
        Mix_FreeMusic(music); // Make sure you define 'music' as a global or pass it properly
        Mix_Quit();
        SDL_Quit();
    }
    return 0;
}
    // TICTACTOE FINISH

    // SCRABBLE START
int compute_score(char word[50])
{
    int n = 0 , score = 0;
    char word_l;

    for (int i = 0 ; i < strlen(word) ; i++)
    {
        word_l = tolower(word[i]);
        if( word_l >= 'a' && word_l <= 'z')
        {
            n = word_l - 'a';
            score = score + POINTS[n];
        }
        else
        {
            score = score + 0;
        }
    }
    return score;
}
void clear_input_buffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}
int scrabble(int POINTS[])
{
        system("clear");
    // Get input words from both players
    char word1[50];
    char word2[50];
    printYellow("  Welcome to Scrabble!\n\n\n");
    printYellow("The rules are simple\n");
    printYellow("Score more than your opponent\n");
    int redo = 0;
    do
    {
        system("clear");
        printYellow("\tEvery letter has a specific point value\n\n");
        char alpha = 'A';
        int n = 0;
        for(int i = 0; i < 6; i++)
        {
            printYellow("%c = %i\t%c = %i\t%c = %i\t%c = %i\n", alpha, POINTS[n], alpha + 1, POINTS[n+1], alpha + 2, POINTS[n+2], alpha + 3, POINTS[n+3]);
            alpha = alpha + 4;
            n = n + 4;
        }
        printYellow("%c = %i\t%c = %i\n\n", alpha, POINTS[24], alpha + 1, POINTS[25]);    
        printYellow("Player 1: ");
        fgets(word1, sizeof(word1), stdin);
        printYellow("Player 2: ");
        fgets(word2, sizeof(word2), stdin);

        // Score both words
        int score1 = compute_score(word1);
        int score2 = compute_score(word2);
        
        printf("\n");
        printYellow("Player 1's score is %i\n", score1);
        printYellow("Player 2's score is %i\n\n", score2);


        if( score1 > score2 )
        {
            printGreen("Player 1 wins!\n");
        }
        else if( score2 > score1)
        {
            printGreen("Player 2 wins!\n");
        }
        else
        {
            printBlue("Tie!\n");
        }

        printYellow("\n\nEnter 1 if you want to play another game, else enter 0: ");
        scanf("%i", &redo);
        clear_input_buffer(); // taakey ooper waley player 1 ka fgets \n ko read karkey input skip na karwadey

    }
    while (redo == 1);

    printf("\n\n\n");
    printYellow("Thanks for playing!\n");
    //int wow3 = 1;
    return 0;
}