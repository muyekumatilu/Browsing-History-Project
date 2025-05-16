#include "page.h"
#include <sstream>
#include <algorithm>

using namespace std;

Page::Page(const string& url, const string& title, time_t timestamp, const string&sessionID)
	: url(url), title(title), timestamp(timestamp), parent(nullptr), exitTime(0), scrollPosition(0), sessionID(sessionID) {}

Page::~Page()
{
	for (Page* child : children)
	{
		delete child;
	}
	children.clear();
}

string Page::getUrl() const 
{
	return url;
}
string Page::getTitle() const
{
	return title;
}
time_t Page::getTimestamp() const
{
	return timestamp;
}
Page* Page::getPrev() const
{
	return parent;
}
Page* Page::getParent() const
{
	return parent;
}
Page* Page::getNext() const
{
	return children.empty() ? nullptr : children[0];
}
time_t Page::getExitTime() const
{
	return exitTime;
}
int Page::getScrollPosition() const
{
	return scrollPosition;
}
string Page::getSessionID() const
{
	return sessionID;
}
map<string, string> Page::getFormData() const
{
	return formData;
}
const vector<Page*>& Page::getChildren() const
{
	return children;
}

void Page::setParent(Page* parent)
{
	this->parent = parent;
}
void Page::setPrev(Page* prev)
{
	this->parent = prev;
}
void Page::setNext(Page* next)
{
	if (next && find(children.begin(), children.end(), next) == children.end())
	{
		children.push_back(next);
	}
}
void Page::setExitTime(time_t time) 
{
	exitTime = time;
}
void Page::setScrollPosition(int position)
{
	scrollPosition = position;
}
void Page::setFormData(const string& key, const string& value)
{
	formData[key] = value;
}

Page* Page::addChild(const string& url, const string& title, time_t timestamp, const string& sessionID)
{
	Page* newPage = new Page(url, title, timestamp, sessionID);
	newPage->setParent(this);
	children.push_back(newPage);
	return newPage;
}

bool Page::removeChild(Page* child)
{
	auto it = find(children.begin(), children.end(), child);
	if (it != children.end())
	{
		children.erase(it);
		delete child;
		return true;
	}
	return false;
}

Page* Page::findChild(const string& url) const
{
	for (Page* child : children)
	{
		if (child->url == url)
		{
			return child;
		}
	}
	return nullptr;
}

int Page::childCount() const
{
	return children.size();
}

string Page::toString() const
{
	stringstream ss;
	ss << "URL: " << url << "\n";
	ss << "Title: " << title << "\n";
	ss << "Timestamp: " << timestamp << "\n";
	return ss.str();

	char timeStr[26];
	ctime_s(timeStr, sizeof(timeStr), &timestamp);
	string timeString(timeStr);
	if (!timeString.empty() && timeString[timeString.length() - 1] == '\n')
	{
		timeString.erase(timeString.length() - 1);
	}

	if (exitTime > 0)
	{
		char exitTimeStr[26];
		ctime_s(exitTimeStr, sizeof(exitTimeStr), &exitTime);

		string exitTimeString(exitTimeStr);
		if(!exitTimeString.empty() && exitTimeString[exitTimeString.length() - 1 == 'n'])
		{
			exitTimeString.erase(exitTimeString.length() - 1);
		}
		ss << "\nExited: " << exitTimeString;
	}

	ss << "\nScroll Position: " << scrollPosition;

	if (!sessionID.empty())
	{
		ss << "\nSession ID: " << sessionID;
	}

	if (!formData.empty())
	{
		ss<<"\nForm Data: ";
		for (const auto& pair : formData)
		{
			ss << "\n" << pair.first << ":"<<pair.second;
		}
	}
	ss << "\nBranches: " << children.size();
	
}