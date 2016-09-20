#if 0
g++ -static -std=c++11 -o ${0/cc/out} $0
exit
#endif

#include <iostream>
#include <vector>
#include <sys/time.h>

#include "order_cover.hh"
#include "rbtree.hh"


int COMP(value_t k1, value_t k2) {
	if (k1 < k2) return -1;
	if (k1 > k2) return 1;
	return 0;
}

struct NODE
{
	rbtree_node<NODE> node;
	value_t key;
};
using TREE = rbtree_<
    value_t,                 // KEY_TYPE
    NODE,                   // VALUE_TYPE
    &COMP,               // KEY_COMPARE
    rbtree_node<NODE> NODE::*, // LINK_METHOD
    &NODE::node,         // LINK
    value_t NODE::*,            // KEY_METHOD
    &NODE::key              // KEY
>;

struct time_sum
{
	long secs;
	long usecs;

	time_sum() : secs(0), usecs(0) {}
	void add(const timeval* t1, const timeval* t2) {
		const long USEC = 1000000L;
		long t1_s = t1->tv_sec;
		long t2_s = t2->tv_sec;
		long t1_us = t1->tv_usec;
		long t2_us = t2->tv_usec;
		if (t1_us > t2_us) {
			t2_us += USEC;
			--t2_s;
		}
		usecs += t2_us - t1_us;
		secs += t2_s - t1_s;
		if (usecs >= USEC) {
			usecs -= USEC;
			++secs;
		}
	}
	void print() {
		std::cout << secs << '.';
		std::cout.width(6);
		std::cout.fill('0');
		std::cout << usecs;
	}
} unit_time;

unsigned long unit_cnt = 0;

void unit_one(value_t n, cover& cov)
{
	std::vector<bool> flags(n, false);
	TREE tree;
	NODE* nodes = new NODE[n];

	for (value_t i = 0; i < n; ++i)
		nodes[i].key = i;

	timeval t_t1, t_t2;

	gettimeofday(&t_t1, nullptr);
	for (value_t i = 0; ; ++i) {
		value_t val = cov.get_order(i);
		if (val == VALUE_INVALID)
			break;

		if (!flags[val])
			tree.insert(&nodes[val]);
		else
			tree.remove(&nodes[val]);

		std::cout << tree.validate();
		tree.print();

		flags[val] = !flags[val];
	}
	gettimeofday(&t_t2, nullptr);

	unit_time.add(&t_t1, &t_t2);

	delete [] nodes;
}

void unit(value_t n, cover& cov)
{
	std::vector<bool> flags(n, false);
	TREE tree;
	NODE* nodes = new NODE[n];

	for (value_t i = 0; i < n; ++i)
		nodes[i].key = i;

	timeval t_t1, t_t2;

	gettimeofday(&t_t1, nullptr);
	for (value_t i = 0; ; ++i) {
		value_t val = cov.get_order(i);
		if (val == VALUE_INVALID)
			break;

		if (!flags[val])
			tree.insert(&nodes[val]);
		else
			tree.remove(&nodes[val]);

		flags[val] = !flags[val];
	}
	gettimeofday(&t_t2, nullptr);

	unit_time.add(&t_t1, &t_t2);

	delete [] nodes;
}

void report(const cover& cov)
{
	std::cout <<
	"--------------\nunit/cut  : " <<
	unit_cnt << " / " << cov.get_cut_cnt() <<
	"\nunit time : ";
	unit_time.print();
	std::cout << "\nstate     : ";
	for (value_t i = 0; ; ++i) {
		value_t stat = cov.get_state(i);
		if (stat == VALUE_INVALID) {
			break;
		}
		std::cout << stat << ',';
	}
	std::cout << "\norder     : ";
	for (value_t i = 0; ; ++i) {
		value_t val = cov.get_order(i);
		if (val == VALUE_INVALID) {
			std::cout << std::endl;
			break;
		}
		std::cout << val << ',';
	}
}

void print_order(const cover& cov)
{
	for (value_t i = 0; ; ++i) {
		value_t val = cov.get_order(i);
		if (val == VALUE_INVALID) {
			std::cout << std::endl;
			break;
		}
		std::cout << val << ',';
	}
}

void test_one()
{
	const int n = 6;
	value_t order[n * 2] = {
	0,5,5,4,4,3,3,2,2,1,1,0,
	};

	cover cov(n, 2);
	cov.setup();

	for (value_t i = 0; i < (n * 2); ++i)
		cov.set_order(order[i], i);

	print_order(cov);
	unit_one(n, cov);
	++unit_cnt;

	report(cov);

	cov.unsetup();
}

void test_from()
{
	const int n = 6;
	value_t start_state[n * 2] = {
	0,9,7,3,0,0,2,4,1,0,1,0,
	};

	cover cov(n, 2);
	cov.setup();

	for (value_t i = 0; i < (n * 2); ++i)
		cov.set_state(start_state[i], i);
	cov.update_order();

	for (;;) {
		print_order(cov);
		unit(n, cov);
		++unit_cnt;

		bool next = cov.next_order();
		if (!next)
			break;

		if ((unit_cnt & 0xffff) == 0) {
			report(cov);
		}

	}

	report(cov);

	cov.unsetup();
}

void test(value_t n)
{
	cover cov(n, 2);
	cov.setup();

	for (;;) {
		unit(n, cov);
		++unit_cnt;

		bool next = cov.next_order();
		if (!next)
			break;

		if ((unit_cnt & 0xffff) == 0) {
			report(cov);
		}

	}

	report(cov);

	cov.unsetup();
}

int main(int argc, const char* argv[])
{
	//test_one();

	//test_from();

	value_t from = 1;
	value_t to = 8;
	if (argc >= 2)
		from = atoi(argv[1]);
	if (argc >= 3)
		to = atoi(argv[2]);

	for (value_t n = from; n <= to; ++n) {
		test(n);
	}

	std::cout << "--\nOK" << std::endl;

	return 0;
}

