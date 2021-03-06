
// CamProcessDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "CamProcess.h"
#include "CamProcessDlg.h"
#include "afxdialogex.h"

#define CLIP(x) (((x) <0)?0:(((x)>255)?255:(x)))

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HDC hdc;
// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

														// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCamProcessDlg 대화 상자



CCamProcessDlg::CCamProcessDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CAMPROCESS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCamProcessDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCamProcessDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CCamProcessDlg 메시지 처리기

BOOL CCamProcessDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

									// TODO: 여기에 추가 초기화 작업을 추가합니다.

									/*
									* 제작 : 황 태림
									* 작성시작 : 2018-05-04
									* 내용 : VFW를 이용한 웹캠 인식 프로그램 제작 및 전처리
									*/

	// Capture window 생성
	hdc = ::GetDC(this->m_hWnd);
	// Cam 연결 시 사용
	m_hWndCap = capCreateCaptureWindow("Capture Window", WS_CHILD | WS_VISIBLE, 0, 0, xWidth, yWidth, this->m_hWnd, NULL);
	// Camera Driver와 Capture Window 연결
	if (capDriverConnect(m_hWndCap, 0) == FALSE) return FALSE;
	
	BInfo.bmiHeader.biBitCount = 24;
	BInfo.bmiHeader.biCompression = 0;
	BInfo.bmiHeader.biSizeImage = BInfo.bmiHeader.biWidth * BInfo.bmiHeader.biHeight * 3;
	capSetVideoFormat(m_hWndCap, &BInfo, sizeof(BITMAPINFO));
	capPreviewRate(m_hWndCap, 1);
	capOverlay(m_hWndCap, true);

	// Test. 화면에 어떤 전처리를 한다

	capSetCallbackOnFrame(m_hWndCap, Emb_Draw);
	capPreview(m_hWndCap, true);





	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

LRESULT CALLBACK CCamProcessDlg::Emb_Draw(HWND hWnd, LPVIDEOHDR lp) {
	int xPos;
	int yPos;
	unsigned char rgbg[xWidth * yWidth] = { 0, };
	int Y0, U, Y1, V, C, D, E;
	// YUY2 ---> RGB
	unsigned int nYUY2Size = yWidth * (xWidth / 2);
	int yuy2 = 0;
	int rgb = 0;
#pragma omp parallel for
	for (unsigned int i = 0; i < nYUY2Size; i++) {
		Y0 = lp->lpData[yuy2];
		U = lp->lpData[yuy2 + 1];
		Y1 = lp->lpData[yuy2 + 2];
		V = lp->lpData[yuy2 + 3];

		C = Y0 - 16;
		D = U - 128;
		E = V - 128;

		rgbg[rgb] = (int)CLIP(((298 * C + 516 * D + 128) >> 8) * 0.30 + ((298 * C - 100 * D - 208 * E + 128) >> 8) * 0.59 + ((298 * C + 409 * E + 128) >> 8) * 0.11);

		C = Y1 - 16;
		rgbg[rgb +1] = (int)CLIP(((298 * C + 409 * E + 128) >> 8) * 0.30 + ((298 * C - 100 * D - 208 * E + 128) >> 8) * 0.59 + ((298 * C + 516 * D + 128) >> 8) * 0.11);
		yuy2 += 4;
		rgb += 2;
		
	}
	yuy2 = 0;
	rgb = 0;
	int iCnt = 0;
#pragma omp parallel for
	for (yPos = 0; yWidth > yPos; yPos++) {

		for (xPos = 0; xWidth > xPos; xPos++) {
			
			int gray = rgbg[iCnt];
			SetPixel(hdc, xWidth + xPos, yPos, RGB(gray, gray, gray));
			//SetPixel(hdc, xWidth + xPos, yPos, RGB(rgb[yPos][xPos][2], rgb[yPos][xPos][1], rgb[yPos][xPos][0]));
			iCnt += 1;
		}
	}
	iCnt = 0;
	return 0;
}

void CCamProcessDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CCamProcessDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CCamProcessDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

