
#include "rbtree.hh"


rbtree::COLOR rbtree::color_of(node* n)
{
	return static_cast<COLOR>(n->parent_and_color & COLOR_MASK);
}

void rbtree::set_color(COLOR c, node* n)
{
	uptr pc = n->parent_and_color;
	n->parent_and_color = (pc & ~COLOR_MASK) | c;
}

bool rbtree::is_red(node* n)
{
	return (n->parent_and_color & COLOR_MASK) == RED;
}

bool rbtree::is_black(node* n)
{
	return (n->parent_and_color & COLOR_MASK) == BLACK;
}

rbtree::node* rbtree::parent_of_red(node* n)
{
	return reinterpret_cast<node*>(n->parent_and_color);
}

rbtree::node* rbtree::parent_of(node* n)
{
	uptr p = n->parent_and_color & ~COLOR_MASK;

	return reinterpret_cast<node*>(p);
}

rbtree::node* rbtree::left_of(node* n)
{
	return n->left;
}

rbtree::node* rbtree::right_of(node* n)
{
	return n->right;
}

void rbtree::set_parent_and_color(node* parent, COLOR c, node* n)
{
	uptr p = reinterpret_cast<uptr>(parent);

	n->parent_and_color = p | c;
}

void rbtree::set_parent(node* parent, node* n)
{
	uptr p = reinterpret_cast<uptr>(parent);
	uptr pc = n->parent_and_color;

	n->parent_and_color = p | (pc & COLOR_MASK);
}

void rbtree::set_left(node* parent, node* left)
{
	parent->left = left;
}

void rbtree::set_right(node* parent, node* right)
{
	parent->right = right;
}

void rbtree::change_child(node* parent, node* old_child, node* new_child)
{
	if (parent) {
		if (parent->left == old_child)
			parent->left = new_child;
		else
			parent->right = new_child;
	} else {
		root = new_child;
	}
}

/// Left rotate at n.
/// @return  This function returns n->right which was replaced instead of n.
rbtree::node* rbtree::rotate_left(node* n)
{
	auto right = n->right;
	auto rightleft = right->left;
	right->left = n;
	set_parent(right, n);
	n->right = rightleft;
	if (rightleft)
		set_parent(n, rightleft);
	return right;
}

/// Right rotate at n.
/// @return  This function returns n->left which was replaced instead of n.
rbtree::node* rbtree::rotate_right(node* n)
{
	auto left = n->left;
	auto leftright = left->right;
	left->right = n;
	set_parent(left, n);
	n->left = leftright;
	if (leftright)
		set_parent(n, leftright);
	return left;
}

/// @param red  Inserted node which color is red.
void rbtree::inserted_balance(node* red)
{
	node* parent = parent_of_red(red);
	for (;;) {
		if (!parent) {
			set_color(BLACK, red);
			break;
		} else if (is_black(parent)) {
			break;
		}

		node* grand = parent_of_red(parent);
		if (parent != grand->right) {
			node* g_right = grand->right;
			if (g_right && is_red(g_right)) {
				set_parent_and_color(grand, BLACK, parent);
				set_parent_and_color(grand, BLACK, g_right);
				parent = parent_of(grand);
				set_parent_and_color(parent, RED, grand);
				red = grand;
				continue;
			}
			if (red == parent->right) {
				parent = rotate_left(parent);
				set_parent_and_color(grand, RED, parent);
				grand->left = parent;
			}
			uptr g_pc = grand->parent_and_color;
			set_color(RED, grand);
			node* parent_of_grand = parent_of_red(grand);
			node* grand2 = rotate_right(grand);
			change_child(parent_of_grand, grand, parent);
			grand2->parent_and_color = g_pc;
			break;
		} else {
			node* g_left = grand->left;
			if (g_left && is_red(g_left)) {
				set_parent_and_color(grand, BLACK, parent);
				set_parent_and_color(grand, BLACK, g_left);
				parent = parent_of(grand);
				set_parent_and_color(parent, RED, grand);
				red = grand;
				continue;
			}
			if (red == parent->left) {
				parent = rotate_right(parent);
				set_parent_and_color(grand, RED, parent);
				grand->right = parent;
			}
			uptr g_pc = grand->parent_and_color;
			set_color(RED, grand);
			node* parent_of_grand = parent_of_red(grand);
			node* grand2 = rotate_left(grand);
			change_child(parent_of_grand, grand, parent);
			grand2->parent_and_color = g_pc;
			break;
		}
	}
}

void rbtree::remove(node* n)
{
	node* left = n->left;
	node* right = n->right;
	if (!left) {
		node* parent = parent_of(n);
		change_child(parent, n, right);
		if (right) {
			right->parent_and_color = n->parent_and_color;
		} else if (is_black(n) && parent) {
			removed_balance(parent);
		}
	} else if (!right) {
		left->parent_and_color = n->parent_and_color;
		change_child(parent_of(n), n, left);
	} else {
		auto mr = get_most_right(left);
		auto mr_parent = parent_of(mr);
		auto mr_left = mr->left;
		node* parent;
		if (mr != left) {
			mr_parent->right = mr_left;
			mr->left = left;
			set_parent(mr, left);
			parent = mr_parent;
		} else {
			parent = mr;
		}
		COLOR mr_c = color_of(mr);
		mr->right = right;
		set_parent(mr, right);
		change_child(parent_of(n), n, mr);
		mr->parent_and_color = n->parent_and_color;
		if (mr_left) {
			set_parent_and_color(parent, BLACK, mr_left);
		} else if (mr_c == COLOR::BLACK) {
			removed_balance(parent);
		}
	}
}

void rbtree::removed_balance(node* parent)
{
	node* changed = nullptr;
	for (;;) {
		node* cur = parent->right;
		if (changed != cur) {
			if (is_red(cur)) {
				auto grand = parent_of(parent);
				auto parent2 = rotate_left(parent);
				set_color(RED, parent);
				change_child(grand, parent, parent2);
				set_parent_and_color(grand, BLACK, parent2);
				cur = parent->right;
			}
			auto cur_right = cur->right;
			if (!cur_right || is_black(cur_right)) {
				auto cur_left = cur->left;
				if (!cur_left || is_black(cur_left)) {
					set_color(RED, cur);
					if (is_red(parent)) {
						set_color(BLACK, parent);
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
				set_parent(parent, cur2);
				cur = cur2;
			}
			uptr pc_of_parent = parent->parent_and_color;
			auto grand = parent_of(parent);
			auto parent2 = rotate_left(parent);
			set_color(BLACK, parent);
			change_child(grand, parent, parent2);
			parent2->parent_and_color = pc_of_parent;
			set_parent_and_color(parent2, BLACK, parent2->right);
			break;
		} else {
			cur = parent->left;
			if (is_red(cur)) {
				auto grand = parent_of(parent);
				auto parent2 = rotate_right(parent);
				set_color(RED, parent);
				change_child(grand, parent, parent2);
				set_parent_and_color(grand, BLACK, parent2);
				cur = parent->left;
			}
			auto cur_left = cur->left;
			if (!cur_left || is_black(cur_left)) {
				auto cur_right = cur->right;
				if (!cur_right || is_black(cur_right)) {
					set_color(RED, cur);
					if (is_red(parent)) {
						set_color(BLACK, parent);
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
				set_parent(parent, cur2);
				cur = cur2;
			}
			uptr pc_of_parent = parent->parent_and_color;
			auto grand = parent_of(parent);
			auto parent2 = rotate_right(parent);
			set_color(BLACK, parent);
			change_child(grand, parent, parent2);
			parent2->parent_and_color = pc_of_parent;
			set_parent_and_color(parent2, BLACK, parent2->left);
			break;
		}
	}
}

rbtree::node* rbtree::get_most_right(node* n)
{
	for (;;) {
		auto tmp = n->right;
		if (!tmp)
			return n;
		n = tmp;
	}
}

void rbtree::replace(node* old_node, node* new_node)
{
	new_node->parent_and_color = old_node->parent_and_color;
	node* tmp = parent_of(old_node);
	if (tmp)
		change_child(tmp, old_node, new_node);

	new_node->left = old_node->left;
	if (new_node->left)
		set_parent(new_node->left, new_node);

	new_node->right = old_node->right;
	if (new_node->right)
		set_parent(new_node->right, new_node);
}

