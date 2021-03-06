#pragma once

#include "BlockStore.hpp"
#include "AES.hpp"

#include <cstdint>
#include <memory>
#include <vector>
#include <unordered_map>

using PositionMap = std::vector<int64_t>;
using BlockMap = std::unordered_map<int64_t, std::vector<size_t>>;
using ChangedMap = std::vector<byte_t>;

class Agent: public BlockStore {
	BlockStore *store;
	
	size_t blockCount, blockSize;

	bytes<Key> key;

public:
	Agent(BlockStore *store, size_t blockCount, size_t blockSize, bytes<Key> key);
	~Agent();

	enum Op {
		READ,
		WRITE
	};

	block Access(Op op, int64_t bid, block data);
	
	PositionMap LoadPositionMap();

	ChangedMap LoadChangedMap();
	void SaveChangedMap(ChangedMap changedMap);

	static BlockMap GenerateBlockMap(const PositionMap &posMap);

	block Read(size_t bid);
	void Write(size_t bid, block b);
	
	void Duplicate(int64_t bid, block plaintext);

	size_t GetBlockCount();
	size_t GetBlockSize();
	
	bool WasSerialised();
};
