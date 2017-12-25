#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define MAX_SYMBOLS 256

/*	Generation of Huffman Table	*/
/*      Mouzakitis Nikolaos,Crete 2017.	*/

int symbols;			// total different symbols in file.
char table[MAX_SYMBOLS];	

struct freq {
	char ch;
	int counter;
};

struct node {
	struct node *rc;
	struct node *lc;
	struct freq ff;
};

struct masternode {
	struct masternode *lc, *rc;
	char ch;
	int combo; //0-leaf 1-combined
	int value;
};

void sort(struct freq * sf)	//selection sort.
{
	int i,j,min;
	struct freq tmp;

	for(i = 0; i < symbols-1; i++) {
		
		min = sf[i].counter;
		
		for(j = i+1; j < symbols; j++) {

			if(sf[j].counter < min) {
				min = sf[j].counter;
				tmp = sf[j];					
				sf[j] = sf[i];
				sf[i] = tmp;
			}
		}
	}
}

int count_symbols(FILE *ptr,int *len)	// counts the different symbols in the file.
{	
	int cnt = 0,i = 0,inner;
	char ch;
	
	while(fscanf(ptr,"%c",&ch) != EOF) {

		*len = *len + 1;
		if(cnt == 0) {
			table[0] = ch;
			cnt++;	
		} else {
	
			inner = 0;
	
			for( i = 0; i < cnt; i++) {
	
				if(ch == table[i])
					break;
				else
					inner++;
			}
		
			if(inner == cnt) 
				table[cnt++] = ch;
		}
	}
	*len = *len -1;	//substract the EOF from len also.
	return cnt-1;	// do not count the EOF character.
}

struct masternode * merge_nodes(struct masternode *a,struct masternode *b)
{
	struct masternode *new;
	new = (struct masternode *)malloc(sizeof(struct masternode));

	if(new == NULL) {
		perror("/malloc failed\n");
		exit(-1);
	}

	new->combo = 1;
	new->value = a->value + b->value;
	new->lc = a;
	new->rc = b;
	
	return new;
}
void create_path_mappings(int max_levels,int mappings[][symbols-1])
{
	int i,j,val;
	val = 0;

	for(i = 0; i < max_levels; i++)
		for(j = 0; j < max_levels; j++) {
			mappings[i][j] = val++;
		}

}
void encode(struct masternode *node,struct masternode *root,char *coded)
{
	struct masternode *ptr;
	char *rv;					
	int max_levels = symbols-1,i,j,step,path,decision;
	int mappings[symbols-1][symbols-1];	
	
	create_path_mappings(max_levels,mappings);	// creating every possibility 	/
	rv = (char *)malloc( (symbols)*sizeof(char));

	if(rv == NULL) {
		perror("malloc failed\n");
		exit(-1);
	}

	for( i = 0; i < max_levels; i ++) {
		for( j = 0; j < max_levels; j++) {
		
			ptr = root;
			path = mappings[i][j];
			memset(rv,0,symbols-1);

			for( step = 0; step < max_levels; step++) {
				
				if( (ptr->lc == NULL) && (ptr->rc == NULL) && (ptr == node) ) {
					coded = rv;
					strcpy(coded,rv);
					
					for(int wl = 0; rv[wl] != '\0'; wl++)
						coded[wl] = rv[wl];
					printf("coded:   %c  %s\n",node->ch,coded);
					free(rv);
					return;
			
				}
	
				decision = path & (1<<step); /* 0-lc,1-rc */
	
				if( decision && (ptr->rc != NULL) ) {
					strcat(rv,"1");
					ptr = ptr->rc;

				} else if( (!decision) && (ptr->lc != NULL) ) {
					strcat(rv,"0");
					ptr = ptr->lc;
				} else if(decision && (ptr->rc == NULL)) {
					break;
				} else if( (!decision) && (ptr->lc == NULL) ) {	
					break;
				}
	
				if(ptr == node) {
						coded = rv;
						strcpy(coded,rv);
						
						for(int wl = 0; rv[wl] != '\0'; wl++)
							coded[wl] = rv[wl];
						printf("coded:   %c  %s\n",node->ch,coded);
						free(rv);
						return;
				}
			}
		}
	}
	printf("ABNORMAL EXIT -PROGRAM SHOULD NEVER EXECUTE INSTRUCTIONS HERE i: %d j:%d\n",i,j);
}

void inorder_trv(struct masternode *root,struct masternode *never_change_root)
{
	struct masternode *ptr;
	char *coded;
	ptr = root;
	
	if(ptr->rc == NULL && ptr->lc == NULL) {
		coded = (char *) malloc( (symbols-1) *sizeof(char) );
		
		if(coded == NULL) {
			perror("coded malloc failed.\n");
			exit(-2);
		}
		encode(ptr,never_change_root,coded);
		memset(coded,0,symbols-1);

	}

	if((ptr != NULL) && (ptr->combo != 0)) {
						        	// 	Making sure we pass through 	/
		inorder_trv(ptr->lc,never_change_root);	        // 	every single node on the tree.	/
		inorder_trv(ptr->rc,never_change_root);
	}
}

int main(int argc,char *argv[])
{
	FILE *ptr;
	char ch;
	int i,j,len = 0,index;
	struct node *hnode;

	ptr= fopen("text","r");

	if(ptr == NULL) {
		perror("/fopen failed\n");
		exit(-1);
	}

	symbols = count_symbols(ptr,&len);	
	printf("Symbols counted: %d\n",symbols);

	struct freq symbol_freq[symbols];
	
	for( i = 0; i < symbols; i++) {
		symbol_freq[i].ch = table[i];
		symbol_freq[i].counter = 0;
	}
	
	for(j = 0; j < symbols; j++) {

		fseek(ptr,0L,SEEK_SET);
		
		for( i = 0; i < len; i++) {
			
			fscanf(ptr,"%c",&ch);
			
			if(symbol_freq[j].ch == ch)
				symbol_freq[j].counter++;
		}
	}	

	fclose(ptr);

	/*  we need to sort ascendin' by counter /
	    the records  in symbol_freq table.   */

	sort(symbol_freq);
	
	hnode = (struct node *)malloc(symbols * sizeof(struct node));
	
	if (hnode == NULL) {
		perror("/malloc failed\n");
		exit(-1);
	}

	for( i = 0; i < symbols; i++) {
		hnode[i].rc = hnode[i].lc = NULL;
		hnode[i].ff = symbol_freq[i];
	}

	struct masternode master[symbols][symbols];

	if(master == NULL) {
		perror("/malloc failed\n");
		exit(-1);
	}

	//generic initialization.

	for(int wp = 0; wp < symbols; wp++)
		for(i = 0; i < symbols; i++) {
			master[wp][i].lc = NULL;
	       		master[wp][i].rc = NULL;
			master[wp][i].ch = hnode[i].ff.ch;
			master[wp][i].combo = 0;
			master[wp][i].value = hnode[i].ff.counter;

		}

	index = symbols;

	struct masternode *created;
	int turn = 0;	

	while(index > 1) {		

		created = merge_nodes(&master[turn][0],&master[turn][1]);

		for(i = 0; (master[turn][i].value <= created->value) && (i < index) ; i++)
			;
		i--;
		
		for(j = 2; j <= i; j++) 
			master[turn+1][j-2] = master[turn][j];
		
		i--;
		master[turn+1][i] = *created;
		
		for(j = i+2; j < index; j++)
			master[turn+1][++i] = master[turn][j];

		index--;	
		turn++;
	}
	
	inorder_trv(master[symbols-1],master[symbols-1]);

	free(hnode);
	
	return (0);
}
