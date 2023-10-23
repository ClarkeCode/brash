#pragma once
#include <cstdint>
#include <vector>
#include <iostream>

// template <typename T>
// struct Node;
template <typename T>
struct Node;

template <typename T>
Node<T>* makeNode(T value);

template <typename T>
struct Node {
	using ptr_t = Node<T>*;
	static constexpr ptr_t NoLink = nullptr;
	friend ptr_t makeNode<T>(T value);

	T value;

	ptr_t parent;
	ptr_t left;
	ptr_t right;

	ptr_t getRoot() {
		ptr_t currentNode = this;
		while (currentNode->parent != NoLink) {
			currentNode = currentNode->parent;
		}
		return currentNode;
	}

	//Create a new node with the provided value as the left child
	ptr_t hangLeft(T value) {
		ptr_t newNode = makeNode(value);
		newNode->parent = this;
		left = newNode;
		return newNode;
	}

	//Create a new node with the provided value as the right child
	ptr_t hangRight(T value) {
		ptr_t newNode = makeNode(value);
		newNode->parent = this;
		right = newNode;
		return newNode;
	}

	//Create a new node with the provided value
	//If the current node has a parent, the new node becomes the child of the parent node, taking the place formerly occupied by the current node
	//Current node is made into the left child of the new node
	ptr_t supplantLeft(T value) {
		ptr_t newNode = makeNode(value);
		if (parent) {
			newNode->parent = parent;
			if (parent->left == this) {
				parent->left = newNode;
			}
			if (parent->right == this) {
				parent->right = newNode;
			}
		}

		parent = newNode;
		newNode->left = this;
		return newNode;
	}

	//Create a new node with the provided value
	//If the current node has a parent, the new node becomes the child of the parent node, taking the place formerly occupied by the current node
	//Current node is made into the right child of the new node
	ptr_t supplantRight(T value) {
		ptr_t newNode = makeNode(value);
		if (parent) {
			newNode->parent = parent;
			if (parent->left == this) {
				parent->left = newNode;
			}
			if (parent->right == this) {
				parent->right = newNode;
			}
		}

		parent = newNode;
		newNode->right = this;
		return newNode;
	}

	//Produce Graphviz output to the provided stream representing this node and it's children
	void toGraphviz(std::ostream& os) {
		using namespace std;
		os << "digraph G {" << endl;
		_graphvizHelper(os);
		os << "}" << endl;
	}
	void _graphvizHelper(std::ostream& os) {
		using namespace std;
		os << "\t\"" << value << '"' << endl;
		if (left != NoLink) {
			os << "\t\"" << value << "\" -> \"" << left->value << "\" [label=L]" << endl;
			left->_graphvizHelper(os);
		}
		if (right != NoLink) {
			os << "\t\"" << value << "\" -> \"" << right->value << "\" [label=R]" << endl;
			right->_graphvizHelper(os);
		}
	}


	//Deletes the current node, deletion cascades down it's children
	void deleteNodes() {
		delete this;
	}

	~Node() {
		if (left) delete left;
		if (right) delete right;
	}

	private:
	Node(T val) : value(val), parent(NoLink), left(NoLink), right(NoLink) {}
};

template <typename T>
Node<T>* makeNode(T value) {
	return new Node<T>(value);
};