#if 0
g++ -static -std=c++11 -o ${0/cc/out} $0
exit
#endif

#include <iostream>
#include <vector>
#include <sys/time.h>
#include <time.h>

#include "seqcover.hh"
#include "rbtree.hh"


using covkey_t = unsigned short;
using intcov = seq_cover<covkey_t>;

struct NODE : rbtree_node
{
	covkey_t key;
};
int COMP(const rbtree::node& n1, const rbtree::node& n2)
{
	covkey_t k1 = static_cast<const NODE&>(n1).key;
	covkey_t k2 = static_cast<const NODE&>(n2).key;
	if (k1 < k2) return -1;
	if (k1 > k2) return 1;
	return 0;
}

using TREE = rbtree_of<&COMP>;

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

void unit_one(covkey_t n, intcov& cov)
{
	std::vector<bool> flags(n, false);
	TREE tree;
	NODE* nodes = new NODE[n];

	for (covkey_t i = 0; i < n; ++i)
		nodes[i].key = i;

	timeval t_t1, t_t2;

	gettimeofday(&t_t1, nullptr);
	for (covkey_t i = 0; ; ++i) {
		covkey_t val = cov.get_seq(i);
		if (val == cov.INVALID)
			break;

		if (!flags[val])
			tree.insert(&nodes[val]);
		else
			tree.remove(&nodes[val]);

		//std::cout << tree.validate();
		//tree.print();

		flags[val] = !flags[val];
	}
	gettimeofday(&t_t2, nullptr);

	unit_time.add(&t_t1, &t_t2);

	delete [] nodes;
}

void unit(covkey_t n, intcov& cov)
{
	std::vector<bool> flags(n, false);
	TREE tree;
	NODE* nodes = new NODE[n];

	for (covkey_t i = 0; i < n; ++i)
		nodes[i].key = i;

	timeval t_t1, t_t2;

	gettimeofday(&t_t1, nullptr);
	for (covkey_t i = 0; ; ++i) {
		covkey_t val = cov.get_seq(i);
		if (val == cov.INVALID)
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

void report(const intcov& cov)
{
	std::cout << "--------------\n";

	timeval tv;
	gettimeofday(&tv, nullptr);
	tm* t = localtime(&tv.tv_sec);
	std::cout <<
	t->tm_year + 1900 << '-' <<
	t->tm_mon + 1 << '-' <<
	t->tm_mday << ' ' <<
	t->tm_hour << ':' <<
	t->tm_min << ':' <<
	t->tm_sec << '.';
	std::cout.width(6);
	std::cout.fill('0');
	std::cout << tv.tv_usec <<

	"\nunit/cut  : " << 
	unit_cnt << " / " << cov.get_cut_cnt() <<
	"\nunit time : ";
	unit_time.print();
	std::cout << "\nstate     : ";
	for (covkey_t i = 0; ; ++i) {
		covkey_t stat = cov.get_state(i);
		if (stat == cov.INVALID) {
			break;
		}
		std::cout << stat << ',';
	}
	std::cout << "\norder     : ";
	for (covkey_t i = 0; ; ++i) {
		covkey_t val = cov.get_seq(i);
		if (val == cov.INVALID) {
			std::cout << std::endl;
			break;
		}
		std::cout << val << ',';
	}
}

void print_order(const intcov& cov)
{
	for (covkey_t i = 0; ; ++i) {
		covkey_t val = cov.get_seq(i);
		if (val == cov.INVALID) {
			std::cout << std::endl;
			break;
		}
		std::cout << val << ',';
	}
}

void test_one()
{
	const int n = 6;
	covkey_t order[n * 2] = {
	0,5,5,4,4,3,3,2,2,1,1,0,
	};

	intcov cov(n, 2);
	cov.setup();

	for (covkey_t i = 0; i < (n * 2); ++i)
		cov.set_seq(order[i], i);

	print_order(cov);
	unit_one(n, cov);
	++unit_cnt;

	report(cov);

	cov.unsetup();
}

void test_from()
{
	const int n = 6;
	covkey_t start_state[n * 2] = {
	0,9,7,3,0,0,2,4,1,0,1,0,
	};

	intcov cov(n, 2);
	cov.setup();

	for (covkey_t i = 0; i < (n * 2); ++i)
		cov.set_state(start_state[i], i);
	cov.update_seq();

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

void test(covkey_t n)
{
	intcov cov(n, 2);
	cov.setup();

	for (;;) {
		unit(n, cov);
		++unit_cnt;

		bool next = cov.next_order();
		if (!next)
			break;

		if ((unit_cnt & 0xfffff) == 0) {
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

	covkey_t from = 1;
	covkey_t to = 8;
	if (argc >= 2)
		from = atoi(argv[1]);
	if (argc >= 3)
		to = atoi(argv[2]);

	timeval tv;
	gettimeofday(&tv, nullptr);
	std::cout << ctime(&tv.tv_sec) << '.';
	std::cout.width(6);
	std::cout.fill('0');
	std::cout << tv.tv_usec << std::endl;

	for (covkey_t n = from; n <= to; ++n) {
		test(n);
	}

	std::cout << "--\nOK" << std::endl;

	return 0;
}

