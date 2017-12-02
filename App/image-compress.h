//
// Created by Zizy Zhang on 29/11/2017.
//

#ifndef TESTS_IMAGE_COMPRESS_H
#define TESTS_IMAGE_COMPRESS_H

#endif //TESTS_IMAGE_COMPRESS_H

  

unsigned char *generateRandomChars(int width, int height);

void readBitsTemplate();

void buildTree();

struct Node {
    int frq;// = frq *10 +1
    int parent;
    int lChild;
    int rChild;
    int depth;
    int code;
    int groupIndex;

};


int *calcFrequence(unsigned char *stream, int n);


bool cmp(Node *n1, Node *n2);

void Select(Node huffTree[], int *a, int *b, int n);

Node *buildHuffTree(int *w);

Node *calcHuffCodeLength(Node *huffTree, int *codeLength);

int *calcFirstCode(int *codeLength);

unsigned char *assignCode(Node *huffTree, int *firstCode);

unsigned char *recode(unsigned char *input, Node *huffTree, int n, int &recodeSize);

unsigned char *decode(unsigned char *input, Node *huffTree, int* first, int n, int &recodeSize, unsigned char* codeDict,int *codeLength);

