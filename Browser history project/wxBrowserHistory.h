#pragma once

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/statline.h>
#include <wx/datetime.h>
#include "browser history.h"
#include "page.h"

class BrowserHistoryFrame : public wxFrame
{
public:
    BrowserHistoryFrame(const wxString& title);
    ~BrowserHistoryFrame();

private:
    BrowserHistory* m_history;

    wxSplitterWindow* m_splitter;
    wxPanel* m_leftPanel;
    wxPanel* m_rightPanel;

    wxTreeCtrl* m_historyTree;
    wxTreeItemId m_rootId;
    std::map<Page*, wxTreeItemId> m_pageToTreeItem;

    wxButton* m_backBtn;
    wxButton* m_forwardBtn;
    wxButton* m_homeBtn;
    wxButton* m_refreshBtn;
    wxTextCtrl* m_urlBar;
    wxButton* m_goBtn;

    wxTextCtrl* m_searchBar;
    wxButton* m_searchBtn;
    wxChoice* m_searchTypeChoice;

    wxStaticText* m_titleLabel;
    wxStaticText* m_urlLabel;
    wxStaticText* m_timestampLabel;
    wxStaticText* m_sessionIdLabel;
    wxStaticText* m_scrollPosLabel;

    wxFrame* m_pageViewFrame;

    void CreateControls();
    void BindEvents();
    void InitializeHistory();
    void UpdateHistoryTree();
    void UpdatePageDetails(Page* page);
    void RefreshNavigationButtons();
    void ShowPageViewWindow(Page* page);

    void PopulateTree(Page* page, const wxTreeItemId& parentId);
    void HighlightCurrentPage();
    Page* GetPageFromTreeItem(const wxTreeItemId& item);

    void OnBackButton(wxCommandEvent& event);
    void OnForwardButton(wxCommandEvent& event);
    void OnHomeButton(wxCommandEvent& event);
    void OnRefreshButton(wxCommandEvent& event);
    void OnGoButton(wxCommandEvent& event);
    void OnSearchButton(wxCommandEvent& event);
    void OnTreeSelectionChanged(wxTreeEvent& event);
    void OnTreeItemActivated(wxTreeEvent& event);
    void OnNewSessionButton(wxCommandEvent& event);
    void OnClearHistoryButton(wxCommandEvent& event);

    void OnExit(wxCommandEvent& event);
    void OnAbout(wxCommandEvent& event);

    DECLARE_EVENT_TABLE()
};

class BrowserHistoryApp : public wxApp
{
public:
    virtual bool OnInit();
};

DECLARE_APP(BrowserHistoryApp)
