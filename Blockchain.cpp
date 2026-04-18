#include <iostream>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
using namespace std;

const int MAX_VOTES_PER_BLOCK = 5;
const int MAX_VOTERS = 100;
const int MAX_CANDIDATES = 10;
const string ADMIN_PASSWORD = "admin123";

// Returns a 16-char hex hash string (ensures "00" prefix is reachable for mining)
string getHash(const string& data) {
    unsigned long long h = 5381ULL;
    for (char c : data)
        h = ((h << 5) + h) ^ (unsigned char)c;
    h ^= (unsigned long long)data.size() * 2654435761ULL;
    ostringstream oss;
    oss << hex << setw(16) << setfill('0') << h;
    return oss.str();
}

// Merkle tree node
struct MerkleNode {
    string hashValue;
    MerkleNode* next;
    MerkleNode(string h) : hashValue(h), next(NULL) {}
};

// Frees all nodes in a Merkle linked list
static void freeMerkleList(MerkleNode* head) {
    while (head) {
        MerkleNode* tmp = head;
        head = head->next;
        delete tmp;
    }
}

// Builds and returns the Merkle root from an array of vote strings
string buildMerkleRoot(string votes[], int voteCount) {
    if (voteCount == 0)
        return getHash("empty");

    MerkleNode* head = NULL;
    MerkleNode* tail = NULL;

    for (int i = 0; i < voteCount; i++) {
        MerkleNode* node = new MerkleNode(getHash(votes[i]));
        if(head == NULL){ 
            head = node; 
            tail = node; 
        }
        else{ 
            tail->next = node; 
            tail = node; 
        }
    }

    while (head->next != NULL) {
        MerkleNode* newHead = NULL;
        MerkleNode* newTail = NULL;
        MerkleNode* ptr = head;

        while (ptr != NULL) {
            string leftHash  = ptr->hashValue;
            string rightHash = (ptr->next != NULL) ? ptr->next->hashValue : leftHash;
            MerkleNode* combined = new MerkleNode(getHash(leftHash + rightHash));
            if(newHead == NULL){ 
                newHead = combined; 
                newTail = combined; 
            }
            else{ 
                newTail->next = combined; 
                newTail = combined;
            }
            ptr = (ptr->next != NULL) ? ptr->next->next : NULL;
        }

        freeMerkleList(head);
        head = newHead;
    }

    string root = head->hashValue;
    freeMerkleList(head);
    return root;
}

// Block structure holding votes, hashes, nonce, and chain pointer
struct Block {
    int index;
    string previousHash;
    string currentHash;
    string merkleRoot;
    string voterIDs[MAX_VOTES_PER_BLOCK];
    string candidateNames[MAX_VOTES_PER_BLOCK];
    int voteCount;
    int nonce;
    time_t timestamp;
    Block* next;

    Block(int idx, string prevHash)
        : index(idx), previousHash(prevHash),
          currentHash(""), merkleRoot(""),
          voteCount(0), nonce(0),
          timestamp(time(0)), next(NULL) {}
};

// Blockchain class managing blocks, voters, candidates, and admin access
class Blockchain {
private:
    Block* head;
    Block* currentBlock;
    int totalBlocks;

    string registeredVoters[MAX_VOTERS];
    int registeredCount;

    string candidates[MAX_CANDIDATES];
    int candidateCount;

    bool isAdminLoggedIn;

public:
    Blockchain() {
        head            = NULL;
        currentBlock    = NULL;
        totalBlocks     = 0;
        registeredCount = 0;
        candidateCount  = 0;
        isAdminLoggedIn = false;
        addBlock();
    }

    // Reads candidate names from user input
    void setCandidates() {
        cout << "\nHow many candidates? ";
        cin >> candidateCount;

        if (candidateCount < 2 || candidateCount > MAX_CANDIDATES) {
            cout << "Invalid count. Setting to 2.\n";
            candidateCount = 2;
        }

        for (int i = 0; i < candidateCount; i++) {
            cout << "Enter candidate " << (i + 1) << " name: ";
            cin >> candidates[i];
        }
        cout << "Candidates registered successfully.\n";
    }

    // Returns true if the voter has already cast a vote
    bool checkDupV(const string& voterID) {
        for (int i = 0; i < registeredCount; i++)
            if (registeredVoters[i] == voterID) {
                return true;
            }
        return false;
    }

    // Records a vote into the current block, auto-mines if block is full
    void addVote() {
        string voterID, candidateName;

        cout << "\nEnter your Voter ID    : ";
        cin >> voterID;

        if (checkDupV(voterID)) {
            cout << "Duplicate vote! You have already voted.\n";
            return;
        }

        cout << "Available candidates  : ";
        for (int i = 0; i < candidateCount; i++) {
            cout << candidates[i];
            cout << (i < candidateCount - 1 ? ", " : "\n");
        }

        cout << "Enter candidate name  : ";
        cin >> candidateName;

        bool validCandidate = false;
        for (int i = 0; i < candidateCount; i++) {
            if (candidates[i] == candidateName) { 
                validCandidate = true; break; 
            }
        }

        if (!validCandidate) {
            cout << "Invalid candidate name.\n";
            return;
        }

        if (registeredCount < MAX_VOTERS){
            registeredVoters[registeredCount++] = voterID;
        }
        
        currentBlock->voterIDs[currentBlock->voteCount]       = voterID;
        currentBlock->candidateNames[currentBlock->voteCount] = candidateName;
        currentBlock->voteCount++;

        cout << "Vote cast successfully!\n";

        if (currentBlock->voteCount == MAX_VOTES_PER_BLOCK) {
            cout << "Block full. Auto-mining...\n";
            finalize(currentBlock);
            addBlock();
        }
    }

    // Computes Merkle root and mines the block by finding a nonce that gives a "00"-prefixed hash
    void finalize(Block* block) {
        string votes[MAX_VOTES_PER_BLOCK];
        for (int i = 0; i < block->voteCount; i++)
            votes[i] = block->voterIDs[i] + block->candidateNames[i];

        block->merkleRoot = buildMerkleRoot(votes, block->voteCount);
        block->nonce = 0;

        do {
            block->nonce++;
            string hashInput =
                to_string(block->index)   +
                block->previousHash       +
                block->merkleRoot         +
                to_string(block->nonce)   +
                to_string(block->timestamp);

            for (int i = 0; i < block->voteCount; i++)
                hashInput += block->voterIDs[i] + block->candidateNames[i];
                block->currentHash = getHash(hashInput);

        } while (block->currentHash.substr(0, 2) != "00");

        cout << "Block " << block->index << " mined! Nonce = "
             << block->nonce << " | Hash = " << block->currentHash << "\n";
    }

    // Appends a new empty block to the chain
    void addBlock() {
        string prevHash;
        if (head == NULL)
            prevHash = "0000000000000000";
        else
            prevHash = currentBlock->currentHash.empty() ? "0000000000000000" : currentBlock->currentHash;

        Block* newBlock = new Block(totalBlocks, prevHash);
        totalBlocks++;

        if (head == NULL) { 
            head = newBlock; currentBlock = newBlock; 
        }
        else { 
            currentBlock->next = newBlock; 
            currentBlock = newBlock; 
        }
    }

    // Manually mines the current block if it has votes and hasn't been mined yet
    void mineCurrentBlock() {
        if (currentBlock->voteCount == 0) {
            cout << "\n[INFO] Current block is empty. Nothing to mine.\n";
            return;
        }
        if (!currentBlock->currentHash.empty()) {
            cout << "\n[INFO] Current block is already mined.\n";
            return;
        }

        cout << "\n[INFO] Mining current block with " << currentBlock->voteCount << " vote(s)...\n";
        finalize(currentBlock);
        cout << "[INFO] Block mined successfully!\n";
        addBlock();
    }

    // Displays all blocks in the chain (admin only)
    void displayBlockchain() {
        if (!isAdminLoggedIn) {
            cout << "Access denied. Please login as admin first.\n";
            return;
        }

        Block* ptr = head;
        while (ptr != NULL) {
            cout << "\n========== BLOCK " << ptr->index << " ==========\n";
            cout << "Timestamp    : " << ctime(&ptr->timestamp);

            string ph = ptr->previousHash;
            cout << "Prev Hash    : " << (ph.size() > 16 ? ph.substr(0, 16) + "..." : ph) << "\n";

            string mr = ptr->merkleRoot;
            cout << "Merkle Root  : " << (mr.empty() ? "Not computed yet" : (mr.size() > 16 ? mr.substr(0, 16) + "..." : mr)) << "\n";

            if (ptr->currentHash.empty()) {
                cout << "Current Hash : Not mined yet\n";
            }
            else {
                string ch = ptr->currentHash;
                cout << "Current Hash : " << (ch.size() > 16 ? ch.substr(0, 16) + "..." : ch) << "\n";
            }

            cout << "Nonce        : " << ptr->nonce << "\n";
            cout << "Votes (" << ptr->voteCount << "):\n";

            for (int i = 0; i < ptr->voteCount; i++)
                cout << "  VoterID: " << ptr->voterIDs[i] << "  ->  " << ptr->candidateNames[i] << "\n";

            ptr = ptr->next;
        }
    }

    // Tallies and prints vote counts for each candidate
    void countVotes() {
        int voteTally[MAX_CANDIDATES] = {};
        Block* ptr = head;
        while (ptr != NULL) {
            for (int i = 0; i < ptr->voteCount; i++)
                for (int j = 0; j < candidateCount; j++)
                    if (ptr->candidateNames[i] == candidates[j]) { 
                        voteTally[j]++; 
                        break; 
                    }
            ptr = ptr->next;
        }
        cout << "\n====== VOTE TALLY ======\n";

        for (int j = 0; j < candidateCount; j++)
            cout << candidates[j] << " : " << voteTally[j] << " vote(s)\n";
    }

    // Verifies hash linkage between mined blocks to detect tampering
    void checkValidity() {
        Block* ptr = head;
        bool isValid = true;

        while (ptr != NULL && ptr->next != NULL) {
            if (!ptr->currentHash.empty() && !ptr->next->previousHash.empty())
                if (ptr->next->previousHash != ptr->currentHash) {
                    cout << "TAMPER DETECTED at Block " << ptr->next->index << "!\n";
                    isValid = false;
                }
            ptr = ptr->next;
        }
        if (isValid)
            cout << "Blockchain is valid. No tampering detected.\n";
    }

    // Validates admin password and sets login flag
    void adminLogin() {
        string password;
        cout << "Enter admin password: ";
        cin >> password;

        if (password == ADMIN_PASSWORD) {
            isAdminLoggedIn = true;
            cout << "Admin login successful.\n";
        } 
        else {
            cout << "Incorrect password.\n";
        }
    }
};

int main() {
    Blockchain votingSystem;
    votingSystem.setCandidates();

    int choice;
    do {
        cout << "\n====== BLOCKCHAIN VOTING SYSTEM ======\n";
        cout << "1. Cast Vote\n";
        cout << "2. Mine Current Block\n";
        cout << "3. Display Blockchain (Admin)\n";
        cout << "4. Count Votes\n";
        cout << "5. Check Blockchain Validity\n";
        cout << "6. Admin Login\n";
        cout << "7. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
            case 1: votingSystem.addVote();           break;
            case 2: votingSystem.mineCurrentBlock();  break;
            case 3: votingSystem.displayBlockchain(); break;
            case 4: votingSystem.countVotes();        break;
            case 5: votingSystem.checkValidity();     break;
            case 6: votingSystem.adminLogin();        break;
            case 7: cout << "Exiting. Goodbye!\n";   break;
            default: cout << "Invalid choice.\n";
        }
    } while (choice != 7);

    return 0;
}
