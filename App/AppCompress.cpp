#include "StdAfx.h"
#include "AppCompress.h"
#include <string>
#include <map>
#include <cmath>
#include <bitset>
 #include <unordered_map>

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define length(array, len){len = (sizeof(array) / sizeof(array[0]));}

using namespace std;


struct Node {
    int frq;// = frq *10 +1
    int parent;
    int lChild;
    int rChild;
    int depth;
    int code;
    int groupIndex;

};
//string *huffMap = new string[256];

   
int *calcFrequence(unsigned char *stream, int n) {
    auto *map = new int[256]();
    for (int i = 0; i < n; i++) {
        map[stream[i]]++;
    }
    return map;
}

void Select(Node huffTree[], int *a, int *b, int n) {
    int i;
    double frq = 0; //找最小的数
    for (i = 0; i < n; i++) {
        if (huffTree[i].parent != -1)     //判断节点是否已经选过
            continue;
        else {
            if (frq == 0) {
                frq = huffTree[i].frq;
                *a = i;
            } else {
                if (huffTree[i].frq < frq) {
                    frq = huffTree[i].frq;
                    *a = i;
                }
            }
        }
    }
    frq = 0; //找第二小的数
    for (i = 0; i < n; i++) {
        if (huffTree[i].parent != -1 || (i == *a))//排除已选过的数
            continue;
        else {
            if (frq == 0) {
                frq = huffTree[i].frq;
                *b = i;
            } else {
                if (huffTree[i].frq < frq) {
                    frq = huffTree[i].frq;
                    *b = i;
                }
            }
        }
    }
    int temp;
    if (huffTree[*a].lChild < huffTree[*b].lChild)  //小的数放左边
    {

        temp = *a;
        *a = *b;
        *b = temp;
    }
}

Node *buildHuffTree(int *w) {
    Node *huffTree = new Node[256 * 2];
    for (int i = 0; i < 2 * 256 - 1; i++) //初始过程
    {
        huffTree[i].parent = -1;
        huffTree[i].lChild = -1;
        huffTree[i].rChild = -1;
		huffTree[i].depth =0;
		huffTree[i].frq=0;
		huffTree[i].groupIndex=0;
		 

    }
    for (int i = 0; i < 256; i++) {
        huffTree[i].frq = w[i] * 10 + 1;
    }
    for (int k = 256; k < 2 * 256 - 1; k++) {
        int i1 = 0;
        int i2 = 0;
        Select(huffTree, &i1, &i2, k); //将i1，i2节点合成节点k
        huffTree[i1].parent = k;
        huffTree[i2].parent = k;
        huffTree[k].frq = huffTree[i1].frq + huffTree[i2].frq;
        huffTree[k].lChild = i1;
        huffTree[k].rChild = i2;
    }
    return huffTree;
}

Node *calcHuffCodeLength(Node *huffTree, int *codeLength) {
    int i, j, k;

    for (i = 0; i < 256; i++) {
        j = i;
        while (huffTree[j].parent != -1) //从叶子往上找到根节点
        {
            huffTree[i].depth++;
            j = huffTree[j].parent;
        }
        huffTree[i].groupIndex = codeLength[huffTree[i].depth];
        codeLength[huffTree[i].depth]++;
    }

    return huffTree;
}

Node *rebuildHuffTree(unsigned char * input, int* codeLength){
	Node *huffTree = new Node[256];

	for(int i=0;i<256;i++){
		huffTree[i].depth=input[i];
		huffTree[i].groupIndex =codeLength[huffTree[i].depth];
		codeLength[huffTree[i].depth]++;

	}
	return huffTree;
}

int *calcFirstCode(int *codeLength) {
    //Assign Code
    int *firstCode = new int[32]();
    int minLength = -1;
    //  firstCode
    int lastFirstCodeIndex = 0;
    for (int i = 0; i < 32; i++) {
        if (codeLength[i] == 0)continue;
        if (minLength == -1) {
            minLength = i;
            lastFirstCodeIndex = i;
            firstCode[i] = 0;
        } else {
            firstCode[i] = (firstCode[lastFirstCodeIndex] + codeLength[lastFirstCodeIndex]) << (i - lastFirstCodeIndex);
            lastFirstCodeIndex=i;
        }

    }
    return firstCode;
}

unordered_map<int,unsigned char> assignCode(Node *huffTree, int *firstCode) {
	unordered_map<int,unsigned char> codeDict;
    for (int i = 0; i < 256; i++) {
        huffTree[i].code = firstCode[huffTree[i].depth] + huffTree[i].groupIndex;
        codeDict[firstCode[huffTree[i].depth] + huffTree[i].groupIndex] = i;
    }
    return codeDict;
}

unsigned char *recode(unsigned char *input, Node *huffTree, int n,int &recodeSize) {
    int outputBuffer=0;
    int bitCount = 0;

    recodeSize=0;
    for (int i = 0; i < n; i++) {
        recodeSize+=huffTree[input[i]].depth;
    }
    recodeSize = ceil(recodeSize/8)+1+256;
    unsigned char* output=new unsigned char[recodeSize];
    int byteCount=256;

	for(int i=0;i<256;i++){
		output[i]=huffTree[i].depth;
	}

    for (int i = 0; i < n; i++) {
        int code = huffTree[input[i]].code;
        bitset<32> codeBits(code);
        for (int j = 0; j < huffTree[input[i]].depth; j++) {
            int bit = codeBits[huffTree[input[i]].depth-1-j];


            outputBuffer = (outputBuffer<<1)^bit;
            if (++bitCount == 8) {
                output[byteCount++] = outputBuffer;
                outputBuffer=0;
                bitCount = 0;
            }
            if (i == n - 1 &&j==huffTree[input[i]].depth-1&& bitCount<8) {
                outputBuffer<<=(8-bitCount);
                output[byteCount++] = outputBuffer;
            }
        }
    }


    return output;
}

unsigned char *decode(unsigned char *input, int* first, int n, int &recodeSize, unordered_map<int,unsigned char>  codeDict,int *codeLength,unsigned char * output){

 
    int bitIndex=0;
    int byteIndex =0;

	int lMax=0;
	int*minValue = new int[32]();
	for(lMax=31;lMax>=0;lMax--){
		if(codeLength[lMax]>0)break;
	}
 
	for(int len=1; len<32; len++){
		minValue[len] = first[len] << (lMax-len);
	}
   

    for (int i = 0; i < n; i++) {
        int len = 1;
        bool bit =((input[byteIndex] >> (7-bitIndex)) & 1);
        if (bitIndex < 7) {
            bitIndex++;
        }else{
            bitIndex=0;
            byteIndex++;
        }
        int code = bit;

        while(code-first[len]>=codeLength[len])
        {
            code <<= 1;
            bool bit =((input[byteIndex] >> (7-bitIndex)) & 1);
            if (bitIndex < 7) {
                bitIndex++;
            }else{
                bitIndex=0;
                byteIndex++;
            }
            code |=bit;
            len++;
        }
        len--;
        output[i]= codeDict[code];

    }
    return output;
}

CAppCompress::CAppCompress(void) {
    // Class Constructor
}

CAppCompress::~CAppCompress(void) {
    // Class Destructor
    // Must call Final() function in the base class

    Final();
}

void CAppCompress::CustomInit(CView *pView) {
    // Add custom initialization code here
    // This initialization code will be called when this application is added to a processing task lists
}

void CAppCompress::CustomFinal(void) {
    // Add custom finalization code here
}
 
 


// This function compresses input 24-bit image (8-8-8 format, in pInput pointer).
// This function shall allocate storage space for compressedData, and return it as a pointer.
// The input reference variable cDataSize, is also serve as an output variable to indicate the size (in bytes) of the compressed data.
unsigned char *CAppCompress::Compress(int &cDataSize) {
    unsigned char *reproducedData = new unsigned char[width * height * 3];
    //第一行和第一排赋值
    for (int bgrIndex = 0; bgrIndex < 3; bgrIndex++) {
        for (int i = 0; i < width; i++) {
            reproducedData[i * 3 + bgrIndex] = pInput[i * 3 + bgrIndex];
        }
        for (int i = 0; i < height; i++) {
            reproducedData[i * width * 3 + bgrIndex] = pInput[i * width * 3 + bgrIndex];
        }
    }
    for (int bgrIndex = 0; bgrIndex < 3; bgrIndex++) {
        for (int i = 1; i < height; i++) {
            for (int j = 1; j < width; j++) {
                int A = pInput[(i * width + j - 1) * 3 + bgrIndex];
                int B = pInput[((i - 1) * width + j) * 3 + bgrIndex];
                int C = pInput[((i - 1) * width + (j - 1)) * 3];
                int predict = C >= max(A, B) ? min(A, B) : C <= min(A, B) ? max(A, B) : A + B - C;

                reproducedData[(i * width + j) * 3 + bgrIndex] = pInput[(i * width + j) * 3 + bgrIndex] - predict;//差值
            }
        }
    }
	int *codeLength;
	codeLength = new int[32]();
	int *frq = calcFrequence(reproducedData, width * height*3);

	Node *huffTree = buildHuffTree(frq);
    huffTree = calcHuffCodeLength(huffTree, codeLength);
	int *firstCode   = calcFirstCode(codeLength);
	unordered_map<int,unsigned char> codeDict = assignCode(huffTree, firstCode);
	int recodeSize = 0;
	unsigned char *recoded = recode(reproducedData,huffTree,width*height*3,recodeSize);
	cDataSize=recodeSize;

	return recoded;
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {

	int *codeLength=new int[32]();
	Node * huffTree= rebuildHuffTree(compressedData,codeLength);
	int* firstCode = calcFirstCode(codeLength);
	unordered_map<int,unsigned char> codeDict = assignCode(huffTree, firstCode);

	compressedData+=256;
	cDataSize -= 256;

	decode(compressedData, firstCode, width * height*3, cDataSize, codeDict,codeLength,uncompressedData);
    for (int bgrIndex = 0; bgrIndex < 3; bgrIndex++) {
        for (int i = 1; i < height; i++) {
            for (int j = 1; j < width; j++) {
                int A = uncompressedData[(i * width + j - 1) * 3 + bgrIndex];
                int B = uncompressedData[((i - 1) * width + j) * 3 + bgrIndex];
                int C = uncompressedData[((i - 1) * width + (j - 1)) * 3];
                int predict = C >= max(A, B) ? min(A, B) : C <= min(A, B) ? max(A, B) : A + B - C;

                uncompressedData[(i * width + j) * 3 + bgrIndex] =
                        uncompressedData[(i * width + j) * 3 + bgrIndex] + predict;
            }
        }
    }
}


void CAppCompress::Process(void) {

    // Don't change anything within this function.

    int i, cDataSize;

    unsigned char *compressedData;
    unsigned char *verifyCompressedData;

    SetTitle(pOutput, _T("Lossless Decompressed Image"));

    compressedData = Compress(cDataSize);

    verifyCompressedData = new unsigned char[cDataSize];

    memcpy(verifyCompressedData, compressedData, cDataSize);

   //delete[] compressedData;

    Decompress(verifyCompressedData, cDataSize, pOutput);

    for (i = 0; i < width * height * 3; i++) {
        if (pInput[i] != pOutput[i]) {
            printf(_T("Caution: Decoded Image is not identical to the Original Image!\r\n"));
            break;
        }
    }

    printf(_T("Original Size = %d, Compressed Size = %d, Compression Ratio = %2.2f\r\n"), width * height * 3, cDataSize,
           (double) width * height * 3 / cDataSize);

    PutDC(pOutput);
}
