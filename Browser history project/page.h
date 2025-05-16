#pragma once

#include <string>
#include <ctime>
#include <map>
#include <vector>

using namespace std;

class Page
{
private:
	string url;
	time_t timestamp;
	string title;
	Page* parent;
	vector<Page*> children;
	time_t exitTime;
	int scrollPosition;
	string sessionID;
	map<string, string> formData;
public:
	Page(const string& url, const string& title, time_t timestamp, const string& sessionID = "");
	~Page();

	string getUrl() const;
	string getTitle() const;
	time_t getTimestamp() const;
	Page* getParent() const;
	Page* getPrev() const;
	Page* getNext() const;
	time_t getExitTime() const;
	int getScrollPosition() const;
	string getSessionID() const;
	map<string, string> getFormData() const;
	const vector<Page*>& getChildren() const;

	void setParent(Page* parent);
	void setPrev(Page* prev);
	void setNext(Page* next);
	void setExitTime(time_t time);
	void setScrollPosition(int position);
	void setFormData(const string& key, const string& value);

	Page* addChild(const string& url, const string& title, time_t timestamp, const string& sessionID = "");
	bool removeChild(Page* child);
	Page* findChild(const string& url) const;
	int childCount() const;

	string toString() const;
};