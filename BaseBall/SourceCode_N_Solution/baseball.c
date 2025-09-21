#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

const char *ball = " ";
const char *strike = "□";
const char *ball_loc = "■";
const char *no_out = "○";
const char *out = "●";
const char *inPerson = "▣";
const char *noPerson = "▢";
char *nextHomeRun = NULL;
int zone_maxX = 7;
int zone_maxY = 7;
int isPlayerTurn = 0;

int ground[] = {0, 0, 0};
int count[] = {0, 0, 0};
int history_player[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int history_mntly[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int totalScore_player = 0;
int totalScore_mntly = 0;
int oneMore = 0;

void print_zone(int x, int y);
void print_Start();
int getRandom();
int changeToXY(int random);
void print_score(int who, int curInning, int tb);
void print_stat();
void inning(int idx);
int checkBSO(int curin, int att_x, int att_y, int def_x, int def_y);

void useful_tip1() {
    __asm__("pop %rdi; pop %rsi; pop %rdx; ret;");
}

void useful_tip2() {
    __asm__("pop %rdi; ret;");
}

void reset() {
    for(int i = 0; i < 3; i++) {
        ground[i] = 0;
        count[i] = 0;    
    }
    for(int i = 0; i < 11; i++) {
        history_player[i] = 0;
        history_mntly[i] = 0;
    }
    totalScore_player = 0;
    totalScore_mntly = 0;
    oneMore = 0;
}

int runBaseball() {
    while(1) {
        char homerun[8] = {0, };
        nextHomeRun = homerun;
        reset();

        print_Start();
        isPlayerTurn = rand() % 2;

        for(int i = 0; i < 11; i++) {
            inning(i);
            printf("Top of the %d(st/nd/rd/th) inning is end.\n", i+1);
            print_score((isPlayerTurn + 1) % 2, i, 0);
            printf("Current HomeRun Player!!\n%s\n", homerun);
            count[2] = 0;
            count[0] = 0;
            count[1] = 0;

            ground[0] = 0;
            ground[1] = 0;
            ground[2] = 0;

            inning(i);
            printf("Bottom of the %d(st/nd/rd/th) inning is end.\n", i+1);
            print_score(isPlayerTurn, i, 1);
            printf("Current HomeRun Player!!\n%s\n", homerun);
            count[2] = 0;
            count[0] = 0;
            count[1] = 0;

            ground[0] = 0;
            ground[1] = 0;
            ground[2] = 0;

            if(i >= 8) {
                int totalScore_player = 0;
                for(int j = 0; j < 11; j++) totalScore_player += history_player[j];
                int totalScore_mntly = 0;
                for(int j = 0; j < 11; j++) totalScore_mntly += history_mntly[j];
                if(totalScore_player > totalScore_mntly){
                    printf("You WIN!\nDo you want play one more? [y:1/n:0]\n>> ");
                    fflush(stdout);
                    scanf("%d", &oneMore);
                    if(oneMore) break;
                    else return 0;
                } else if(totalScore_player < totalScore_mntly){
                    printf("You LOSE!\nDo you want play one more? [y:1/n:0]\n>> ");
                    fflush(stdout);
                    scanf("%d", &oneMore);
                    if(oneMore) break;
                    else exit(1);
                }
            }
        }

        if(!oneMore) {
            if(totalScore_player > totalScore_mntly){
                printf("You WIN!\nDo you want play one more? [y:1/n:0]\n>> ");
                fflush(stdout);
                scanf("%d", &oneMore);
                if(oneMore) break;
                else return 0;
            } else if(totalScore_player < totalScore_mntly){
                printf("You LOSE!\nDo you want play one more? [y:1/n:0]\n>> ");
                fflush(stdout);
                scanf("%d", &oneMore);
                if(oneMore) break;
                else exit(1);
            } else {
                printf("DRAW!\nDo you want play one more? [y:1/n:0]\n>> ");
                fflush(stdout);
                scanf("%d", &oneMore);
                if(oneMore) break;
                else exit(1);
            }
        }
    }
}

int main() {
    srand(time(NULL));
    return runBaseball(); // main에서 바로 실행하게 하고 Blind ROP 될까?
}

void print_zone(int x, int y) {
    printf("Strike Zone\n ");
    for(int i = 0; i < zone_maxY; i++) {
        for(int j = 0; j < zone_maxX; j++) {
            if((x == j) && (y == i)) printf("%s", ball_loc);
            else if((j == 0) || (i == 0) || (j == zone_maxX - 1) || (i == zone_maxY - 1)) printf("%s", ball);
            else printf("%s", strike);
        }
        printf("\n ");
    }
    printf("\n");
}

void print_score(int who, int curInning, int tb) {
    printf("       ");
    for(int i = 1; i < 12; i++) {
        printf("| %d ", i);
    }

    if(who) {
        printf("\nPlayer ");
        for(int i = 0; i < 11; i++) {
            if(i > curInning) printf("| - ");
            else printf("| %d ", history_player[i]);
        }

        printf("\n mntly ");
        for(int i = 0; i < 11; i++) {
            if(i > curInning) printf("| - ");
            else if((!tb) && (i == curInning)) printf("| - ");
            else printf("| %d ", history_mntly[i]);
        }
        printf("\n");
    } else {
        printf("\n mntly ");
        for(int i = 0; i < 11; i++) {
            if(i > curInning) printf("| - ");
            else printf("| %d ", history_mntly[i]);
        }

        printf("\nPlayer ");
        for(int i = 0; i < 11; i++) {
            if(i > curInning) printf("| - ");
            else if((!tb) && (i == curInning)) printf("| - ");
            else printf("| %d ", history_player[i]);
        }

        printf("\n");
    }
}

void print_Start(){
    int a = 0;

    puts("Welome to simple baseball game, BaseBall v1.0!");
    puts("1. Assign your team");
    puts("2. Exit (Real...?)");

    printf(">> ");
    fflush(stdout);
    scanf("%d", &a);

    if(a == 2) {
        printf("Good Bye...\n");
        exit(1);
    }
}

int getRandom() {
    return changeToXY(rand() % 49);
}

int changeToXY(int random) {
    int x = random % 7;
    int y = random / 7;
    return 10 * x + y;
}

void print_stat() {
    printf("Current Stat : ");
    if(ground[0]) printf("%s", inPerson);
    else printf("%s", noPerson);

    if(ground[1]) printf("%s", inPerson);
    else printf("%s", noPerson);

    if(ground[2]) printf("%s", inPerson);
    else printf("%s", noPerson);

    printf(" | %d - %d | ", count[0], count[1]);

    if(count[2] == 0) printf("%s%s | ", no_out, no_out);
    else if(count[2] == 1) printf("%s%s | ", out, no_out);
    else printf("%s%s | ", out, out);

    int totalScore_player = 0;
    for(int j = 0; j < 11; j++) totalScore_player += history_player[j];
    int totalScore_mntly = 0;
    for(int j = 0; j < 11; j++) totalScore_mntly += history_mntly[j];

    if(isPlayerTurn) printf("You (Attack) - %d : mntly - %d\n", totalScore_player, totalScore_mntly);
    else printf("You - %d : mntly (Attack) - %d\n", totalScore_player, totalScore_mntly);
    
}

void inning(int idx) {
    int mntlyRand = 0;
    int playerChoice = 0;
    int isHomerun = 0;
    int score = 0;
    while(count[2] < 3) {
        if(isPlayerTurn) {
            // Player : Attack
            mntlyRand = getRandom();
            printf("Choose where to swing the bat (0 ~ 48)\n >> ");
            fflush(stdout);
            scanf("%d", &playerChoice);
            if((playerChoice < 0) || (playerChoice > 48)) {
                printf("Dirty Player!! You OUT!!!\n");
                count[2] += 1;
                count[0] = 0;
                count[1] = 0;
                continue;
            }
            playerChoice = changeToXY(playerChoice);
            print_zone(playerChoice / 10, playerChoice % 10);
            isHomerun = checkBSO(idx, mntlyRand / 10, mntlyRand % 10, playerChoice / 10, playerChoice % 10);
            if(isHomerun) {
                printf("[!!!] HOMERUN [!!!]\n");
                printf("I want to remember the homerun player!!\nPlease enter his/her name!\n>> ");
                fflush(stdout);
                int len = read(0, nextHomeRun, 9);
                // *(nextHomeRun + len) = ',';
                nextHomeRun += len;
            }
            print_stat();
        } else {
            // Player : Defense
            mntlyRand = getRandom();
            printf("Choose where to throw the ball (0 ~ 48)\n >> ");
            fflush(stdout);
            scanf("%d", &playerChoice);
            if((playerChoice < 0) || (playerChoice > 48)) {
                printf("Dirty Player!! mntly gets HOMERUN!!!\n");
                count[0] = 0;
                count[1] = 0;
                for(int i = 0; i < 3; i++) {
                    score += ground[i];
                    ground[i] = 0;
                }
                score += 1;
                history_player[idx] += score;
                continue;
            }
            playerChoice = changeToXY(playerChoice);
            // I want to my team win!
            if((mntlyRand / 10 < 1) || (mntlyRand % 10 < 1)) {
                mntlyRand = playerChoice;
            }
            print_zone(playerChoice / 10, playerChoice % 10);
            isHomerun = checkBSO(idx, playerChoice / 10, playerChoice % 10, mntlyRand / 10, mntlyRand % 10);
            if(isHomerun) {
                printf("[!!!] HOMERUN [!!!]\n");
                printf("I already remembered the homerun player!!\n Because he/she is my team!\n");
            }
            print_stat();
        }
    }
    isPlayerTurn = (isPlayerTurn + 1) % 2;
}

int checkBSO(int curin, int att_x, int att_y, int def_x, int def_y) {
    int score = 0;
    if((def_x == 0) || (def_x == zone_maxX) || (def_y == 0) || (def_y == zone_maxY)) {
        // Don't swing
        if((att_x == 0) || (att_x == zone_maxX) || (att_y == 0) || (att_y == zone_maxY)) {
            count[0] += 1;
        } else {
            count[1] += 1;
        }
    } else if((att_x == def_x) && (att_y == def_y)) {
        if((att_x == 3) && (att_y == 3)) {
            // HomeRun
            for(int i = 0; i < 3; i++) {
                score += ground[i];
                ground[i] = 0;
            }
            score += 1;
            if(isPlayerTurn) {
                // Player : Attack
                history_player[curin] += score;
            } else {
                // Player : Defense
                history_mntly[curin] += score;
            }

            count[0] = 0;
            count[1] = 0;
            return 1;
        } else if((abs(att_x - 3) <= 1) && (abs(att_y - 3) <= 1)) {
            // Two-Base hit
            if(ground[0] == 1) {
                score += 1;
                ground[0] = 0;
            }
            if(ground[1] == 1) {
                score += 1;
                ground[1] = 0;
            }
            if(ground[2] == 1) {
                ground[0] = 1;
            }
            ground[1] = 1;

            if(isPlayerTurn) {
                // Player : Attack
                history_player[curin] += score;
            } else {
                // Player : Defense
                history_mntly[curin] += score;
            }
            count[0] = 0;
            count[1] = 0;
        } else {
            // hit
            if(ground[0] == 1) {
                score += 1;
                ground[0] = 0;
            }
            if(ground[1] == 1) {
                ground[1] = 0;
                ground[0] = 1;
            }
            if(ground[2] == 1) {
                ground[1] = 1;
            }
            ground[2] = 1;

            if(isPlayerTurn) {
                // Player : Attack
                history_player[curin] += score;
            } else {
                // Player : Defense
                history_mntly[curin] += score;
            }

            count[0] = 0;
            count[1] = 0;
        }
    } else if ((abs(att_x - def_x) <= 1) && (abs(att_y - def_y) <= 1) && (att_x == 3) && (att_y == 3)) {
        // Two-Base hit
        if(ground[0] == 1) {
            score += 1;
            ground[0] = 0;
        }
        if(ground[1] == 1) {
            score += 1;
            ground[1] = 0;
        }
        if(ground[2] == 1) {
            ground[0] = 1;
        }
        ground[1] = 1;

        if(isPlayerTurn) {
            // Player : Attack
            history_player[curin] += score;
        } else {
            // Player : Defense
            history_mntly[curin] += score;
        }

        count[0] = 0;
        count[1] = 0;
    } else if ((abs(att_x - def_x) <= 1) && (abs(att_y - def_y) <= 1)) {
        // hit
        if(ground[0] == 1) {
            score += 1;
            ground[0] = 0;
        }
        if(ground[1] == 1) {
            ground[1] = 0;
            ground[0] = 1;
        }
        if(ground[2] == 1) {
            ground[1] = 1;
        }
        ground[2] = 1;

        if(isPlayerTurn) {
            // Player : Attack
            history_player[curin] += score;
        } else {
            // Player : Defense
            history_mntly[curin] += score;
        }

        count[0] = 0;
        count[1] = 0;
    } else if ((abs(att_x - def_x) <= 2) && (abs(att_y - def_y) == 2)) {
        // foul
        if(count[1] < 2) count[1] += 1;
    } else {
        // Whiff
        count[1] += 1;
    }
    
    if(count[0] >= 4) {
        // Walk
        count[0] = 0;
        count[1] = 0;
        int idx = 2;
        while(ground[idx]) {
            if(idx <= 0) {
                if(isPlayerTurn) {
                    // Player : Attack
                    history_player[curin] += 1;
                } else {
                    // Player : Defense
                    history_mntly[curin] += 1;
                }
                break;
            }
            idx -= 1;
        }
        ground[idx] = 1;
    }
    if(count[1] >= 3) {
        count[0] = 0;
        count[1] = 0;
        count[2] += 1;
    }
    return 0;
}