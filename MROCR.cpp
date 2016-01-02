#include "afxwin.h" 
#include "resource.h"
#include "strngs.h" 
#include "mropencv.h"//opencv自动链接头文件
#include "mrduilib.h"//duilib自动链接头文件
#include "mrcvutil.h"//把Mat绘制到HWnd所需文件
#include "allheaders.h" //tesseract自动链接头文件
#include "capi.h" 
void die(const char *errstr) {
	fputs(errstr, stderr);
	exit(1);
}
// 文件筛选
const TCHAR STR_FILE_FILTER[] =
	_T("Image Files(*.jpg,*.png,etc)\0*.jpg;*.png;*.tif;*.bmp;\0")
	_T("All Files(*.*)\0*.*\0");

#define CAPTION_HEIGHT 32
class CDuiFrameWnd : public WindowImplBase//,public INotifyUI
{
public:
	virtual LPCTSTR    GetWindowClassName() const   {   return _T("DUIMainFrame");  }
	virtual CDuiString GetSkinFile()                {   return _T("duilib.xml");  }
	virtual CDuiString GetSkinFolder()              {   return _T("skin");  }
	HWND    m_hWnd ;
	Mat m_srcimg;
	Mat m_dstimg;
	cv::Point m_ptBegin,m_ptEnd;
	bool m_bIsDraw;
	string m_sCurrentImagePath;
	CLabelUI* m_pResults;
	CButtonUI * m_pbtnPic;
	CDuiFrameWnd() 
	{
		m_srcimg=NULL;
		m_dstimg=NULL;
		m_ptBegin=cvPoint(0,0);
		m_ptEnd=cvPoint(0,0);
		m_bIsDraw=false;
	}
	void SetBtnImage(
		CButtonUI * pBtn, 
		const TCHAR * pcPicFileName_n,
		const TCHAR * pcPicFileName_h,
		const TCHAR * pcPicFileName_d)
	{
		if (NULL == pBtn)
		{
			return;
		}

		if (NULL != pcPicFileName_n)
			pBtn->SetNormalImage((LPCTSTR)pcPicFileName_n);
		if (NULL != pcPicFileName_h)
			pBtn->SetHotImage((LPCTSTR)pcPicFileName_h);
		if (NULL != pcPicFileName_d)
			pBtn->SetDisabledImage((LPCTSTR)pcPicFileName_d);
// 		pBtn->SetNormalImage((NULL != pcPicFileName_n) ? pcPicFileName_n : TCHAR(""));
// 		pBtn->SetHotImage((NULL != pcPicFileName_h) ? pcPicFileName_h : "");
// 		pBtn->SetPushedImage((NULL != pcPicFileName_d) ? pcPicFileName_d : "");
	}

	void InitWindow()
	{
		UINT    uStyle = 0;  
		CLabelUI* m_pLabelMsg = static_cast<CLabelUI *>(m_PaintManager.FindControl(_T("statusbar")));  
		if (NULL != m_pLabelMsg)  
		{   
			uStyle = m_pLabelMsg->GetTextStyle();  
			uStyle |= DT_WORDBREAK;  
			m_pLabelMsg->SetTextStyle(uStyle /*DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_WORD_ELLIPSIS */);  
		}  
		m_pResults = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("resultText")));
		m_pbtnPic = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("btnPic")));
// 		SetBtnImage(
// 			m_pbtnPic,
// 			_T("sample.jpg"),
// 			_T("sample.jpg"),
// 			_T("sample.jpg"));
	}
	void CDuiFrameWnd::Notify( TNotifyUI& msg )
	{
		if( msg.sType == _T("click") ) 
		{
				if( msg.pSender->GetName() == _T("btnPrevious") ) 
				{
					
				}
				else if( msg.pSender->GetName() == _T("btnNext") ) 
				{
					
				}
				else if( msg.pSender->GetName() == _T("btnPlay") ) 
				{
					if(OpenFileDlg())
					{
						m_srcimg = imread(m_sCurrentImagePath.c_str());
						SetBtnImage(
							m_pbtnPic,
							_T(m_sCurrentImagePath.c_str()),
							_T(m_sCurrentImagePath.c_str()),
							_T(m_sCurrentImagePath.c_str()));
						OnRecog(m_sCurrentImagePath.c_str());
					}
					//DrawMatToWnd((CWnd*)m_hWnd, m_srcimg);
					//OnRecog("code.jpg");
				}
		}
		__super::Notify(msg);
	}

	bool OpenFileDlg()
	{
		OPENFILENAME ofn;      
		TCHAR szFile[MAX_PATH] = _T("");
		ZeroMemory(&ofn, sizeof(ofn));  
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner   = *this;
		ofn.lpstrFile   = szFile;   
		ofn.nMaxFile    = sizeof(szFile);  
		ofn.lpstrFilter = STR_FILE_FILTER;
		ofn.nFilterIndex    = 1;  
		ofn.lpstrFileTitle  = NULL;  
		ofn.nMaxFileTitle   = 0;  
		ofn.lpstrInitialDir = NULL;  
		ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;  

		if (GetOpenFileName(&ofn))  
		{
			char *buffer=szFile;
			m_sCurrentImagePath=buffer;
			//free(buffer);
			return true;
		} 
		return false;
	}
	void OnRecog(const char *image_path)
	{
		CDuiString str(_T(""));
		TessBaseAPI *handle;
		PIX *img;
		char *text;
		if ((img = pixRead(image_path)) == NULL)
			die("Error reading image\n");
		handle = TessBaseAPICreate();
		if (TessBaseAPIInit3(handle, NULL, "eng") != 0)
			die("Error initialising tesseract\n");
		TessBaseAPISetImage2(handle, img);
		if (TessBaseAPIRecognize(handle, NULL) != 0)
			die("Error in Tesseract recognition\n");
		if ((text = TessBaseAPIGetUTF8Text(handle)) == NULL)
			die("Error getting text\n");
		str = text;
		TessDeleteText(text);
		TessBaseAPIEnd(handle);
		TessBaseAPIDelete(handle);
		pixDestroy(&img);
		if (m_pResults)
			m_pResults->SetText((LPCTSTR)str);
	}
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(m_srcimg.empty())
			return FALSE;
		m_dstimg = m_srcimg.clone();
		m_bIsDraw=true;
		int x=LOWORD(lParam);
		int y=HIWORD(lParam)-CAPTION_HEIGHT; 
		if(y<0)
			return FALSE;
		m_ptBegin=cv::Point(x,y);
		m_ptEnd=cv::Point(x,y);
 		if(x<m_srcimg.cols&&y<m_srcimg.rows)
 		{
			DrawMatToWnd((CWnd*)m_hWnd, m_srcimg);
		}
		bHandled = FALSE;
		return 0;
	}
 	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(m_bIsDraw)
		{
			if (!m_srcimg.empty())
			{
				m_dstimg=m_srcimg.clone();
				int x=LOWORD(lParam);
				int y=HIWORD(lParam)-CAPTION_HEIGHT; 
				if(y<0)
					return FALSE;
				m_ptEnd=cv::Point(x,y);
				CvFont font; 
				cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
				cv::rectangle(m_dstimg,m_ptBegin,m_ptEnd,CV_RGB(0,255,0),3);
				DrawMatToWnd((CWnd*)m_hWnd, m_dstimg);
			}
		}
		bHandled = FALSE;
		return 0;
	}
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_bIsDraw=false;
		int x=LOWORD(lParam);
		int y=HIWORD(lParam)-CAPTION_HEIGHT; 
		m_ptEnd=cv::Point(x,y);
		bHandled = FALSE;
		return 0;
	}
};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	HRESULT Hr = ::CoInitialize(NULL);
	if( FAILED(Hr) ) return 0;
	CDuiFrameWnd *pFrame = new CDuiFrameWnd;
	pFrame->Create(NULL, _T("DUIWnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	pFrame->CenterWindow();
	pFrame->ShowModal();
	delete pFrame;
	::CoUninitialize();
	return 0;
}

