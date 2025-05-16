#include "browser history.h"
#include <iostream>
#include <string> 

void displayMenu() {
    cout << "\n========== Browser History Navigator ==========\n";
    cout << "1. Visit a new page\n";
    cout << "2. Go back\n";
    cout << "3. Go forward\n";
    cout << "4. Show current page\n";
    cout << "5. Show entire history\n";
    cout << "6. Start new session\n";
    cout << "7. Go back multiple steps\n";
    cout << "8. Go forward multiple steps\n";
    cout << "9. Go to specific index\n";
    cout << "10. Find and go to URL\n";
    cout << "11. Go home\n";
    cout << "12. Go to end\n";
    cout << "13. Refresh current page\n";
    cout << "14. Set scroll position\n";
    cout << "15. Add form data\n";
    cout << "16. Set maximum history size\n";
    cout << "17. Clear History\n";
    cout << "18. Find pages by domain\n";
    cout << "19. Show available branches\n"; 
    cout << "20. Navigate to specific branch\n";  
    cout << "21. Show history tree\n";  
    cout << "22. Search by title\n";    
    cout << "23. Advanced search\n";
    cout << "24. Exit\n";
    cout << "Choose an option: ";
}

int main() {
    BrowserHistory history(10);

    int choice = 0;
    string url, title, key, value, domain;
    bool running = true;
    int steps, index, position, branchIndex;

    cout << "Welcome to Browser History Navigator!\n";

    history.addPage("https://www.example.com", "Example Homepage");
    history.addPage("https://www.example.com/about", "About Example");
    history.addPage("https://www.example.com://products", "Example Products");
    history.goBack();
    history.addPage("https://www.example.com/services", "Example Services");
    history.goToHome();
    history.addPage("https://www.example.com/contact", "Contact Example");
    history.goBack();
    history.addPage("https://www.example.com/blog", "Example Blog");
    cout << "Initialized with 5 sample pages.\n";

    while (running) {
        displayMenu();
        cin >> choice;

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch (choice) {
        case 1:
            cout << "Enter URL: ";
            getline(cin, url);
            cout << "Enter page title: ";
            getline(cin, title);
            history.addPage(url, title);
            cout << "Visited: " << title << endl;
            break;

        case 2:
            if (history.canGoBack()) {
                history.goBack();
                cout << "Navigated back to: " << history.getCurrentPage()->getTitle() << endl;
            }
            else {
                cout << "Cannot go back - at the beginning of history." << endl;
            }
            break;

        case 3:
            if (history.canGoForward()) {
                history.goForward();
                cout << "Navigated forward to: " << history.getCurrentPage()->getTitle() << endl;
            }
            else {
                cout << "Cannot go forward - at the end of history." << endl;
            }
            break;

        case 4:
            history.showCurrentPage();
            break;

        case 5:
            history.showHistory();
            break;

        case 6:
            history.startNewSession();
            cout << "Started new browsing session: " << history.getCurrentSessionID() << endl;
            break;

        case 7:
            cout << "Enter number of steps to go back...";
            cin >> steps;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (history.goBack(steps))
            {
                cout << "Navigated back to: " << history.getCurrentPage()->getTitle() << endl;
            }
            else
            {
                cout << "Could not go back requested number of steps" << endl;
            }
            break;

        case 8:
            cout << "Enter number of steps to go forward...";
            cin >> steps;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (history.goForward(steps))
            {
                cout << "Navigated forward to: " << history.getCurrentPage()->getTitle() << endl;
            }
            else
            {
                cout << "Could not go forward requested number of steps" << endl;
            }
            break;

        case 9:
            cout << "Enter index to navigate to: ";
            cin >> index;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            if (history.goToIndex(index))
            {
                cout << "Navigated to index " << index << ": " << history.getCurrentPage()->getTitle() << endl;
            }
            else
            {
                cout << "Invalid index. Valid range is 0-" << (history.getSize() - 1) << endl;
            }
            break;
        case 10:
            cout << "Enter URL to find: ";
            getline(cin, url);
            if (history.goToURL(url))
            {
                cout << "Navigated to: " << history.getCurrentPage()->getTitle() << endl;
            }
            else
            {
                cout << "URL not found in history"<<endl;
            }
            break;

        case 11:
            if (history.goToHome())
            {
                cout << "Navigated to first page: " << history.getCurrentPage()->getTitle() << endl;
            }
            else
            {
                cout << "History is empty " << endl;
            }
            break;

        case 12:
            if (history.goToEnd())
            {
                cout << "Navigated to recent page: " << history.getCurrentPage()->getTitle() << endl;
            }
            else
            {
                cout << "History is empty " << endl;
            }
            break;

        case 13:
            history.refresh();
            cout << "Refreshed current page: " << history.getCurrentPage()->getTitle() << endl;
            break;

        case 14:
            cout << "Enter scroll position in pixels: ";
            cin >> position;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            history.setScrollPosition(position);
            cout << "Scroll position set for " << history.getCurrentPage()->getTitle() << endl;
            break;

        case 15:
            cout << "Enter form field name: ";
            getline(cin, key);
            cout << "Enter form field value: ";
            getline(cin, value);
            history.addFormData(key, value);
            history.setScrollPosition(position);
            cout << "Form data added to " << history.getCurrentPage()->getTitle() << endl;
            break;

        case 16:
            cout << "Enter maximum history size: ";
            cin >> index;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            history.setMaxSize(index);
            cout << "Maximum history size set to " << history.getMaxSize() << endl;
            break;

        case 17:
            history.clearHistory();
            cout << "History cleared" << endl;
            break;

        case 18:
            cout << "Enter domain to search for: ";
            getline(cin, domain);
            {
                vector<Page*>results = history.findPagesByDomain(domain);
                cout << "Found " << results.size() << " pages on domain " << domain << " :" << endl;
                for (size_t i = 0; i < results.size(); i++)
                {
                    cout << i + 1 << ". " << results[i]->getTitle() << endl;
                    cout << "   " << results[i]->getUrl() << endl;
                }
            }
            break; 

        case 19:
        {
            int branchCount = history.branchCount();
            if (branchCount > 0) {
                vector<string> branches = history.listBranches();
                cout << "Available branches from current page (" << history.getCurrentPage()->getTitle() << "):" << endl;
                for (size_t i = 0; i < branches.size(); i++) {
                    cout << i << ": " << branches[i] << endl;
                }
            }
            else {
                cout << "No branches available from current page." << endl;
            }
        }
        break;

        case 20:
        {
            int branchCount = history.branchCount();
            if (branchCount > 0) {
                vector<string> branches = history.listBranches();
                cout << "Available branches from current page (" << history.getCurrentPage()->getTitle() << "):" << endl;
                for (size_t i = 0; i < branches.size(); i++) {
                    cout << i << ": " << branches[i] << endl;
                }

                cout << "Enter branch index to navigate to: ";
                cin >> branchIndex;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                if (history.forward(branchIndex)) {
                    cout << "Navigated to branch: " << history.getCurrentPage()->getTitle() << endl;
                }
                else {
                    cout << "Invalid branch index." << endl;
                }
            }
            else {
                cout << "No branches available from current page." << endl;
            }
        }
        break;

        case 21:
            history.printHistoryTree();
            break;

        case 22:
            cout << "Enter title to search for: ";
            getline(cin, title);
            {
                vector<Page*> results = history.findPagesByTitle(title);
                cout << "Found " << results.size() << " pages with title containing '" << title << "':" << endl;
                for (size_t i = 0; i < results.size(); i++)
                {
                    cout << i + 1 << ". " << results[i]->getTitle() << endl;
                    cout << "   " << results[i]->getUrl() << endl;
                }

                if (!results.empty())
                {
                    cout << "Navigate to one of these pages? (y/n): ";
                    string response;
                    getline(cin, response);
                    if (response == "y" || response == "Y")
                    {
                        cout << "Enter result number: ";
                        int resultIndex;
                        cin >> resultIndex;
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');

                        if (resultIndex > 0 && resultIndex <= results.size())
                        {
                            Page* selectedPage = results[resultIndex - 1];
                            history.goToURL(selectedPage->getUrl());
                            cout << "Navigated to: " << history.getCurrentPage()->getTitle() << endl;
                        }
                        else
                        {
                            cout << "Invalid result number." << endl;
                        }
                    }
                }
            }
            break;

        case 23:
        {
            string titleSearch, urlSearch, sessionSearch;
            time_t startTime = 0, endTime = 0;
            int useTimeFilter = 0;

            cout << "Enter title substring (leave empty to skip): ";
            getline(cin, titleSearch);

            cout << "Enter URL substring (leave empty to skip): ";
            getline(cin, urlSearch);

            cout << "Enter session ID (leave empty to skip): ";
            getline(cin, sessionSearch);

            cout << "Use time filter? (1: Yes, 0: No): ";
            cin >> useTimeFilter;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if (useTimeFilter)
            {
                int hoursStart, hoursEnd;
                cout << "Enter start time (hours back from now, 0 for no limit): ";
                cin >> hoursStart;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                cout << "Enter end time (hours back from now, 0 for no limit): ";
                cin >> hoursEnd;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                time_t currentTime = time(nullptr);
                if (hoursStart > 0)
                {
                    startTime = currentTime - (hoursStart * 3600);
                }
                if (hoursEnd > 0)
                {
                    endTime = currentTime - (hoursEnd * 3600);
                }
            }

            vector<Page*> results = history.searchPages(titleSearch, urlSearch, sessionSearch, startTime, endTime);

            cout << "Found " << results.size() << " matching pages:" << endl;
            for (size_t i = 0; i < results.size(); i++)
            {
                cout << i + 1 << ". " << results[i]->getTitle() << endl;
                cout << "   " << results[i]->getUrl() << endl;
                cout << "   Session: " << results[i]->getSessionID() << endl;

                char timeStr[26];
                time_t timestamp = results[i]->getTimestamp();
                ctime_s(timeStr, sizeof(timeStr), &timestamp);
                string timeString(timeStr);
                if (!timeString.empty() && timeString[timeString.length() - 1] == '\n')
                {
                    timeString.erase(timeString.length() - 1);
                }
                cout << "   Time: " << timeString << endl;
                cout << "----------------------------" << endl;
            }

            if (!results.empty())
            {
                cout << "Navigate to one of these pages? (y/n): ";
                string response;
                getline(cin, response);
                if (response == "y" || response == "Y")
                {
                    cout << "Enter result number: ";
                    int resultIndex;
                    cin >> resultIndex;
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    if (resultIndex > 0 && resultIndex <= results.size())
                    {
                        Page* selectedPage = results[resultIndex - 1];
                        history.goToURL(selectedPage->getUrl());
                        cout << "Navigated to: " << history.getCurrentPage()->getTitle() << endl;
                    }
                    else
                    {
                        cout << "Invalid result number." << endl;
                    }
                }
            }
        }
        break;

        case 24:
            cout << "Exiting program. Goodbye!" << endl;
            running = false;
            break;

        default:
            cout << "Invalid option. Please try again." << endl;
        }
    }

    return 0;
}