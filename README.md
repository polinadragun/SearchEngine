# Search Program Using a Trie-based Inverted Index and BM25 Ranking

This project implements a search engine that leverages a **Trie** data structure for storing terms in documents and uses **BM25** ranking to score and retrieve the most relevant documents for a given search query.

The program reads document information from binary files and constructs a trie-based inverted index. It allows users to perform both Boolean and ranked search queries, with results ordered based on relevance scores calculated by the BM25 algorithm.

## Core Components

### Trie Structure and Document Loading
The program uses a **Trie** data structure to efficiently store and search for terms within documents. Each **Trie node** stores:
- A map of children nodes (representing the characters in terms),
- location of term information in the file,
- frequency of the term across documents, and
- A list of document line occurrences.

The trie is built and loaded from a file containing serialized document information. Each term maps to a set of document identifiers (IDs) where it appears, along with its frequency and position in the text.

### AST (Abstract Syntax Tree) Construction for Query Parsing
The program constructs an **AST (Abstract Syntax Tree)** to parse complex Boolean queries. For example, for a search expression like `"term1 AND term2 OR term3"`, each operand (term) and operator (AND, OR) is represented as a node in the tree.

The AST is built by tokenizing the input string and recursively parsing it into nodes. The nodes are then processed in a post-order traversal, applying "AND" or "OR" operations to merge posting lists.

### BM25 Ranking
The **BM25 algorithm** scores document relevance by analyzing term frequency, document length, and other factors. This score is used to rank documents within search results, based on:
- Term frequency within each document,
- The average document length, and
- Inverse document frequency (IDF).
  
  ![image](https://github.com/user-attachments/assets/18118b5f-0c26-43fc-a067-82b6e6817b2a)


### Execution of Search Queries
The query parser breaks down user queries into Boolean operators and terms, building an AST to represent the query structure. Each term’s posting list is retrieved by searching the trie, and the AST nodes are evaluated to combine results based on "AND" or "OR" operations. Relevant documents are then scored and sorted based on their BM25 relevance scores.

### Top-K Results Retrieval
After calculating relevance scores, the program collects the top-K results, displaying document identifiers and line positions where the terms appear.

## Files Used
- **Main Dictionary File**: Stores serialized term data, including document occurrences and line information.
- **Lines Dictionary File**: Contains specific line occurrences for each term in each document.
- **Auxiliary Files**: Store metadata such as the total document count and term count.
