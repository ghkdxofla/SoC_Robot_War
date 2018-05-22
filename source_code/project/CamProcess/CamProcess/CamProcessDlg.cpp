
// CamProcessDlg.cpp: 구현 파일
//

#include "stdafx.h"
#include "CamProcess.h"
#include "CamProcessDlg.h"
#include "afxdialogex.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLIP(x) (((x) <0)?0:(((x)>255)?255:(x)))

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
// SetBitmapBits 로 바꾸기 위함
CDC* pDCc;
HDC hdc;
HWND m_hWndCap;
CWnd* alt_hWndCap;
BITMAPINFO BInfo;

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
	ON_BN_CLICKED(IDOK, &CCamProcessDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CCamProcessDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CCamProcessDlg::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CCamProcessDlg::OnBnClickedButtonSave)
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
	// SetBitmapBits를 위해 HDC->CDC 변환
	hdc = ::GetDC(this->m_hWnd);

	// Cam 연결 시 사용
	m_hWndCap = capCreateCaptureWindow("Capture Window", WS_CHILD | WS_VISIBLE, 0, 0, xWidth, yWidth, this->m_hWnd, NULL);
	alt_hWndCap = GetDlgItem(IDC_TEST);
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
	//unsigned char a[yWidth][xWidth] = { 0, }; // rgbg -> 이름 통일을 위해 a로 변경
	
	unsigned char** a = new unsigned char* [yWidth];
	for (int i = 0; i < yWidth; i++) {
		a[i] = new unsigned char [xWidth];
		memset(a[i], 0, sizeof(unsigned char) * xWidth);
	}

	//****************************************************************************
	// Grayscale

	int Y0, U, Y1, V, C, D, E;
	// YUY2 ---> RGB
	unsigned int nYUY2Size = yWidth * (xWidth / 2);
	int yuy2 = 320 * 4;
	int gray = 0;
#pragma omp parallel for
	// test 1 : fast 알고리즘에 맞게 xPos, yPos 를 4에서 width-3의 범위로 한정
	for (yPos = 4; yWidth - 2 > yPos; yPos++) {
		for (xPos = 4; xWidth - 2 > xPos; xPos += 2) {
			// pixel color 처리
			Y0 = lp->lpData[yuy2];
			U = lp->lpData[yuy2 + 1];
			Y1 = lp->lpData[yuy2 + 2];
			V = lp->lpData[yuy2 + 3];

			C = Y0 - 16;
			D = U - 128;
			E = V - 128;

			gray = (int)CLIP(((298 * C + 516 * D + 128) >> 8) * 0.30 + ((298 * C - 100 * D - 208 * E + 128) >> 8) * 0.59 + ((298 * C + 409 * E + 128) >> 8) * 0.11);
			a[yPos][xPos] = gray;
			//SetPixel(hdc, xWidth + xPos, yPos, RGB(gray, gray, gray));

			C = Y1 - 16;

			gray = (int)CLIP(((298 * C + 409 * E + 128) >> 8) * 0.30 + ((298 * C - 100 * D - 208 * E + 128) >> 8) * 0.59 + ((298 * C + 516 * D + 128) >> 8) * 0.11);
			a[yPos][xPos + 1] = gray;
			yuy2 += 4;

		}
		yuy2 += (2 * 2 * 3); // 2씩 저장된 pixel data * 양 끝 2군데 빼기 * 3개씩
	}
	yuy2 = 320 * 4;

	//**************************************************
	//fast algorithm
	unsigned char corner[yWidth][xWidth] = { 0, };

	int T = 30;
	//**************************************************
	CCamProcessDlg::RecAlgorithm(a, corner);
/*
#pragma omp parallel for

	for (int i = 4; i < yWidth - 3; i++) {
		for (int j = 4; j < xWidth - 3; j++) {
			unsigned char check = 0;
			for (int temp1 = 0; temp1 < 5; temp1++) {
				for (int temp2 = 0; temp2 < 5; temp2++) {
					if (corner[i - 2 + temp1][j - 2 + temp2] == 0) {
					}
					else check = 1;
				}
			}
			if (check == 1) {
			}
			else {

				int fast = 0;
				if (a[i - 3][j] - a[i][j] > T || a[i][j] - a[i - 3][j] > T)  // 1
					fast++;
				if (a[i - 3][j + 1] - a[i][j] > T || a[i][j] - a[i - 3][j + 1] > T)  // 2
					fast++;
				if (a[i - 2][j + 2] - a[i][j] > T || a[i][j] - a[i - 2][j + 2] > T)  // 3
					fast++;
				if (a[i - 1][j + 3] - a[i][j] > T || a[i][j] - a[i - 1][j + 3] > T)  // 4
					fast++;
				if (a[i][j + 3] - a[i][j] > T || a[i][j] - a[i][j + 3] > T)  // 5
					fast++;
				if (a[i + 1][j + 3] - a[i][j] > T || a[i][j] - a[i + 1][j + 3] > T)  // 6
					fast++;
				if (a[i + 2][j + 2] - a[i][j] > T || a[i][j] - a[i + 2][j + 2] > T)  // 7
					fast++;
				if (a[i + 3][j + 1] - a[i][j] > T || a[i][j] - a[i + 3][j + 1] > T)  // 8
					fast++;
				if (a[i + 3][j] - a[i][j] > T || a[i][j] - a[i + 3][j] > T)  // 9
					fast++;
				if (a[i + 3][j - 1] - a[i][j] > T || a[i][j] - a[i + 3][j - 1] > T)  // 10
					fast++;
				if (a[i + 2][j - 2] - a[i][j] > T || a[i][j] - a[i + 2][j - 2] > T)  // 11
					fast++;
				if (a[i + 1][j - 3] - a[i][j] > T || a[i][j] - a[i + 1][j - 3] > T)  // 12
					fast++;
				if (a[i][j - 3] - a[i][j] > T || a[i][j] - a[i][j - 3] > T)  // 13
					fast++;
				if (a[i - 1][j - 3] - a[i][j] > T || a[i][j] - a[i - 1][j - 3] > T)  // 14
					fast++;
				if (a[i - 2][j - 2] - a[i][j] > T || a[i][j] - a[i - 2][j - 2] > T)  // 15
					fast++;
				if (a[i - 3][j - 1] - a[i][j] > T || a[i][j] - a[i - 3][j - 1] > T)  // 16
					fast++;
				if (fast >= 9) {

					//corner[i][j][1] = 255;
					//corner[i][j][2] = 255;
					corner[i][j] = 255; //if fast >= 9
									  //SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
				}
				else {}
				//SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
			}
		}
	}
*/

/*
#pragma omp parallel for
	for (int i = 4; i < 120; i++) {
		for (int j = 4; j < 160; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
		for (int j = 160; j < 320; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}for (int j = 320; j < 480; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
		for (int j = 480; j < 638; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
	}

#pragma omp parallel for
	for (int i = 120; i < 240; i++) {
		for (int j = 4; j < 160; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
		for (int j = 160; j < 320; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}for (int j = 320; j < 480; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
		for (int j = 480; j < 638; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
	}

#pragma omp parallel for
	for (int i = 240; i < 360; i++) {
		for (int j = 4; j < 160; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
		for (int j = 160; j < 320; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}for (int j = 320; j < 480; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
		for (int j = 480; j < 638; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
	}

#pragma omp parallel for
	for (int i = 360; i < 478; i++) {
		for (int j = 4; j < 160; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
		for (int j = 160; j < 320; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}for (int j = 320; j < 480; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
		for (int j = 480; j < 638; j++) {
			if (corner[i][j] == 1)
				SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
			else
				SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
		}
	}
	*/
	
	//****************************************************************************
	// Grayscale end

	//****************************************************************************
	// test 2 : SetBitmapBits로 화면 보이기
	// 화면표시

	//CWnd* pWnd = NULL;
	
	//pWnd = (CWnd*)::GetDlgItem(m_hWndCap, IDC_TEST);
	//pWnd = CWnd::FromHandle(m_hWndCap);
	CDC* pDCc = alt_hWndCap->GetDC();

	CRect rect;
	alt_hWndCap->GetClientRect(&rect);

	CDC memDC;
	CBitmap *pOldBitmap, bitmap;

	memDC.CreateCompatibleDC(pDCc);
	bitmap.CreateCompatibleBitmap(pDCc, rect.right, rect.bottom);
	pOldBitmap = memDC.SelectObject(&bitmap);

	BITMAP bm;
	bitmap.GetObject(sizeof(BITMAP), (LPSTR)&bm);
	BYTE* pData = NULL;
	pData = (BYTE*)malloc(bm.bmWidthBytes * bm.bmHeight);
	memset(pData, 0x00, bm.bmWidthBytes * bm.bmHeight);
	bitmap.GetBitmapBits(bm.bmWidthBytes * bm.bmHeight, pData);
	
	RGBQUAD *pRgb = (RGBQUAD*)pData;
	for (int y = 0; y < bm.bmHeight; y++)
	{
		for (int x = 0; x < bm.bmWidth; x++)
		{
			int nPos = x + (y * bm.bmWidth);

			BYTE r = pRgb[nPos].rgbRed;
			BYTE g = pRgb[nPos].rgbGreen;
			BYTE b = pRgb[nPos].rgbBlue;

			pRgb[nPos].rgbRed = corner[y][x];
			pRgb[nPos].rgbGreen = corner[y][x];
			pRgb[nPos].rgbBlue = corner[y][x];
		}
	}
	
	bitmap.SetBitmapBits(bm.bmWidthBytes * bm.bmHeight, pData);
	//*********************************************************
	// free part
	for (int i = 0; i < yWidth; i++) {
		delete[] a[i];
	}
	delete[] a;
	free(pData);
	pData = NULL;
	//초기 두 int는 좌표
	pDCc->BitBlt(0, 0, rect.right, rect.bottom, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBitmap);

	bitmap.DeleteObject();
	memDC.DeleteDC();

	alt_hWndCap->ReleaseDC(pDCc);
	//*********************************************************
	// free end

	//****************************************************************************
	// 화면 표시 end
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


void CCamProcessDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnOK();
}


void CCamProcessDlg::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}


void CCamProcessDlg::OnBnClickedButtonLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString m_strPath;
	CStdioFile file;

	CFileException ex;
	CFileDialog dlg(FALSE, _T("*.txt"), NULL, OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT, _T("TXT FILES(*.txt)|*.txt|"), NULL);
	if (dlg.DoModal() == IDOK) {
		m_strPath = dlg.GetPathName();
		if (m_strPath.Right(4) != ".txt") {
			m_strPath += ".txt";
		}
		file.Open(m_strPath, CFile::modeCreate | CFile::modeReadWrite, &ex);
		UpdateData(TRUE);
		//file.WriteString(m_strStatus);
		file.Close();
	}
}


void CCamProcessDlg::OnBnClickedButtonSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

void CCamProcessDlg::RecAlgorithm(unsigned char* a[], unsigned char corner[][xWidth]) {

#pragma omp parallel for

	int T = 30;
	for (int i = 4; i < yWidth - 3; i++) {
		for (int j = 4; j < xWidth - 3; j++) {
			unsigned char check = 0;
			for (int temp1 = 0; temp1 < 5; temp1++) {
				for (int temp2 = 0; temp2 < 5; temp2++) {
					if (corner[i - 2 + temp1][j - 2 + temp2] == 0) {
					}
					else check = 1;
				}
			}
			if (check == 1) {
			}
			else {

				int fast = 0;
				if (a[i - 3][j] - a[i][j] > T || a[i][j] - a[i - 3][j] > T)  // 1
					fast++;
				if (a[i - 3][j + 1] - a[i][j] > T || a[i][j] - a[i - 3][j + 1] > T)  // 2
					fast++;
				if (a[i - 2][j + 2] - a[i][j] > T || a[i][j] - a[i - 2][j + 2] > T)  // 3
					fast++;
				if (a[i - 1][j + 3] - a[i][j] > T || a[i][j] - a[i - 1][j + 3] > T)  // 4
					fast++;
				if (a[i][j + 3] - a[i][j] > T || a[i][j] - a[i][j + 3] > T)  // 5
					fast++;
				if (a[i + 1][j + 3] - a[i][j] > T || a[i][j] - a[i + 1][j + 3] > T)  // 6
					fast++;
				if (a[i + 2][j + 2] - a[i][j] > T || a[i][j] - a[i + 2][j + 2] > T)  // 7
					fast++;
				if (a[i + 3][j + 1] - a[i][j] > T || a[i][j] - a[i + 3][j + 1] > T)  // 8
					fast++;
				if (a[i + 3][j] - a[i][j] > T || a[i][j] - a[i + 3][j] > T)  // 9
					fast++;
				if (a[i + 3][j - 1] - a[i][j] > T || a[i][j] - a[i + 3][j - 1] > T)  // 10
					fast++;
				if (a[i + 2][j - 2] - a[i][j] > T || a[i][j] - a[i + 2][j - 2] > T)  // 11
					fast++;
				if (a[i + 1][j - 3] - a[i][j] > T || a[i][j] - a[i + 1][j - 3] > T)  // 12
					fast++;
				if (a[i][j - 3] - a[i][j] > T || a[i][j] - a[i][j - 3] > T)  // 13
					fast++;
				if (a[i - 1][j - 3] - a[i][j] > T || a[i][j] - a[i - 1][j - 3] > T)  // 14
					fast++;
				if (a[i - 2][j - 2] - a[i][j] > T || a[i][j] - a[i - 2][j - 2] > T)  // 15
					fast++;
				if (a[i - 3][j - 1] - a[i][j] > T || a[i][j] - a[i - 3][j - 1] > T)  // 16
					fast++;
				if (fast >= 9) {

					//corner[i][j][1] = 255;
					//corner[i][j][2] = 255;
					corner[i][j] = 255; //if fast >= 9
										//SetPixel(hdc, xWidth + j, i, RGB(0, 0, 0));
				}
				else {}
				//SetPixel(hdc, xWidth + j, i, RGB(255, 255, 255));
			}
		}
	}
}



