#include "ORAM.hpp"
#include "Timer.hpp"
#include "FileSystem.hpp"
#include "File.hpp"
#include "Agent.hpp"
#include "RAMStore.hpp"
#include "FileStore.hpp"

#include <functional>
#include <fstream>

#include <cstdio>

int MBtoDepth(size_t mb, size_t blockSize = 4096);
double Profile(std::function<void()> fun);
void Write(ORAM &oram, FileInfo info);
void read(Agent &agent);
void right(Agent &agent, size_t length);

int main(int argc, char **argv)
{
	AES::Setup();

	srand(time(NULL));
	
//	int blockSize = 1000000;

//	BlockStore *store = new FileStore("moo.bin", 1000*2, IV + AES::GetCiphertextLength(blockSize));
	
	bytes<Key> key {0};
//	Agent agent(store, 1000, blockSize, key);

	//read(agent);

//	right(agent, 1000*1000*1000);
	
//	AES::Cleanup();


	if (argc != 2) {
		fputs("requires a depth argument", stderr);
		return 1;
	}
	
	//size_t mb = 20;
	//size_t depth = MBtoDepth(mb);
	//printf("%zuMB = depth %zu\n", mb, depth);
	
	size_t depth = strtol(argv[1], nullptr, 10);
	printf("depth = %zu\n", depth);

	size_t blockSize = 4096;
	size_t storeBlockSize = IV + AES::GetCiphertextLength(Z*(sizeof (int32_t) + blockSize));

	BlockStore *store = new FileStore("tree.bin", pow(2, depth + 1) - 1, storeBlockSize);
	ORAM *oram = new ORAM(store, depth, blockSize, key);
	printf("#blocks = %zu\n", oram->GetBlockCount());

	/*
	for (size_t bid = 0; bid < oram->GetBlockCount(); bid++) {
		block block(blockSize);

		for (size_t i = 0; i < block.size(); i++) {
			block[i] = bid % 256;
		}

		oram->Access(ORAM::WRITE, bid, block);
	}
	*/

	for (size_t bid = 0; bid < oram->GetBlockCount(); bid++) {
		block result;
		
		oram->Access(ORAM::READ, bid, result);

		printf("\nBlock %zu\n", bid);

		for (auto c : result) {
			printf("%d ", c);
		}
		puts("");
	}

	/*
	FileSystem files(oram);
	files.Load();
	
	std::string filename = "benchmark/Image.ppm";
	
	double elapsedTime = 0;
	
	for (int i = 1; i <= 10; i++) {
		double time = Profile([&]() {
			files.Add("benchmark/tiny/tiny" + std::to_string(i) + ".bin");
			
			if (!files.Add(filename)) {
				// File already exists
				FileInfo info = files.GetFileInfo(filename);
			
				// Retrieve the file
				Write(*oram, info);
			}
		});
		
		elapsedTime += time;
	}

	printf("\nAverage time = %f\n", elapsedTime/10.0);
	
	files.Save();
	*/

	delete oram;

	AES::Cleanup();
}

void read(Agent &agent)
{
	puts("reading");
	
	std::fstream file;
	file.open("input.bin", std::ios::in | std::ios::binary);

	if (!file.good()) {
		return;
	}

	size_t length = File::GetLength(file);
	size_t blockSize = agent.GetBlockSize();
	
	for (size_t i = 0; i < length; i += blockSize) {
		size_t readLength = std::min(blockSize, length - i);
		int64_t bid = i/blockSize;

		block b(blockSize);
		
		file.read((char *) b.data(), readLength);
		agent.Access(Agent::WRITE, bid, b);

		printf("\r%zu/%zu", i/blockSize + 1, length/blockSize);
		fflush(stdout);
	}
	puts("\n");

	file.close();
}

void right(Agent &agent, size_t length)
{
	puts("righting");

	std::fstream file;
	file.open("output.bin", std::ios::out | std::ios::binary | std::ios::trunc);

	size_t blockSize = agent.GetBlockSize();

	for (size_t i = 0; i < length; i += blockSize) {
		size_t writeLength = std::min(blockSize, length - i);
		int64_t bid = i/blockSize;

		block b = agent.Access(Agent::READ, bid, {});
		file.write((char *) b.data(), writeLength);

		printf("\r%zu/%zu", i/blockSize + 1, length/blockSize);
		fflush(stdout);
	}
	puts("\n");

	file.close();
}

void Write(ORAM &oram, FileInfo info)
{
    puts("writing");
    
    std::fstream file;
    file.open("output.bin", std::fstream::out | std::fstream::binary | std::fstream::trunc);

	size_t blockSize = oram.GetBlockSize();

    for (size_t i = 0; i < info.length; i += blockSize) {
        int writeLength = std::min(blockSize, info.length - i);
        int bid = info.blocks[i/blockSize];
        
        block buffer;
        oram.Access(ORAM::READ, bid, buffer);
        file.write((char *) buffer.data(), writeLength);
        
        printf("\r%zu / %zu", i/blockSize + 1, info.length/blockSize);
        fflush(stdout);
    }
    file.close();
    puts("\n");
}

int MBtoDepth(size_t mb, size_t blockSize /*= 4096*/)
{
	mb *= 1024*1024;
	
	size_t blocks = ceil(mb/(double) blockSize);
	size_t buckets = ceil(blocks/(double) Z);
	
	return ceil(log2(buckets+1))-1;
}

double Profile(std::function<void()> fun)
{
	Timer timer;
	timer.Start();
	
	fun();
	
	timer.Stop();
	double elapsedTime = timer.GetElapsedTime();
	
	printf("Elapsed time = %f\n", elapsedTime);
	
	return elapsedTime;
}
