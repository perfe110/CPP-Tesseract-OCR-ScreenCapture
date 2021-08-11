#pragma once

#include "ScreenImage.h"
#include "resource.h"

// CScreenSelectionDialog dialog

class CScreenSelectionDialog : public CDialogEx
{
   DECLARE_DYNAMIC(CScreenSelectionDialog)
private:
   CRect& m_rect;
   CRectTracker m_tracker;
   CScreenImage m_image;
public:
	CScreenSelectionDialog(CRect& rect);
	virtual ~CScreenSelectionDialog();
   //const CRect& GetRect() const {return m_rect;}
// Dialog Data
	enum { IDD = IDD_SCREEN_SELECTION };

protected:
	DECLARE_MESSAGE_MAP()
public:
   virtual BOOL OnInitDialog();
   afx_msg BOOL OnEraseBkgnd(CDC* pDC);
   afx_msg void OnPaint();
   afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
};
