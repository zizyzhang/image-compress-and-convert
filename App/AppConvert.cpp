#include "StdAfx.h"
#include "AppConvert.h"
#include <math.h>

CAppConvert::CAppConvert(void)
{
	// Class Constructor
	mode = 0 ;
}

CAppConvert::~CAppConvert(void)
{
	// Class Destructor
	// Must call Final() function in the base class

	Final() ;
}

void CAppConvert::CustomInit(CView *pView) {
	// Add custom initialization code here
	// This initialization code will be called when this application is added to a processing task lists
	if(mode == 0) {
		CreateTunnel(width, height, &pOutput420) ;
	}
}

void CAppConvert::CustomFinal(void) {
	// Add custom finalization code here

	RemoveTunnel(pOutput420) ;
}

double round(double r)
{
    return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

// This function converts input RGB image to a YUV image.
void CAppConvert::RGBtoYUV(unsigned char *pRGB, unsigned char *pYUV) {

	// You should implement RGB to YUV conversion here.


	// Sample code start - You may delete these sample code
	// The following code illustrates generation of grayscale images YUV images

	int i, j ;
	int r, g, b ;
	double Y, U, V ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			b = pRGB[(i + j * width) * 3] ;
			g = pRGB[(i + j * width) * 3 + 1] ;
			r = pRGB[(i + j * width) * 3 + 2] ;
			Y = r * .299000 + g * .587000 + b * .114000;
			U =r * -.168736 + g * -.331264 + b * .500000 + 128;
			V = r * .500000 + g * -.418688 + b * -.081312 + 128;


			pYUV[(i + j * width) * 3] = Y>255?255:Y<0?0:Y ;
			pYUV[(i + j * width) * 3 + 1] = U>255?255:U<0?0:U ;
			pYUV[(i + j * width) * 3 + 2] = V>255?255:V<0?0:V ;
		}
	}

	// Sample Code end
}

// This function converts input YUV image to a RGB image.
void CAppConvert::YUVtoRGB(unsigned char *pYUV, unsigned char *pRGB) {

	// You should implement YUV to RGB conversion here.

	// Sample code start - You may delete these sample code
	int i, j ;
	int y,u,v,r,b,g ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			y = pYUV[(i + j * width) * 3] ;
			u = pYUV[(i + j * width) * 3+1] ;
			v = pYUV[(i + j * width) * 3+2] ;
			r = round(y + 1.4075 * (v - 128));
			g = round(y - 0.3455 * (u - 128) - (0.7169 * (v - 128)));
			b = round(y + 1.7790 * (u - 128));
			

			pRGB[(i + j * width) * 3] = b>255?255:b<0?0:b ;
			pRGB[(i + j * width) * 3 + 1] =g>255?255:g<0?0:g;
			pRGB[(i + j * width) * 3 + 2] =r>255?255:r<0?0:r ;
		}
	}

	// Sample Code end
}



void CAppConvert::YUVtoYUV420(unsigned char *pYUV, unsigned char *pYUV420) {

	int i, j ;
	int sum ;
	int si0, si1, sj0, sj1 ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			pYUV420[(i + j * width) * 3] = pYUV[(i + j * width) * 3] ;
		}
	}

	for(j = 0; j < height; j+=2) {
		sj0 = j ;
		sj1 = (j + 1 < height) ? j + 1 : j ;
	
		for(i = 0; i < width; i+=2) {
			si0 = i ;
			si1 = (i + 1 < width) ? i + 1 : i ;

			sum =  pYUV[(si0 + sj0 * width) * 3 + 1] ; 
			sum += pYUV[(si1 + sj0 * width) * 3 + 1] ; 
			sum += pYUV[(si0 + sj1 * width) * 3 + 1] ; 
			sum += pYUV[(si1 + sj1 * width) * 3 + 1] ; 

			sum = sum / 4 ;

			pYUV420[(si0 + sj0 * width) * 3 + 1] = sum ;
			pYUV420[(si1 + sj0 * width) * 3 + 1] = sum ;
			pYUV420[(si0 + sj1 * width) * 3 + 1] = sum ;
			pYUV420[(si1 + sj1 * width) * 3 + 1] = sum ;

			sum =  pYUV[(si0 + sj0 * width) * 3 + 2] ; 
			sum += pYUV[(si1 + sj0 * width) * 3 + 2] ; 
			sum += pYUV[(si0 + sj1 * width) * 3 + 2] ; 
			sum += pYUV[(si1 + sj1 * width) * 3 + 2] ; 

			sum = sum / 4 ;

			pYUV420[(si0 + sj0 * width) * 3 + 2] = sum ;
			pYUV420[(si1 + sj0 * width) * 3 + 2] = sum ;
			pYUV420[(si0 + sj1 * width) * 3 + 2] = sum ;
			pYUV420[(si1 + sj1 * width) * 3 + 2] = sum ;
		}
	}
}

void CAppConvert::Process(void) {

	// Don't change anything within this function.

	unsigned char *yuvBuf = new unsigned char[width * height * 3] ;
	unsigned char *yuv420Buf = new unsigned char[width * height * 3] ;

	RGBtoYUV(pInput, yuvBuf) ;

	if(mode == 0) {
		SetTitle(pOutput, _T("YUV444")) ;
		SetTitle(pOutput420, _T("YUV420")) ;

		YUVtoYUV420(yuvBuf, yuv420Buf) ;

		YUVtoRGB(yuvBuf, pOutput) ;
		YUVtoRGB(yuv420Buf, pOutput420) ;
	} else if(mode == 1) {
		SetTitle(pOutput, _T("Y")) ;
	
		int i ;

		for(i = 0; i < width * height; i++) {
			yuvBuf[i * 3 + 1] = 128 ;
			yuvBuf[i * 3 + 2] = 128 ;
		}

		YUVtoRGB(yuvBuf, pOutput) ;
	} else if(mode == 2) {
		SetTitle(pOutput, _T("U")) ;

		int i ;

		for(i = 0; i < width * height; i++) {
			yuvBuf[i * 3 + 0] = 128 ;
			yuvBuf[i * 3 + 2] = 128 ;
		}

		YUVtoRGB(yuvBuf, pOutput) ;
	} else if(mode == 3) {
		SetTitle(pOutput, _T("V")) ;

		int i ;

		for(i = 0; i < width * height; i++) {
			yuvBuf[i * 3 + 0] = 128 ;
			yuvBuf[i * 3 + 1] = 128 ;
		}

		YUVtoRGB(yuvBuf, pOutput) ;
	}

	delete [] yuvBuf ;
	delete [] yuv420Buf ;

	PutDC(pOutput) ;
	if(mode == 0) {
		PutDC(pOutput420) ;
	}
}
