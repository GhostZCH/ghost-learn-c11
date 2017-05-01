#include "stdafx.h"

#include <mutex>
#include <thread>
#include <iostream>
#include <unordered_map>
#include <atomic>
#include <vector>


using namespace std;


class RWLock
{
public:
	RWLock():is_writing_(false), reader_count_(0){}

	void BeginWrite() {
		is_writing_ = true;
		while (reader_count_ > 0);
	}

	void EndWrite() {
		is_writing_ = false;
	}

	void BeginRead() {
		while (is_writing_);
		reader_count_++;
	}

	void EndRead() {
		reader_count_--;
	}

private:
	atomic_bool is_writing_;
	atomic_int reader_count_;
};


class ReadLock
{
public:
	ReadLock(RWLock* lock)
		:lock_(lock)
	{
		lock_->BeginRead();
	}

	~ReadLock()
	{
		lock_->EndRead();
	}

private:
	RWLock* lock_;
};


class WriteLock
{
public:
	WriteLock(RWLock* lock)
		:lock_(lock)
	{
		lock_->BeginWrite();
	}

	~WriteLock()
	{
		lock_->EndWrite();
	}

private:
	RWLock* lock_;
};



class ShareMap
{
public:
	ShareMap()
	{
		for (int i =0; i < 10; i++) {
			map_[i] = i;
		}
	}
	
	void List() 
	{
		ReadLock lock(&lock_);

		unordered_map<int, int>::iterator iter = map_.begin();
		for (; iter != map_.end(); iter++) {
			cout << iter->first << '\t' << iter->second << endl;
		}
	}

	void Update()
	{
		WriteLock lock(&lock_);

		unordered_map<int, int>::iterator iter = map_.begin();
		for (; iter != map_.end(); iter++) {
			iter->second += 10;
		}
	}

private:
	RWLock lock_;
	unordered_map<int, int> map_;
};


void Reader(int id, ShareMap *map)
{
	int max_run_count = 10;
	while (max_run_count-- > 0){
		cout << id << endl;
		map->List();
	}
}

void Writer(ShareMap *map)
{
	int max_update_count = 1000;

	while (max_update_count-- > 0) {
		cout << "Writer" << endl;
		map->Update();
	}
}


int main()
{
	const int reader_count = 4;

	ShareMap map;

	vector<thread> readers;
	for (int i = 0; i < reader_count; i++) {
		readers.push_back(thread(Reader, i, &map));
	}

	thread writer(Writer, &map);
	
	vector<thread>::iterator iter = readers.begin();
	for (; iter != readers.end(); iter++) {
		(*iter).join();
	}

	writer.join();

	return 0;
}
