
// CurrecyRateDlg.h : header file
//

#pragma once

#include <tchar.h>
#include "EASendMailObj.tlh"
using namespace EASendMailObjLib;

const int ConnectNormal = 0;
const int ConnectSSLAuto = 1;
const int ConnectSTARTTLS = 2;
const int ConnectDirectSSL = 3;
const int ConnectTryTLS = 4;

// CCurrecyRateDlg dialog
class CCurrecyRateDlg : public CDialogEx
{
// Construction
public:
	CCurrecyRateDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CURRECYRATE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClickedBtnStart();

	void StartProcess();
	void InitVariable(bool fShow);

	bool m_fStart;
	CRect m_rect;

	afx_msg void OnClickedBtnStop();
	afx_msg void OnClickedBtnOpen();
	afx_msg void OnClickedBtnSelect();
};
