# Blockchain Voting System

## Overview
This project implements a simple blockchain-based voting system in C++.  
It ensures secure vote recording, prevents duplicate voting, and maintains integrity using:
- Merkle Trees for vote aggregation
- Proof-of-Work mining with nonce discovery
- Hash linkage between blocks for tamper detection

## Features
- Candidate registration by admin
- Voter ID validation (prevents duplicate votes)
- Automatic block mining when full
- Manual mining option for partially filled blocks
- Merkle root computation for vote integrity
- Blockchain validity check (tamper detection)
- Admin-only blockchain display
- Vote tallying for all candidates

## System Details
- **Max Votes per Block:** 5  
- **Max Voters:** 100  
- **Max Candidates:** 10  
- **Admin Password:** `admin123`  

Each block stores:
- Index, timestamp, nonce
- Previous hash & current hash
- Merkle root of votes
- Voter IDs and candidate names

## Menu Options
1. Cast Vote  
2. Mine Current Block  
3. Display Blockchain (Admin only)  
4. Count Votes  
5. Check Blockchain Validity  
6. Admin Login  
7. Exit  

## How It Works
1. **Candidate Setup:** Admin registers candidates at startup.  
2. **Voting:** Voters enter their ID and select a candidate. Duplicate IDs are rejected.  
3. **Block Mining:** When a block reaches 5 votes, it is auto-mined. Mining finds a hash starting with `"00"`.  
4. **Blockchain Integrity:** Each block links to the previous block via hash. Any tampering breaks the chain.  
5. **Admin Access:** Only admins can view full blockchain details.  
6. **Vote Counting:** The system tallies votes across all mined blocks.  

## Security
- **Merkle Root:** Ensures vote data integrity.  
- **Proof-of-Work:** Prevents easy manipulation of blocks.  
- **Hash Linkage:** Detects tampering between blocks.  
- **Admin Control:** Restricts sensitive blockchain data access.
