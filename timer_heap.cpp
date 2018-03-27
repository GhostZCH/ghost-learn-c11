#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>

using namespace std;


class Timers
{
public:
	Timers() {};
	~Timers() {};

	void Add(uint32_t timer) {
		heap_.push_back(timer);
		push_heap(heap_.begin(), heap_.end(), greater<uint32_t>());
	}

	uint32_t Get() {
		if (heap_.size() == 0) {
			return 0;
		}
		pop_heap(heap_.begin(), heap_.end(), greater<uint32_t>());
		
		uint32_t re = heap_[heap_.size() - 1];
		heap_.pop_back();
		return re;
	}

private:
	vector<uint32_t> heap_;
};


int main()
{
	Timers tiemrs;

	tiemrs.Add(25);
	tiemrs.Add(13);
	tiemrs.Add(3);
	tiemrs.Add(73);

	while (true)
	{
		uint32_t u = tiemrs.Get();
		cout << u << endl;
		if (u == 0) {
			break;
		}
	}

    return 0;
}
