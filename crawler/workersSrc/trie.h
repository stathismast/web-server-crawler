#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Contains the term frequency (count) for a given document id as well as a pointer to the next posting list node
typedef struct PostingListNode{
    int id;                            //Document id
    int count;                        //Term frequency in the given document
    struct PostingListNode * next;    //Next posting list node
} PostingListNode;

//Head of a posting list. Every posting lists starts with this.
//It contains the document frequency of the word and a pointer the first posting list node
typedef struct PostingListHead{
    int totalCount;                //Document frequency of the word
    struct PostingListNode * next;    //Pointer to the first posting list node
    struct PostingListNode * last;    //Pointer to the last node of the list
} PostingListHead;

typedef struct TrieNode{
    char letter;                    //Character/letter of the node
    struct TrieNode * otherLetter;    //A letter of a different word
    struct TrieNode * nextLetter;    //The letter following this one
    PostingListHead * postingList;    //Posting list for the word ending in this letter
} TrieNode;


PostingListHead * newPostingListHead();
PostingListNode * newPostingListNode(int id);
void freePostingList(PostingListHead * head);
void freePostingListNode(PostingListNode * node);
void addToPostingList(int id, PostingListHead ** head);
PostingListNode ** getPosting(int id, PostingListNode ** node);

TrieNode * newTrieNode(char letter);
void freeTrie(TrieNode * trie);
int findNextWord(int * start, int * end, char * line);
void addWord(char * word, int id, TrieNode ** node);
int addWordsIntoTrie(char * line, int id, TrieNode ** trie);
PostingListHead * getPostingList(char * word, TrieNode * node);
