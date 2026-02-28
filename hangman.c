#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <curses.h>

#define MAX_LENGTH_WORD 50 // with null character \0
#define MAX_REPONSE 10
#define MAX_Y 60
#define MAX_X 30

typedef struct Text {
    int nbWords;
    char** words;
}S_text;

 typedef struct HiddenWord{
    char* hidden;
    char* word;
 } S_hiddenWord;

void readEntries(S_text *words){
    FILE *fptr = fopen("words.txt","r");
    if ( fptr == NULL){
        printf("no file found");
        exit(1);
    }
    char buffer[MAX_LENGTH_WORD];
    int ch;
    int count = 0;
    words->nbWords = 0;
    words->words = (char**) malloc(sizeof(char *));
    if(words->words == NULL){
        printf("can not allocate memory for the strings");
        exit(1);
    }
    while ((ch = fgetc(fptr)) != EOF){
        if( ferror(fptr) != 0){
            printf("problem during reading the file: %d\n",ferror(fptr));
        }
        if (ch == '\0' || ch == '\n'){
            // put null chartacter at the end of each word
            buffer[count] = '\0';
            words->words[words->nbWords] = (char*) strdup(buffer);
            words->nbWords++;
            char **tmp = (char**) realloc(words->words, (words->nbWords + 1) * sizeof(char*));
            if (!tmp){
                printf("cannot reallocate memory\n");
                fclose(fptr);
                exit(1);
            }
            words->words = tmp;
            count = 0;
        }
        else{
            if (count < (MAX_LENGTH_WORD - 1)){
                buffer[count] = (char) ch;
                count++;
            }
            else{
                printf("At  least one word is more than %d\n",MAX_LENGTH_WORD);
                break;
            }
        }
    }
    fclose(fptr);
}

void chooseWord(S_text *words, S_hiddenWord *hiddenWord){
    int randNumb = rand() % words->nbWords;
    hiddenWord->word = strdup(words->words[randNumb]);
    size_t len = strlen(hiddenWord->word);
    hiddenWord->hidden = malloc((len+1)* sizeof(char));
    for (int i = 0; i<len; i++){
        hiddenWord->hidden[i] = '_';
    }
    // the null character at the end
    hiddenWord->hidden[len] = '\0';
}

void freeText(S_text *words) {
    for (int i = 0; i < words->nbWords; i++) {
        free(words->words[i]);
    }
    free(words->words);
    words->nbWords = 0;
}

void freeHiddenWord (S_hiddenWord *hiddenWord){
    free(hiddenWord->hidden);
    free(hiddenWord->word);
}

// Simple gallows centered around (originY, originX)
static void drawGallows(int originY, int originX) {
    mvprintw(originY,     originX,     " +---+");
    mvprintw(originY + 1, originX,     " |   |");
    mvprintw(originY + 2, originX,     " |");
    mvprintw(originY + 3, originX,     " |");
    mvprintw(originY + 4, originX,     " |");
    mvprintw(originY + 5, originX,     "_|_");
}

// Draw hangman parts depending on remaining lives
// Assume maxLives = 5. You start at 5 and go down to 0.
static void drawHangman(int lives, int originY, int originX) {
    int maxLives = 5;
    int mistakes = maxLives - lives; // how many wrong guesses

    // Head
    if (mistakes >= 1) {
        mvaddch(originY + 2, originX + 4, 'O');
    }
    // Body
    if (mistakes >= 2) {
        mvaddch(originY + 3, originX + 4, '|');
    }
    // Left arm
    if (mistakes >= 3) {
        mvaddch(originY + 3, originX + 3, '/');
    }
    // Right arm
    if (mistakes >= 4) {
        mvaddch(originY + 3, originX + 5, '\\');
    }
    // Legs
    if (mistakes >= 5) {
        mvaddch(originY + 4, originX + 3, '/');
        mvaddch(originY + 4, originX + 5, '\\');
    }
}


void drawBoard(S_hiddenWord *hiddenWord, char *pLife, int row, int col) {
    int lives = (int)(*pLife);

    clear();

    // 1) Show lives in top-right corner
    mvprintw(1, col - 15, "Lives: %d", lives);

    // 2) Compute position for gallows/hangman (roughly center)
    int midY = row / 2;
    int midX = col / 2;
    int originY = midY - 6;   // shift up a bit
    int originX = midX - 10;  // shift left a bit

    // 3) Draw gallows + hangman parts based on lives
    drawGallows(originY, originX);
    drawHangman(lives, originY, originX);

    // 4) Show the hidden word at the bottom center
    int len = (int)strlen(hiddenWord->hidden);
    int wordY = row - 2;
    int wordX = (col - len) / 2;
    mvprintw(wordY, wordX, "%s", hiddenWord->hidden);

    refresh();
}


char inputPlayer(){
    noecho();
    cbreak();
    int ch = getch();
    echo();
    nocbreak();
    return (char) ch;
}

void isCharInHiddenWord(char answer, S_hiddenWord *hiddenWord, char *pLife){
    char tmp[2] = { answer, '\0'};
    if(strcspn(hiddenWord->word, tmp) != strlen(hiddenWord->word)){
            for(int i =0; hiddenWord->word[i] != '\0'; i++){
                if(hiddenWord->word[i] == answer){
                    hiddenWord->hidden[i] = answer;
                }
            }
    }
    else{
        (*pLife)--;
    }
}

void isPlayerWin(S_hiddenWord *HiddenWord, char *wordFound, char *life, int row){
    char tmpWin = 1;
    for (int i =0; HiddenWord->hidden[i] != '\0'; i++){
        if(HiddenWord->hidden[i] == '_'){
            tmpWin = 0;
        }
    }
    if(tmpWin == 1){
        clear();
        mvprintw(row-1, 0, " YOU WIN\n");
        refresh();
        sleep(2);
        *wordFound = tmpWin;
    }
    if(life == 0){
        clear();
        mvprintw(row-1, 0, " YOU LOOSE\n");
        refresh();
        sleep(2);
    }

}

void guess(S_hiddenWord *hiddenWord, char* isWordFound, char *pLife,int row, int col){
    drawBoard(hiddenWord, pLife, row, col);
    char answer = inputPlayer();
    isCharInHiddenWord(answer, hiddenWord, pLife);
    isPlayerWin(hiddenWord, isWordFound, pLife, row);
}

int doYouContinue(int col , int row){
    char reponse[MAX_REPONSE];
    while(1){
        /*fflush(stdout); */
        //fgets doesn't stop at space llike scanf
        char msg_DoContinue[] = "Do you want to continue y/n: ";
        mvprintw(row/2,(col-strlen(msg_DoContinue))/2, "%s", msg_DoContinue);
        refresh();
        //printf("Do you want to continue y/n: ");
        if (getnstr(reponse, MAX_REPONSE) == ERR){
            return 1;
        }
        if (strcspn(reponse, "\n") != strlen(reponse)){
            reponse[strcspn(reponse, "\n")] = '\0';
        }
        for (int i = 0; reponse[i] != '\0'; i++){
            reponse[i]= tolower((unsigned char) reponse[i]);
        }
         // DEBUG: show what was typed and its length
        int len = (int)strlen(reponse);
        mvprintw(row/2 + 1, 0, "DEBUG: [%s], len=%d   ", reponse, len);
        refresh();
        if (strcmp(reponse, "y") == 0){
            return 1;
        }
        if (strcmp(reponse, "n") == 0){
            return 0;
        }
        else{
            mvprintw(row -1, 0, "Invalid input. Please enter 'y' or 'n'.\n");
            refresh();
        }

    }
}

int main(){
    S_text listOfWords;
    int row,col;
    srand((unsigned int)time(NULL));
    readEntries(&listOfWords);
    initscr();
    //cbreak();
    getmaxyx(stdscr,row,col);	
    int keepPlaying = 1;
    do{
        S_hiddenWord choosenWord;
        char wordFound = 0;
        char life = 5;
        chooseWord(&listOfWords, &choosenWord);
        while ((wordFound == 0) && (life != 0)){
                guess(&choosenWord, &wordFound, &life, row,col);
        }

        //printf("CHOOOSEN WORD: %s",choosenWord);
        clear();
        keepPlaying = doYouContinue(col, row);
        clear();
        freeHiddenWord(&choosenWord);
    }while(keepPlaying == 1);
    freeText(&listOfWords);
    endwin();
    return 0;
}

