#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

void *runnerFib(void* data);

typedef struct _listnode
{
    int data;
    struct _listnode *next;
} ListNode;

typedef struct _list
{
    ListNode *front, *back;
    int size;
} List;

List* createList();
void addBack(List* list, int node);
bool isEmpty(List* list);
void listPrint(List* list);

List *fiblist;

int num;

int main() {
    int res;
    scanf("%d", &num);

    fiblist = createList();

    printf("input : %d", num);

    //num = num - 1;

        pthread_t fithread;

        pthread_create(&fithread, NULL, (void *) runnerFib, NULL);

        pthread_join(fithread, (void **) &res);

        listPrint(fiblist);

    return 0;
}

void *runnerFib(void *data){
    int i;
    int a = 0;
    int b = 1;
    for(i = 0; i < num; i++){
        addBack(fiblist, a);
        a += b;
        b = a - b;
    }

}

List* createList()
{
    List* list = (List*) malloc (sizeof(List));
    list->size = 0;
    list->front = list->back = NULL;

    return list;
}

bool isEmpty(List* list)
{
    assert(list);
    return list->size == 0;
}

void addBack(List* list, int node)
{
    assert(list);
    ListNode* n = (ListNode*) malloc (sizeof(ListNode));
    n->data = node;
    n->next = NULL;

    if (isEmpty(list))  // add first node
    {
        list->front = list->back = n;
    }
    else
    {
        list->back->next = n;
        list->back = n;
    }
    list->size++;
}

void listPrint(List* list){
    ListNode* n;

    n = list->front;
    printf("\nOutput : ");
    while (n)
    {
        printf("%d ", n->data);
        n = n->next;
    }
}