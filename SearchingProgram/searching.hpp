#include <queue>
#include <sstream>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <fstream>
#include <set>
#include <cmath>

class SearchingProgramm {
    public:
    struct TrieNode {
    std::map<char, TrieNode*> children;
    int pageOffset = -1;  
    int count_appear_in_doc = 0;
    std::vector<int> lines;
};

class Trie {

public:
    TrieNode root;

    void loadTrieFromFileHelper(std::fstream& file, TrieNode* node) {
            if (!file.read(reinterpret_cast<char*>(&node->count_appear_in_doc), sizeof(node->count_appear_in_doc))) return;
            file.read(reinterpret_cast<char*>(&node->pageOffset), sizeof(node->pageOffset));

            size_t linesSize;
            file.read(reinterpret_cast<char*>(&linesSize), sizeof(linesSize));
            node->lines.resize(linesSize);
            file.read(reinterpret_cast<char*>(node->lines.data()), linesSize * sizeof(int));

            size_t childrenSize;
            file.read(reinterpret_cast<char*>(&childrenSize), sizeof(childrenSize));
            for (size_t i = 0; i < childrenSize; ++i) {
                char childChar;
                file.read(&childChar, sizeof(childChar));
                TrieNode* childNode = new TrieNode();
                node->children[childChar] = childNode;
                loadTrieFromFileHelper(file, childNode);
            }
        }
    
    void loadTrieFromFile(std::fstream& inFile) {
            if (!inFile) {
                std::cerr << "Failed to open file for loading Trie." << std::endl;
                return;
            }
            loadTrieFromFileHelper(inFile, &root);
            inFile.close();
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
    
};
void LoadTree(std::fstream& file) {
    trie.loadTrieFromFile(file);
}
private:
    Trie trie;
    std::map<std::string, std::vector<int>> termToLines;
    std::vector<int> Paths;
    int all_documents_count;
    int all_lens;
    struct InfoAboutFileForSearch {
        int d_id;
        int path_pos;
        int lines_vec_pos;
        int all_terms_;
        int all_words_;
        int accordance_score = 0;
        InfoAboutFileForSearch() : accordance_score(0) {}
    };
    
    struct Node {
    std::string value;
    std::string type;
    std::vector<int> posting_list;
    std::vector<double> scores_;
    std::vector<InfoAboutFileForSearch> infos;
    Node* left;
    Node* right;

    Node(const std::string& val, const std::string& t, int all_documents_count) : value(val), type(t), left(nullptr), right(nullptr), scores_(all_documents_count + 1, 0), infos(all_documents_count + 1, InfoAboutFileForSearch()) {}
    ~Node() {
        delete left;
        delete right;
    }
    };
    Node* ast_root_;

std::vector<std::string> break_down(const std::string& input) {
    std::istringstream iss(input);
    std::vector<std::string> parts;
    std::string token;
    while (iss >> token) {
        parts.push_back(token);
    }
    return parts;
}

Node* parseExpression(const std::vector<std::string>& parts, int start, int end) {
    if (start > end) return nullptr;
    if (start == end) return new Node(parts[start], "OPERAND", this->all_documents_count);

    int andIndex = -1;
    int orIndex = -1;
    int level = 0;

    for (int i = end; i >= start; --i) {
        if (parts[i] == ")") {
            level++;
        } else if (parts[i] == "(") {
            level--;
        } else if (level == 0) {
            if (parts[i] == "AND" && andIndex == -1) {
                andIndex = i;
            } else if (parts[i] == "OR" && orIndex == -1) {
                orIndex = i;
            }
        }
    }

    int mid;
    if (andIndex!= -1) {
        mid = andIndex;
    } else {
        mid = orIndex;
    }

    if (mid == -1) {
        return new Node(parts[start], "OPERAND", this->all_documents_count);
    }

    Node* node = new Node(parts[mid], "OPERATOR", this->all_documents_count);
    node->left = parseExpression(parts, start, mid - 1);
    node->right = parseExpression(parts, mid + 1, end);
    return node;
}


Node* buildAST(const std::string& input) {
    std::vector<std::string> parts = break_down(input);
    return parseExpression(parts, 0, parts.size() - 1);
}

public:
void printInOrder(Node* node) {
    if (!node) return;
    std::cout << node->value << '\n';
    printInOrder(node->left);
    printInOrder(node->right);
}


void printAST(Node* root) {
    printInOrder(root);
    std::cout << std::endl;
}


struct CompareAccordanceScore {
    bool operator()(const InfoAboutFileForSearch& lhs, const InfoAboutFileForSearch& rhs) {
        return lhs.accordance_score < rhs.accordance_score;
    }
};
using MaxHeap = std::priority_queue<InfoAboutFileForSearch, std::vector<InfoAboutFileForSearch>, CompareAccordanceScore>;

double BM25(int terms_in_doc_amount, int all_words_in_doc, int total_docs, int posting_list_len) {
    double k = 1.2;
    double b = 0.75;
    int tf = static_cast<double>(terms_in_doc_amount);
    int df = total_docs; 
    int dl = all_words_in_doc;
    int dl_avg = posting_list_len / total_docs; 

    double qtw = std::log((total_docs - df + 0.5) / (df + 0.5));

    double score = 0.0;
    for (int t = 0; t < terms_in_doc_amount; ++t) {
        score += (tf * (k + 1)) / (tf + k * (1 - b + b * static_cast<double>(dl) / dl_avg));
    }

    return score;
}


MaxHeap heap;
std::vector<InfoAboutFileForSearch> pool1;
std::vector<InfoAboutFileForSearch> pool2;
std::vector<InfoAboutFileForSearch> pool3;

std::vector<int> posting_list2;
void fillForTerm(std::fstream& file, std::string term) {
    std::cout << "ALL_DOCS_COUNT in searching" << all_documents_count << '\n';
    int pageOffset = trie.search(term);
    std::cout << "found: " << pageOffset << '\n';
  
    if (pageOffset == -1) {
    } else {
        file.seekg(pageOffset);
        int numDocs;
        file.read(reinterpret_cast<char*>(&numDocs), sizeof(numDocs));
        std::cout << "NUM docs " << numDocs <<'\n';
        
        for (int i = 0; i < numDocs; ++i) {
            InfoAboutFileForSearch info;
            int d_id_pos;
            int docs_num;
          
            file.read(reinterpret_cast<char*>(&info.path_pos), sizeof(info.path_pos));
            file.read(reinterpret_cast<char*>(&docs_num), sizeof(docs_num));
            file.read(reinterpret_cast<char*>(&info.lines_vec_pos), sizeof(info.lines_vec_pos));
            
            file.read(reinterpret_cast<char*>(&info.all_terms_), sizeof(info.all_terms_));
            file.read(reinterpret_cast<char*>(&info.all_words_), sizeof(info.all_words_));
            std:: cout << "LEN OF FILE " << info.all_terms_ << '\n';
            int av_len = all_lens/all_documents_count;
            int score = BM25(info.all_terms_, info.all_words_, all_documents_count, av_len);
            info.accordance_score = score;
             std::cout << "docs_num  " <<  docs_num << '\n';

            
        }
    }
}

std::vector<int> fillPostingListForTerm(std::fstream& file, std::string term, std::vector<double>& scores,std::vector<InfoAboutFileForSearch>& infos) {
    std::vector<int> posting_list1;
    scores.resize(all_documents_count + 1, 0);
    infos.resize(all_documents_count + 1);
    int pageOffset = trie.search(term);
    
    if (pageOffset == -1) {
    } else {
        file.seekg(pageOffset);
        int numDocs;
        file.read(reinterpret_cast<char*>(&numDocs), sizeof(numDocs));
        std::cout << "NUM docs " << numDocs <<'\n';
        
        for (int i = 0; i < numDocs; ++i) {
            InfoAboutFileForSearch info;
            int d_id_pos;
            int docs_num;
            int path_pos;
            file.read(reinterpret_cast<char*>(&path_pos), sizeof(path_pos));
            file.read(reinterpret_cast<char*>(&docs_num), sizeof(docs_num));
            file.read(reinterpret_cast<char*>(&info.lines_vec_pos), sizeof(info.lines_vec_pos));
            
            file.read(reinterpret_cast<char*>(&info.all_terms_), sizeof(info.all_terms_));
            file.read(reinterpret_cast<char*>(&info.all_words_), sizeof(info.all_words_));
            int av_len = all_lens/all_documents_count;
            double score = BM25(info.all_terms_, info.all_words_, all_documents_count, av_len);
            info.accordance_score = score;
            Paths[docs_num] = path_pos;
            infos[docs_num] = info;
            scores[docs_num] = score;
            auto it = termToLines.find(term);
            if (it!= termToLines.end()) {
                
                it->second[docs_num] = info.lines_vec_pos;
            } else {

                termToLines[term] = std::vector<int>(all_documents_count + 1, 0);
                termToLines[term][docs_num] = info.lines_vec_pos;
            }
            
            posting_list1.push_back(docs_num);
            
        }
    }
    return posting_list1;
}

void AndFunc(std::vector<int>& posting_middle, const std::vector<int>& posting_left, const std::vector<int>& posting_right, std::vector<double>& scores_in_node, std::vector<double>& scores_left, std::vector<double>& scores_right) {
    posting_middle.clear();
    std::cout << "Perform And" << '\n';
    for (int elem : posting_left) {
        if (std::find(posting_right.begin(), posting_right.end(), elem)!= posting_right.end()) {
            scores_in_node[elem] = scores_left[elem] * scores_right[elem];
            posting_middle.push_back(elem);
        }
    }
}
void OrFunc(std::vector<int>& posting_middle, const std::vector<int>& posting_left, const std::vector<int>& posting_right, std::vector<double>& scores_in_node, std::vector<double>& scores_left, std::vector<double>& scores_right) {
    std::set<int> uniqueElements;
    std::cout << "Perform Or" << '\n';
    for (auto& elem : posting_left) {
        uniqueElements.insert(elem); 
        std::cout << "elem " << elem << " ";
        scores_in_node[elem] += scores_left[elem];
    } 

    for (auto& elem : posting_right) {
        scores_in_node[elem] += scores_right[elem];
        std::cout << "elem " << elem << " ";
        uniqueElements.insert(elem); 
    }

    posting_middle.assign(uniqueElements.begin(), uniqueElements.end());
}

void postorderTraverse(Node* node, std::fstream& file) {
    if (!node) return;
    
    postorderTraverse(node->left, file);
    postorderTraverse(node->right, file);
    if (node-> value != "AND" && node->value != "OR") {
        
        std::string term1 = node->value;
        node->posting_list = fillPostingListForTerm(file, term1, node->scores_, node->infos);
        std::cout << "TERM posting list:  ";
        for (int i = 0; i < node->posting_list.size();++i) {
            std::cout << node->posting_list[i];
        }
        std::cout << '\n';
    } else if (node->value == "AND") {
        AndFunc(node->posting_list, node->left->posting_list, node->right->posting_list, node->scores_, node->left->scores_, node->right->scores_);
        
        std::cout << "AND posting list:  ";
        for (int i = 0; i < node->posting_list.size();++i) {
            std::cout << node->posting_list[i];
        }
        if (node->posting_list.size() == 0) {
            std::cout << "There are no such file ooopsie";
            exit(0);
        }
        std::cout <<'\n';
    } else if (node->value == "OR") {
        OrFunc(node->posting_list, node->left->posting_list, node->right->posting_list, node->scores_, node->left->scores_, node->right->scores_);
        std::cout << "OR posting list:  ";
        for (int i = 0; i < node->posting_list.size();++i) {
            std::cout << node->posting_list[i];
        }
        std::cout <<'\n';
        
    }
}
void validQuery(const std::string& query) {
    bool isValid = true;
    for (char c : query) {
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))) {
            if (c != ' ' && c != '(' && c != ')') {
                isValid = false;
                break;
            }
        }
    }
    if (!isValid) {
        std::cerr << "Error: Query contains non-latin characters" << std::endl;
        exit(0); 
    }
}
std::vector<std::string> answer;
void TakeTopK(int k, std::string& query, std::fstream& file, std::fstream& lines_dict, std::fstream& dict) {
    validQuery(query);
    Paths.resize(all_documents_count + 1);
    ast_root_ = buildAST(query);
    postorderTraverse(ast_root_, file);
    std::cout << "Posting list of the root:  ";
    std::cout << ast_root_->posting_list.size();
    std::vector<InfoAboutFileForSearch> vec_of_structs;
    for (int i = 0; i <= ast_root_->posting_list.size() - 1; ++i) {
        InfoAboutFileForSearch info;
        info.d_id = ast_root_->posting_list[i];
        info.accordance_score = ast_root_->scores_[i];
        std::cout << ast_root_->posting_list[i];
        vec_of_structs.push_back(info);
    }
    std::sort(vec_of_structs.begin(), vec_of_structs.end(), [](const InfoAboutFileForSearch& a, const InfoAboutFileForSearch& b) {
        return a.accordance_score > b.accordance_score;
    });
    std::vector<int> topK;
    std::cout << vec_of_structs.size();
    for (int i = vec_of_structs.size() - 1; i >= 0; --i) {
        if (k == 0) {
            break;
        }
        --k;
        topK.push_back(vec_of_structs[i].d_id);
    }
    std::cout << "HERE IS FINE";
    for (int i = 0; i < topK.size(); ++i) {
        dict.seekg(Paths[topK[i]]);
            int len;
            dict.read(reinterpret_cast<char*>(&len), sizeof(len));
            char* stringa = new char[len];
                dict.read(stringa, len);
                std::string ans = "";
                for (int j = 0; j < len; ++j) {
                    ans.push_back(stringa[j]);
                }
                answer.push_back(ans);
                std::cout << ans << '\n';
    }
    std::vector<std::string> queryParts = break_down(query);
    for (const auto& part : queryParts) {
        auto it = termToLines.find(part);
        if (it!= termToLines.end()) {
            std::cout << "Lines for term \"" << part << "\":\n";
            for (int i = 0; i <= topK.size() - 1; ++i) {
                if (it->second[topK[i]]!= 0) {
                    std::cout << "Document ID: " << topK[i] << ", Line Count: " << it->second[ast_root_->posting_list[i]] << "\n";
                    int lines_vec_pos = it->second[topK[i]];
                    lines_dict.seekg(lines_vec_pos);
                    int lines_vec_len;
                    lines_dict.read(reinterpret_cast<char*>(&lines_vec_len), sizeof(lines_vec_len));
                    std::vector<int> lines_vec(lines_vec_len);
                    lines_dict.read(reinterpret_cast<char*>(lines_vec.data()), lines_vec_len * sizeof(int));
                    std::set<int> lines_set(lines_vec.begin(), lines_vec.end());
                    std::cout << std::endl;
                    std::cout << "Lines   ";
                    for (int line : lines_set) {
                        std::cout << line << " ";
                    }
                    std::cout << std::endl;

                }
            }
        }
    }
}

void LoadValues(std::fstream& file) {
    if (!file) {
        std::cerr << "Failed to open file for loading values." << std::endl;
        return;
    }
    file.read(reinterpret_cast<char*>(&all_documents_count), sizeof(all_documents_count));
    file.read(reinterpret_cast<char*>(&all_lens), sizeof(all_lens));
    std::cout << all_documents_count << " ";
    std::cout << all_lens;
}

};
