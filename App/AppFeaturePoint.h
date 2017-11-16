#pragma once
#include "../processing.h"
#include "../Tools/fixedPoint_c.h"

typedef struct {
	short x, y ;
	FixedPoint score ;
} FeaturePoint ;

class CAppFeaturePoint :
	public CProcessing
{
public:
	unsigned char *frameCurYUV ;
	unsigned char *frameRefYUV ;
	short *gradX, *gradY ;
	short *absGradX, * absGradY ;
	int frameNum ;
	int numFeatures ;
	FeaturePoint *featurePointList1 ;
	FeaturePoint *featurePointList2 ;
	double s1, ss1 ;
	double n1 ;
	double s2, ss2 ;
	double n2 ;
public:
	CAppFeaturePoint(void);
	~CAppFeaturePoint(void);
	void Gradient(void) ;
	void InitFeaturePoints(FeaturePoint *featurePointList) ;
	void SelectFeaturesSimple(FeaturePoint *featurePointList) ;
	void SelectFeaturesEigen(FeaturePoint *featurePointList) ;
	void CustomInit(CView *pView) ;
	void RegisterReference(int numFeatures) ;
	void DrawFeaturePoints(int numFeatures) ;
	int Correlation(FeaturePoint *featurePoint, int halfWidth) ;
	void Perform() ;
	void CustomFinal() ;
};
