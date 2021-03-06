
// CamProcessDlg.h: 헤더 파일
//

#pragma once

// Cam 연결을 위한 header 추가
#include <Vfw.h>
#pragma comment(lib, "vfw32.lib")

// Cam 연결 시 사용 - 영상 처리 부분


// CCamProcessDlg 대화 상자
class CCamProcessDlg : public CDialogEx
{
// 생성입니다.
public:
	HWND m_hWndCap;
	
	BITMAPINFO BInfo;
	static const int xWidth = 640;
	static const int yWidth = 480;

	CCamProcessDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	static LRESULT CALLBACK Emb_Draw(HWND hWnd, LPVIDEOHDR lp);
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CAMPROCESS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

};
