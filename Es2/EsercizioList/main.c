#include <stdio.h>
#include "list.h"

int main(int argc, char* argv[])
{
	LIST mylist;
	ItemType i;
	
	mylist = NewList();
	
	i.value = 1;
	mylist = EnqueueLast(mylist, i);
	
	i.value = 2;
	mylist = EnqueueLast(mylist, i);
	
	i.value = 3;
	mylist = EnqueueLast(mylist, i);
	
	i.value = 4;
	mylist = EnqueueLast(mylist, i);
	
	i.value = 0.5;
	mylist = EnqueueFirst(mylist, i);
	
	printf("Contenuto della lista: [");
	PrintList(mylist);
	printf("]\n");
	
	mylist = DequeueLast(mylist);
	i.value = 5;
	mylist = EnqueueOrdered(mylist, i);
	
	printf("Contenuto della lista: [");
	PrintList(mylist);
	printf("]\n");
	
	return 0;
}
