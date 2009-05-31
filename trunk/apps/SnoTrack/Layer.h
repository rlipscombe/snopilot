#pragma once
#include <vector>
#include "SelectionTimeSpan.h"

class DrawSite;

class Layer
{
	CString m_strName;	///< User-visible name of this layer.
	int m_weight;
	bool m_isEnabled;	///< Is this layer visible or not?

	/** Is this layer special?
	 * Special layers don't appear in the View|Options dialog,
	 * and are turned on/off by some other mechanism.
	 */
	bool m_isSpecial;

public:
	Layer(const TCHAR *name, int weight, bool isEnabled, bool isSpecial)
		: m_strName(name), m_weight(weight), m_isEnabled(isEnabled), m_isSpecial(isSpecial)
	{
	}

	virtual ~Layer() { /* nothing */ }

	CString GetName() const { return m_strName; }
	int GetWeight() const { return m_weight; }

	bool IsEnabled() const { return m_isEnabled; }
	void Enable(bool enable) { m_isEnabled = enable; }
	bool IsSpecial() const { return m_isSpecial; }

	virtual void OnRecalc(const CRect &logicalRange) { /* nothing */ }
	virtual void OnSelect(const SelectionTimeSpanCollection &selection) { /* nothing */ }
	virtual void OnScroll(CWnd *pWnd, int xAmount, int yAmount) { /* nothing */ }

	virtual void OnDraw(DrawSite *drawSite, CDC *pDC) = 0;
};
typedef std::vector< Layer *> Layers;

struct LayerWeightLess
{
	bool operator() (const Layer *lhs, const Layer *rhs) const
	{
		return (lhs->GetWeight() < rhs->GetWeight());
	}
};