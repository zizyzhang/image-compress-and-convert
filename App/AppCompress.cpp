#include "StdAfx.h"
#include "AppCompress.h"
#include <string>
#include <map>
#include <cmath>
#include <bitset>

#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#define length(array, len){len = (sizeof(array) / sizeof(array[0]));}

using namespace std;

struct Node {
    double weight;
    string ch;
    string code;
    int lchild, rchild, parent;
};

string *huffMap = new string[256];



void Select(Node huffTree[], int *a, int *b, int n)//找权值最小的两个a和b
{
    int i;
    double weight = 0; //找最小的数
    for (i = 0; i < n; i++) {
        if (huffTree[i].parent != -1)     //判断节点是否已经选过
            continue;
        else {
            if (weight == 0) {
                weight = huffTree[i].weight;
                *a = i;
            } else {
                if (huffTree[i].weight < weight) {
                    weight = huffTree[i].weight;
                    *a = i;
                }
            }
        }
    }
    weight = 0; //找第二小的数
    for (i = 0; i < n; i++) {
        if (huffTree[i].parent != -1 || (i == *a))//排除已选过的数
            continue;
        else {
            if (weight == 0) {
                weight = huffTree[i].weight;
                *b = i;
            } else {
                if (huffTree[i].weight < weight) {
                    weight = huffTree[i].weight;
                    *b = i;
                }
            }
        }
    }
    int temp;
    if (huffTree[*a].lchild < huffTree[*b].lchild)  //小的数放左边
    {
        temp = *a;
        *a = *b;
        *b = temp;
    }
}

void Huff_Tree(Node huffTree[], int w[], string ch[], int n) {
    for (int i = 0; i < 2 * n - 1; i++) //初始过程
    {
        huffTree[i].parent = -1;
        huffTree[i].lchild = -1;
        huffTree[i].rchild = -1;
        huffTree[i].code = "";
    }
    for (int i = 0; i < n; i++) {
        huffTree[i].weight = w[i];
        huffTree[i].ch = ch[i];
    }
    for (int k = n; k < 2 * n - 1; k++) {
        int i1 = 0;
        int i2 = 0;
        Select(huffTree, &i1, &i2, k); //将i1，i2节点合成节点k
        huffTree[i1].parent = k;
        huffTree[i2].parent = k;
        huffTree[k].weight = huffTree[i1].weight + huffTree[i2].weight;
        huffTree[k].lchild = i1;
        huffTree[k].rchild = i2;
    }
}

map<string,int> rebuildHuffTree(string &data,int &offset){
	offset = 0;
    map<string,int> deCodeMap;
    for (int i = 0; i < 256; i++) {
        bitset<8> codeLengthBits;
        for(int j=0;j<8;j++){
            codeLengthBits[7-j]  =data[offset+j]-'0';
        }
        offset+=8;

        string code="";
        for(int j=0;j<codeLengthBits.to_ulong();j++){
            code+=data[offset+j];
        }

        deCodeMap[code] = i;

        offset+=codeLengthBits.to_ulong();

    }
    return deCodeMap;
}

void Huff_Code(Node huffTree[], int n) {
    int i, j, k;
    string s = "";
    for (int i2 = 0; i2 < 256; i2++) {
        huffMap[i2] = "";
    }

    for (i = 0; i < n; i++) {
        s = "";
        j = i;
        while (huffTree[j].parent != -1) //从叶子往上找到根节点
        {
            k = huffTree[j].parent;
            if (j == huffTree[k].lchild) //如果是根的左孩子，则记为0
            {
                s = s + "0";
            } else {
                s = s + "1";
            }
            j = huffTree[j].parent;
        }
        for (int l = s.size() - 1; l >= 0; l--) {
            huffTree[i].code += s[l]; //保存编码
        }
        huffMap[stoi(huffTree[i].ch)] = huffTree[i].code;
    }
}

unsigned char *Huff_Decode(Node huffTree[], int n, string s, unsigned char *res,int offset,map<string, int> deCodeMap) {
    string temp = "", str = "";//保存解码后的字符串

    int res_length = 0;
    for (int i = offset; i < s.size(); i++) {
        temp.append(s[i] == '0' ? "0" : "1");

        if (deCodeMap.find(temp) != deCodeMap.end()) {
            res[res_length] = deCodeMap[temp];
            temp = "";
            res_length++;

        } else if (i == s.size() - 1 && temp != "")//全部遍历后没有
        {
            str = "wrong decode";
        }

    }
    return res;
}

int *calcFrequence(unsigned char s[], int s_length) {

    int *freq = new int[256];
    for (int i = 0; i < 256; i++) {
        freq[i] = 0;
    }


    for (int i = 0; i < s_length; i++) {
        int ch = (int) s[i];
        freq[ch]++;
    }

    return freq;

}


string recode(unsigned char s[], int s_len) {
    //[(256 bits of huffMap)]
    string res = "";
    for (int i = 0; i < 256; i++) {
        int code_length =huffMap[i].size();

        bitset<8> code_length_bits(code_length);
        res += code_length_bits.to_string();

        res+=huffMap[i];

    }

    for (int i = 0; i < s_len; i++) {
        res += huffMap[(int) s[i]];
    }
    return res;

}

unsigned char *encodeFromBits(string &s, unsigned char res[]) {
    string sub = s.substr(s.size() - 8, 8);
    int index = 0;



    for (int cur = 0; cur < s.size(); cur += 8, index++) {
        if (cur + 8 >= s.size()) {

            sub = s.substr(cur, s.size() - cur);
            bitset<8> bs2(sub);
            res[index] = (int) bs2.to_ulong();
            int lengthLast = s.size() - cur;
            bitset<8> lengthFlag(lengthLast);
            res[index + 1] = (int) lengthFlag.to_ulong();
            break;
        } else {

            sub = s.substr(cur, 8);
            bitset<8> bs(sub);
            res[index] = (char) bs.to_ulong();

        }
    }

    return res;

}

string decodeFromUnsignedChars(unsigned char *chars, int n) {
    string res = "";
    for (int i = 0; i < n; i++) {
        if (i == n - 1) {
            int lastBitsize = chars[i + 1];

            bitset<8> v(chars[i]);
            res += v.to_string().substr(8 - lastBitsize, lastBitsize);
            break;
        } else {
            int ch = chars[i];
            bitset<8> v(ch);
            res += v.to_string();
        }

    }
    return res;


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


Node *huffTreeG;

// This function compresses input 24-bit image (8-8-8 format, in pInput pointer).
// This function shall allocate storage space for compressedData, and return it as a pointer.
// The input reference variable cDataSize, is also serve as an output variable to indicate the size (in bytes) of the compressed data.
unsigned char *CAppCompress::Compress(int &cDataSize) {
    unsigned char *compressedData;
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


    cDataSize = width * height * 3;
    compressedData = new unsigned char[cDataSize * 2];

    string strs[256] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16",
                        "17", "18", "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32",
                        "33", "34", "35", "36", "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47", "48",
                        "49", "50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "60", "61", "62", "63", "64",
                        "65", "66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "80",
                        "81", "82", "83", "84", "85", "86", "87", "88", "89", "90", "91", "92", "93", "94", "95", "96",
                        "97", "98", "99", "100", "101", "102", "103", "104", "105", "106", "107", "108", "109", "110",
                        "111", "112", "113", "114", "115", "116", "117", "118", "119", "120", "121", "122", "123",
                        "124", "125", "126", "127", "128", "129", "130", "131", "132", "133", "134", "135", "136",
                        "137", "138", "139", "140", "141", "142", "143", "144", "145", "146", "147", "148", "149",
                        "150", "151", "152", "153", "154", "155", "156", "157", "158", "159", "160", "161", "162",
                        "163", "164", "165", "166", "167", "168", "169", "170", "171", "172", "173", "174", "175",
                        "176", "177", "178", "179", "180", "181", "182", "183", "184", "185", "186", "187", "188",
                        "189", "190", "191", "192", "193", "194", "195", "196", "197", "198", "199", "200", "201",
                        "202", "203", "204", "205", "206", "207", "208", "209", "210", "211", "212", "213", "214",
                        "215", "216", "217", "218", "219", "220", "221", "222", "223", "224", "225", "226", "227",
                        "228", "229", "230", "231", "232", "233", "234", "235", "236", "237", "238", "239", "240",
                        "241", "242", "243", "244", "245", "246", "247", "248", "249", "250", "251", "252", "253",
                        "254", "255"};
    int *weights = calcFrequence(reproducedData, cDataSize);
    int strs_len = 0;
    string recodedStr = "";

    length(strs, strs_len);

    Node *huffTree = new Node[strs_len * 2];
    huffTreeG = huffTree;
    Huff_Tree(huffTree, weights, strs, strs_len);
    Huff_Code(huffTree, strs_len);
    recodedStr = recode(reproducedData, cDataSize);

    // string uncompressedStr = Huff_Decode(huffTree, strs_len, recodedStr);

    cDataSize = (int) ceil(recodedStr.size() / 8.0) + 1;
    return encodeFromBits(recodedStr, new unsigned char[(int) ceil(recodedStr.size() / 8.0) + 1]);
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {


    string res = decodeFromUnsignedChars(compressedData, cDataSize - 1);
	int offset;
    unsigned char *uncompressedStr = Huff_Decode(huffTreeG, 256 * 2, res, uncompressedData,offset,rebuildHuffTree(res,offset));
    for (int bgrIndex = 0; bgrIndex < 3; bgrIndex++) {
        for (int i = 1; i < height; i++) {
            for (int j = 1; j < width; j++) {
                int A = pInput[(i * width + j - 1) * 3 + bgrIndex];
                int B = pInput[((i - 1) * width + j) * 3 + bgrIndex];
                int C = pInput[((i - 1) * width + (j - 1)) * 3];
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

    delete[] compressedData;

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
