
#ifndef RBTREE_HH_
#define RBTREE_HH_


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


class rbtree
{
	enum COLOR { RED = 0, BLACK = 1, };

public:
	class node
	{
		friend class rbtree;
	public:
		node() {}

	private:
		node* parent;
		node* left;
		node* right;
		COLOR color;
	};

public:
	rbtree() :
		root(nullptr)
	{}
	void remove(node* val);

protected:
	static COLOR get_color(node* n);
	static bool is_red(node* n);
	static bool is_black(node* n);
	static node* parent_of_red(node* n);
	static node* parent_of(node* n);
	static node* left_of(node* n);
	static node* right_of(node* n);
	static void set_parent_and_color(node* parent, COLOR c, node* n);
	static void set_parent(node* n, node* parent);
	static void set_left(node* parent, node* left);
	static void set_right(node* parent, node* right);
	void change_child(node* parent, node* old_child, node* new_child);
	node* rotate_left(node* val);
	node* rotate_right(node* val);
	node* rotate_leftright(node* val);
	node* rotate_rightleft(node* val);
	void inserted_balance(node* parent);
	void  removed_balance(node* parent);
	node* get_most_right(node* val);
	void replace(node* old_node, node* new_node);

protected:
	node* root;
};

template <
    int (*COMPARE)(const rbtree::node&, const rbtree::node&)
>
class rbtree_of : public rbtree
{
public:
	/// Insert a node.
	/// @return
	///   If the tree has node which has same key, this function
	///   replaces to new node, and returns old node.
	///   Otherwise this function returns nullptr.
	node* insert(node* n) {
		return put(n);
	}

private:
	node* put(node* n) {
		node* cur = root;
		if (cur == nullptr) {
			root = n;
			set_parent_and_color(nullptr, BLACK, n);
			set_left(n, nullptr);
			set_right(n, nullptr);
			return nullptr;
		}
		for (;;) {
			auto cur_node = cur;
			int c = COMPARE(*n, *cur);
			if (c < 0) {
				if (left_of(cur)) {
					cur = left_of(cur);
				} else {
					set_left(cur, n);
					set_parent_and_color(cur, RED, n);
					break;
				}
			} else if (c > 0) {
				if (right_of(cur)) {
					cur = right_of(cur);
				} else {
					set_right(cur, n);
					set_parent_and_color(cur, RED, n);
					break;
				}
			} else {
				replace(cur, n);
				return cur;
			}
		}
		set_left(n, nullptr);
		set_right(n, nullptr);
		inserted_balance(n);
		return nullptr;
	}
};


#endif  // RBTREE_HH_
