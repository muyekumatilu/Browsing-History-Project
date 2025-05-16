#include "browser history.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <stack>

using namespace std;

string extractDomain(const string& url)
{
	size_t start = 0;
	if (url.find("http://") == 0)
	{
		start = 7;
	}
	else if (url.find("https://") == 0)
	{
		start = 8;
	}
	size_t end = url.find('/', start);
	if (end == string::npos)
	{
		end = url.length();
	}
	return url.substr(start, end - start);
}

BrowserHistory::BrowserHistory(int maxHistorySize)                                                
	: root(nullptr), current(nullptr), size(0), maxSize(maxHistorySize), currentSessionID(generateSessionID()) {}

BrowserHistory::~BrowserHistory()                                               
{
	clearHistory();
}

string BrowserHistory::generateSessionID() const
{
	time_t now = time(nullptr);
	stringstream ss;
	ss << "session_" << hex << now;
	return ss.str();
}

void BrowserHistory::updatePrevExitTime()
{
	if (current != nullptr)
	{
		current->setExitTime(time(nullptr));
	}
}
void BrowserHistory::countPages(Page* page, int& count) const
{
	if (page == nullptr)
	{
		return;
	}
	count++;
	for (const auto& child : page->getChildren())
	{
		countPages(child, count);
	}
}

void BrowserHistory::pruneOldestPages()
{
	while (size <= maxSize && root == nullptr)
	{
		return;
	}
	vector<Page*> allPages;
	collectAllPages(root, allPages);
	sort(allPages.begin(), allPages.end(), [](Page* a, Page* b) {return a->getTimestamp() < b->getTimestamp(); });
	int pagesToRemove = size - maxSize;
	vector<Page*> pathToCurrent;
	for (Page* p = current; p != nullptr; p = p->getParent())
	{
		pathToCurrent.push_back(p);
	}
	for (Page* page : allPages)
	{
		if (pagesToRemove <= 0)
		{
			break;
		}
		if (page == root || find(pathToCurrent.begin(), pathToCurrent.end(), page) != pathToCurrent.end())
		{
			continue;
		}
		Page* parent = page->getParent();
		if (parent)
		{
			parent->removeChild(page);
			size--;
			pagesToRemove--;
		}
	}
}

void BrowserHistory::collectAllPages(Page* page, vector<Page*>& pages) const
{
	if (page == nullptr) 
	{
		return;
	}
	pages.push_back(page);
	for (const auto& child : page->getChildren())
	{
		collectAllPages(child, pages);
	}

}

vector <Page*> BrowserHistory::findPagesByDomain(const string& domain) const
{
	vector<Page*> results;
	vector<Page*> allPages;
	collectAllPages(root, allPages);
	for (Page* page : allPages)
	{
		string pageDomain = extractDomain(page->getUrl());
		if (pageDomain == domain)
		{
			results.push_back(page);
		}
	}
	return results;
}

bool BrowserHistory::containsSubstring(const string& str, const string& substring) const
{
	string strLower = str;
	string substringLower = substring;
	transform(strLower.begin(), strLower.end(), strLower.begin(), ::tolower);
	transform(substringLower.begin(), substringLower.end(), substringLower.begin(), ::tolower);
	return strLower.find(substringLower) != string::npos;
}

vector<Page*> BrowserHistory::findPagesByTitle(const string& titleSubstring) const
{
	vector<Page*> results;
	if (root == nullptr || titleSubstring.empty())
	{
		return results;
	}

	vector<Page*> allPages;
	collectAllPages(root, allPages);

	for (Page* page : allPages)
	{
		if (containsSubstring(page->getTitle(), titleSubstring))
		{
			results.push_back(page);
		}
	}

	return results;
}

bool BrowserHistory::goToTitle(const string& titleSubstring)
{
	vector<Page*> matchingPages = findPagesByTitle(titleSubstring);
	if (!matchingPages.empty())
	{
		updatePrevExitTime();
		current = matchingPages[0];
		return true;
	}
	return false;
}

vector<Page*> BrowserHistory::searchPages(const string& titleSubstring, const string& urlSubstring, const string& sessionID, time_t startTime, time_t endTime) const
{
	vector<Page*> results;
	vector<Page*> allPages;
	collectAllPages(root, allPages);

	for (Page* page : allPages)
	{
		bool match = true;
		if (!titleSubstring.empty() && !containsSubstring(page->getTitle(), titleSubstring))
		{
			match = false;
		}
		if (match && !urlSubstring.empty() && !containsSubstring(page->getUrl(), urlSubstring))
		{
			match = false;
		}
		if (match && !sessionID.empty() && page->getSessionID() != sessionID)
		{
			match = false;
		}
		time_t timestamp = page->getTimestamp();
		if (match && startTime > 0 && timestamp < startTime)
		{
			match = false;
		}
		if (match && endTime > 0 && timestamp > endTime)
		{
			match = false;
		}

		if (match)
		{
			results.push_back(page);
		}
	}

	return results;
}

void BrowserHistory::addPage(const string& url, const string& title)            
{
	updatePrevExitTime();

	Page* newPage = new Page(url, title, time(nullptr), currentSessionID);                        
	if (root == nullptr)                                                        
	{
		root = current = new Page(url, title, time(nullptr), currentSessionID);                                        
	}
	else
	{
		if (current->getNext() != nullptr)                                                   
		{
			vector<Page*> children = current->getChildren();
			for (Page* child : children)
			{
				current->removeChild(child);
			}
		}

		current = current->addChild(url, title, time(nullptr), currentSessionID);
	}
	size++; 		

	pruneOldestPages();
}

void BrowserHistory::visit(const string& url, const string& title)
{
	updatePrevExitTime();
	if (root == nullptr)
	{
		root = current = new Page(url, title, time(nullptr), currentSessionID);
	}
	else
	{
		Page* existingChild = current->findChild(url);
		if (existingChild)
		{
			current = existingChild;
		}
		else
		{
			current = current->addChild(url, title, time(nullptr), currentSessionID);
		}
	}
	size++;
	pruneOldestPages();
}

bool BrowserHistory::canGoBack() const                                        
{
	return (current !=nullptr && current != root);                            
}

bool BrowserHistory::canGoForward() const                                      
{
	return (current != nullptr && !current->getChildren().empty());                            
}

bool BrowserHistory::goBack()  
{
	if (canGoBack()) 
	{
		current = current->getParent();
		return true;  
	}
	else
	{
		return false;
	}
}

bool BrowserHistory::goForward() 
{
	if(canGoForward()) 
	{
		current = current->getChildren()[0];
		return true;
	}
	else
	{
		return false;
	}
}

bool BrowserHistory::goBack(int steps)
{
	if (steps <= 0)
	{
		return false;
	}
	for (int i = 0; i < steps; i++)
	{
		if (!canGoBack())
		{
			return(i > 0);
		}
		updatePrevExitTime();
		current = current->getPrev();
	}
	return true;
}

bool BrowserHistory::forward(int branchIndex)
{
	if (current != nullptr)
	{
		const vector<Page*> children = current->getChildren();
		if (branchIndex >= 0 && branchIndex < children.size())
		{
			updatePrevExitTime();
			current = children[branchIndex];
			return true;
		}
	}
	return false;
}

bool BrowserHistory::goForward(int steps)
{
	if (steps <= 0)
	{
		return false;
	}
	for (int i = 0; i < steps; i++)
	{
		if (!canGoForward())
		{
			return(i > 0);
		}
		updatePrevExitTime();
		current = current->getNext();
	}
	return true;
}

bool BrowserHistory::goToIndex(int index)
{
	if (index < 0 || index >= size)
	{
		return false;
	}
	vector<Page*> orderedPages;
	vector<Page*> queue = { root };
	while (!queue.empty())
	{
		Page* current = queue.front();
		queue.erase(queue.begin());
		orderedPages.push_back(current);
		for (Page* child : current->getChildren())
		{
			queue.push_back(child);
		}
	}
	if (index < orderedPages.size())
	{
		updatePrevExitTime();
		current = orderedPages[index];
		return true;
	}
	return false;
}

bool BrowserHistory::goToURL(const string& url) 
{
	vector<Page*> path;
	if (findPageInTree(root, url, path) && !path.empty())
	{
		updatePrevExitTime();
		current = path.back();
		return true;
	}

	return false;
}

bool BrowserHistory::findPageInTree(Page* root, const string& url, vector<Page*>& path) const
{
	if (root == nullptr) 
	{
		return false;
	}
	path.push_back(root);
	if (root->getUrl() == url)
	{
		return true;
	}
	for (Page* child : root->getChildren())
	{
		if (findPageInTree(child, url, path))
		{
			return true;
		}
	}
	path.pop_back();
	return false;
}

bool BrowserHistory::goToHome()
{
	if (root == nullptr)
	{
		return false;
	}
	updatePrevExitTime();
	current = root;
	return true;
}

bool BrowserHistory::goToEnd()
{
	if (root == nullptr)
	{
		return false;
	}
	vector<Page*> allPages;
	collectAllPages(root, allPages);
	Page* mostRecent = nullptr;
	time_t latestTime = 0;
	for (Page* page : allPages)
	{
		if (page->getChildren().empty() && page->getTimestamp() > latestTime)
		{
			mostRecent = page;
			latestTime = page->getTimestamp();
		}
	}
	if (mostRecent)
	{
		updatePrevExitTime();
		current = mostRecent;
		return true;
	}
	return false;
}

void BrowserHistory::refresh()
{
	if (current == nullptr)
	{
		return;
	}
	visit(current->getUrl(), current->getTitle());
}

void BrowserHistory::setScrollPosition(int position)
{
	if (current != nullptr)
	{
		current->setScrollPosition(position);
	}
}

void BrowserHistory::addFormData(string& key, string& value)
{
	if (current != nullptr)
	{
		current->setFormData(key, value);
	}
}

void BrowserHistory::startNewSession()
{
	currentSessionID = generateSessionID();
}

void BrowserHistory::setMaxSize(int newMaxSize)
{
	if (newMaxSize < 1)
	{
		return;
	}
	maxSize = newMaxSize;
	pruneOldestPages();
}

Page* BrowserHistory::getCurrentPage() const
{
	return current;

}

int BrowserHistory::getSize() const
{
	return size;
}

int BrowserHistory::getMaxSize() const
{
	return maxSize;
}

int BrowserHistory::getCurrentIndex() const
{
	if (current == nullptr || root == nullptr)
	{
		return -1;
	}
	vector<Page*> queue = { root }; 
	int index = 0;
	while (!queue.empty())
	{
		Page* page = queue.front();
		queue.erase(queue.begin());
		if (page == current)
		{
			return index;
		}
		index++;
		for (Page* child : page->getChildren())
		{
			queue.push_back(child);
		}
	}
	return -1;
}

string BrowserHistory::getCurrentSessionID() const
{
	return currentSessionID;
}

int BrowserHistory::branchCount() const
{
	return current ? current->getChildren().size() : 0;
}

vector<string> BrowserHistory::listBranches() const
{
	vector<string> branches;
	if (current)
	{
		for (const Page* child : current->getChildren())
		{
			branches.push_back(child->getTitle() + "(" + child->getUrl() + ")");
		}
	}
	return branches;
}

void BrowserHistory::showCurrentPage() const
{
	if (current == nullptr)
	{
		cout << "No current page" << endl;
	}
	else
	{
		cout << "Current page: " << current->toString() << endl;

		const vector<Page*>& branches = current->getChildren();
		if (!branches.empty())
		{
			cout << "\nAvailable branches: "<<endl;
			for (size_t i = 0; i < branches.size(); i++)
			{
				cout << i << ": " << branches[i]->getTitle() << ": " << branches[i]->getUrl() << ")" << endl;
			}
		}
	}
}

void BrowserHistory::showHistory() const
{
	if (root == nullptr)
	{
		cout << "No browsing history" << endl;
		return;
	}
	cout << "Browsing history: " << size << " pages" << endl;
	printHistoryTree();
}

void::BrowserHistory::printHistoryTree() const
{
	if (root == nullptr)
	{
		cout << "Empty history" << endl;
		return;
	}
	cout << "History tree: " << endl;
	printPage(root, 0);
}

void BrowserHistory::printPage(const Page* page, int level) const
{
	if (page == nullptr)
	{
		return;
	}
	string indent(level * 2, ' ');
	cout << indent << "_ " << page->getTitle();
	if (page == current)
	{
		cout << "(current)";
	}
	cout << endl;
	cout << indent << " " << page->getUrl() << endl;
	for (const Page* child : page->getChildren())
	{
		printPage(child, level + 1);
	}
}

void BrowserHistory::clearHistory(time_t olderThan)
{
	if (root == nullptr)
	{
		return;
	}
	vector<Page*> allPages;
	collectAllPages(root, allPages);

	sort(allPages.begin(), allPages.end(), [](Page* a, Page* b) {return a->getTimestamp() < b->getTimestamp(); });
	vector<Page*> pathToCurrent;
	for (Page* p = current; p != nullptr; p = p->getParent())
	{
		pathToCurrent.push_back(p);
	}
	for (Page* page : allPages)
	{
		if (page->getTimestamp() >= olderThan)
		{
			break;
		}
		if (page == root || find(pathToCurrent.begin(), pathToCurrent.end(), page) != pathToCurrent.end())
		{
			continue;
		}
		Page* parent = page->getParent();
		if (parent)
		{
			parent->removeChild(page);
			size--;
		}
	}
}

void BrowserHistory::clearHistory()
{
	if (root != nullptr)
	{
		delete root;
		root = nullptr;
	}
	size = 0;
	current = nullptr;
}

