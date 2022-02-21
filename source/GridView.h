#if !defined(AFX_GRIDVIEW_H__EF823E64_6EA0_11D4_8C1E_901544B20C2F__INCLUDED_)
#define AFX_GRIDVIEW_H__EF823E64_6EA0_11D4_8C1E_901544B20C2F__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GridView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridView view

class CGridView : public CListView
{
protected:
	CGridView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CGridView)

// Attributes
protected:
	int m_NumColumns;

// Operations
public:
	void EditFocusItem();

// Overrides
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CGridView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	static UINT GetDT_JUSTIFY(UINT lvcfmt);
	void FocusStateReset(UINT statemask);
	void FocusStateSet(UINT statemask);
	void GetColumnEdges(int nCol, CRect& rect);
	void MoveFocus(int dx, int dy);
	void StoreEditValue();
	int ColumnTest(int x);
	void FitFrame();

	int GetColumnCount();
	bool ActivateColoredCells();

	COLORREF m_clrText;
	COLORREF m_clrTextBk;
	COLORREF m_clrBkgnd;

	struct CELLCOLORIDX
	{
		char back;		// 0 is default. Index to m_Cell CPalette
		char text;		// 0 is default
		CELLCOLORIDX() { back = 0; text = 0; }
	};
	CELLCOLORIDX* m_CellColor;	// 2D array for each cell in grid
															// NULL if colored cells not active
	COLORREF m_CellPalette[16];	// Colors indexed by CELLCOLORIDX

	int m_iFocusItem;
	int m_iFocusSubItem;
	
	CEdit* m_pItemEdit;

	// Generated message map functions
protected:
	//{{AFX_MSG(CGridView)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnEditUpdate();
	afx_msg void OnEditChange();
	afx_msg void OnEditSetFocus();
	afx_msg void OnEditKillFocus();
	afx_msg void OnEditEnd();
	afx_msg void OnEditCancel();
	afx_msg void OnLVKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRIDVIEW_H__EF823E64_6EA0_11D4_8C1E_901544B20C2F__INCLUDED_)
