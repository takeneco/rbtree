#if 0
g++ -std=c++11 -o ${0/cc/out} $0
exit
#endif

template<class A, class B, class D, D E>
class member_ptr
{};
template<class RETURN, class CLASS, RETURN CLASS::* VARIABLE>
class member_ptr<RETURN, CLASS, RETURN CLASS::*, VARIABLE>
{
public:
	static RETURN* p(CLASS* x) { return &(x->*VARIABLE); }
};
template<class RETURN, class CLASS, RETURN* (CLASS::* FUNC)()>
class member_ptr<RETURN, CLASS, RETURN* (CLASS::*)(), FUNC>
{
public:
	static RETURN* p(CLASS* x) { return (x->*FUNC)(); }
};

template<class VALUE_TYPE>
class rbtree_node
{
	using self_t = rbtree_node<VALUE_TYPE>;
public:
	rbtree_node() :
		parent(),
		child(),
		// hand(),  // no appropriate initial value.
		color(RED)
	{}

	self_t& operator = (const rbtree_node<VALUE_TYPE>& x) {
		parent = x.parent;
		child[LEFT] = child[LEFT];
		child[RIGHT] = child[RIGHT];
		side = x.side;
		color = x.color;
		return *this;
	}

	void clear() {
		color = 0;
		child[RIGHT] = child[LEFT] = parent = nullptr;
	}

	enum COLOR { RED = 0, BLACK = 1, };
	enum SIDE { LEFT = 0, RIGHT = 1, };

//private:
	VALUE_TYPE* parent;
	VALUE_TYPE* child[2]; // left and right
	SIDE side;   // this = parent->child[side]
	COLOR color;
};

template<
    class KEY_TYPE,
    class VALUE_TYPE,
    int (*KEY_COMPARE)(KEY_TYPE, KEY_TYPE),
    class NODE_METHOD,
    NODE_METHOD NODE,
    class KEY_METHOD,
    KEY_METHOD KEY
>
class rbtree_
{
	using COLOR = typename rbtree_node<VALUE_TYPE>::COLOR;
	using SIDE = typename rbtree_node<VALUE_TYPE>::SIDE;
	using node_type = rbtree_node<VALUE_TYPE>;
	using val_t = VALUE_TYPE;

public:
	rbtree_() :
		root(nullptr)
	{}

	VALUE_TYPE* insert(VALUE_TYPE* val) {
		auto r = _insert(val);
		return r;
	}
	void remove(KEY_TYPE key) {
		auto cur = root;
		while (cur) {
			int c = comp(key, key_of(cur));
			if (c < 0) {
				cur = left_of(cur);
			} else if (c > 0) {
				cur = right_of(cur);
			} else {
				_remove(cur);
				break;
			}
		}
		//return nullptr;
	}
	void remove(VALUE_TYPE* val) {
		_remove(val);
	}
	bool validate() {
		if (!root)
			return true;
		if (parent_of(root) != nullptr)
			return false;
		return _validate(root);
	}
	bool _validate(VALUE_TYPE* val) {
		auto val_key = key_of(val);
		VALUE_TYPE* left = left_of(val);
		if (left) {
			if (key_of(left) >= val_key)
				return false;
			if (parent_of(left) != val)
				return false;
			if (!_validate(left))
				return false;
		}
		VALUE_TYPE* right = right_of(val);
		if (right) {
			if (key_of(right) <= val_key)
				return false;
			if (parent_of(right) != val)
				return false;
			if (!_validate(right))
				return false;
		}
		return true;
	}
	void print(const char* note = nullptr) {
		if (note)
			std::cout << note << ':';
		_print(root);
		std::cout << std::endl;
	}
	void _print(VALUE_TYPE* val) {
		std::cout << '[';
		if (val) {
			VALUE_TYPE* left = left_of(val);
			if (left)
				_print(left);
			std::cout << '-' << key_of(val);
			if (is_red(val))
				std::cout << '*';
			std::cout << '-';
			VALUE_TYPE* right = right_of(val);
			if (right)
				_print(right);
		}
		std::cout << ']' << std::flush;
	}

private:
	rbtree_node<VALUE_TYPE>* node_of(VALUE_TYPE* val) {
		return member_ptr
		    <rbtree_node<VALUE_TYPE>, VALUE_TYPE, NODE_METHOD, NODE>::p
		    (val);
	}
	KEY_TYPE key_of(VALUE_TYPE* val) {
		return *member_ptr
		    <KEY_TYPE, VALUE_TYPE, KEY_METHOD, KEY>::p
		    (val);
	}
	VALUE_TYPE* parent_of(VALUE_TYPE* val) {
		return node_of(val)->parent;
	}
	SIDE side_of_parent(VALUE_TYPE* val) {
		return node_of(val)->side;
	}
	VALUE_TYPE* left_of(VALUE_TYPE* val) {
		return node_of(val)->child[SIDE::LEFT];
	}
	VALUE_TYPE* right_of(VALUE_TYPE* val) {
		return node_of(val)->child[SIDE::RIGHT];
	}
	int comp(KEY_TYPE a, KEY_TYPE b) {
		return (*KEY_COMPARE)(a, b);
	}
	bool is_red(VALUE_TYPE* val) {
		return node_of(val)->color == COLOR::RED;
	}
	bool is_black(VALUE_TYPE* val) {
		return node_of(val)->color == COLOR::BLACK;
	}
	bool set_color(VALUE_TYPE* val, COLOR c) {
		node_of(val)->color = c;
	}
	void set_parent(VALUE_TYPE* val, VALUE_TYPE* parent) {
		node_of(val)->parent = parent;
	}
	void set_left(VALUE_TYPE* parent, VALUE_TYPE* left) {
		node_of(parent)->child[SIDE::LEFT] = left;
		//auto left_node = node_of(left);
		//left_node->parent = parent;
		//left_node->side = SIDE::LEFT;
	}
	void set_right(VALUE_TYPE* parent, VALUE_TYPE* right) {
		node_of(parent)->child[SIDE::RIGHT] = right;
		//auto right_node = node_of(right);
		//right_node->parent = parent;
		//right_node->side = SIDE::RIGHT;
	}
	void change_child(
	    VALUE_TYPE* parent, VALUE_TYPE* old_child, VALUE_TYPE* new_child) {
		if (parent) {
			if (left_of(parent) == old_child)
				set_left(parent, new_child);
			else
				set_right(parent, new_child);
		} else {
			root = new_child;
		}
	}

	VALUE_TYPE* rotate_left(VALUE_TYPE* val)
	{
		auto right = right_of(val);
		auto rightleft = left_of(right);
		set_left(right, val);
		set_parent(val, right);
		set_right(val, rightleft);
		if (rightleft)
			set_parent(rightleft, val);
		return right;
	}
	VALUE_TYPE* rotate_right(VALUE_TYPE* val)
	{
		auto left = left_of(val);
		auto leftright = right_of(left);
		set_right(left, val);
		set_parent(val, left);
		set_left(val, leftright);
		if (leftright)
			set_parent(leftright, val);
		return left;
	}
	VALUE_TYPE* rotate_leftright(VALUE_TYPE* val)
	{
		auto tmp = rotate_left(left_of(val));
		set_left(val, tmp);
		set_parent(tmp, val);
		return rotate_right(val);
	}
	VALUE_TYPE* rotate_rightleft(VALUE_TYPE* val)
	{
		auto tmp = rotate_right(right_of(val));
		set_right(val, tmp);
		set_parent(tmp, val);
		return rotate_left(val);
	}
	VALUE_TYPE* _insert(VALUE_TYPE* val)
	{
		KEY_TYPE val_key = key_of(val);
		VALUE_TYPE* cur = root;
		if (cur == nullptr) {
			root = val;
			set_color(val, COLOR::BLACK);
			return nullptr;
		}
		for (;;) {
			auto cur_node = node_of(cur);
			int c = comp(val_key, key_of(cur));
			if (c < 0) {
				if (left_of(cur)) {
					cur = left_of(cur);
				} else {
					set_left(cur, val);
					set_parent(val, cur);
					break;
				}
			} else if (c > 0) {
				if (right_of(cur)) {
					cur = right_of(cur);
				} else {
					set_right(cur, val);
					set_parent(val, cur);
					break;
				}
			} else {
				// 同値にあたった
				//node_of(val) = r_node;
				//r_node.clear();
				return cur;
			}
		}
		for (;;) {
			auto parent = parent_of(cur);
			auto tmp = balance(cur);
			if (parent == nullptr) {
				root = tmp;
				set_parent(root, nullptr);
				set_color(root, COLOR::BLACK);
				break;
			} else {
				if (left_of(parent) == cur)
					set_left(parent, tmp);
				else
					set_right(parent, tmp);
				set_parent(tmp, parent);
			}
			cur = parent;
		}
		return nullptr;
	}
	VALUE_TYPE* balance(VALUE_TYPE* val)
	{
		auto val_node = node_of(val);
		auto left = left_of(val);
		auto right = right_of(val);
		if (is_red(val)) {
			return val;
		} else if (left && is_red(left) &&
		           left_of(left) && is_red(left_of(left))) {
			val = rotate_right(val);
			set_color(left_of(val), COLOR::BLACK);
		} else if (left && is_red(left) &&
		           right_of(left) && is_red(right_of(left))) {
			val = rotate_leftright(val);
			set_color(left_of(val), COLOR::BLACK);
		} else if (right && is_red(right) &&
		           left_of(right) && is_red(left_of(right))) {
			val = rotate_rightleft(val);
			set_color(right_of(val), COLOR::BLACK);
		} else if (right && is_red(right) &&
		           right_of(right) && is_red(right_of(right))) {
			val = rotate_left(val);
			set_color(right_of(val), COLOR::BLACK);
		}
/*
		 else if (left && is_red(left)) {
			auto left_left = left_of(left);
			auto left_right = right_of(left);
			if (left_left && is_red(left_left)) {
				val = rotate_right(val);
				set_color(left_of(val), COLOR::BLACK);
			} else if (left_right && is_red(left_right)) {
				val = rotate_leftright(val);
				set_color(left_of(val), COLOR::BLACK);
			}
		} else if (right && is_red(right)) {
			auto right_left = left_of(right);
			auto right_right = right_of(right);
			if (right_left && is_red(right_left)) {
				val = rotate_rightleft(val);
				set_color(right_of(val), COLOR::BLACK);
			} else if (right_right && is_red(right_right)) {
				val = rotate_left(val);
				set_color(right_of(val), COLOR::BLACK);
			}
		}
*/
		return val;
	}

	void _remove(VALUE_TYPE* val) {
		VALUE_TYPE* val_left = left_of(val);
		VALUE_TYPE* val_right = right_of(val);
		if (!val_left) {
			VALUE_TYPE* val_parent = parent_of(val);
			VALUE_TYPE* val_right = right_of(val);
			change_child(val_parent, val, val_right);
			if (val_right) {
				set_parent(val_right, val_parent);
				set_color(val_right, node_of(val)->color);
			}
			//if (!val_parent) {
			//	if (root)
			//		set_color(root, COLOR::BLACK); // need?
			//}
			else if (is_black(val) && val_parent) {
				remove_balance(val_parent);
			}
		} else if (!val_right) {
			VALUE_TYPE* val_parent = parent_of(val);
			set_parent(val_left, val_parent);
			set_color(val_left, node_of(val)->color);
			change_child(val_parent, val, val_left);
		} else {
			auto mr = get_most_right(val_left);
			auto mr_parent = parent_of(mr);
			auto mr_left = left_of(mr);
			VALUE_TYPE* parent;
			if (mr != val_left) {
				set_right(mr_parent, mr_left);
				set_left(mr, val_left);
				set_parent(val_left, mr);
				parent = mr_parent;
			} else {
				parent = mr;
			}
			COLOR mr_c = node_of(mr)->color;
			set_right(mr, val_right);
			set_parent(val_right, mr);
			change_child(parent_of(val), val, mr);
			set_parent(mr, parent_of(val));
			set_color(mr, node_of(val)->color);
			if (mr_left) {
				set_parent(mr_left, parent);
				set_color(mr_left, COLOR::BLACK);
			} else {
				if (mr_c == COLOR::BLACK)
					remove_balance(parent);
			}
		}
	}
	void remove_balance(val_t* parent) {
		val_t* changed = nullptr;
		for (;;) {
			val_t* cur = right_of(parent);
			if (changed != cur) {
				if (is_red(cur)) {
					auto parent_of_parent = parent_of(parent);
					auto parent2 = rotate_left(parent);
					change_child(parent_of_parent, parent, parent2);
					set_parent(parent2, parent_of_parent);
					set_color(parent2, COLOR::BLACK);
					set_color(parent, COLOR::RED);
					cur = right_of(parent);
				}
				auto cur_right = right_of(cur);
				if (!cur_right || is_black(cur_right)) {
					auto cur_left = left_of(cur);
					if (!cur_left || is_black(cur_left)) {
						set_color(cur, COLOR::RED);
						if (is_red(parent)) {
							set_color(parent, COLOR::BLACK);
						} else {
							changed = parent;
							parent = parent_of(parent);
							if (parent)
								continue;
						}
						break;
					}
					auto cur2 = rotate_right(cur);
					change_child(parent, cur, cur2);
					set_parent(cur2, parent);
					cur = cur2;
				}
				auto parent_of_parent = parent_of(parent);
				auto parent2 = rotate_left(parent);
				change_child(parent_of_parent, parent, parent2);
				set_parent(parent2, parent_of_parent);
				set_color(right_of(parent2), COLOR::BLACK);
				set_color(parent2, node_of(parent)->color);
				set_color(parent, COLOR::BLACK);
				break;
			} else {
				cur = left_of(parent);
				if (is_red(cur)) {
					auto parent_of_parent = parent_of(parent);
					auto parent2 = rotate_right(parent);
					change_child(parent_of_parent, parent, parent2);
					set_parent(parent2, parent_of_parent);
					set_color(parent2, COLOR::BLACK);
					set_color(parent, COLOR::RED);
					cur = left_of(parent);
				}
				auto cur_left = left_of(cur);
				if (!cur_left || is_black(cur_left)) {
					auto cur_right = right_of(cur);
					if (!cur_right || is_black(cur_right)) {
						set_color(cur, COLOR::RED);
						if (is_red(parent)) {
							set_color(parent, COLOR::BLACK);
						} else {
							changed = parent;
							parent = parent_of(parent);
							if (parent)
								continue;
						}
						break;
					}
					auto cur2 = rotate_left(cur);
					change_child(parent, cur, cur2);
					set_parent(cur2, parent);
					cur = cur2;
				}
				auto parent_of_parent = parent_of(parent);
				auto parent2 = rotate_right(parent);
				change_child(parent_of_parent, parent, parent2);
				set_parent(parent2, parent_of_parent);
				set_color(left_of(parent2), COLOR::BLACK);
				set_color(parent2, node_of(parent)->color);
				set_color(parent, COLOR::BLACK);
				break;
			}
		}
	}

	VALUE_TYPE* get_most_right(VALUE_TYPE* val) {
		for (;;) {
			auto tmp = right_of(val);
			if (!tmp)
				return val;
			val = tmp;
		}
	}
	VALUE_TYPE* get_most_left(VALUE_TYPE* val) {
		for (;;) {
			auto tmp = left_of(val);
			if (!tmp)
				return val;
			val = tmp;
		}
	}

private:
	VALUE_TYPE* root;
};

