// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "crypto/scrypt.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"
#include "chainparams.h"

uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

uint256 CBlockHeader::GetPoWHash() const
{
    uint256 thash;
    uint256 chash = GetHash();
    unsigned int target_N_size;

    if ( nVersion < Params().GetConsensus().PMC2MinVersionRequired )
        target_N_size = BLOCK_HASH_PRE_PMC2_N_SIZE;
    else
        target_N_size = Params().GetConsensus().PMC2ScryptNFactor;

    printf("GetPoWHash: using N size %d for nVersion %d at block %s \n", target_N_size, nVersion, chash.ToString().c_str());

    scrypt_N_1_1_256(BEGIN(nVersion), BEGIN(thash), target_N_size);
    
    return thash;
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        s << "  " << vtx[i].ToString() << "\n";
    }
    return s.str();
}

int64_t GetBlockWeight(const CBlock& block)
{
    // This implements the weight = (stripped_size * 4) + witness_size formula,
    // using only serialization with and without witness data. As witness_size
    // is equal to total_size - stripped_size, this formula is identical to:
    // weight = (stripped_size * 3) + total_size.
    return ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * (WITNESS_SCALE_FACTOR - 1) + ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION);
}
