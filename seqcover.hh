
#ifndef SEQCOVER_HH_
#define SEQCOVER_HH_

#include <cstring>
#include <vector>


template <class KEY_T>
class seq_cover
{
public:
	using key_t    = KEY_T;
	using seqnum_t = unsigned long;
	using cnt_t    = unsigned long;
	using self_t   = seq_cover<KEY_T>;

	enum : key_t {
		INVALID = 0xffff,
		MAX = 0xfffe,
	};

public:
	seq_cover(key_t key_number, int duplicates) :
		key_num(key_number),
		key_dups(duplicates),
		state(nullptr),
		seq(nullptr),
		passed_seq(nullptr),
		cut_cnt(0)
	{
		seq_num = static_cast<seqnum_t>(key_num) * key_dups;
	}

	bool     setup();
	void     unsetup();
	bool     next_order();
	void     update_seq();

	seqnum_t get_state(seqnum_t index) const {
		return index < seq_num ? state[index] : INVALID;
	}
	key_t get_seq(seqnum_t index) const {
		return index < seq_num ? seq[index] : INVALID;
	}
	key_t get_passed_seq(seqnum_t index) const {
		return index < seq_num ? passed_seq[index] : INVALID;
	}
	cnt_t get_cut_cnt() const {
		return cut_cnt;
	}
	void set_state(key_t stat, seqnum_t index) {
		state[index] = stat;
	}
	void set_seq(key_t key, seqnum_t index) {
		seq[index] = key;
	}


private:
	bool    next_state();
	bool    look_back_order();
	void    commit_seq();

private:
	key_t     key_num;
	int       key_dups;
	seqnum_t  seq_num;
	seqnum_t* state;
	key_t*    seq;
	key_t*    passed_seq;
	cnt_t     cut_cnt;
};

template<class KEY_T>
bool seq_cover<KEY_T>::setup()
{
	state = new seqnum_t[seq_num];
	if (!state)
		return false;

	seq = new key_t[seq_num];
	if (!seq)
		return false;

	passed_seq = new key_t[seq_num];
	if (!passed_seq)
		return false;

	for (seqnum_t i = 0; i < seq_num; ++i) {
		state[i] = 0;
		seq[i] = 0;
		passed_seq[i] = 0;
	}

	update_seq();
	commit_seq();

	return true;
}

template<class KEY_T>
void seq_cover<KEY_T>::unsetup()
{
	delete [] state;
	state = nullptr;

	delete [] seq;
	seq = nullptr;

	delete [] passed_seq;
	passed_seq = nullptr;
}

template<class KEY_T>
bool seq_cover<KEY_T>::next_order()
{
	for (;;) {
		if (!next_state())
			return false;

		update_seq();

		if (look_back_order())
			break;
		else
			++cut_cnt;
	}

	commit_seq();

	return true;
}

template<class KEY_T>
void seq_cover<KEY_T>::update_seq()
{
	std::vector<key_t> key_vec(seq_num);

	for (key_t key = 0; key < key_num; ++key)
		for (int dup = 0; dup < key_dups; ++ dup)
			key_vec[key_dups * key + dup] = key;

	for (cnt_t i = 0; i < seq_num; ++i) {
		cnt_t key_index = state[i];

		auto key_itr = key_vec.begin() + key_index;

		seq[i] = *key_itr;

		key_vec.erase(key_itr);
	}
}







template<class KEY_T>
bool seq_cover<KEY_T>::next_state()
{
	key_t i;
	for (i = 1; i < seq_num; ++i) {
		key_t state_index = seq_num - 1 - i;
		const seqnum_t combination = i;
		if (state[state_index] < combination) {
			++state[state_index];
			break;
		} else /* if (state[state_index] == last_combination) */ {
			state[state_index] = 0;
		}
	}

	return i < seq_num;
}

template<class KEY_T>
bool seq_cover<KEY_T>::look_back_order()
{
	for (key_t i = 0; i < seq_num; ++i) {
		if (seq[i] < passed_seq[i])
			return false;
		if (seq[i] > passed_seq[i])
			return true;
	}

	return false;
}

template<class KEY_T>
void seq_cover<KEY_T>::commit_seq()
{
	std::memcpy(passed_seq, seq, sizeof (key_t[seq_num]));
}


#endif  // SEQCOVER_HH_

