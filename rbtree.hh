// ビットフィールドは順番が処理系依存なので使えない

#ifndef RBTREE_HH_
#define RBTREE_HH_


class rbtree
{
	using uptr = unsigned long;

	enum COLOR {
		RED = 0,
		BLACK = 1,
		COLOR_MASK = 0x1,
	};

public:
	class node
	{
		friend class rbtree;
	public:
		node() {}

	private:
		uptr parent_and_color;
		node* left;
		node* right;
	};

public:
	rbtree() :
		root(nullptr)
	{}
	void remove(node* val);

protected:
	static COLOR color_of(node* n);
	static void set_color(COLOR c, node* n);
	static bool is_red(node* n);
	static bool is_black(node* n);
	static node* parent_of_red(node* n);
	static node* parent_of(node* n);
	static node* left_of(node* n);
	static node* right_of(node* n);
	static void set_parent_and_color(node* parent, COLOR c, node* n);
	static void set_parent(node* parent, node* n);
	static void set_left(node* parent, node* left);
	static void set_right(node* parent, node* right);
	void change_child(node* parent, node* old_child, node* new_child);
	static node* rotate_left(node* n);
	static node* rotate_right(node* n);
	void inserted_balance(node* parent);
	void removed_balance(node* parent);
	static node* get_most_right(node* val);
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

