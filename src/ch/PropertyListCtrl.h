/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef __PROPERTYLIST_H__
#define __PROPERTYLIST_H__

// Property Type
#define ID_PROPERTY_TEXT		1
#define ID_PROPERTY_BOOL		2
#define ID_PROPERTY_COLOR		3
#define ID_PROPERTY_FONT		4
#define ID_PROPERTY_PATH		5
#define ID_PROPERTY_DIR			6
#define ID_PROPERTY_CUSTOM		7
#define ID_PROPERTY_STATIC		8
#define ID_PROPERTY_COMBO_BTN	9
#define ID_PROPERTY_COMBO_LIST	10

// Message ID to parent
#define ID_PROPERTY_CHANGED		WM_USER+15

// Holds an item
typedef struct PropertyItem_t
{
	int					nType;
	int					nWidth;
	int					nAlignment;
	int					nPropertySelected;
	BOOL				bComboEditable;
	LOGFONT				LogFont;
	CBrush*				pBrush;
	CStringArray		csProperties;

	// custom
	void (*pfnCallback)(LPVOID, int, CPtrList*, int);
	LPVOID lpParam;		// ptr to the dialog
	int iParam;			// other data

} PROPERTYITEM;

/////////////////////////////////////////////////////////////////////////////
// CComboButton window
class CComboButton : public CButton
{
// Construction
public:
	CComboButton();
	virtual ~CComboButton();

	BOOL Create(CRect Rect, CWnd* pParent, UINT uID);

private:
	void DrawTriangle(CDC* pDC, CRect Rect);

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct );
	virtual void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);

protected:
	DECLARE_MESSAGE_MAP()

private:
	CPen*		m_pBkPen = nullptr;
	CBrush*		m_pBkBrush = nullptr;
	CBrush*		m_pBlackBrush = nullptr;
};


/////////////////////////////////////////////////////////////////////////////
// CPropertyListCtrl window
class CPropertyListCtrl : public CListBox
{
	int				m_nWidestItem = 0;
	BOOL			m_bDeleteFont = TRUE;
	BOOL			m_bBoldSelection = TRUE;
	BOOL			m_bChanged = FALSE;
	CPen*			m_pBorderPen = nullptr;
	CRect			m_CurRect;
	CFont*			m_pTextFont = nullptr;
	CFont*			m_pSelectedFont = nullptr;
	CFont*			m_pCurFont = nullptr;
	CString			m_csText;
	CBrush*			m_pCurBrush = nullptr;
	CBrush*			m_pBkBrush = nullptr;
	CBrush*			m_pBkHighlightBrush = nullptr;
	CBrush*			m_pBkPropertyBrush = nullptr;
	CButton*		m_pFontButton = nullptr;
	CButton*		m_pPathButton = nullptr;
	CButton*		m_pDirButton = nullptr;
	CButton*		m_pCustomButton = nullptr;
	CComboButton*	m_pComboButton = nullptr;
	CListBox*		m_pListBox = nullptr;

	COLORREF		m_crBorderColor = 0;
	COLORREF		m_crBkColor = 0;
	COLORREF		m_crTextColor = 0;
	COLORREF		m_crTextHighlightColor = 0;
	COLORREF		m_crHighlightColor = 0;
	COLORREF		m_crPropertyBkColor = 0;
	COLORREF		m_crPropertyTextColor = 0;

	// Controls
	CEdit*			m_pEditWnd = nullptr;

	// The item list
	CPtrList		m_Items;
	PROPERTYITEM*	m_pCurItem = nullptr;
	PROPERTYITEM*	m_pCurDrawItem = nullptr;

// Construction
public:
	CPropertyListCtrl();

// Attributes
private:
	// Helper Functions
	void	DrawItem(CDC* pDC, CRect ItemRect, BOOL bSelected);
	void	DrawPropertyText(CDC* pDC, CRect ItemRect);
	void	CreateControl(int nType);
	BOOL	SetProperty(PROPERTYITEM* pPropertyItem, int nType, CString csData);
	void	ParseProperties(PROPERTYITEM* pPropertyItem, CString csData);
public:
	void	HideControls();

// Operations
public:
	// GUI Functions
	void			SetFont(CFont* pFont);
	void			SetBkColor(COLORREF crColor);
	void			SetPropertyBkColor(COLORREF crColor);
	void			SetHighlightColor(COLORREF crColor);
	void			SetLineStyle(COLORREF crColor, int nStyle = PS_SOLID);
	inline	void	SetBoldSelection(BOOL bBoldSelection)			{ m_bBoldSelection = bBoldSelection; };
	inline	void	SetTextColor(COLORREF crColor)					{ m_crTextColor = crColor; };
	inline	void	SetTextHighlightColor(COLORREF crColor)			{ m_crTextHighlightColor = crColor; };
	inline	void	SetPropertyTextColor(COLORREF crColor)			{ m_crPropertyTextColor = crColor; };

	// Add the data
	BOOL	AddString(CString csText);
	BOOL	AddString(UINT nIDString);

	BOOL	AddString(CString csText, int nType, CString csData, int nPropertySelected = 0, int nAlignment = DT_LEFT, BOOL bComboEditable = FALSE);
	BOOL	AddString(UINT nIDString, int nType, UINT nIDData, int nPropertySelected = 0, int nAlignment = DT_LEFT, BOOL bComboEditable = FALSE);
	BOOL	AddString(UINT nIDString, int nType, CString csData, int nPropertySelected = 0, int nAlignment = DT_LEFT, BOOL bComboEditable = FALSE);
	BOOL	AddString(UINT nIDString, int nType, CString csData, void (*pfnCallback)(LPVOID, int, CPtrList*, int), LPVOID lpParam, int iParam, int nPropertySelected, int nAlignment = DT_LEFT, BOOL bComboEditable = FALSE);

	BOOL	AddString(CString csText, COLORREF crColor, int nAlignment = DT_LEFT);
	BOOL	AddString(UINT nIDString, COLORREF crColor, int nAlignment = DT_LEFT);

	BOOL	AddString(CString csText, CFont* pFont, int nAlignment = DT_LEFT);
	BOOL	AddString(UINT nIDString, CFont* pFont, int nAlignment = DT_LEFT);
	
	// Get the Data
	bool	GetProperty(int nItem, CString* pText);	
	bool	GetProperty(int nItem, bool* bValue);	
	bool	GetProperty(int nItem, COLORREF* crColor);	
	bool	GetProperty(int nItem, LOGFONT* LogFont);	
	bool	GetProperty(int nItem, CStringArray* pArray, int* /*SelectedItem = NULL*/);	
	bool	GetProperty(int nItem, int* SelectedItem, CString* csText = NULL);	

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyListCtrl)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT /*lpMeasureItemStruct*/);
	//}}AFX_VIRTUAL

// Implementation
public:
	void Init();
	virtual ~CPropertyListCtrl();
	void	Reset();
	void Reinit();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyListCtrl)
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);
	afx_msg void OnSelchange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDblclk();
	afx_msg void OnEditLostFocus();
	afx_msg void OnEditChange();
	afx_msg void OnFontPropertyClick();
	afx_msg void OnPathPropertyClick();
	afx_msg void OnDirPropertyClick();
	afx_msg void OnCustomPropertyClick();
	afx_msg void OnComboBoxClick();
	afx_msg void OnSelChange();
	afx_msg void OnListboxLostFocus();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
