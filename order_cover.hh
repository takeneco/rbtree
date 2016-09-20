
#include <cstring>
#include <vector>


using value_t = unsigned short;
enum : value_t {
	VALUE_INVALID = 0xffff,
	VALUE_MAX = 0xfffe,
};

class cover
{
public:
	cover(value_t value_count, int duplicates) :
		val_cnt(value_count),
		dups(duplicates),
		state(nullptr),
		order(nullptr),
		commited_order(nullptr),
		cut_cnt(0)
	{
		val_cnt *= dups;
	}

	bool    setup();
	void    unsetup();
	bool    next_order();
	void    update_order();
	value_t get_state(value_t index) const;
	value_t get_order(value_t index) const;
	value_t get_order_last(value_t index) const;
	unsigned long get_cut_cnt() const;
	void    set_state(value_t state, value_t index);
	void    set_order(value_t order, value_t index);

private:
	bool    next_state();
	bool    look_back_order();
	void    commit_order();

private:
	value_t  val_cnt;
	int      dups;
	value_t* state;
	value_t* order;
	value_t* commited_order;
	unsigned long cut_cnt;
};

bool cover::setup()
{
	state = new value_t[val_cnt];
	if (!state)
		return false;

	order = new value_t[val_cnt];
	if (!order)
		return false;

	commited_order = new value_t[val_cnt];
	if (!commited_order)
		return false;

	for (value_t i = 0; i < val_cnt; ++i) {
		state[i] = 0;
		order[i] = 0;
		commited_order[i] = 0;
	}

	update_order();
	commit_order();

	return true;
}

void cover::unsetup()
{
	delete [] state;
	state = nullptr;

	delete [] order;
	order = nullptr;

	delete [] commited_order;
	commited_order = nullptr;
}

bool cover::next_order()
{
	for (;;) {
		if (!next_state())
			return false;

		update_order();

		if (look_back_order())
			break;
		else
			++cut_cnt;
	}

	commit_order();

	return true;
}

void cover::update_order()
{
	std::vector<bool> val_vec(val_cnt, true);

	for (value_t i = 0; i < val_cnt; ++i) {
		value_t index = state[i];
		value_t val = 0;
		for (val = 0; ; ++val) {
			if (val_vec[val]) {
				if (index == 0) {
					val_vec[val] = false;
					break;
				} else {
					--index;
				}
			}
		}
		order[i] = val / dups;
	}
}

value_t cover::get_state(value_t index) const
{
	if (index >= val_cnt)
		return VALUE_INVALID;

	return state[index];
}

value_t cover::get_order(value_t index) const
{
	if (index >= val_cnt)
		return VALUE_INVALID;

	return order[index];
}

value_t cover::get_order_last(value_t index) const
{
	if (index >= val_cnt)
		return VALUE_INVALID;

	return commited_order[index];
}

unsigned long cover::get_cut_cnt() const
{
	return cut_cnt;
}

void cover::set_state(value_t stat, value_t index)
{
	state[index] = stat;
}

void cover::set_order(value_t _order, value_t index)
{
	order[index] = _order;
}

bool cover::next_state()
{
	value_t i;
	for (i = 1; i < val_cnt; ++i) {
		value_t state_index = val_cnt - 1 - i;
		const value_t last_combination = i;
		if (state[state_index] < last_combination) {
			++state[state_index];
			break;
		} else /* if (state[state_index] == last_combination) */ {
			state[state_index] = 0;
		}
	}

	return i < val_cnt;
}

bool cover::look_back_order()
{
	for (value_t i = 0; i < val_cnt; ++i) {
		if (order[i] < commited_order[i])
			return false;
		if (order[i] > commited_order[i])
			return true;
	}

	return false;
}

void cover::commit_order()
{
	std::memcpy(commited_order, order, sizeof (value_t[val_cnt]));
}


