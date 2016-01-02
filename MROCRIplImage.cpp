#include "afxwin.h"
#include "allheaders.h"  
#include "baseapi.h"  
#include "strngs.h"  
#include "mropencv.h"
#include "mrduilib.h"
#include "CvvImage.h"
#include "renderer.h"
#include "resource.h"
#include "mrcvutil.h"
// ÎÄ¼þÉ¸Ñ¡
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
	IplImage *m_srcimg;
	IplImage *m_dstimg;
	CvPoint m_ptBegin,m_ptEnd;
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
		SetBtnImage(
			m_pbtnPic,
			_T("sample.jpg"),
			_T("sample.jpg"),
			_T("sample.jpg"));
	}
// 	void CDuiFrameWnd::Notify( TNotifyUI& msg )
// 	{
// 		if( msg.sType == _T("click") ) 
// 		{
// 				if( msg.pSender->GetName() == _T("btnPrevious") ) 
// 				{
// 					
// 				}
// 				else if( msg.pSender->GetName() == _T("btnNext") ) 
// 				{
// 					
// 				}
// 				else if( msg.pSender->GetName() == _T("btnPlay") ) 
// 				{
// 					if(OpenFileDlg())
// 					{
// 						m_srcimg=cvLoadImage(m_sCurrentImagePath.c_str());
// 						ShowIplImage(m_srcimg);
// //						OnRecog(m_sCurrentImagePath.c_str());
// 					}
// //					OnRecog("test.jpg");
// 				}
// 		}
// 		__super::Notify(msg);
// 	}

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
			free(buffer);
			return true;
		} 
		return false;
	}
	void OnRecog(const char *image_path)
	{
		const char * image = "code.jpg";
		const char * outputbase = "out";
		tesseract::TessBaseAPI api;
		api.SetOutputName(outputbase);

		int rc = api.Init(NULL, "eng", tesseract::OEM_DEFAULT);
		if (rc) {
			printf("Could not initialize tesseract.\n");
			exit(1);
		}
		//api.SetVariable("tessedit_char_whitelist", "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
		Pix* pixs = pixRead(image);
		if (!pixs) {
			printf("Cannot open input file: %s\n", image);
			exit(2);
		}
		api.SetImage(pixs);
		tesseract::TessResultRenderer* renderer = new tesseract::TessTextRenderer(outputbase);
		if (!api.ProcessPages(image, NULL, 0, renderer)) {
			printf("Error during processing.\n");
		}
		delete renderer;
		pixDestroy(&pixs);
		CDuiString str(_T(""));
		str = "test";
		if(m_pResults)
			m_pResults->SetText((LPCTSTR)str);
	}
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(m_srcimg==NULL)
			return FALSE;
 		if(m_dstimg!=NULL)
			cvReleaseImage(&m_dstimg);
		m_dstimg=cvCloneImage(m_srcimg);
		m_bIsDraw=true;
		int x=LOWORD(lParam);
		int y=HIWORD(lParam)-CAPTION_HEIGHT; 
		if(y<0)
			return FALSE;
		m_ptBegin=cvPoint(x,y);
		m_ptEnd=cvPoint(x,y);
		if(x<m_srcimg->width&&y<m_srcimg->height)
		{
			CvScalar s;  
			s = cvGet2D(m_srcimg,y, x);// get the (i,j) pixel value
		int i=0,j=0;
		for(i=0;i<y;i++)
			for(j=0;j<x;j++)
			{
				cvSet2D(m_srcimg, i, j, s);// set the (i,j) pixel value 
			}
			ShowIplImage(m_dstimg);
			// 			CString str;
			// 			str.Format("%d,%d,(%0.0lf,%0.0lf,%0.0lf)",x,y,s.val[0],s.val[1],s.val[2]);
			// 			GetDlgItem(IDC_POSITION)->SetWindowText(str);
		}
		bHandled = FALSE;
		return 0;
	}
 	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if(m_bIsDraw)
		{			
			if(m_srcimg)
			{
				m_dstimg=cvCloneImage(m_srcimg);
				int x=LOWORD(lParam);
				int y=HIWORD(lParam)-CAPTION_HEIGHT; 
				if(y<0)
					return FALSE;
				m_ptEnd=cvPoint(x,y);
				CvFont font; 
				cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.5, 0.5, 0, 1, CV_AA);
				cvRectangle(m_dstimg,m_ptBegin,m_ptEnd,CV_RGB(0,255,0),3);
				ShowIplImage(m_dstimg);
				// 				CString str;
				// 				str.Format("%d,%d->%d,%d",m_ptBegin.x,m_ptBegin.y,point.x,point.y);
				// 				GetDlgItem(IDC_POSITION)->SetWindowText(str);
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
		m_ptEnd=cvPoint(x,y);
		bHandled = FALSE;
		return 0;
	}
 	void ShowIplImage(IplImage* img) 
	{
		HDC hdc=GetDC(m_hWnd);
		CRect rect; 
		rect.SetRect(0 ,0, img->width ,img->height);
		CvvImage cimg;
		cimg.CopyOf(img);
		cimg.DrawToHDC(hdc,&rect); 
		ReleaseDC(m_hWnd,hdc);
	}
};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	//HICON hIcon = AfxGetApp()->LoadIcon(IDI_BLUEROSEICON);
	//AfxGetApp()->GetMainWnd()->SetIcon(hIcon, FALSE);
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

