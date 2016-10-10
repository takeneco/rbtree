
#include "rbtree.hh"

#define _parent parent

rbtree::COLOR rbtree::get_color(node* n)
{
	return n->color;
}

bool rbtree::is_red(node* n)
{
	return n->color == RED;
}

bool rbtree::is_black(node* n)
{
	return n->color == BLACK;
}

rbtree::node* rbtree::parent_of_red(node* n)
{
	return n->_parent;
}

rbtree::node* rbtree::parent_of(node* n)
{
	return n->_parent;
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
	n->_parent = parent;
	n->color = c;
}

void rbtree::set_parent(node* n, node* parent)
{
	n->_parent;
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

rbtree::node* rbtree::rotate_left(node* n)
{
	auto right = n->right;
	auto rightleft = right->left;
	right->left = n;
	n->parent = right;
	n->right = rightleft;
	if (rightleft)
		rightleft->parent = n;
	return right;
}

rbtree::node* rbtree::rotate_right(node* n)
{
	auto left = n->left;
	auto leftright = left->right;
	left->right = n;
	n->parent = left;
	n->left = leftright;
	if (leftright)
		leftright->parent = n;
	return left;
}

rbtree::node* rbtree::rotate_leftright(node* n)
{
	auto tmp = rotate_left(n->left);
	n->left = tmp;
	tmp->parent = n;
	return rotate_right(n);
}

rbtree::node* rbtree::rotate_rightleft(node* n)
{
	auto tmp = rotate_right(n->right);
	n->right = tmp;
	tmp->parent = n;
	return rotate_left(n);
}

/// @param red  Inserted node which color is red.
void rbtree::inserted_balance(node* red)
{
	for (;;) {
		node* parent = parent_of_red(red);
		if (!parent) {
			red->color = BLACK;
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
				grand->color = RED;
				red = grand;
				continue;
			}
			node* p_right = parent->right;
			if (red == p_right) {
				parent = rotate_left(parent);
				parent->parent = grand;
				grand->left = parent;
			}
			parent->color = grand->color; // BLACK
			grand->color = RED;
			node* parent_of_grand = grand->parent;
			node* grand2 = rotate_right(grand);
			change_child(parent_of_grand, grand, parent);
			grand2->parent = parent_of_grand;
			break;
		} else {
			node* g_left = grand->left;
			if (g_left && is_red(g_left)) {
				set_parent_and_color(grand, BLACK, parent);
				set_parent_and_color(grand, BLACK, g_left);
				grand->color = RED;
				red = grand;
				continue;
			}
			node* p_left = parent->left;
			if (red == p_left) {
				parent = rotate_right(parent);
				parent->parent = grand;
				grand->right = parent;
			}
			parent->color = grand->color; // BLACK
			grand->color = RED;
			node* parent_of_grand = grand->parent;
			node* grand2 = rotate_left(grand);
			change_child(parent_of_grand, grand, parent);
			grand2->parent = parent_of_grand;
			break;
		}
	}
}

void rbtree::remove(node* val)
{
	node* val_left = val->left;
	node* val_right = val->right;
	if (!val_left) {
		node* val_parent = val->parent;
		node* val_right = val->right;
		change_child(val_parent, val, val_right);
		if (val_right) {
			val_right->parent = val_parent;
			val_right->color = val->color;
		}
		//if (!val_parent) {
		//	if (root)
		//		set_color(root, COLOR::BLACK); // need?
		//}
		else if (val->color == BLACK && val_parent) {
			removed_balance(val_parent);
		}
	} else if (!val_right) {
		node* val_parent = val->parent;
		val_left->parent = val_parent;
		val_left->color = val->color;
		change_child(val_parent, val, val_left);
	} else {
		auto mr = get_most_right(val_left);
		auto mr_parent = mr->parent;
		auto mr_left = mr->left;
		node* parent;
		if (mr != val_left) {
			mr_parent->right = mr_left;
			mr->left = val_left;
			val_left->parent = mr;
			parent = mr_parent;
		} else {
			parent = mr;
		}
		COLOR mr_c = mr->color;
		mr->right = val_right;
		val_right->parent = mr;
		change_child(val->parent, val, mr);
		mr->parent = val->parent;
		mr->color = val->color;
		if (mr_left) {
			mr_left->parent = parent;
			mr_left->color = BLACK;
		} else {
			if (mr_c == COLOR::BLACK)
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
			if (cur->color == RED) {
				auto parent_parent = parent->parent;
				auto parent2 = rotate_left(parent);
				change_child(parent_parent, parent, parent2);
				parent2->parent = parent_parent;
				parent2->color = BLACK;
				parent->color = RED;
				cur = parent->right;
			}
			auto cur_right = cur->right;
			if (!cur_right || cur_right->color == BLACK) {
				auto cur_left = cur->left;
				if (!cur_left || cur_left->color == BLACK) {
					cur->color = RED;
					if (parent->color == RED) {
						parent->color = BLACK;
					} else {
						changed = parent;
						parent = parent->parent;
						if (parent)
							continue;
					}
					break;
				}
				auto cur2 = rotate_right(cur);
				change_child(parent, cur, cur2);
				cur2->parent = parent;
				cur = cur2;
			}
			auto parent_parent = parent->parent;
			auto parent2 = rotate_left(parent);
			change_child(parent_parent, parent, parent2);
			parent2->parent = parent_parent;
			parent2->right->color = BLACK;
			parent2->color = parent->color;
			parent->color = BLACK;
			break;
		} else {
			cur = parent->left;
			if (cur->color == RED) {
				auto parent_parent = parent->parent;
				auto parent2 = rotate_right(parent);
				change_child(parent_parent, parent, parent2);
				parent2->parent = parent_parent;
				parent2->color = BLACK;
				parent->color = RED;
				cur = parent->left;
			}
			auto cur_left = cur->left;
			if (!cur_left || cur_left->color == BLACK) {
				auto cur_right = cur->right;
				if (!cur_right || cur_right->color == BLACK) {
					cur->color = RED;
					if (parent->color == RED) {
						parent->color = BLACK;
					} else {
						changed = parent;
						parent = parent->parent;
						if (parent)
							continue;
					}
					break;
				}
				auto cur2 = rotate_left(cur);
				change_child(parent, cur, cur2);
				cur2->parent = parent;
				cur = cur2;
			}
			auto parent_parent_and_color = parent->_parent;
			auto parent_color = parent->color;
			auto grand = parent_of(parent);
			auto parent2 = rotate_right(parent);
			change_child(grand, parent, parent2);
			parent2->_parent = parent_parent_and_color;
			parent2->color = parent_color;
			set_parent_and_color(
			    parent2, BLACK, parent2->left);
			parent->color = BLACK;
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
	new_node->_parent = old_node->_parent;
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

