#include <iostream>
#include <vector>
using namespace std;
//5234. Find Resultant Array After Removing Anagrams
bool isAnagrams(const string& s1, const string& s2) {
    if (s1.size() != s2.size())
        return false;
    vector<int> b_map(26, 0);
    for (char i: s1)
        b_map[i - 'a'] += 1;
    for (char i: s2)
        b_map[i - 'a'] -= 1;
    if (std::all_of(b_map.cbegin(), b_map.cend(), [](int i) { return i == 0; }))
        return true;
    return false;
}
vector<string> removeAnagrams(vector<string>& words) {
    for (int i = 1; i < words.size(); i++){
        if(isAnagrams(words[i],words[i-1]))
            words.erase(words.begin() + i--);
    }
    return words;
}
int main() {
    vector<string> words;
    words.emplace_back("bbaa");
    words.emplace_back("abab");
    vector<string> new_words = removeAnagrams(words);
    for(const string& s : new_words)
        cout<<s<<endl;
}
