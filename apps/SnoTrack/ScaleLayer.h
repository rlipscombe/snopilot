#pragma once
#include "Layer.h"

class ScaleLayer : public Layer
{
	// Remember where we last drew anything, thus making it easier to
	// invalidate it when scrolling.
	CRect m_lastDrawRect;

public:
	ScaleLayer(const TCHAR *name, int weight)
		: Layer(name, weight, true, false), m_lastDrawRect(0,0,0,0)
	{
	}

	virtual void OnScroll(CWnd *pWnd, int xAmount, int yAmount);
	virtual void OnDraw(DrawSite *drawSite, CDC *pDC);

private:
	// The scale is to be drawn in two parts: a metric part and an imperial part.
	struct ScaleValues
	{
		int maxScaleFactor;

		int scaleDecimetres;
		int parts;
		const TCHAR *lhsText;
		const TCHAR *midText;
		const TCHAR *rhsText;
	};
	static ScaleValues metricScaleTable[];
	static ScaleValues imperialScaleTable[];
	static ScaleValues largeMetricScale;
	static ScaleValues largeImperialScale;

	const ScaleValues *CalculateMetricScaleValues(int scaleFactor);
	const ScaleValues *CalculateImperialScaleValues(int scaleFactor);

	CRect CalculateMetricRect(int scaleFactor, const CRect &clientRect, int metricScaleDecimetres);
	CRect CalculateImperialRect(int scaleFactor, const CRect &clientRect, int imperialScaleDecimetres);
	CRect CalculateScrollRect(int scaleFactor, const CRect &clientRect);

	void DrawMetricScale(CDC *pDC, int scaleFactor, const CRect &clientRect, int metricScaleDecimetres, int numPortions,
							const CString &lhsText, const CString &midText, const CString &rhsText, CRect *lastDrawRect);
	void DrawImperialScale(CDC *pDC, int scaleFactor, const CRect &clientRect, int imperialScaleDecimetres, int numPortions,
							const CString &lhsText, const CString &midText, const CString &rhsText, CRect *lastDrawRect);

	void DrawScaleTextAbove(CDC *pDC, CRect *scaleRect, const CString &lhsText, const CString &midText, const CString &rhsText);
	void DrawScaleTextBelow(CDC *pDC, CRect *scaleRect, const CString &lhsText, const CString &midText, const CString &rhsText);

	void DrawPartitionedRectangle(CDC *pDC, const CRect &rect, int numPortions);
};
