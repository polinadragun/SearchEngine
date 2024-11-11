
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <filesystem>
#include <cctype>

class Indexingprogramm {
public:
    struct TrieNode {
    std::map<char, TrieNode*> children;
    int pageOffset = -1;  
    int count_appear_in_doc = 0;
    std::vector<int> lines;
};

static const int PAGE_SIZE = 4096;  

class Trie {

public:
    TrieNode root;

    void saveTrieToFileHelper(std::fstream& file, TrieNode* node) {
            if (node == nullptr) return;

            file.write(reinterpret_cast<const char*>(&node->count_appear_in_doc), sizeof(node->count_appear_in_doc));
            file.write(reinterpret_cast<const char*>(&node->pageOffset), sizeof(node->pageOffset));

            size_t linesSize = node->lines.size();
            file.write(reinterpret_cast<const char*>(&linesSize), sizeof(linesSize));
            file.write(reinterpret_cast<const char*>(node->lines.data()), linesSize * sizeof(int));

            size_t childrenSize = node->children.size();
            file.write(reinterpret_cast<const char*>(&childrenSize), sizeof(childrenSize));
            for (const auto& pair : node->children) {
                file.write(&pair.first, sizeof(pair.first));
                saveTrieToFileHelper(file, pair.second);
            }
        }

    void saveTrieToFile(std::fstream& file) {
            if (!file) {
                std::cerr << "Failed to open file for saving Trie." << std::endl;
                return;
            }
            saveTrieToFileHelper(file, &root);
            file.close();
        }
    
    int insert(const std::string& term, std::fstream& file) {
    TrieNode* node = &root;
    for (char c : term) {
        if (node->children.count(c) == 0) {
            node->children[c] = new TrieNode();
        }
        node = node->children[c];
    }
    if (node->pageOffset == -1) {  

        
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << std::endl;
            return -1; 
        }

        int initialSize = 0;
        file.seekp(0, std::ios_base::end);
        node->pageOffset = file.tellp();
        char* pagePlaceholder = new char[PAGE_SIZE];
        for (int i = 0; i < PAGE_SIZE; ++i) {
            pagePlaceholder[i] = 0;
        }
        file.write(pagePlaceholder, PAGE_SIZE); 
        file.seekp(node->pageOffset);
        file.write(reinterpret_cast<char*>(&initialSize), sizeof(initialSize)); 
        
        delete[] pagePlaceholder; 

        file.seekp(node->pageOffset + PAGE_SIZE); 
        if (file.fail()) {
            std::cerr << "Error occurred while writing to the file." << std::endl;
            file.clear(); 
        }
    }
    return node->pageOffset;
}


    int search(const std::string& term) {
        TrieNode* node = &root;
        for (char c : term) {
            if (node->children.count(c) == 0) {
                return -1;  
            }
            node = node->children[c];
        }
        return node->pageOffset;
    }
    TrieNode* search_node(const std::string& term) {
        TrieNode* node = &root;
        for (char c : term) {
            if (node->children.count(c) == 0) {
                return nullptr;  
            }
            node = node->children[c];
        }
        return node;
    }
    void resetCounts() {
    resetCountsHelper(&root);
}

void resetCountsHelper(TrieNode* node) {
    if (node == nullptr) return;

    node->count_appear_in_doc = 0;

    for (auto& child : node->children) {
        resetCountsHelper(child.second);
    }
}
};

Trie trie;
int all_lens = 0;
int all_documents_count = 0;

void AddDocument(const std::string& term, int doc_path_pos, std::string& path, std::fstream& file, int file_len, int lines_vec_pos) {
    file.seekp(0, std::ios_base::end);
    int pageOffset = trie.insert(term, file);
    int appeared_already = trie.search_node(term)->count_appear_in_doc;
    if (appeared_already == -1) {
        return;
    }
    trie.search_node(term)->count_appear_in_doc = -1;
    file.seekg(pageOffset);
    int numDocs;
    file.read(reinterpret_cast<char*>(&numDocs), sizeof(numDocs));
    numDocs++;
    file.seekp(pageOffset);
    file.write(reinterpret_cast<char*>(&numDocs), sizeof(numDocs));  
    int all_terms = appeared_already;
    int all_words = file_len;  
    int docs_uniq_num = all_documents_count;
    file.seekp(pageOffset + sizeof(int) + numDocs * sizeof(int)*5 - sizeof(int) * 5);
    file.write(reinterpret_cast<char*>(&doc_path_pos), sizeof(doc_path_pos));
    file.write(reinterpret_cast<char*>(&docs_uniq_num), sizeof(docs_uniq_num));
    file.write(reinterpret_cast<char*>(&lines_vec_pos), sizeof(lines_vec_pos));

    file.write(reinterpret_cast<char*>(&all_terms), sizeof(all_terms)); 
    file.write(reinterpret_cast<char*>(&all_words), sizeof(all_words));
    file.seekp(pageOffset + PAGE_SIZE);
        
}
    int getAllDocumentsCount() {
            return all_documents_count;
        }

    int getAllLens() {
        return all_lens;
    }

void getDocuments(std::string term, std::fstream& file, std::fstream& dict, std::fstream& lines_dict) {
    int pageOffset = trie.search(term);
    if (pageOffset == -1) {
    } else {
        file.seekg(pageOffset);
        int numDocs;
        int all_words, all_terms;
        file.read(reinterpret_cast<char*>(&numDocs), sizeof(numDocs));
        for (int i = 0; i < numDocs; ++i) {
            int d_id_pos;
            int docs_num;
            int lines_vec_pos;
            file.read(reinterpret_cast<char*>(&d_id_pos), sizeof(d_id_pos));
            file.read(reinterpret_cast<char*>(&docs_num), sizeof(docs_num));
            file.read(reinterpret_cast<char*>(&lines_vec_pos), sizeof(lines_vec_pos));

            file.read(reinterpret_cast<char*>(&all_terms), sizeof(all_terms));
            file.read(reinterpret_cast<char*>(&all_words), sizeof(all_words));
            std:: cout << "LEN OF FILE " << all_words << '\n';
            dict.seekg(d_id_pos);
            int len;
            dict.read(reinterpret_cast<char*>(&len), sizeof(len));
            char* stringa = new char[len];
                dict.read(stringa, len);
                std::string ans = "";
                for (int j = 0; j < len; ++j) {
                    ans.push_back(stringa[j]);
                }

            lines_dict.seekg(lines_vec_pos);
            int lines_vec_len;
            lines_dict.read(reinterpret_cast<char*>(&lines_vec_len), sizeof(lines_vec_len));
            std::vector<int> lines_vec(lines_vec_len);
            lines_dict.read(reinterpret_cast<char*>(lines_vec.data()), lines_vec_len * sizeof(int));

            for (int line : lines_vec) {
                //std::cout << line << " ";
            }

        }
    }
}

std::map<int, std::string> mapa;

    void addDocument(std::string& filePath, std::fstream& output_file, std::fstream& dict_file, std::fstream& lines_dict_file) {
        all_documents_count++;
        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Error opening file: " << filePath << std::endl;
            return;
        }
      
    
        int string_filename_len = filePath.length();
        const char* input_str = filePath.c_str();
        int index_of_name = dict_file.tellg();
      
        dict_file.write(reinterpret_cast<char*>(&string_filename_len), sizeof(string_filename_len));
        dict_file.write((input_str), string_filename_len);
      
        std::string term;
        int all_words_in_doc = 0;
        int count_terms_in_doc = 0;
        std::string line;
        int lineNumber = 0;
        while (std::getline(file, line)) { 
        lineNumber++; 
        std::istringstream iss(line); 
        std::string word;
            while (iss >> word) { 
                int pageOffset = trie.insert(word, output_file);
                all_words_in_doc++;
                trie.search_node(word)->count_appear_in_doc++;
                trie.search_node(word)->lines.push_back(lineNumber); 
            }
        }
        all_lens += all_words_in_doc;
        
        file.close();
        std::ifstream filetwice(filePath);
        if (!filetwice.is_open()) {
            std::cerr << "Error opening file: " << filePath << std::endl;
            return;
        }
        
        while (std::getline(filetwice, line)) {
            std::istringstream iss(line); 
            std::string term;
            while (iss >> term) {

                if (term[term.size() - 1] == '\n') {
                    //std::cout << "caught another line" << '\n';
                    //term.erase(term.size() - 1);
                    
                }
                std::vector<int> lines(trie.search_node(term)->lines);
                if (trie.search_node(term) == nullptr) {
                    //std::cout << "IS NULLPTR" << '\n';
                }
                int vec_size = lines.size();
                int lines_vec_pos = lines_dict_file.tellg();
                lines_dict_file.write(reinterpret_cast<const char*>(&vec_size), sizeof(vec_size)); 
                lines_dict_file.write(reinterpret_cast<const char*>(lines.data()), vec_size * sizeof(int));
                AddDocument(term, index_of_name, filePath, output_file, all_words_in_doc, lines_vec_pos);
                }
            

        }  
        std::cout << all_lens << " ";
        trie.resetCounts();
        filetwice.close();
    }
void SaveTrie(std::fstream& file) {
    trie.saveTrieToFile(file);
}
 
};
