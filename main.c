#include <cbm.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ITEMS 20
#define MAX_NAME 16
#define MAX_OPTIONS 5
#define MAX_OPTIONS2 3
#define MAX_SAVE_ATTEMPTS 5

const char *options[MAX_OPTIONS] = {
    "Pridat zbozi",
    "Vypsat seznam",
    "Upravit polozku",
    "Smazat polozku",
    "Konec"
};

const char *options2[MAX_OPTIONS2] = {
    "Smazat podle ID",
    "Smazat podle nazvu",
    "Zpet"
};

typedef struct {
    unsigned char id;
    char name[MAX_NAME];
    unsigned char price;
    unsigned char stock;
} Item;

Item items[MAX_ITEMS];

// ------------------- MENU -------------------

int menu(const char *opts[], int n) {
    int selected = 0;
    unsigned char key;
    int i;

    clrscr();
    while(1) {
        gotoxy(0,1);
        cputs("Adava Software: Pokladna: Menu");
        for(i=0;i<n;i++) {
            gotoxy(1,i+3);
            cputc(i==selected?'>':' ');
            cprintf(" %s", opts[i]);
        }

        key = cgetc();
        if(key==145 && selected>0) selected--;
        else if(key==17 && selected<n-1) selected++;
        else if(key==13) return selected;
    }
}

// ------------------- RAM OPERACE -------------------

void load_items(void) {
    int handle = 1;
    int idx = 0;
    unsigned char c;
    char buffer[64];
    int buf_idx = 0;

    memset(items,0,sizeof(items));

    if(cbm_open(handle, 8, CBM_READ, "items,s,r")) {
        cputs("Chyba pri otevreni souboru!\n");
        return;
    }

    while(cbm_read(handle, &c, 1) > 0 && idx < MAX_ITEMS) {
        if(c == '\r' || c == '\n') {
            buffer[buf_idx]='\0';
            buf_idx=0;

            if(strcmp(buffer,"END")==0) break;

            sscanf(buffer,"%hhu,%15[^,],%hhu,%hhu",
                   &items[idx].id,
                   items[idx].name,
                   &items[idx].price,
                   &items[idx].stock);
            idx++;
        } else {
            if(buf_idx < sizeof(buffer)-1) buffer[buf_idx++] = c;
        }
    }

    cbm_close(handle);

    if(idx == MAX_ITEMS && strcmp(buffer,"END")!=0) {
        cputs("Varovani: Soubor mohl byt poskozen!\n");
    } else {
        cputs("Polozky nacteny.\n");
    }
}

void save_all_items(void) {
    int attempt;
    char line[64];
    int i, success;

    for(attempt=0; attempt<MAX_SAVE_ATTEMPTS; attempt++) {
        // nejprve smažeme starý soubor
        // smažeme soubor items
        if (cbm_open(15, 8, CBM_WRITE, "S:items") == 0) {
            cbm_close(15);
        }


        // otevřeme nový soubor pro zápis
        if(cbm_open(1, 8, CBM_WRITE, "items,s,w")) {
            cputs("Chyba pri otevreni souboru pro zapis!\n");
            continue;
        }

        // zapíšeme všechny položky
        for(i=0;i<MAX_ITEMS && items[i].id!=0;i++) {
            int len = sprintf(line,"%d,%s,%d,%d",
                              items[i].id,
                              items[i].name,
                              items[i].price,
                              items[i].stock);
            cbm_write(1,line,len);
            cbm_write(1,"\r",1);
        }

        // zapíšeme END sentinel
        cbm_write(1,"END\r",4);

        cbm_close(1);

        // ověření
        load_items();
        success = 1;
        for(i=0;i<MAX_ITEMS && items[i].id!=0;i++) {
            if(strlen(items[i].name)==0 || items[i].price==0 && items[i].stock==0) {
                success = 0;
                break;
            }
        }

        if(success) {
            cputs("Ukladani probehlo uspesne.\n");
            return;
        } else {
            cputs("Chyba pri kontrole zapisu, znovu...\n");
        }
    }

    cputs("CHYBA: Ukladani se nezdarilo!\n");
}

void save_item(const char *name,unsigned char price,unsigned char stock) {
    int i,slot=0;
    unsigned char max_id=0;

    while(slot<MAX_ITEMS && items[slot].id!=0) slot++;
    for(i=0;i<MAX_ITEMS && items[i].id!=0;i++)
        if(items[i].id>max_id) max_id=items[i].id;

    items[slot].id = max_id+1;
    strncpy(items[slot].name,name,MAX_NAME-1);
    items[slot].name[MAX_NAME-1]='\0';
    items[slot].price=price;
    items[slot].stock=stock;

    printf("Polozka ulozena ");
    cputc(186);
    printf("\n");
}

void delete_item_id(unsigned char id) {
    int i,j;
    for(i=0;i<MAX_ITEMS && items[i].id!=0;i++) {
        if(items[i].id==id) {
            for(j=i;j<MAX_ITEMS-1;j++) {
                items[j]=items[j+1];
                if(items[j].id==0) break;
            }
            break;
        }
    }
    for(i=0;i<MAX_ITEMS && items[i].id!=0;i++) items[i].id=i+1;

    printf("Polozka smazana ");
    cputc(186);
    printf("\n");
}

void delete_item_name(const char *name) {
    int i,j;
    for(i=0;i<MAX_ITEMS && items[i].id!=0;i++) {
        if(strcmp(items[i].name,name)==0) {
            for(j=i;j<MAX_ITEMS-1;j++) {
                items[j]=items[j+1];
                if(items[j].id==0) break;
            }
            break;
        }
    }
    for(i=0;i<MAX_ITEMS && items[i].id!=0;i++) items[i].id=i+1;

    printf("Polozka smazana ");
    cputc(186);
    printf("\n");
}

void print_items(void) {
    int i, found=0;
    for(i=0;i<MAX_ITEMS;i++) {
        if(items[i].id!=0 && items[i].name[0]!='\0') {
            printf("%d: %s, Cena %d, Mnozstvi %d\n",
                   items[i].id,
                   items[i].name,
                   items[i].price,
                   items[i].stock);
            found=1;
        }
    }
    if(!found) cputs("Zadne polozky.\n");
}

// ------------------- MAIN -------------------

int main(void) {
    char name[MAX_NAME];
    unsigned char price,stock,id;
    int choice,choice2;

    load_items();

    while(1) {
        clrscr();
        choice = menu(options,MAX_OPTIONS);

        if(choice==0) {
            clrscr();
            printf("Nazev: "); scanf("%15s",name);
            printf("Cena: "); scanf("%hhu",&price);
            printf("Mnozstvi: "); scanf("%hhu",&stock);
            save_item(name,price,stock);
            cgetc();
        } else if(choice==1) {
            clrscr();
            printf("Seznam zbozi:\n");
            print_items();
            cgetc();
        } else if(choice==2) {
            clrscr();
            printf("Nic\n");
            cgetc();
        } else if(choice==3) {
            choice2 = menu(options2,MAX_OPTIONS2);
            if(choice2==0) {
                clrscr();
                printf("ID: "); scanf("%hhu",&id);
                delete_item_id(id);
                cgetc();
            } else if(choice2==1) {
                clrscr();
                printf("Nazev: "); scanf("%15s",name);
                delete_item_name(name);
                cgetc();
            } else if(choice2==2) continue;
        } else if(choice==4) {
            break;
        }
    }

    save_all_items(); // uloží vše najednou na disketu

    clrscr();
    printf("Pro pripadne chyby BASICu resetujte C64.\n");
    printf("Hezky den!\n");
    return 0;
}
