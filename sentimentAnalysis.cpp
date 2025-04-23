#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cctype>
#include <set>
using namespace std;

// dictionary
vector<string> positiveWords = {
    "good", "great", "excellent", "happy", "love", "awesome", "nice", "fantastic", "amazing", "wonderful", "positive", "joy", "delight", "brilliant", "pleasant"};
vector<string> negativeWords = {
    "bad", "terrible", "sad", "poor", "hate", "awful", "worse", "horrible", "disappointing", "angry", "negative", "miserable", "dreadful", "worst", "frustrated"};

int nextCommentID = 1;

// convert to lowercase for case-insensitive comparison
string toLower(string word)
{
    transform(word.begin(), word.end(), word.begin(), ::tolower);
    return word;
}

// sentiment score calculation
int getSentimentScore(const string &comment)
{
    stringstream ss(comment);
    string word;
    int score = 0;
    while (ss >> word)
    {
        word = toLower(word);
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        if (find(positiveWords.begin(), positiveWords.end(), word) != positiveWords.end())
            score++;
        else if (find(negativeWords.begin(), negativeWords.end(), word) != negativeWords.end())
            score--;
    }
    return score;
}

// BST node
struct Node
{
    int id;
    string user;
    string comment;
    int score;
    Node *left, *right;
    Node(int i, string u, string c, int s) : id(i), user(u), comment(c), score(s), left(nullptr), right(nullptr) {}
};

// insert into BST
Node *insert(Node *root, int id, string user, string comment, int score)
{
    if (!root)
        return new Node(id, user, comment, score);
    if (score < root->score)
        root->left = insert(root->left, id, user, comment, score);
    else
        root->right = insert(root->right, id, user, comment, score);
    return root;
}

// display BST (descending score)
void displayRanked(Node *root)
{
    if (!root)
        return;
    displayRanked(root->right);
    cout << "User: " << root->user << " | ID: " << root->id << " | Score: " << root->score << " | " << root->comment << endl;
    displayRanked(root->left);
}

// search by username
bool searchByUser(Node *root, const string &username)
{
    if (!root)
        return false;
    bool foundLeft = searchByUser(root->left, username);
    bool foundHere = false;
    if (toLower(root->user) == toLower(username))
    {
        cout << "ID: " << root->id << " | Score: " << root->score << " | Comment: " << root->comment << endl;
        foundHere = true;
    }
    bool foundRight = searchByUser(root->right, username);
    return foundLeft || foundHere || foundRight;
}

// delete comment by id
Node *deleteByID(Node *root, int id, bool &deleted)
{
    if (!root)
        return nullptr;

    root->left = deleteByID(root->left, id, deleted);
    root->right = deleteByID(root->right, id, deleted);

    if (root->id == id && !deleted)
    {
        char confirm;
        cout << "Are you sure you want to delete comment ID " << id << "? (y/n): ";
        cin >> confirm;
        if (tolower(confirm) != 'y')
        {
            cout << "Deletion cancelled.\n";
            return root;
        }

        deleted = true;

        // case 1 no children
        if (!root->left && !root->right)
        {
            delete root;
            return nullptr;
        }

        // case 2 one child
        if (!root->left)
        {
            Node *temp = root->right;
            delete root;
            return temp;
        }
        if (!root->right)
        {
            Node *temp = root->left;
            delete root;
            return temp;
        }

        // case 3: two children
        Node *succParent = root;
        Node *succ = root->right;
        while (succ->left)
        {
            succParent = succ;
            succ = succ->left;
        }

        root->id = succ->id;
        root->user = succ->user;
        root->comment = succ->comment;
        root->score = succ->score;

        root->right = deleteByID(root->right, succ->id, deleted);
    }

    return root;
}

// save tree
void saveToFile(Node *root, ofstream &file)
{
    if (!root)
        return;
    saveToFile(root->left, file);
    file << root->id << "|" << root->user << "|" << root->score << "|" << root->comment << endl;
    saveToFile(root->right, file);
}

void saveTree(Node *root, const string &filename)
{
    ofstream file(filename);
    if (!file)
    {
        cerr << "Could not save file.\n";
        return;
    }
    saveToFile(root, file);
    file.close();
}

// load tree
Node *loadTree(const string &filename)
{
    ifstream file(filename);
    if (!file)
        return nullptr;
    Node *root = nullptr;
    string line;
    while (getline(file, line))
    {
        stringstream ss(line);
        string idStr, user, scoreStr, comment;
        getline(ss, idStr, '|');
        getline(ss, user, '|');
        getline(ss, scoreStr, '|');
        getline(ss, comment);
        int id = stoi(idStr);
        int score = stoi(scoreStr);
        nextCommentID = max(nextCommentID, id + 1);
        root = insert(root, id, user, comment, score);
    }
    file.close();
    return root;
}

// display users
void collectUsers(Node *root, set<string> &users)
{
    if (!root)
        return;
    collectUsers(root->left, users);
    users.insert(root->user);
    collectUsers(root->right, users);
}

// top 5 comments
void topComments(Node *root, vector<Node *> &top)
{
    if (!root)
        return;
    topComments(root->left, top);
    top.push_back(root);
    topComments(root->right, top);
    sort(top.begin(), top.end(), [](Node *a, Node *b)
         { return b->score < a->score; });
    if (top.size() > 5)
        top.resize(5);
}

// average score per user
void collectScores(Node *root, map<string, pair<int, int>> &userScores)
{
    if (!root)
        return;
    collectScores(root->left, userScores);
    userScores[root->user].first += root->score;
    userScores[root->user].second++;
    collectScores(root->right, userScores);
}

int main()
{
    const string filename = "C:\\Users\\user\\OneDrive\\Documents\\Projects\\CPP\\SentimentAnalysis\\Sentiment.txt";
    Node *root = loadTree(filename);
    int choice;

    do
    {
        cout << "\n--- Sentiment Dashboard ---\n";
        cout << "1. Add comment\n";
        cout << "2. Display ranked comments\n";
        cout << "3. Search comments by username\n";
        cout << "4. Delete comment by ID\n";
        cout << "5. Users\n";
        cout << "6. Top 5 comments\n";
        cout << "7. Average score per user\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        while (!(cin >> choice))
        {
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Try again: ";
        }
        cin.ignore();

        if (choice == 1)
        {
            string user, comment;
            cout << "Enter username: ";
            getline(cin, user);
            cout << "Enter comment: ";
            getline(cin, comment);
            int score = getSentimentScore(comment);
            cout << "Score = " << score << endl;
            root = insert(root, nextCommentID++, user, comment, score);
        }

        else if (choice == 2)
        {
            displayRanked(root);
        }

        else if (choice == 3)
        {
            string name;
            cout << "Enter username to search: ";
            getline(cin, name);
            bool found = searchByUser(root, name);
            if (!found)
            {
                cout << "No comments found for user: " << name << ". Please try again.\n";
            }
        }

        else if (choice == 4)
        {
            int id;
            cout << "Enter comment ID to delete: ";
            while (!(cin >> id))
            {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid input. Try again: ";
            }
            bool deleted = false;
            root = deleteByID(root, id, deleted);
            if (deleted)
            {
                cout << "Comment deleted successfully.\n";
            }
        }

        else if (choice == 5)
        {
            set<string> users;
            collectUsers(root, users);
            cout << "--- Users displayed alphabetically ---\n";
            for (const auto &u : users)
                cout << u << endl;
        }

        else if (choice == 6)
        {
            vector<Node *> top;
            topComments(root, top);
            cout << "--- Top 5 comments ---\n";
            for (auto *n : top)
                cout << "User: " << n->user << " | Score: " << n->score << " | " << n->comment << endl;
        }

        else if (choice == 7)
        {
            map<string, pair<int, int>> userScores;
            collectScores(root, userScores);
            cout << "--- Average sentiment per user ---\n";
            for (const auto &entry : userScores)
            {
                double avg = (double)entry.second.first / entry.second.second;
                cout << "User: " << entry.first << " | Avg score: " << avg << endl;
            }
        }

        else if (choice == 0)
        {
            saveTree(root, filename);
            cout << "Saved and exiting...\n";
        }

        else
        {
            cout << "Invalid option.\n";
        }

    } while (choice != 0);

    return 0;
}
