#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>


using namespace std;


template<class DataType>
struct ShareArea
{
	mutex lock;
	atomic_bool finished;

	condition_variable empty;
	condition_variable full;

	queue<DataType> queue;
};


template<class DataType>
class Pruducer
{
public:
	Pruducer(ShareArea<DataType> *area, size_t push_cout=20):
		area_(area), push_cout_(push_cout){};

	void operator () ()
	{
		size_t count = 0;

		while (count < push_cout_) {
			unique_lock<mutex> lock(area_->lock);

			if (!area_->queue.empty()) {
				area_->empty.wait(lock);
			}

			area_->queue.push(count++);
			
			area_->full.notify_all();
		}

		area_->finished = true;
	}

private:
	ShareArea<DataType> *area_;
	size_t push_cout_;
};

template<class DataType>
class Consumer
{
public:
	Consumer(ShareArea<DataType> *area) :area_(area) {};

	void operator () ()
	{
		while (true){
			unique_lock<mutex> lock(area_->lock);

			if (area_->queue.empty()) {
				if (area_->finished) {
					return;
				}
				area_->full.wait(lock);
			}

			cout << area_->queue.front() << endl;
			area_->queue.pop();

			area_->empty.notify_all();
		}
	}

private:
	ShareArea<DataType> *area_;
};



int main()
{
	ShareArea<size_t> area;
	area.finished = false;

	Pruducer<size_t> p(&area);
	Consumer<size_t> c(&area);

	thread pro_th(p);
	thread com_th(c);

	pro_th.join();
	com_th.join();

    return 0;
} 
