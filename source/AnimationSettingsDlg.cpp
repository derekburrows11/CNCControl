// AnimationSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "AnimationSettingsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnimationSettingsDlg dialog


CAnimationSettingsDlg::CAnimationSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimationSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimationSettingsDlg)
	m_Rate = 0.0;
	//}}AFX_DATA_INIT
}


void CAnimationSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimationSettingsDlg)
	DDX_Text(pDX, IDC_EDIT_RATE, m_Rate);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimationSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimationSettingsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimationSettingsDlg message handlers
