
// CurrecyRateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CurrecyRate.h"
#include "CurrecyRateDlg.h"
#include "afxdialogex.h"

#include <string>
// #include <iterator>
// #include <iosfwd>
#include <sstream>
#include <fstream>

#include <ctime>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

// #include <tchar.h>
#include "ScreenImage.h"
#include "ScreenSelectionDialog.h"

#include <opencv2/opencv.hpp>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define TEMP_IMG "\\temp.png"
#define TEMP_TXT "\\temp.txt"
// CCurrecyRateDlg dialog

using namespace std;
using namespace EASendMailObjLib;
using namespace tesseract;
using namespace cv;

string csToStr(CString csString)
{
	CT2CA pszConvertedAnsiString(csString);
	std::string strStd(pszConvertedAnsiString);

	return strStd;
}

vector<string> split(string path) {
	vector<string> r;
	int j = 0;
	for (int i = 0; i < path.length(); i++) {
		if (path[i] == ' ' || path[i] == '\n') {
			string cur = path.substr(j, i - j);
			if (cur.length()) {
				r.push_back(cur);
			}
			j = i + 1;
		}
	}
	if (j < path.length()) {
		r.push_back(path.substr(j));
	}
	return r;
}

bool isFloat(string myString) 
{
	istringstream iss(myString);
	float f;
	iss >> noskipws >> f; // noskipws considers leading whitespace invalid
						  // Check the entire string was consumed and if either failbit or badbit is set
	return iss.eof() && !iss.fail();
}


CCurrecyRateDlg::CCurrecyRateDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CURRECYRATE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCurrecyRateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCurrecyRateDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_START, &CCurrecyRateDlg::OnClickedBtnStart)
	ON_BN_CLICKED(IDC_BTN_STOP, &CCurrecyRateDlg::OnClickedBtnStop)
	ON_BN_CLICKED(IDC_BTN_OPEN, &CCurrecyRateDlg::OnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_SELECT, &CCurrecyRateDlg::OnClickedBtnSelect)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CCurrecyRateDlg message handlers

BOOL CCurrecyRateDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	GetDlgItem(IDC_EDIT_INTERVAL)->SetWindowTextW(CString("5000"));
	GetDlgItem(IDC_EDIT_TH)->SetWindowTextW(CString("1.3"));
	((CButton*)GetDlgItem(IDC_RADIO_GREATER))->SetCheck(1);
	
	GetDlgItem(IDC_BTN_OPEN)->ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCurrecyRateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCurrecyRateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CCurrecyRateDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->hwnd == this->m_hWnd && pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
		{
			return TRUE;                // Do not process further
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CCurrecyRateDlg::InitVariable(bool fShow)
{
	GetDlgItem(IDC_BTN_START)->EnableWindow(fShow);
	GetDlgItem(IDC_BTN_SELECT)->EnableWindow(fShow);

	GetDlgItem(IDC_EDIT_INTERVAL)->EnableWindow(fShow);
 	GetDlgItem(IDC_RADIO_GREATER)->EnableWindow(fShow);
 	GetDlgItem(IDC_RADIO_LOWER)->EnableWindow(fShow);
	GetDlgItem(IDC_EDIT_TH)->EnableWindow(fShow);

	GetDlgItem(IDC_EDIT_FROM_EMAIL)->EnableWindow(fShow);
	GetDlgItem(IDC_EDIT_FROM_PASSWORD)->EnableWindow(fShow);
	GetDlgItem(IDC_EDIT_TO_EMAIL)->EnableWindow(fShow);
}


void process(void* param)
{
	((CCurrecyRateDlg*)param)->StartProcess();
	_endthread();
}

void CCurrecyRateDlg::OnClickedBtnStart()
{
	_beginthread(process, 0, this);
}

void CCurrecyRateDlg::StartProcess()
{
	if (m_rect.Height() <= 0 || m_rect.Width() <= 0)
	{
		AfxMessageBox(_T("Select Region!"));
		return;
	}
	
	CString csInterval;
	GetDlgItem(IDC_EDIT_INTERVAL)->GetWindowTextW(csInterval);
	int nInterval = atoi(csToStr(csInterval).c_str());

	CString csField1, csField2;
	CString csTh, csFromEmail, csFromPassword, csToEmail;
	GetDlgItem(IDC_EDIT_TH)->GetWindowTextW(csTh);
	GetDlgItem(IDC_EDIT_FROM_EMAIL)->GetWindowTextW(csFromEmail);
	GetDlgItem(IDC_EDIT_FROM_PASSWORD)->GetWindowTextW(csFromPassword);
	GetDlgItem(IDC_EDIT_TO_EMAIL)->GetWindowTextW(csToEmail);

// 	if (csFromEmail == "" || csFromPassword == "" || csToEmail == "")
// 	{
// 		AfxMessageBox(_T("Input Mail Information!"));
// 		return;
// 	}

	InitVariable(false);
	float rTh = atof(csToStr(csTh).c_str());

	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	CString csSelfPath = buffer;
	int nNum = csSelfPath.ReverseFind('\\');
	csSelfPath = csSelfPath.Left(nNum);

	tesseract::TessBaseAPI api;
	if (api.Init(csToStr(csSelfPath + _T("\\tessdata")).c_str(), "digits1"))
	{
		AfxMessageBox(_T("Have no Dictionary!"));
		InitVariable(true);
		return;
	}

	IMailPtr oSmtp = NULL;

// 	if (!oSmtp)
// 	{
// 		::CoInitialize(NULL);
// 		oSmtp.CreateInstance(__uuidof(EASendMailObjLib::Mail));
// 		oSmtp->LicenseCode = _T("TryIt");
// 		
// 		oSmtp->FromAddr = (_bstr_t)(csToStr(csFromEmail).c_str());
// 		oSmtp->UserName = (_bstr_t)(csToStr(csFromEmail).c_str());
// 		oSmtp->Password = (_bstr_t)(csToStr(csFromPassword).c_str());
// 		
// 		oSmtp->ServerAddr = _T("smtp.live.com");
// 		oSmtp->ServerPort = 587;
// 		oSmtp->ConnectType = ConnectTryTLS;
// 
// 		oSmtp->AddRecipientEx((_bstr_t)(csToStr(csToEmail).c_str()), 0);
// 		
// 		oSmtp->Subject = _T("Hello!");
// 		oSmtp->BodyText = _T("Nice to meet you.");
// 	}
// 
// 	if (oSmtp->SendMail() != 0)
// 	{
// 		CString csError = oSmtp->GetLastErrDescription();
// 		
// 		AfxMessageBox(csError);
// 		oSmtp.Release();
// 		api.End();
// 		InitVariable(true);
// 		return;
// 	}

	int nAlarm = 0;
	CScreenImage img;

	m_fStart = true;
	while (m_fStart)
	{
		img.CaptureRect(m_rect);
		img.Save(csSelfPath + _T(TEMP_IMG));

		int nWidth = img.GetWidth();
		int nHeight = img.GetHeight();

		Mat src = imread(csToStr(csSelfPath + _T(TEMP_IMG)), IMREAD_GRAYSCALE);
		resize(src, src, Size(src.cols * 4, src.rows * 4));
		imwrite(csToStr(csSelfPath + _T(TEMP_IMG)), src);

		char* outText;
		Pix* image = pixRead(csToStr(csSelfPath + _T(TEMP_IMG)).c_str());
		api.SetImage(image);
		outText = api.GetUTF8Text();
		
		vector<string> vecStr = split(string(outText));
		int nNum = vecStr.size();

		nAlarm = 0;

		ofstream out(csToStr(csSelfPath + _T(TEMP_TXT)), ios::out | ios::binary);
		for (int i = 0; i < nNum; i++)
		{
			string strTemp = vecStr[i];
			if (isFloat(strTemp))
			{
				float rValue = atof(strTemp.c_str());
				if (rValue > 100000)
					rValue /= 100000;
			
				if (((CButton*)GetDlgItem(IDC_RADIO_GREATER))->GetCheck())
				{
					if (rValue > rTh)
					{
						nAlarm = 2;
						break;
					}
				}
				else
				{
					if (rValue < rTh)
					{
						nAlarm = 1;
						break;
					}
				}

				out << rValue << '\n';
			}
		}
		out.close();

// 		if (nAlarm)
//  		{
// 			if (nAlarm == 2)
// 				oSmtp->Subject = _T("Greater!");
// 			else
// 				oSmtp->Subject = _T("Lower!");
// 
// 			if (oSmtp->SendMail() != 0)
// 			{
// 				AfxMessageBox(_T("No send email!\nInput Mail Info and Confirm your network!"));
// 				delete[] outText;
// 				pixDestroy(&image);
// 				oSmtp.Release();
// 				api.End();
// 				InitVariable(true);
// 				return;
// 			}
//  		}
  
		delete[] outText;
		pixDestroy(&image);
		::Sleep(nInterval);

// 		time_t now = time(0);
// 		tm curTime;
// 		localtime_s(&curTime, &now);
// 		int nMonth = curTime.tm_mon + 1;
// 		int nDay = curTime.tm_mday;
// 		
// 		if (nMonth >= 5 && nDay >= 3)
// 			break;
	}

	api.End();

	if (oSmtp)
		oSmtp.Release();

	oSmtp = NULL;

	InitVariable(true);
}


void CCurrecyRateDlg::OnClickedBtnStop()
{
	m_fStart = false;
}

void CCurrecyRateDlg::OnClickedBtnSelect()
{
	// TODO: Add your control notification handler code here
	CScreenSelectionDialog dlg(m_rect);
	dlg.DoModal();
	AfxGetMainWnd()->ShowWindow(SW_SHOW);

	CScreenImage img;

	wchar_t buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	CString csSelfPath = buffer;
	int nNum = csSelfPath.ReverseFind('\\');
	csSelfPath = csSelfPath.Left(nNum);
	
	img.CaptureRect(m_rect);
	img.Save(csSelfPath + _T(TEMP_IMG));

	int nWidth = img.GetWidth();
	int nHeight = img.GetHeight();

	Mat src = imread(csToStr(csSelfPath + _T(TEMP_IMG)), IMREAD_COLOR);

// 	cv::Mat mat;
// 	mat = cv::Mat(nHeight, nWidth, CV_8UC4, img.GetBits()).clone();

	cv::namedWindow("demo");
	cv::imshow("demo", src);
	cv::waitKey(0);
}

void CCurrecyRateDlg::OnClickedBtnOpen()
{
	tesseract::TessBaseAPI api;
 	api.Init("C:\\src\\tessdata", "digits1");
//  	api.Init("C:\\src\\tessdata", "eng");

	Mat src = imread("E:\\Temp\\Img\\3.png", IMREAD_GRAYSCALE);
// 	pyrUp(src, src, Size(src.cols * 2, src.rows * 2));
// 	pyrUp(src, src, Size(src.cols * 2, src.rows * 2));
	resize(src, src, Size(src.cols * 4, src.rows * 4));

	imwrite("E:\\Temp\\Img\\3.png", src);


	char* outText;
	Pix* image = pixRead("E:\\Temp\\Img\\3.png");
	api.SetImage(image);

	outText = api.GetUTF8Text();

	delete[] outText;
	api.End();

}
