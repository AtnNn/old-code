// quick example of using cards in c by Etienne Laurin

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

union card {
	short card;
	struct {
		char type;
		char suite;
	};
};

typedef union card *pack;

char *(typeName[14]) = { "Joker", "Ace", "2", "3", "4", "5", "6", "7",
			"8", "9", "10", "Jack", "Queen", "King" };

char *suiteName[5] = { "None", "Hearts", "Spades", "Diamonds", "Clovers" };

void shufflePack(pack pack, int size){
	short t;
	int r;
	if(!size){
		while(pack[size].card)
			size++;
	}
	for(size--;size>1;size--){
		r = rand() % size;
		t = pack[r].card;
		pack[r].card = pack[size].card;
		pack[size].card = t;
	}
}

void resetPack(pack pack){
	int i;
	for(i=0;i<54;i++){
		pack[i].type = (i + 2) / 4;
		pack[i].suite = (i%4) + 1;
	}
	pack[54].card = 0;
}

void printPack(pack pack){
	for(;;pack++){
		printf("%s",typeName[pack->type]);
		if(pack->type)
			printf(" of %s",suiteName[pack->suite]);
		if(pack[1].card)
			printf(", ");
		else break;
	}
	putchar('\n');
}
main(){
	union card pack[55];
	srand(time(NULL));
	resetPack(pack);
	shufflePack(pack,0);
	printPack(pack);
}
