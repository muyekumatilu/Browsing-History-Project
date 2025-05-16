#include "wxBrowserHistory.h"
#include <wx/aboutdlg.h>
#include <wx/datetime.h>
#include <sstream>
#include <wx/valtext.h>

enum
{
    ID_Back = wxID_HIGHEST + 1,
    ID_Forward,
    ID_Home,
    ID_Refresh,
    ID_Go,
    ID_Search,
    ID_SearchType,
    ID_HistoryTree,
    ID_NewSession,
    ID_ClearHistory,
    ID_URL_BAR,
    ID_SEARCH_BAR

};

BEGIN_EVENT_TABLE(BrowserHistoryFrame, wxFrame)
EVT_BUTTON(ID_Back, BrowserHistoryFrame::OnBackButton)
EVT_BUTTON(ID_Forward, BrowserHistoryFrame::OnForwardButton)
EVT_BUTTON(ID_Home, BrowserHistoryFrame::OnHomeButton)
EVT_BUTTON(ID_Refresh, BrowserHistoryFrame::OnRefreshButton)
EVT_BUTTON(ID_Go, BrowserHistoryFrame::OnGoButton)
EVT_BUTTON(ID_Search, BrowserHistoryFrame::OnSearchButton)
EVT_BUTTON(ID_NewSession, BrowserHistoryFrame::OnNewSessionButton)
EVT_BUTTON(ID_ClearHistory, BrowserHistoryFrame::OnClearHistoryButton)
EVT_TREE_SEL_CHANGED(ID_HistoryTree, BrowserHistoryFrame::OnTreeSelectionChanged)
EVT_TREE_ITEM_ACTIVATED(ID_HistoryTree, BrowserHistoryFrame::OnTreeItemActivated)
EVT_MENU(wxID_EXIT, BrowserHistoryFrame::OnExit)
EVT_MENU(wxID_ABOUT, BrowserHistoryFrame::OnAbout)
END_EVENT_TABLE()

IMPLEMENT_APP(BrowserHistoryApp)

bool BrowserHistoryApp::OnInit()
{
    BrowserHistoryFrame* frame = new BrowserHistoryFrame("Browser History Visualizer");
    frame->SetSize(1000, 700);
    frame->Center();
    frame->Show();
    return true;
}

BrowserHistoryFrame::BrowserHistoryFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title), m_pageViewFrame(nullptr)
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT, "E&xit\tAlt-X", "Quit this program");

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT, "&About\tF1", "Show about dialog");

    wxMenuBar* menuBar = new wxMenuBar();
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");
    SetMenuBar(menuBar);

    CreateStatusBar(2);
    SetStatusText("Ready");

    InitializeHistory();

    CreateControls();

    BindEvents();

    UpdateHistoryTree();
    RefreshNavigationButtons();
}

BrowserHistoryFrame::~BrowserHistoryFrame()
{
    if (m_history)
        delete m_history;

    if (m_pageViewFrame)
        m_pageViewFrame->Destroy();
}

void BrowserHistoryFrame::CreateControls()
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* navPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* navSizer = new wxBoxSizer(wxHORIZONTAL);

    m_backBtn = new wxButton(navPanel, ID_Back, "Back");
    m_forwardBtn = new wxButton(navPanel, ID_Forward, "Forward");
    m_homeBtn = new wxButton(navPanel, ID_Home, "Home");
    m_refreshBtn = new wxButton(navPanel, ID_Refresh, "Refresh");
    m_urlBar = new wxTextCtrl(navPanel, ID_URL_BAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    wxTextValidator urlValidator(wxFILTER_NONE);
    m_urlBar->SetValidator(urlValidator);
    m_goBtn = new wxButton(navPanel, ID_Go, "Go");

    navSizer->Add(m_backBtn, 0, wxALL, 5);
    navSizer->Add(m_forwardBtn, 0, wxALL, 5);
    navSizer->Add(m_homeBtn, 0, wxALL, 5);
    navSizer->Add(m_refreshBtn, 0, wxALL, 5);
    navSizer->Add(m_urlBar, 1, wxALL | wxEXPAND, 5);
    navSizer->Add(m_goBtn, 0, wxALL, 5);

    navPanel->SetSizer(navSizer);

    wxPanel* searchPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer* searchSizer = new wxBoxSizer(wxHORIZONTAL);

    m_searchBar = new wxTextCtrl(searchPanel, ID_SEARCH_BAR, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    wxTextValidator searchValidator(wxFILTER_NONE);
    m_searchBar->SetValidator(searchValidator);
    m_searchTypeChoice = new wxChoice(searchPanel, ID_SearchType);
    m_searchTypeChoice->Append("Title");
    m_searchTypeChoice->Append("URL");
    m_searchTypeChoice->Append("Session ID");
    m_searchTypeChoice->SetSelection(0);
    m_searchBtn = new wxButton(searchPanel, ID_Search, "Search");

    wxButton* newSessionBtn = new wxButton(searchPanel, ID_NewSession, "New Session");
    wxButton* clearHistoryBtn = new wxButton(searchPanel, ID_ClearHistory, "Clear History");

    searchSizer->Add(new wxStaticText(searchPanel, wxID_ANY, "Search:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    searchSizer->Add(m_searchBar, 1, wxALL | wxEXPAND, 5);
    searchSizer->Add(m_searchTypeChoice, 0, wxALL, 5);
    searchSizer->Add(m_searchBtn, 0, wxALL, 5);
    searchSizer->Add(newSessionBtn, 0, wxALL, 5);
    searchSizer->Add(clearHistoryBtn, 0, wxALL, 5);

    searchPanel->SetSizer(searchSizer);

    m_splitter = new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_BORDER | wxSP_LIVE_UPDATE);

    m_leftPanel = new wxPanel(m_splitter, wxID_ANY);
    m_rightPanel = new wxPanel(m_splitter, wxID_ANY);

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    m_historyTree = new wxTreeCtrl(m_leftPanel, ID_HistoryTree, wxDefaultPosition, wxDefaultSize,
        wxTR_DEFAULT_STYLE | wxTR_HIDE_ROOT | wxTR_MULTIPLE | wxTR_HAS_BUTTONS);
    leftSizer->Add(m_historyTree, 1, wxEXPAND | wxALL, 5);
    m_leftPanel->SetSizer(leftSizer);

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    wxStaticBox* detailsBox = new wxStaticBox(m_rightPanel, wxID_ANY, "Page Details");
    wxStaticBoxSizer* detailsSizer = new wxStaticBoxSizer(detailsBox, wxVERTICAL);

    m_titleLabel = new wxStaticText(m_rightPanel, wxID_ANY, "Title: ");
    m_urlLabel = new wxStaticText(m_rightPanel, wxID_ANY, "URL: ");
    m_timestampLabel = new wxStaticText(m_rightPanel, wxID_ANY, "Timestamp: ");
    m_sessionIdLabel = new wxStaticText(m_rightPanel, wxID_ANY, "Session ID: ");
    m_scrollPosLabel = new wxStaticText(m_rightPanel, wxID_ANY, "Scroll Position: ");

    detailsSizer->Add(m_titleLabel, 0, wxALL | wxEXPAND, 5);
    detailsSizer->Add(m_urlLabel, 0, wxALL | wxEXPAND, 5);
    detailsSizer->Add(m_timestampLabel, 0, wxALL | wxEXPAND, 5);
    detailsSizer->Add(m_sessionIdLabel, 0, wxALL | wxEXPAND, 5);
    detailsSizer->Add(m_scrollPosLabel, 0, wxALL | wxEXPAND, 5);

    rightSizer->Add(detailsSizer, 1, wxEXPAND | wxALL, 5);

    m_rightPanel->SetSizer(rightSizer);

    m_splitter->SplitVertically(m_leftPanel, m_rightPanel);
    m_splitter->SetMinimumPaneSize(200);
    m_splitter->SetSashPosition(350);

    mainSizer->Add(navPanel, 0, wxEXPAND);
    mainSizer->Add(searchPanel, 0, wxEXPAND);
    mainSizer->Add(m_splitter, 1, wxEXPAND);

    SetSizer(mainSizer);
    Layout();

    m_rootId = m_historyTree->AddRoot("Root");
}

void BrowserHistoryFrame::BindEvents()
{
    m_urlBar->Bind(wxEVT_COMMAND_TEXT_ENTER, &BrowserHistoryFrame::OnGoButton, this);
    m_searchBar->Bind(wxEVT_COMMAND_TEXT_ENTER, &BrowserHistoryFrame::OnSearchButton, this);
}

void BrowserHistoryFrame::InitializeHistory()
{
    m_history = new BrowserHistory(100);

    m_history->addPage("https://www.example.com", "Example Home");
    m_history->addPage("https://www.example.com/about", "About Example");
    m_history->addPage("https://www.example.com/contact", "Contact Example");

    m_history->goBack();
    m_history->addPage("https://www.example.com/services", "Services Example");
    m_history->addPage("https://www.example.com/services/web", "Web Services");

    m_history->goBack(2);
    m_history->addPage("https://www.example.com/products", "Products Example");

    m_history->startNewSession();
    m_history->addPage("https://www.search.com", "Search Engine");
    m_history->addPage("https://www.news.com", "News Site");
}

void BrowserHistoryFrame::UpdateHistoryTree()
{
    m_historyTree->DeleteAllItems();
    m_pageToTreeItem.clear();

    m_rootId = m_historyTree->AddRoot("Root");

    Page* rootPage = m_history->getCurrentPage();
    while (rootPage->getParent() != nullptr) {
        rootPage = rootPage->getParent();
    }

    PopulateTree(rootPage, m_rootId);
    m_historyTree->ExpandAll();

    HighlightCurrentPage();
}

void BrowserHistoryFrame::PopulateTree(Page* page, const wxTreeItemId& parentId)
{
    if (!page) return;

    wxString itemText = wxString::Format("%s (%s)",
        wxString(page->getTitle()),
        wxString(page->getUrl()));

    wxTreeItemId itemId = m_historyTree->AppendItem(parentId, itemText);

    m_pageToTreeItem[page] = itemId;

    const vector<Page*>& children = page->getChildren();
    for (Page* child : children) {
        PopulateTree(child, itemId);
    }
}

void BrowserHistoryFrame::HighlightCurrentPage()
{
    Page* currentPage = m_history->getCurrentPage();
    if (!currentPage) return;

    auto it = m_pageToTreeItem.find(currentPage);
    if (it != m_pageToTreeItem.end()) {
        wxTreeItemId itemId = it->second;
        m_historyTree->SelectItem(itemId);
        m_historyTree->EnsureVisible(itemId);

        UpdatePageDetails(currentPage);
    }
}

void BrowserHistoryFrame::UpdatePageDetails(Page* page)
{
    if (!page) {
        m_titleLabel->SetLabel("Title: ");
        m_urlLabel->SetLabel("URL: ");
        m_timestampLabel->SetLabel("Timestamp: ");
        m_sessionIdLabel->SetLabel("Session ID: ");
        m_scrollPosLabel->SetLabel("Scroll Position: ");
        return;
    }

    m_titleLabel->SetLabel(wxString::Format("Title: %s", page->getTitle()));
    m_urlLabel->SetLabel(wxString::Format("URL: %s", page->getUrl()));

    wxDateTime dt((time_t)page->getTimestamp());
    m_timestampLabel->SetLabel(wxString::Format("Timestamp: %s", dt.Format("%Y-%m-%d %H:%M:%S")));

    m_sessionIdLabel->SetLabel(wxString::Format("Session ID: %s", page->getSessionID()));
    m_scrollPosLabel->SetLabel(wxString::Format("Scroll Position: %d", page->getScrollPosition()));

    m_urlBar->SetValue(page->getUrl());

    m_rightPanel->Layout();
}

void BrowserHistoryFrame::RefreshNavigationButtons()
{
    m_backBtn->Enable(m_history->canGoBack());
    m_forwardBtn->Enable(m_history->canGoForward());
}

Page* BrowserHistoryFrame::GetPageFromTreeItem(const wxTreeItemId& item)
{
    for (const auto& pair : m_pageToTreeItem) {
        if (pair.second == item) {
            return pair.first;
        }
    }
    return nullptr;
}

void BrowserHistoryFrame::ShowPageViewWindow(Page* page)
{
    if (m_pageViewFrame) {
        m_pageViewFrame->Destroy();
        m_pageViewFrame = nullptr;
    }

    if (!page) return;

    m_pageViewFrame = new wxFrame(this, wxID_ANY, "Page View: " + page->getTitle());

    wxPanel* panel = new wxPanel(m_pageViewFrame);
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxStaticText* title = new wxStaticText(panel, wxID_ANY, "Title: " + page->getTitle());
    wxStaticText* url = new wxStaticText(panel, wxID_ANY, "URL: " + page->getUrl());

    wxDateTime dt((time_t)page->getTimestamp());
    wxStaticText* timestamp = new wxStaticText(panel, wxID_ANY,
        "Visited: " + dt.Format("%Y-%m-%d %H:%M:%S"));

    wxStaticText* session = new wxStaticText(panel, wxID_ANY, "Session: " + page->getSessionID());

    sizer->Add(title, 0, wxALL, 10);
    sizer->Add(url, 0, wxALL, 10);
    sizer->Add(timestamp, 0, wxALL, 10);
    sizer->Add(session, 0, wxALL, 10);

    wxStaticText* note = new wxStaticText(panel, wxID_ANY,
        "This is a placeholder for the actual page content.");
    sizer->Add(note, 0, wxALL, 10);

    panel->SetSizer(sizer);

    m_pageViewFrame->SetSize(400, 300);
    m_pageViewFrame->Center();
    m_pageViewFrame->Show();
}

void BrowserHistoryFrame::OnBackButton(wxCommandEvent& event)
{
    if (m_history->goBack()) {
        UpdateHistoryTree();
        RefreshNavigationButtons();
    }
}

void BrowserHistoryFrame::OnForwardButton(wxCommandEvent& event)
{
    if (m_history->goForward()) {
        UpdateHistoryTree();
        RefreshNavigationButtons();
    }
}

void BrowserHistoryFrame::OnHomeButton(wxCommandEvent& event)
{
    if (m_history->goToHome()) {
        UpdateHistoryTree();
        RefreshNavigationButtons();
    }
}

void BrowserHistoryFrame::OnRefreshButton(wxCommandEvent& event)
{
    m_history->refresh();
    UpdateHistoryTree();
    RefreshNavigationButtons();
}

void BrowserHistoryFrame::OnGoButton(wxCommandEvent& event)
{
    wxString url = m_urlBar->GetValue();
    if (!url.IsEmpty()) {
        m_history->visit(url.ToStdString(), "Page at " + url.ToStdString());
        UpdateHistoryTree();
        RefreshNavigationButtons();
    }
}

void BrowserHistoryFrame::OnSearchButton(wxCommandEvent& event)
{
    wxString searchText = m_searchBar->GetValue();
    if (searchText.IsEmpty()) return;

    vector<Page*> results;
    int searchType = m_searchTypeChoice->GetSelection();

    if (searchType == 0) { 
        results = m_history->findPagesByTitle(searchText.ToStdString());
    }
    else if (searchType == 1) {  
        vector<Page*> allPages;
        Page* rootPage = m_history->getCurrentPage();
        while (rootPage->getParent() != nullptr) {
            rootPage = rootPage->getParent();
        }
        m_history->collectAllPages(rootPage, allPages);

        for (Page* page : allPages) {
            if (page->getUrl().find(searchText.ToStdString()) != string::npos) {
                results.push_back(page);
            }
        }
    }
    else if (searchType == 2) {  
        vector<Page*> allPages;
        Page* rootPage = m_history->getCurrentPage();
        while (rootPage->getParent() != nullptr) {
            rootPage = rootPage->getParent();
        }
        m_history->collectAllPages(rootPage, allPages);

        for (Page* page : allPages) {
            if (page->getSessionID() == searchText.ToStdString()) {
                results.push_back(page);
            }
        }
    }

    if (results.empty()) {
        wxMessageBox("No pages found matching '" + searchText + "'", "Search Results");
        return;
    }

    wxDialog* dialog = new wxDialog(this, wxID_ANY, "Search Results", wxDefaultPosition, wxSize(500, 400));
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxListBox* resultsList = new wxListBox(dialog, wxID_ANY);

    for (Page* page : results) {
        resultsList->Append(page->getTitle() + " (" + page->getUrl() + ")");
    }

    sizer->Add(resultsList, 1, wxEXPAND | wxALL, 10);

    wxButton* openBtn = new wxButton(dialog, wxID_ANY, "Open Selected");
    sizer->Add(openBtn, 0, wxALIGN_CENTER | wxALL, 10);

    dialog->SetSizer(sizer);
    openBtn->Bind(wxEVT_BUTTON, [this, resultsList, results, dialog](wxCommandEvent&) {
        int selection = resultsList->GetSelection();
        if (selection != wxNOT_FOUND && selection < (int)results.size()) {
            Page* selectedPage = results[selection];

            vector<Page*> path;
            Page* rootPage = m_history->getCurrentPage();
            while (rootPage->getParent() != nullptr) {
                rootPage = rootPage->getParent();
            }

            if (m_history->findPageInTree(rootPage, selectedPage->getUrl(), path)) {
                for (Page* pathPage : path) {
                    m_history->goToURL(pathPage->getUrl());
                }

                UpdateHistoryTree();
                RefreshNavigationButtons();
            }

            dialog->EndModal(wxID_OK);
        }
        });

    dialog->ShowModal();
    dialog->Destroy();
}

void BrowserHistoryFrame::OnTreeSelectionChanged(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    Page* page = GetPageFromTreeItem(itemId);

    if (page) {
        UpdatePageDetails(page);
    }
}

void BrowserHistoryFrame::OnTreeItemActivated(wxTreeEvent& event)
{
    wxTreeItemId itemId = event.GetItem();
    Page* page = GetPageFromTreeItem(itemId);

    if (page) {
        m_history->goToURL(page->getUrl());
        UpdateHistoryTree();
        RefreshNavigationButtons();

        ShowPageViewWindow(page);
    }
}

void BrowserHistoryFrame::OnNewSessionButton(wxCommandEvent& event)
{
    m_history->startNewSession();
    SetStatusText("New browsing session started.");
}

void BrowserHistoryFrame::OnClearHistoryButton(wxCommandEvent& event)
{
    wxMessageDialog dialog(this, "Are you sure you want to clear all browsing history?",
        "Confirm Clear History", wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);

    if (dialog.ShowModal() == wxID_YES) {
        m_history->clearHistory();
        UpdateHistoryTree();
        RefreshNavigationButtons();
        SetStatusText("Browsing history cleared.");
    }
}

void BrowserHistoryFrame::OnExit(wxCommandEvent& event)
{
    Close(true);
}

void BrowserHistoryFrame::OnAbout(wxCommandEvent& event)
{
    wxAboutDialogInfo info;
    info.SetName("Browser History Visualizer");
    info.SetVersion("1.0");
    info.SetDescription("A tool for visualizing and managing browser history with branching support.");
    info.SetCopyright("(C) 2025");

    wxAboutBox(info);
}
