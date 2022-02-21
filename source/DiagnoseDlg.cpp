// DiagnoseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "cnccontrol.h"
#include "MainFrm.h"

#include "DiagnoseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDiagnoseDlg dialog


CDiagnoseDlg::CDiagnoseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDiagnoseDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDiagnoseDlg)
	m_iAddrStyle = 1;
	m_iValueStyle = 1;
	m_bIncReg = FALSE;
	m_bIncRegBank = FALSE;
	m_bIncMemAddr = FALSE;
	m_strMemAddr = _T("");
	m_strMemValue = _T("");
	m_strRegBank = _T("");
	m_strRegAddr = _T("");
	m_strRegValue = _T("");
	//}}AFX_DATA_INIT
	m_iMemWordSize = 1;
	m_bMemValueValid = false;
	m_iMemAddr = 0;

	m_iRegWordSize = 1;
	m_bRegValueValid = false;
	m_iRegBank = 0;
	m_iRegAddr = 0;

	m_pCNCComms = NULL;

	Create(IDD);
	ShowValues();
}


void CDiagnoseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDiagnoseDlg)
	DDX_Radio(pDX, IDC_RADIO1, m_iAddrStyle);
	DDX_Radio(pDX, IDC_RADIO3, m_iValueStyle);
	DDX_Check(pDX, IDC_CHECK_INC_REG, m_bIncReg);
	DDX_Check(pDX, IDC_CHECK_INC_REG_BANK, m_bIncRegBank);
	DDX_Check(pDX, IDC_CHECK_INC_MEM_ADDR, m_bIncMemAddr);
	DDX_Text(pDX, IDC_EDIT_REG_BANK, m_strRegBank);
	DDV_MaxChars(pDX, m_strRegBank, 1);
	DDX_Text(pDX, IDC_EDIT_REG_ADDR, m_strRegAddr);
	DDV_MaxChars(pDX, m_strRegAddr, 10);
	DDX_Text(pDX, IDC_EDIT_REG_VALUE, m_strRegValue);
	DDV_MaxChars(pDX, m_strRegValue, 10);
	DDX_Text(pDX, IDC_EDIT_MEM_ADDR, m_strMemAddr);
	DDV_MaxChars(pDX, m_strMemAddr, 10);
	DDX_Text(pDX, IDC_EDIT_MEM_VALUE, m_strMemValue);
	DDV_MaxChars(pDX, m_strMemValue, 10);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDiagnoseDlg, CDialog)
	//{{AFX_MSG_MAP(CDiagnoseDlg)
	ON_BN_CLICKED(IDC_BUTTON_BYTES_MEM, OnSetBytesMem)
	ON_BN_CLICKED(IDC_BUTTON_REQ_MEM, OnRequestMem)
	ON_BN_CLICKED(IDC_BUTTON_SEND_MEM, OnSendMem)
	ON_BN_CLICKED(IDC_BUTTON_BYTES_REG, OnSetBytesReg)
	ON_BN_CLICKED(IDC_BUTTON_REQ_REG, OnRequestReg)
	ON_BN_CLICKED(IDC_BUTTON_SEND_REG, OnSendReg)
	ON_BN_CLICKED(IDC_RADIO1, OnChangeStyle)
	ON_BN_CLICKED(IDC_RADIO2, OnChangeStyle)
	ON_BN_CLICKED(IDC_RADIO3, OnChangeStyle)
	ON_BN_CLICKED(IDC_RADIO4, OnChangeStyle)
	//}}AFX_MSG_MAP
	ON_CONTROL_RANGE(EN_KILLFOCUS, 1002, 1035, OnKillfocusEdit)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDiagnoseDlg message handlers

void CDiagnoseDlg::OnOK()
{
//	((CMainFrame*)GetParent())->CloseDiagnoseBox();
}

void CDiagnoseDlg::OnCancel()
{
	((CMainFrame*)GetParent())->CloseDiagnoseBox();
}

void CDiagnoseDlg::PostNcDestroy() 
{
// TODO: Add your specialized code here and/or call the base class

	CDialog::PostNcDestroy();		// Does nothing anyway
//	delete this;						// Done in CMainFrame which new'ed it
}



void CDiagnoseDlg::OnSetBytesMem() 
{
	if (++m_iMemWordSize > 2)
		m_iMemWordSize = 1;
	char label[1+1];
	itoa(m_iMemWordSize, label, 10);
	GetDlgItem(IDC_BUTTON_BYTES_MEM)->SetWindowText(label);
	ShowValues();			// update value formats
}

void CDiagnoseDlg::OnSetBytesReg() 
{
	if (++m_iRegWordSize > 2)
		m_iRegWordSize = 1;
	char label[1+1];
	itoa(m_iRegWordSize, label, 10);
	GetDlgItem(IDC_BUTTON_BYTES_REG)->SetWindowText(label);
	ShowValues();			// update value formats
}

void CDiagnoseDlg::OnRequestMem() 
{
	UpdateData(true);
	int iBytes = m_iMemWordSize;
	if (m_bIncMemAddr)
	{
		iBytes |= 0x10;
		m_iMemAddr += m_iMemWordSize;
	}
	m_pCNCComms->SendMonitorMessage(WMU_REQUESTMEM, m_iMemAddr, iBytes);
}

void CDiagnoseDlg::OnSendMem() 
{
	UpdateData(true);
	int iValueBytes = m_iMemWordSize;
	if (m_bIncMemAddr)
	{
		iValueBytes |= 0x10;
		m_iMemAddr += m_iMemWordSize;
	}
	iValueBytes = MAKELONG(iValueBytes, m_iMemValue);
	m_pCNCComms->SendMonitorMessage(WMU_SENDMEM, m_iMemAddr, iValueBytes);
}

void CDiagnoseDlg::OnRequestReg()
{
	UpdateData(true);
	int iBankAddr = MAKEWORD(m_iRegAddr, m_iRegBank);
	int iBytes = m_iRegWordSize;
	m_pCNCComms->SendMonitorMessage(WMU_REQUESTREG, iBankAddr, iBytes);
}

void CDiagnoseDlg::OnSendReg()
{
	UpdateData(true);
	int iBankAddr = MAKEWORD(m_iRegAddr, m_iRegBank);
	int iValueBytes = MAKELONG(m_iRegWordSize, m_iRegValue);
	m_pCNCComms->SendMonitorMessage(WMU_SENDREG, iBankAddr, iValueBytes);
}




int CDiagnoseDlg::ScanValue(const char* sz, int* pVal, int iStyle)
{
	if (iStyle == 0)				// dec by default, hex or oct if spec
		return sscanf(sz, "%i", pVal);
	else if (iStyle == 1)		// hex
		return sscanf(sz, "%x", pVal);
	return 0;
}

void CDiagnoseDlg::PrintValue(char* sz, int iVal, int iStyle, int iMinWidth)
{
	if (iStyle == 0)				// dec
		sprintf(sz, "%i", iVal);
	else if (iStyle == 1)		// hex
		sprintf(sz, "0x%.*x", iMinWidth, iVal);
}

void CDiagnoseDlg::InterperateValues() 
{
	UpdateData(true);
	ScanValue(m_strMemAddr, &m_iMemAddr, m_iAddrStyle);
	m_bMemValueValid = ScanValue(m_strMemValue, &m_iMemValue, m_iValueStyle) == 1;
	ScanValue(m_strRegBank, &m_iRegBank);
	ScanValue(m_strRegAddr, &m_iRegAddr, m_iAddrStyle);
	m_bRegValueValid = ScanValue(m_strRegValue, &m_iRegValue, m_iValueStyle) == 1;

}

void CDiagnoseDlg::ShowValues() 
{
	PrintValue(m_strMemAddr.GetBuffer(10), m_iMemAddr, m_iAddrStyle, 4);
	if (m_iMemWordSize == 1)
		m_iMemValue &= 0x00ff;
	else
		m_iMemValue &= 0xffff;
	if (m_bMemValueValid)
		PrintValue(m_strMemValue.GetBuffer(10), m_iMemValue, m_iValueStyle, 2*m_iMemWordSize);
	else
		m_strMemValue = "";
	m_strMemAddr.ReleaseBuffer();
	m_strMemValue.ReleaseBuffer();

	PrintValue(m_strRegBank.GetBuffer(2), m_iRegBank);
	PrintValue(m_strRegAddr.GetBuffer(10), m_iRegAddr, m_iAddrStyle, 2);
	if (m_iRegWordSize == 1)
		m_iRegValue &= 0x00ff;
	else
		m_iRegValue &= 0xffff;
	if (m_bRegValueValid)
		PrintValue(m_strRegValue.GetBuffer(10), m_iRegValue, m_iValueStyle, 2*m_iRegWordSize);
	else
		m_strRegValue = "";
	m_strRegBank.ReleaseBuffer();
	m_strRegAddr.ReleaseBuffer();
	m_strRegValue.ReleaseBuffer();

	UpdateData(false);
}


void CDiagnoseDlg::OnChangeStyle() 
{
	UpdateData(true);
	ShowValues();
}

int CDiagnoseDlg::RxMemValue(int iAddr, int /*iBytes*/, int iValue)
{
	if ((BYTE)iAddr != (BYTE)m_iMemAddr)
		return 0;
	m_iMemValue = iValue;
	m_bMemValueValid = true;
	ShowValues();
	return 1;
}

int CDiagnoseDlg::RxRegValue(int iReg, int /*iBytes*/, int iValue)
{
	if (iReg != MAKEWORD(m_iRegAddr, m_iRegBank))
		return 0;
	m_iRegValue = iValue;
	m_bRegValueValid = true;
	ShowValues();
	return 1;
}

void CDiagnoseDlg::OnSetfocusEdit(UINT nID) 
{
	CEdit* pEdit = (CEdit*)GetDlgItem(nID);
	int iStart, iEnd;
	pEdit->GetSel(iStart, iEnd);
//	pEdit->SetSel(0, -1);
}


void CDiagnoseDlg::OnKillfocusEdit(UINT /* nID */) 
{
//	CEdit* pEdit = (CEdit*)GetDlgItem(nID);
	InterperateValues();
	ShowValues();
}
