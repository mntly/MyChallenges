#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXNOTES 0x1002

typedef struct {
    int check;
    int leftLength;
    char Name[8];
    char passwd[8];
    char content[100];
} Note;

int choice, idx, totalAccounts, i, check;
int leftLen = 100;
Note notes[MAXNOTES];

void printMenu();
void printAccounts();
int getRandomNum(int bytes);
void init();
void addAccount();
void changeAccount(int *leftLen, int *check, char *name, char *passwd, char **content);
void deleteAccount(int idx);
int checkCheck(int check, int idx);
int getInput(char *buffer, int size);

void useful_tip1() {
    __asm__("pop %rdi; pop %rsi; pop %rdx; ret;");
}

void useful_tip2() {
    __asm__("pop %rdi; ret;");
}

int main() {
    char name[8] = {0};
    char passwd[8] = {0};
    char *content = NULL;
    init();

    puts("Welcome to NotePad v1.0!");
    puts("Initially, there are 2 accounts, mntly and guest.");
    puts("At first, you should login the 'guest' account with the password 'guest'.");

    while (content == NULL) {
        puts("[+] Please enter your name:");
        getInput(name, 8);
        puts("[+] Please enter your password:");
        getInput(passwd, 32);

        for (i = 0; i < totalAccounts; i++) {
            if (!strcmp(notes[MAXNOTES - 1 -i].Name, name)) {
                if (!strcmp(notes[MAXNOTES - 1 -i].passwd, passwd)) {
                    idx = i;
                    content = notes[MAXNOTES - 1 -i].content;
                    leftLen = notes[MAXNOTES - 1 -i].leftLength;
                    check = notes[MAXNOTES - 1 -i].check;
                    printf("[+] Login successful! Welcome %s!\n", name);
                    break;
                } else {
                    printf("[-] Invalid password for user %s. Please try again.\n", name);
                    break;
                }
            }
        }

        if (idx < 0) {
            printf("[-] Invalid user %s. Please try again.\n", name);
        }
    }

    while (1) {
        printMenu();
        scanf("%d", &choice);
        switch (choice) {
            case 1:
                if (checkCheck(check, idx)) {
                    puts("[-] Random check failed or You aren't login yet.");
                    break;
                }
                addAccount();
                break;
            case 2:
                changeAccount(&leftLen, &check, name, passwd, &content);
                break;
            case 3:
                if (checkCheck(check, idx)) {
                    puts("[-] Random check failed or You aren't login yet.");
                    break;
                }
                deleteAccount(idx);
                break;
            case 4:
                printAccounts(idx);
                break;
            case 5:
                if (checkCheck(check, idx)) {
                    puts("[-] Random check failed or You aren't login yet.");
                    break;
                }
                printf("[+] Append Note for %s:\n", name);
                printf("[+] Write Note content with in %d bytes:\n", 100);
                leftLen = 100 - read(0, content + (100-leftLen), 100);
                notes[MAXNOTES - 1 -idx].leftLength = leftLen;
                puts("[+] Note is written successfully.");
                break;
            case 6:
                if (checkCheck(check, idx)) {
                    puts("[-] Random check failed or You aren't login yet.");
                    break;
                }
                printf("[+] Append Note for %s:\n", name);
                printf("[+] Write Note content with in %d bytes:\n", leftLen);
                leftLen -= getInput(content + (100-leftLen), leftLen);
                notes[MAXNOTES - 1 -idx].leftLength = leftLen;
                puts("[+] Note is written successfully.");
                break;
            case 7:
                if (checkCheck(check, idx)) {
                    puts("[-] Random check failed or You aren't login yet.");
                    break;
                }
                printf("[+] Read Note for %s\n[ Content ] %s\n", name, content);
                break;
            case 8:
                if (checkCheck(check, idx)) {
                    puts("[-] Random check failed or You aren't login yet.");
                    break;
                }
                printf("[+] Clear Note for %s:\n", name);
                leftLen = 100;
                *content = '\0';
                notes[MAXNOTES - 1 -idx].leftLength = leftLen;
                puts("[+] Note is cleared successfully.");
                break;
            case 9:
                puts("[*] Thank you for using NotePad! Goodbye!");
                return 0;
            default:
                puts("[-] Invalid choice. Please try again.");
        }
    }

}

void printMenu() {
    puts("1. Add Account");
    puts("2. Change Account");
    puts("3. Delete Account");
    puts("4. Print Accounts");
    puts("5. Write Note");
    puts("6. Append Note");
    puts("7. Read Note");
    puts("8. Clear Note");
    puts("9. Exit");

    puts("Please choose an option:");
}

void printAccounts(int id) {
    puts("Notes Information");

    for(int i = 0; i < totalAccounts; i++) {
        if (i == id) {
            printf("===> User: %s, Notes: %s\n", notes[MAXNOTES - 1 -i].Name, notes[MAXNOTES - 1 -i].content);
        } else {
            printf("     User: %s, Notes: HIDDEN\n", notes[MAXNOTES - 1 -i].Name);
        }
    }
}

int getRandomNum(int bytes) {
    FILE *fp = fopen("/dev/urandom", "rb");
    unsigned char randBuffer[8];
    
    if (!fp) return 1;

    fread(randBuffer, 1, bytes, fp);
    fclose(fp);

    return *(int *)randBuffer;
}

void init() {
    notes[MAXNOTES - 1].check = getRandomNum(4);
    notes[MAXNOTES - 1 ].leftLength = 83;
    strcpy(notes[MAXNOTES - 1 ].Name, "mntly");
    for(int i = 0; i < 8; i++) {
        notes[MAXNOTES - 1 ].passwd[i] = getRandomNum(1);
    }
    strcpy(notes[MAXNOTES - 1 ].content, "Do you want hint?\0");

    notes[MAXNOTES - 2].check = getRandomNum(4);
    notes[MAXNOTES - 2].leftLength = 100;
    strcpy(notes[MAXNOTES - 2].Name, "guest");
    strcpy(notes[MAXNOTES - 2].passwd, "guest");
    notes[MAXNOTES - 2].content[0] = '\0';

    totalAccounts = 2;
    idx = -1;
}

void addAccount() {
    if (totalAccounts >= 10) {
        puts("[-] Maximum number of accounts reached. You should delete account");
        return;
    }

    puts("[+] User Name:");
    getInput(notes[MAXNOTES - 1 - totalAccounts].Name, 8);
    puts("[+] Password:");
    getInput(notes[MAXNOTES - 1 - totalAccounts].passwd, 8);

    notes[MAXNOTES - 1 - totalAccounts].check = getRandomNum(4);
    notes[MAXNOTES - 1 - totalAccounts].leftLength = 100;

    printf("[+] New User, %s, is created.\n", notes[MAXNOTES - 1 -totalAccounts].Name);
    totalAccounts++;    
}

void changeAccount(int *leftLen, int *check, char *name, char *passwd, char **content) {
    int leftchance = 5;
    int new_check = getRandomNum(4);
    char new_name[8];
    char new_passwd[8];
    char *new_content = NULL;

    for (int j = 0; j < leftchance; j++) {
        puts("[+] Which account do you want to change?:");
        getInput(new_name, 8);
        puts("[+] Password:");
        getInput(new_passwd, 32);

        for (int i = 0; i < totalAccounts; i++) {
            if (!strcmp(notes[MAXNOTES - 1 -i].Name, new_name)) {
                if (!strcmp(notes[MAXNOTES - 1 -i].passwd, new_passwd)) {
                    idx = i;
                    *leftLen = notes[MAXNOTES - 1 -i].leftLength;
                    *check = notes[MAXNOTES - 1 -i].check;
                    strcpy(name, new_name);
                    strcpy(passwd, new_passwd);
                    new_content = notes[MAXNOTES - 1 -i].content;
                    printf("[+] Login successful! Welcome %s!\n", name);
                    *content = notes[MAXNOTES - 1 -i].content;
                } else {
                    printf("[-] Invalid password for user %s. Please try again.\n", name);
                    printf("[+] You have %d chances left.\n", leftchance - j - 1);
                    break;
                }
            }
            if(i == totalAccounts - 1) {
                printf("[-] Invalid user %s. Please try again.\n", new_name);
                printf("[+] You have %d chances left.\n", leftchance - j - 1);
            }
        }
    }
    puts("[-] Change Account Failed.");
}

void deleteAccount(int id) {
    if (id == 0) {
        char name[8];
        puts("[+] Which Account do you want to delete?");
        printAccounts(0);
        puts("[+] Enter Corresponding User name:");
        getInput(name, 8);
        for (int i = 0; i < totalAccounts; i++) {
            if (!strcmp(notes[MAXNOTES - 1 -i].Name, name)) {
                deleteAccount(i);
                return;
            }
        }
        printf("[-] User, %s, is not found.\n", name);
    } else if ((id > 0) && (id < totalAccounts)) {
        char name[8];
        strncpy(name, notes[MAXNOTES - 1 -id].Name, 8);

        notes[MAXNOTES - 1 -id].check = 0;
        notes[MAXNOTES - 1 -id].leftLength = 0;
        notes[MAXNOTES - 1 -id].Name[0] = '\0';
        notes[MAXNOTES - 1 -id].passwd[0] = '\0';
        notes[MAXNOTES - 1 -id].content[0] = '\0';

        for (int i = id; i < totalAccounts - 1; i++) {
            notes[MAXNOTES - 1 -i].check = notes[MAXNOTES - 2 - i].check;
            notes[MAXNOTES - 1 -i].leftLength = notes[MAXNOTES - 2 - i].leftLength;
            strncpy(notes[MAXNOTES - 1 -i].Name, notes[MAXNOTES - 2 - i].Name, 8);
            strncpy(notes[MAXNOTES - 1 -i].passwd, notes[MAXNOTES - 2 - i].passwd, 8);
            strncpy(notes[MAXNOTES - 1 -i].content, notes[MAXNOTES - 2 - i].content, 100);
        }
        notes[MAXNOTES - 1 -totalAccounts].check = 0;
        notes[MAXNOTES - 1 -totalAccounts].leftLength = 0;
        notes[MAXNOTES - 1 -totalAccounts].Name[0] = '\0';
        notes[MAXNOTES - 1 -totalAccounts].passwd[0] = '\0';
        notes[MAXNOTES - 1 -totalAccounts].content[0] = '\0';

        totalAccounts--;
        printf("[+] User, %s, is deleted.\n", name);
        idx = -1;
    } else {
        puts("[-] You aren't login yet");
        return;
    }
}

int checkCheck(int check, int idx) {
    if (idx < 0 || idx >= totalAccounts) {
        return -1;
    }
    return notes[MAXNOTES - 1 -idx].check - check;
}

int getInput(char *buffer, int size) {
    char *lineptr = NULL;
    if (read(0, buffer, size) < 0) {
        perror("Read Input Error");
        exit(-1);
    }
    lineptr = strchr(buffer, '\n');
    if (lineptr) {
        *lineptr = '\0';
    }

    return strlen(buffer);
}