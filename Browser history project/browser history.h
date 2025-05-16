#pragma once

#include <string>
#include "page.h"
#include <vector>

using namespace std;

class BrowserHistory
{
private:
	Page* root;
	Page* current;
	int size;
	int maxSize;
	string currentSessionID;

	void updatePrevExitTime();
	string generateSessionID() const;
	void countPages(Page* page, int& count) const;

	void printPage(const Page* page, int level) const;

public:
	BrowserHistory(int maxHistorySize = 50);
	~BrowserHistory();

	void addPage(const string& url, const string& title);
	bool canGoBack() const;
	bool canGoForward() const;
	bool goBack();
	bool goForward();
	bool goBack(int steps);
	bool goForward(int steps);
	bool goToIndex(int index);
	bool goToURL(const string& url);
	bool goToHome();
	bool goToEnd();
	void refresh();
	void pruneOldestPages();

	void visit(const string& url, const string& title);
	bool forward(int branchIndex);
	int branchCount() const;
	vector<string> listBranches() const;

	void setScrollPosition(int position);
	void addFormData(string& key, string& value);
	void startNewSession();

	void setMaxSize(int newMaxSize);


	Page* getCurrentPage() const;
	int getSize() const;
	int getMaxSize() const;
	int getCurrentIndex() const;
	string getCurrentSessionID() const;

	void showCurrentPage() const;
	void showHistory() const;

	void printHistoryTree() const;

	void clearHistory();
	void clearHistory(time_t olderThan);

	vector <Page*> findPagesByDomain(const string& domain) const;
	vector<Page*> findPagesByTitle(const string& titleSubstring) const;
	bool goToTitle(const string& titleSubstring);
	vector<Page*> searchPages(const string& titleSubstring = "", const string& urlSubstring = "", const string& sessionID = "", time_t startTime = 0, time_t endTime = 0) const;
	bool containsSubstring(const string& str, const string& substring) const;
	void collectAllPages(Page* page, vector<Page*>& pages) const;
	bool findPageInTree(Page* root, const string& url, vector<Page*>& path) const;
};
