#include "StdAfx.h"
#include "AppCompress.h"

CAppCompress::CAppCompress(void)
{
	// Class Constructor
}

CAppCompress::~CAppCompress(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
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

	unsigned char *compressedData ;
    cDataSize = width * height * 3 ;    

    // 存储压缩后的数据,最差的情况尺寸也不会到大于cDataSize * 2
    compressedData = new unsigned char[cDataSize * 2];
    // 实际压缩字符长度
    int compressedSize = 0;

    // 采用分通道游离的方法，按照每个通道相邻像素的重复性进行压缩

	for(int bgrIndex=0;bgrIndex<3;bgrIndex++){
		unsigned short cur = pInput[bgrIndex];// 第一个像素
    	unsigned short repeat = 1;// 重复次数
		for (int i = 1; i < cDataSize / 3; i++)
		{
			unsigned short next = pInput[i * 3 + bgrIndex];// 下一个像素的b
			if (next == cur && repeat < 127)
			{
				++repeat;
				// 如果是最后一个则存储
				if (i == cDataSize / 3 - 1)
				{
					// 存储最后一个b值组
					compressedData[compressedSize] = repeat;
					compressedData[compressedSize + 1] = cur;
					// 增加编码数据长度
					compressedSize += 2;
				}
			}
			else
			{
				// 存储上一个b值组
				compressedData[compressedSize] = repeat;
				compressedData[compressedSize + 1] = cur;
				// 增加编码数据长度
				compressedSize += 2;
				// 换下一种b值
				cur = next;
				repeat = 1;
				// 如果是最后一个
				if (i == cDataSize / 3 - 1)
				{
					// 存储最后一个b值
					compressedData[compressedSize] = 1;
					compressedData[compressedSize + 1] = cur;
					// 增加编码数据长度
					compressedSize += 2;
				}
			}
		}
	}
    

    // 取出压缩后的纯数据
    cDataSize = compressedSize;
    unsigned char *finalData = new unsigned char[cDataSize];
    for (int i = 0; i < cDataSize; i++)
    {
        unsigned char temp = compressedData[i];
        finalData[i] = temp;
    }
    delete compressedData;
    compressedData = finalData;

    return compressedData;
}

// This function takes in compressedData with size cDatasize, and decompresses it into 8-8-8 image.
// The decompressed image data should be stored into the uncompressedData buffer, with 8-8-8 image format
void CAppCompress::Decompress(unsigned char *compressedData, int cDataSize, unsigned char *uncompressedData) {

	 // 寻找g通道和r通道在压缩数据数组中的偏移坐标
    int offset_r = 0, offset_g = 0;
    int pixelCount = 0;
    for (int i = 0; i < cDataSize;)
    {
        int curRpeat = compressedData[i];
        pixelCount += curRpeat;
        i += 2;
        if (pixelCount == width*height)
        {
            offset_g = i;// g通道的开始坐标
        }
        if (pixelCount == width*height * 2)
        {
            offset_r = i;// r通道的开始坐标
        }
    }

    unsigned int b, g, r;
    int repeat;
    // 1.还原b通道
    for (int i = 0, j = 0; i < width*height, j < offset_g; j += 2)
    {
        // 恢复一组重复的b值
        repeat = compressedData[j];
        for (int p = 0; p < repeat; p++)
        {
            int d = compressedData[j + 1];
            uncompressedData[i * 3 + p*3 + 0] = compressedData[j + 1];
        }
        i += repeat;
    }

    // 2.还原g通道
    for (int i = 0, j = offset_g; i < width*height, j < offset_r; j += 2)
    {
        repeat = compressedData[j];
        for (int p = 0; p < repeat; p++)
        {
            int d = compressedData[j + 1];
            uncompressedData[i * 3 + p * 3 + 1] = compressedData[j + 1];
        }
        i += repeat;
    }

    // 3.还原r通道
    for (int i = 0, j = offset_r; i < width*height, j < cDataSize; j += 2)
    {
        repeat = compressedData[j];
        for (int p = 0; p < repeat; p++)
        {
            int d = compressedData[j + 1];
            uncompressedData[i * 3 + p * 3 + 2] = compressedData[j + 1];
        }
        i += repeat;
    }
}


void CAppCompress::Process(void) {

	// Don't change anything within this function.

	int i, cDataSize ;

	unsigned char *compressedData ;
	unsigned char *verifyCompressedData ;

	SetTitle(pOutput, _T("Lossless Decompressed Image")) ;

	compressedData = Compress(cDataSize) ;

	verifyCompressedData = new unsigned char [cDataSize] ;

	memcpy(verifyCompressedData, compressedData, cDataSize) ;

	delete [] compressedData ;

	Decompress(verifyCompressedData, cDataSize, pOutput) ;

	for(i = 0; i < width * height * 3; i++) {
		if(pInput[i] != pOutput[i]) {
 			printf(_T("Caution: Decoded Image is not identical to the Original Image!\r\n")) ;
			break ;
		}
	}

	printf(_T("Original Size = %d, Compressed Size = %d, Compression Ratio = %2.2f\r\n"), width * height * 3, cDataSize, (double) width * height * 3 / cDataSize) ;

	PutDC(pOutput) ;
}
