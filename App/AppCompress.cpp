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
    double frq = 0; //find the smallest
    for (i = 0; i < n; i++) {
        if (huffTree[i].parent != -1)     //determine whether the node is selected
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
    frq = 0; //find the second smallest frequency
    for (i = 0; i < n; i++) {
        if (huffTree[i].parent != -1 || (i == *a))//Exclude the selected number
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
    if (huffTree[*a].lChild < huffTree[*b].lChild)  //put the smaller number one the left
    {

        temp = *a;
        *a = *b;
        *b = temp;
    }
}

Node *buildHuffTree(int *w) {
    Node *huffTree = new Node[256 * 2];
    for (int i = 0; i < 2 * 256 - 1; i++) //Initial process
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
        Select(huffTree, &i1, &i2, k); //Merge node i1，i2 to node k
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
        while (huffTree[j].parent != -1) //Find the root node from the leaf node
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

struct FastMap {
public:
    unsigned char ch;
    int codeLength;
    FastMap(){
        codeLength=-1;
    }
};

FastMap * buildFastMap(int *first, int *codeLength, unordered_map<int, unsigned char> codeDict) {
    int *minValue = new int[32]();
    FastMap * map=new FastMap [65536]();
 
    for (int i = 1; i <= 16; i++) {
        for (int j = 0; j < codeLength[i]; j++) {
			int sp =(first[i] + j + 1) << (16 - i);
            for (int k = (first[i] + j) << (16 - i);k<sp ; k++) {
                unsigned char ch = codeDict[first[i] + j];
                
                map[k].ch = ch;
				map[k].codeLength = i;

            }
        }
    }
    return map;
};
int getBits(unsigned int &buffer, int &bufferSize, unsigned char *input, int &byteForward) {
    //bufferSize : the number of available bits left in buffer，initialized 0，increase when added，decrease when shifted
    //get a piece of bitsfrom an inputget . save the fetched bits in the buffer

    byteForward = 0;
    if (bufferSize < 16) {
        //clear buffer
        buffer >>= (32 - bufferSize);
        //append data
        for (int i = 0; i < 2; i++) {
            buffer <<= 8;
            buffer |= input[0];
            input++;
            byteForward++;
        }
        //append zero
        buffer <<= (16 - bufferSize);
        bufferSize += 16;
    }

    int result = (buffer >> 16);
    return result;
}

bool getBit(unsigned int &buffer, int &bufferSize, unsigned char *input, int &byteForward) {
    byteForward = 0;
    if (bufferSize < 16) {
        //clear buffer
        buffer >>= (32 - bufferSize);
        //append data
        for (int i = 0; i < 2; i++) {
            buffer <<= 8;
            buffer |= input[0];
            input++;
            byteForward++;
        }
        //append zero
        buffer <<= (16 - bufferSize);
        bufferSize += 16;
    }

    bool result = (bool) (buffer >> 31);
    return result;
}


void shiftBuffer(unsigned int &buffer, int &bufferSize, int n) {
    buffer <<= n;
    bufferSize -= n;
}

unsigned char *decode(unsigned char *input, int *first, int n, int &recodeSize,
                      unordered_map<int, unsigned char> codeDict, int *codeLength, unsigned char* output) {

 

    auto fm = buildFastMap(first, codeLength, codeDict);
//    printf("\n%d\n",fm[226]->ch);
    int bitForward = 0;
    int bits = 0;
    unsigned int buffer = 0;
    int bufferSize = 0;

    for (int i = 0; i < n; i++) {
        bits = getBits(buffer, bufferSize, input, bitForward);
        input += bitForward;
        if (fm[bits].codeLength!=-1) {
            output[i] = fm[bits].ch;

            shiftBuffer(buffer, bufferSize, fm[bits].codeLength);

        } else {
			shiftBuffer(buffer, bufferSize,16);
            int len = 16;
            int code = bits;
			unsigned int bufferCpy = buffer;
			int bufferSizeCpy =bufferSize;
			auto inputCpy = input;

			
            while (code - first[len] >= codeLength[len]) {
                code <<= 1;
                bool bit = getBit(bufferCpy, bufferSizeCpy, inputCpy, bitForward);
                inputCpy += bitForward;
				shiftBuffer(bufferCpy, bufferSizeCpy, 1);
                code |= bit;
                len++;
            }
			 getBits(buffer, bufferSize, input, bitForward);
			 input+=bitForward;
            shiftBuffer(buffer, bufferSize, len-16);
            output[i] = codeDict[code];
        }


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
 
 
void decorrelation(unsigned char * reproducedData,int width, int height,unsigned char * pInput){
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
				int C = pInput[((i - 1) * width + (j - 1)) * 3+bgrIndex];
                int predict = C >= max(A, B) ? min(A, B) : C <= min(A, B) ? max(A, B) : A + B - C;

                reproducedData[(i * width + j) * 3 + bgrIndex] = pInput[(i * width + j) * 3 + bgrIndex] - predict;//差值
            }
        }
    }
}

void reCorrelation(unsigned char * uncompressedData,int width, int height){
	  for (int bgrIndex = 0; bgrIndex < 3; bgrIndex++) {
        for (int i = 1; i < height; i++) {
            for (int j = 1; j < width; j++) {
                int A = uncompressedData[(i * width + j - 1) * 3 + bgrIndex];
                int B = uncompressedData[((i - 1) * width + j) * 3 + bgrIndex];
                int C = uncompressedData[((i - 1) * width + (j - 1)) * 3+ bgrIndex];
                int predict = C >= max(A, B) ? min(A, B) : C <= min(A, B) ? max(A, B) : A + B - C;

                uncompressedData[(i * width + j) * 3 + bgrIndex] =
                        uncompressedData[(i * width + j) * 3 + bgrIndex] + predict;
            }
        }
    }
}



// This function compresses input 24-bit image (8-8-8 format, in pInput pointer).
// This function shall allocate storage space for compressedData, and return it as a pointer.
// The input reference variable cDataSize, is also serve as an output variable to indicate the size (in bytes) of the compressed data.
unsigned char *CAppCompress::Compress(int &cDataSize) {
    unsigned char *reproducedData = new unsigned char[width * height * 3];
    //decorrelation according to jpeg standard
	decorrelation(reproducedData,width,height,pInput);

	//calculate frequency
	int *codeLength;
	codeLength = new int[32]();
	int *frq = calcFrequence(reproducedData, width * height*3);

	//establish a Canonical Huffman tree,including codeLength and group index
	Node *huffTree = buildHuffTree(frq);
	//establish HuffCodeLength frequency table
    huffTree = calcHuffCodeLength(huffTree, codeLength);
	//establish FirstCode table for each codeLength
	int *firstCode   = calcFirstCode(codeLength);
	// Assign code according to FirstCode and group index of each symble(in Node.groupIndex）。
	unordered_map<int,unsigned char> codeDict = assignCode(huffTree, firstCode);
	
	//recode，add code length information on the head，（Spatial optimizable，not here）
	int recodeSize = 0;
	unsigned char *recoded = recode(reproducedData,huffTree,width*height*3,recodeSize);
	cDataSize=recodeSize;

	return recoded;
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {

	//recode huffmanTree，which only record codeLength、groupIndex and codeLength frequency table。
	int *codeLength=new int[32]();
	Node * huffTree= rebuildHuffTree(compressedData,codeLength);

	//establish firstCode table
	int* firstCode = calcFirstCode(codeLength);
	
	//Assign code and establish decode table
	unordered_map<int,unsigned char> codeDict = assignCode(huffTree, firstCode);

	compressedData+=256;
	cDataSize -= 256;

	//establish 16-bits fastMap and decode
	decode(compressedData, firstCode, width * height*3, cDataSize, codeDict,codeLength,uncompressedData);
	
	//reCorrelation
	reCorrelation(uncompressedData,width,height);
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
