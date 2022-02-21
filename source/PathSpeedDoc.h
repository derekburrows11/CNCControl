// PathSpeedDoc.h : header file
//

#if !defined(AFX_PATHSPEEDDOC_H__83DA8440_BA59_11D6_86C3_9AEDBD5B9926__INCLUDED_)
#define AFX_PATHSPEEDDOC_H__83DA8440_BA59_11D6_86C3_9AEDBD5B9926__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

//#include "PathDataObjects.h"
//#include "PathMove.h"
#include "PathSpeed.h"
#include "PathTimeStep.h"
#include "ControllerTracker.h"



class CPathDoc;
//class CLimitList;



/////////////////////////////////////////////////////////////////////////////
// CPathSpeedDoc document

class CPathSpeedDoc : public CDocument
{
protected:
	CPathSpeedDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPathSpeedDoc)

// Attributes
public:

	CMotionStateBasicArray m_MotionArray;
	CMotionStateBasicArray m_BezMotionArray;

	bool m_bArrayExtentsSet;
	SMotionStateBasic m_MinArrayVals;
	SMotionStateBasic m_MaxArrayVals;


	CControllerTracker m_ContTracker;		// for simulating
	CMotionStateBasicArray m_ActualMotionArray;
	CMotionStateBasicArray m_BezActualMotionArray;
	CMotionStateBasicArray m_RequestMotionArray;
	CMotionStateBasicArray m_BezRequestMotionArray;




// Operations
public:
	void FindSpeeds();
	void FindSpeedsStart();
	bool FindSpeedsMore();		// returns false when finished
	void FindSpeedsAllLimits();
	void SetPathDoc(CPathDoc* pPathDoc);
	int GetLastScanSegment() { return m_PathSpeed.GetLastScanSegment(); }


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPathSpeedDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPathSpeedDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	void FindBezierArray(CMotionStateBasicArray& arrBez, const CMotionStateBasicArray& arr);
	void FindArrayExtents(const CMotionStateBasicArray& arr);


protected:
	CPathDoc* m_pPathDoc;		// path the speeds are based on
	CLimitList m_LimitList;		// resulting speed limit list for path

	CPathSpeed m_PathSpeed;
	CPathTimeStep m_PathStepper;

	bool m_bProcessingPath;
	bool m_bSaveLimits;
	bool m_bSaveTimeResults;


	// Generated message map functions
protected:
	//{{AFX_MSG(CPathSpeedDoc)
	afx_msg void OnShowPathAnimate();
	afx_msg void OnUpdateFileSave(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATHSPEEDDOC_H__83DA8440_BA59_11D6_86C3_9AEDBD5B9926__INCLUDED_)
