#if 0
#g++ -O2 -static -std=c++11 -o ${0/cc/out} $0 rbtree.cc
g++ -g -std=c++11 -o ${0/cc/out} $0 rbtree.cc
exit
#endif

#include <algorithm>
#include <iostream>
#include <vector>
#include <sys/time.h>
#include <time.h>

#include "seqcover.hh"
#include "rbtree.hh"


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

std::ostream& operator << (std::ostream& os, const timeval& tv)
{
	tm* t = localtime(&tv.tv_sec);
	char buf[21];
	strftime(buf, sizeof buf, "%F %T.", t);

	std::cout << buf;
	std::cout.width(6);
	std::cout.fill('0');
	std::cout << tv.tv_usec;
}

using testkey_t = unsigned short;
using seqnum_t = unsigned int;

struct NODE : rbtree::node
{
	testkey_t key;
};
int COMP(const rbtree::node& n1, const rbtree::node& n2)
{
	auto k1 = static_cast<const NODE&>(n1).key;
	auto k2 = static_cast<const NODE&>(n2).key;
	if (k1 < k2) return -1;
	if (k1 > k2) return 1;
	return 0;
}
inline testkey_t key_of(rbtree::node* n) {
	return static_cast<NODE*>(n)->key;
}

/// rbtree class extension for test.
class test_rbtree : public rbtree_of<COMP>
{
public:
	test_rbtree() {}

	int validate();
	void print();

private:
	static int _validate(rbtree::node* n);
	static void _print(rbtree::node* n);
};

int test_rbtree::validate()
{
	if (!root)
		return 0;

	if (parent_of(root))
		return 1;

	if (is_red(root))
		return 2;

	return _validate(root);
}

void test_rbtree::print()
{
	_print(root);
}

int test_rbtree::_validate(rbtree::node* n)
{
	testkey_t key = key_of(n);
	auto left = left_of(n);
	if (left) {
		if (key_of(left) >= key)
			return 3;
		if (parent_of(left) != n)
			return 4;
		int r = _validate(left);
		if (r)
			return r;
	}
	auto* right = right_of(n);
	if (right) {
		if (key_of(right) <= key)
			return 5;
		if (parent_of(right) != n)
			return 6;
		int r = _validate(right);
		if (r)
			return r;
	}
	return 0;
}

void test_rbtree::_print(rbtree::node* n)
{
	std::cout << '[';
	if (n) {
		auto* left = left_of(n);
		if (left)
			_print(left);
		std::cout << '-' << key_of(n);
		if (is_red(n))
			std::cout << '*';
		std::cout << '-';
		auto* right = right_of(n);
		if (right)
			_print(right);
	}
	std::cout << ']';
}

class rbtree_tester
{
	enum {
		KEY_DUPS = 2,
	};
public:
	using cnt_t    = unsigned long;

	enum : testkey_t {
		INVALID = 0xffff,
		MAX = 0xfffe,
	};

public:
	rbtree_tester(testkey_t key_number) :
		key_num(key_number),
		seq_num(static_cast<seqnum_t>(key_num) * KEY_DUPS),
		test_cnt(0),
		seq(nullptr),
		insert_flags(seq_num, false),
		nodes(nullptr)
	{
	}

	bool setup();
	void unsetup();
	bool next();
	void test_one();
	void test();
	void print_sequence();
	void print_report();

public:
	testkey_t key_num;
	seqnum_t  seq_num;
	cnt_t     test_cnt;

	testkey_t*  seq;

private:
	std::vector<bool> insert_flags;
	NODE* nodes;
};

bool rbtree_tester::setup()
{
	seq = new testkey_t[seq_num];
	if (!seq)
		return false;

	nodes = new NODE[seq_num];
	if (!nodes)
		return false;

	for (testkey_t k = 0; k < key_num; ++k) {
		for (int dup = 0; dup < KEY_DUPS; ++dup) {
			seqnum_t n = static_cast<seqnum_t>(k) * KEY_DUPS + dup;
			seq[n] = k;
		}
	}

	for (testkey_t i = 0; i < key_num; ++i)
		nodes[i].key = i;

	return true;
}

void rbtree_tester::unsetup()
{
	delete [] seq;
	seq = nullptr;

	delete [] nodes;
	nodes = nullptr;
}

bool rbtree_tester::next()
{
	return std::next_permutation(&seq[0], &seq[seq_num-1]);
}

void rbtree_tester::test_one()
{
	insert_flags.assign(seq_num, false);

	timeval t_t1, t_t2;

	gettimeofday(&t_t1, nullptr);

	test_rbtree tree;
	for (seqnum_t i = 0; i < seq_num; ++i) {
		testkey_t key = nodes[seq[i]].key;

		if (!insert_flags[key])
			tree.insert(&nodes[key]);
		else
			tree.remove(&nodes[key]);

		insert_flags[key] = !insert_flags[key];

		std::cout << tree.validate();
		tree.print();
		std::cout << std::endl;
	}

	gettimeofday(&t_t2, nullptr);

	unit_time.add(&t_t1, &t_t2);

	++test_cnt;
}

void rbtree_tester::test()
{
	insert_flags.assign(seq_num, false);

	timeval t_t1, t_t2;

	gettimeofday(&t_t1, nullptr);

	test_rbtree tree;
	for (seqnum_t i = 0; i < seq_num; ++i) {
		testkey_t key = nodes[seq[i]].key;

		if (!insert_flags[key])
			tree.insert(&nodes[key]);
		else
			tree.remove(&nodes[key]);

		insert_flags[key] = !insert_flags[key];

		int valid = tree.validate();
		if (valid) {
			std::cout << "!!!" << valid << ":";
			print_sequence();
		}
	}

	gettimeofday(&t_t2, nullptr);

	unit_time.add(&t_t1, &t_t2);

	++test_cnt;
}

void rbtree_tester::print_report()
{
	timeval tv;
	gettimeofday(&tv, nullptr);

	std::cout <<
	"--------------\n" <<
	tv <<
	"\ncount : " << test_cnt <<
	"\ntime  : "; unit_time.print();

	std::cout << "\nseq   : ";
	for (testkey_t i = 0; i < seq_num; ++i) {
		std::cout << seq[i] << ',';
	}

	std::cout << std::endl;
}

void rbtree_tester::print_sequence()
{
	for (seqnum_t i = 0; i < seq_num; ++i) {
		std::cout << seq[i] << ',';
	}
	std::cout << std::endl;
}

void test_one()
{
	const int n = 5;
	testkey_t start_seq[n * 2] = {
	0,2,3,1,3,0,1,2,4,4,
	};

	rbtree_tester tester(n);
	tester.setup();

	for (testkey_t i = 0; i < (n * 2); ++i)
		tester.seq[i] = start_seq[i];

	tester.print_sequence();

	tester.test_one();

	tester.unsetup();
}

void test_from()
{
	const int n = 5;
	testkey_t start_seq[n * 2] = {
	0,2,3,1,3,0,1,2,4,4,
	};

	rbtree_tester tester(n);
	tester.setup();

	for (seqnum_t i = 0; i < (n * 2); ++i)
		tester.seq[i] = start_seq[i];

	for (;;) {
		tester.print_sequence();

		tester.test();

		if ((tester.test_cnt & 0xffff) == 0) {
			tester.print_report();
		}

		if (!tester.next())
			break;
	}

	tester.print_report();

	tester.unsetup();
}

void test(int n)
{
	rbtree_tester tester(n);
	tester.setup();

	for (;;) {
		tester.test();

		if ((tester.test_cnt & 0xffffff) == 0) {
			tester.print_report();
		}

		if (!tester.next())
			break;
	}

	tester.print_report();

	tester.unsetup();
}

int main(int argc, const char* argv[])
{
	//test_one();

	//test_from();

	int from = 1;
	int to = 9;
	if (argc >= 2)
		from = atoi(argv[1]);
	if (argc >= 3)
		to = atoi(argv[2]);

	timeval tv;
	gettimeofday(&tv, nullptr);
	std::cout << tv << std::endl;

	for (int n = from; n <= to; ++n) {
		test(n);
	}

	std::cout << "--\nOK" << std::endl;

	return 0;
}

