#include "StdAfx.h"
#include "../Tools/ImageConvert.h"
#include "AppFeaturePoint.h"
#include "math.h"
#include "TCHAR.H"

#define ABS(x) ((x) > 0 ? (x) : (-(x)))

CAppFeaturePoint::CAppFeaturePoint(void)
{
}

CAppFeaturePoint::~CAppFeaturePoint(void)
{
}

void CAppFeaturePoint::InitFeaturePoints(FeaturePoint *featurePointList) {

	int i, j ;

	for(j = 0; j < height; j++) {
		for(i = 0; i < width; i++) {
			featurePointList[i + j * width].x = i ;
			featurePointList[i + j * width].y = j ;
			FP_INIT(&featurePointList[i + j * width].score, 8) ;
			FP_SET(0, &featurePointList[i + j * width].score) ;
		}
	}
}

void CAppFeaturePoint::CustomInit(CView *pView) {

	frameCurYUV = new unsigned char[width * height * 2] ;
	frameRefYUV = new unsigned char[width * height * 2] ;
	gradX = new short[width * height] ;
	gradY = new short[width * height] ;
	absGradX = new short[width * height] ;
	absGradY = new short[width * height] ;
	featurePointList1 = new FeaturePoint[width * height] ;
	featurePointList2 = new FeaturePoint[width * height] ;
//	CreateTunnel(width, height, &outOfAxisRef) ;
	numFeatures = 100 ;

	InitFeaturePoints(featurePointList1) ;
	InitFeaturePoints(featurePointList2) ;

	s1 = s2 = ss1 = ss2 = 0.0 ;
	n1 = n2 = 0.0 ;

	RegisterReference(numFeatures) ;

	frameNum = 0 ;
}

void CAppFeaturePoint::CustomFinal() {
//	RemoveTunnel(outOfAxisRef) ;
	delete [] frameCurYUV ;
	delete [] frameRefYUV ;
	delete [] gradX ;
	delete [] gradY ;
	delete [] absGradX ;
	delete [] absGradY ;
	delete [] featurePointList1 ;
	delete [] featurePointList2 ;
}

void CAppFeaturePoint::Gradient() {

	int i, j ;
	int lt0 ;
	int grad ;

	memset(gradX, 0, sizeof(short) * width * height) ;
	memset(gradY, 0, sizeof(short) * width * height) ;
	memset(absGradX, 0, sizeof(short) * width * height) ;
	memset(absGradY, 0, sizeof(short) * width * height) ;
	for(j = 1; j < height - 1; j++) {
		for(i = 1; i < width - 1; i++) {
//			grad = gradX[i + j * width] = frameCurYUV[i + 1 + j * width] - frameCurYUV[i + j * width] ;
			grad = gradX[i + j * width] = ((int) frameCurYUV[i + 1 + (j - 1) * width] - (int) frameCurYUV[i - 1 + (j - 1) * width] +
				(int) frameCurYUV[i + 1 + (j + 1) * width] - (int) frameCurYUV[i - 1 + (j + 1) * width] +
				2 * (int) frameCurYUV[i + 1 + (j + 0) * width] - 2 * (int) frameCurYUV[i - 1 + (j + 0) * width]) / 4 ;
			lt0 = (65535 - grad) >> 16 ;
			absGradX[i + j * width] = (1 - 2 * lt0) * grad ;

//			grad = gradY[i + j * width] = frameCurYUV[i + (j + 1) * width] - frameCurYUV[i + j * width] ;
			grad = gradY[i + j * width] = ((int) frameCurYUV[i - 1 + (j + 1) * width] - (int) frameCurYUV[i - 1 + (j - 1) * width] +
				(int) frameCurYUV[i + 1 + (j + 1) * width] - (int) frameCurYUV[i + 1 + (j - 1) * width] +
				2 * (int) frameCurYUV[i + 0 + (j + 1) * width] - 2 * (int) frameCurYUV[i - 0 + (j - 1) * width]) / 4 ;
			lt0 = (65535 - grad) >> 16 ;
			absGradY[i + j * width] = (1 - 2 * lt0) * grad ;
		}
	}
}

int FPScoreCmp(const void *a, const void *b) {

	FeaturePoint *fa ;
	FeaturePoint *fb ;
	FixedPoint v ;

	fa = (FeaturePoint *) a ;
	fb = (FeaturePoint *) b ;

	FP_INIT(&v, 8) ;

	FP_SUB(&fa->score, &fb->score, &v) ;

	if(v.value > 0) return -1 ;
	if(v.value < 0) return 1 ;
	return 0 ;
}

int CAppFeaturePoint::Correlation(FeaturePoint *featurePoint, int halfWidth) {

	int i, j ;
	int x, y ;
	int sad = 0 ;
	int v ;
	int lt0 ;
	int s, ss ;
	int n ;

	s = ss = 0 ;
	for(j = -halfWidth; j <= halfWidth; j++) {
		for(i = -halfWidth; i <= halfWidth; i++) {
			x = featurePoint->x + i ;
			y = featurePoint->y + j ;
			if(x >= 0 && y >= 0 && x < width && y < height) {
				v = frameCurYUV[x + y * width] - frameRefYUV[x + y * width] ;
				s += v ;
				ss += v * v ;
				lt0 = (65535 - v) >> 16 ;
				v = (1 - 2 * lt0) * v ;
				sad += v ;
			}
		}
	}

	n = (halfWidth * 2 + 1) * (halfWidth * 2 + 1) ;


	sad = (n * ss - s * s) / n / n ;

	return sad ;
}

void CAppFeaturePoint::SelectFeaturesSimple(FeaturePoint *featurePointList) {

	int i, j, m, n, x, y ;
	int min ;
	int w ;
	LongInt maxScore ;
	int maxX, maxY ;

	for(i = 0; i < width * height; i++) {
		if(absGradX[i] >= absGradY[i]) {
			min = absGradY[i] ;
		} else {
			min = absGradX[i] ;
		}
		FP_SET(min, &featurePointList[i].score) ;
	}

	w = 8 ;
	for(j = 0; j < height; j+=w) {
		for(i = 0; i < width; i+=w) {
			maxScore = -1 ;
			for(n = 0; n < w; n++) {
				for(m = 0; m < w; m++) {
					x = i + m ;
					y = j + n ;
					if(featurePointList[x + y * width].score.value > maxScore) {
						if(maxScore > 0) featurePointList[maxX + maxY * width].score.value = 0 ;
						maxScore = featurePointList[x + y * width].score.value ;
						maxX = x ;
						maxY = y ;
					} else {
						featurePointList[x + y * width].score.value = 0 ;
					}
				}
			}			
		}
	}
	for(j = w / 2; j < height - w / 2; j+=w) {
		for(i = 0; i < width; i+=w) {
			maxScore = -1 ;
			for(n = 0; n < w; n++) {
				for(m = 0; m < w; m++) {
					x = i + m ;
					y = j + n ;
					if(featurePointList[x + y * width].score.value > maxScore) {
						if(maxScore > 0) featurePointList[maxX + maxY * width].score.value = 0 ;
						maxScore = featurePointList[x + y * width].score.value ;
						maxX = x ;
						maxY = y ;
					} else {
						featurePointList[x + y * width].score.value = 0 ;
					}
				}
			}			
		}
	}
	for(j = 0; j < height; j+=w) {
		for(i = w / 2; i < width - w / 2; i+=w) {
			maxScore = -1 ;
			for(n = 0; n < w; n++) {
				for(m = 0; m < w; m++) {
					x = i + m ;
					y = j + n ;
					if(featurePointList[x + y * width].score.value > maxScore) {
						if(maxScore > 0) featurePointList[maxX + maxY * width].score.value = 0 ;
						maxScore = featurePointList[x + y * width].score.value ;
						maxX = x ;
						maxY = y ;
					} else {
						featurePointList[x + y * width].score.value = 0 ;
					}
				}
			}			
		}
	}

	qsort(featurePointList, width * height, sizeof(FeaturePoint), FPScoreCmp) ;
}

void CAppFeaturePoint::SelectFeaturesEigen(FeaturePoint *featurePointList) {

	int i, j, m, n, x, y ;
	double gxx, gxy, gyy ;
	int score ;
	int w = 2 ;
	double db, dc ;
	double dscore ;
	double r1 ;
	LongInt maxScore ;
	int maxX, maxY ;

	for(j = w; j < height - w; j++) {
		for(i = w; i < width - w; i++) {
			gxx = gxy = gyy = 0.0 ;
			for(n = -w; n <= w; n++) {
				for(m = -w; m <= w; m++) {
					x = i + m ;
					y = j + n ;
					gxx += (double) (gradX[x + y * width] * gradX[x + y * width]) ;
					gyy += (double) (gradY[x + y * width] * gradY[x + y * width]) ;
					gxy += (double) (gradX[x + y * width] * gradY[x + y * width]) ;
				}
			}
			gxx /= (double) ((2 * w + 1) * (2 * w + 1)) ;
			gyy /= (double) ((2 * w + 1) * (2 * w + 1)) ;
			gxy /= (double) ((2 * w + 1) * (2 * w + 1)) ;
			db = -(gxx + gyy) ;
			dc = gxx * gyy - gxy * gxy ;

			r1 = (-db - sqrt(db * db - 4.0 * dc)) / 2.0 ;
			dscore = r1 ;
//			dscore = dc / (100.0 + db * db - 4 * dc) ;
//			dscore = dc / (-db / 2.0 + db * db - 4 * dc) ;

//			score = (c) / (1000 + b * b - 4 * c) ;
			score = (int) (dscore) ;
//			if(r1 < 25.0) score = 0 ;
			FP_SET(score, &featurePointList[i + j * width].score) ;
		}
	}

	w = 8 ;
	for(j = 0; j < height; j+=w) {
		for(i = 0; i < width; i+=w) {
			maxScore = -1 ;
			for(n = 0; n < w; n++) {
				for(m = 0; m < w; m++) {
					x = i + m ;
					y = j + n ;
					if(featurePointList[x + y * width].score.value > maxScore) {
						if(maxScore > 0) featurePointList[maxX + maxY * width].score.value = 0 ;
						maxScore = featurePointList[x + y * width].score.value ;
						maxX = x ;
						maxY = y ;
					} else {
						featurePointList[x + y * width].score.value = 0 ;
					}
				}
			}			
		}
	}
	for(j = w / 2; j < height - w / 2; j+=w) {
		for(i = 0; i < width; i+=w) {
			maxScore = -1 ;
			for(n = 0; n < w; n++) {
				for(m = 0; m < w; m++) {
					x = i + m ;
					y = j + n ;
					if(featurePointList[x + y * width].score.value > maxScore) {
						if(maxScore > 0) featurePointList[maxX + maxY * width].score.value = 0 ;
						maxScore = featurePointList[x + y * width].score.value ;
						maxX = x ;
						maxY = y ;
					} else {
						featurePointList[x + y * width].score.value = 0 ;
					}
				}
			}			
		}
	}
	for(j = 0; j < height; j+=w) {
		for(i = w / 2; i < width - w / 2; i+=w) {
			maxScore = -1 ;
			for(n = 0; n < w; n++) {
				for(m = 0; m < w; m++) {
					x = i + m ;
					y = j + n ;
					if(featurePointList[x + y * width].score.value > maxScore) {
						if(maxScore > 0) featurePointList[maxX + maxY * width].score.value = 0 ;
						maxScore = featurePointList[x + y * width].score.value ;
						maxX = x ;
						maxY = y ;
					} else {
						featurePointList[x + y * width].score.value = 0 ;
					}
				}
			}			
		}
	}


	qsort(featurePointList, width * height, sizeof(FeaturePoint), FPScoreCmp) ;
}

void CAppFeaturePoint::DrawFeaturePoints(int numFeatures) {
/*
	{
		int i ;
	
		GLStart(pInput) ;
			for(i = 0; i < numFeatures; i++) {
				glColor3d(1.0 - (double) i / (double) numFeatures, 0.0, (double) i / (double) numFeatures) ;
				if(featurePointList1[i].score.value > 0) {
					glBegin(GL_LINES) ;
						glVertex2i(featurePointList1[i].x - 4, height - 1 - featurePointList1[i].y - 4) ;
						glVertex2i(featurePointList1[i].x + 4, height - 1 - featurePointList1[i].y + 4) ;
						glVertex2i(featurePointList1[i].x + 4, height - 1 - featurePointList1[i].y - 4) ;
						glVertex2i(featurePointList1[i].x - 4, height - 1 - featurePointList1[i].y + 4) ;
					glEnd() ;
				}
			}
		GLFinish(pInput) ;
	}
*/
	{
		int i ;
	
		GLStart(pOutput) ;
			for(i = 0; i < numFeatures; i++) {
				glColor3d(1.0 - (double) i / (double) numFeatures, 0.0, (double) i / (double) numFeatures) ;
				if(featurePointList2[i].score.value > 0) {
					glBegin(GL_LINES) ;
						glVertex2i(featurePointList2[i].x - 4, height - 1 - featurePointList2[i].y - 4) ;
						glVertex2i(featurePointList2[i].x + 4, height - 1 - featurePointList2[i].y + 4) ;
						glVertex2i(featurePointList2[i].x + 4, height - 1 - featurePointList2[i].y - 4) ;
						glVertex2i(featurePointList2[i].x - 4, height - 1 - featurePointList2[i].y + 4) ;
					glEnd() ;
				}
			}
		GLFinish(pOutput) ;
	}

	UpdateTunnel() ;
}

void CAppFeaturePoint::RegisterReference(int numFeatures) {

	CImageConvert imgConvert ;

	GetSrc() ;
	imgConvert.RGB24_to_YUV420(pInput, frameCurYUV, width, height) ;
	memcpy(frameRefYUV, frameCurYUV, width * height * 2) ;

	Gradient() ;

//	InitFeaturePoints(featurePointList1) ;
	InitFeaturePoints(featurePointList2) ;

//	SelectFeaturesSimple(featurePointList1) ;
	SelectFeaturesEigen(featurePointList2) ;

	DrawFeaturePoints(numFeatures) ;
}

void CAppFeaturePoint::Perform() {
	CImageConvert imgConvert ;
	TCHAR frameNumStr[256] ;
//	char messageStr[256] ;
	int i ;
	double sad, sum_sad ;
	double sd ;
	int movedCount ;
	int sadThreshold = 256 ;

	GetSrc() ;
	imgConvert.RGB24_to_YUV420(pInput, frameCurYUV, width, height) ;

	_stprintf_s(frameNumStr, _T("Frame # = %d"), frameNum) ;
	TextOut(GetDC(pInput), 5, 5, frameNumStr, (int) _tcslen(frameNumStr)) ;

	memcpy(pOutput, pInput, width * height * 3) ;
/*
	{
		int min ;
		int max ;


		for(i = 0; i < width * height; i++) {
			if(absGradX[i] >= absGradY[i]) {
				min = absGradY[i] ;
				max = absGradX[i] ;
			} else {
				min = absGradX[i] ;
				max = absGradY[i] ;
			}

			pOutput[3 * i + 0] = min ;
			pOutput[3 * i + 1] = min ;
			pOutput[3 * i + 2] = min ;
		}
	}
*/
	PutDC(pOutput) ;

/*	{
		Gradient() ;

		RegisterReference(numFeatures) ;
	} */

	RegisterReference(numFeatures) ;
//	DrawFeaturePoints(numFeatures) ;

	movedCount = 0 ;
	sum_sad = 0.0 ;
/*
	for(i = 0; i < numFeatures; i++) {
		sad = (double) Correlation(&featurePointList1[i], 5) ;
		sum_sad += sad ;
		s1 = (s1 * n1 + sad) / (n1 + 1.0) ;
		ss1 = (ss1 * n1 + sad * sad) / (n1 + 1.0) ;
		n1 += 1.0 ;
		sd = ss1 - s1 * s1 ;
		if(sad > sadThreshold) {
			movedCount++ ;
			GLStart(pInput) ;
				glColor3d(0.0, 1.0, 0.0) ;
				glBegin(GL_LINES) ;
					glVertex2i(featurePointList1[i].x - 4, height - 1 - featurePointList1[i].y - 4) ;
					glVertex2i(featurePointList1[i].x + 4, height - 1 - featurePointList1[i].y + 4) ;
					glVertex2i(featurePointList1[i].x + 4, height - 1 - featurePointList1[i].y - 4) ;
					glVertex2i(featurePointList1[i].x - 4, height - 1 - featurePointList1[i].y + 4) ;
				glEnd() ;
			GLFinish(pInput) ;
		}
//		sprintf(messageStr, "Average Sad # = %f", sum_sad / 50.0) ;
//		TextOut(GetDC(pInput), 5, height / 2 - 20, messageStr, (int) strlen(messageStr)) ;
//		sprintf(messageStr, "Variance of Sad # = %f", sd) ;
//		TextOut(GetDC(pInput), 5, height / 2 + 20, messageStr, (int) strlen(messageStr)) ;
//		sprintf(messageStr, "Moved Points # = %d (%d %%)", movedCount, movedCount * 100 / numFeatures) ;
//		TextOut(GetDC(pInput), 5, height / 2 + 40, messageStr, (int) strlen(messageStr)) ;
//		sprintf(messageStr, "Min Score # = %d", FP_ROUND(&featurePointList1[numFeatures - 1].score)) ;
//		TextOut(GetDC(pInput), 5, height / 2 + 60, messageStr, (int) strlen(messageStr)) ;
	}
*/
	movedCount = 0 ;
	sum_sad = 0.0 ;
	for(i = 0; i < numFeatures; i++) {
		sad = (double) Correlation(&featurePointList2[i], 5) ;
		sum_sad += sad ;
		s2 = (s2 * n2 + sad) / (n2 + 1.0) ;
		ss2 = (ss2 * n2 + sad * sad) / (n2 + 1.0) ;
		n2 += 1.0 ;
		sd = ss2 - s2 * s2 ;
		if(sad > sadThreshold) {
			movedCount++ ;
			GLStart(pOutput) ;
				glColor3d(0.0, 1.0, 0.0) ;
				glBegin(GL_LINES) ;
					glVertex2i(featurePointList2[i].x - 4, height - 1 - featurePointList2[i].y - 4) ;
					glVertex2i(featurePointList2[i].x + 4, height - 1 - featurePointList2[i].y + 4) ;
					glVertex2i(featurePointList2[i].x + 4, height - 1 - featurePointList2[i].y - 4) ;
					glVertex2i(featurePointList2[i].x - 4, height - 1 - featurePointList2[i].y + 4) ;
				glEnd() ;
			GLFinish(pOutput) ;
		}
		/*
		sprintf(messageStr, "Average Sad # = %f", sum_sad / 50.0) ;
		TextOut(GetDC(pOutput), 5, height / 2 - 20, messageStr, (int) strlen(messageStr)) ;
		sprintf(messageStr, "Variance of Sad # = %f", sd) ;
		TextOut(GetDC(pOutput), 5, height / 2 + 20, messageStr, (int) strlen(messageStr)) ;
		sprintf(messageStr, "Moved Points # = %d (%d %%)", movedCount, movedCount * 100 / numFeatures) ;
		TextOut(GetDC(pOutput), 5, height / 2 + 40, messageStr, (int) strlen(messageStr)) ;
		sprintf(messageStr, "Min Score # = %d", FP_ROUND(&featurePointList2[numFeatures - 1].score)) ;
		TextOut(GetDC(pOutput), 5, height / 2 + 60, messageStr, (int) strlen(messageStr)) ;
		*/
	}

	frameNum++ ;
}
