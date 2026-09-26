#pragma once

#include <cmath>
#include <memory>
#include <cassert>
#include <print>
#include <vector>

namespace btree
{
    template<typename T_Arr>
    void shiftRight(T_Arr* arr, std::size_t i, std::size_t& size)
    {
        for (std::size_t idx = size; idx > i; --idx) {
            arr[idx] = std::move(arr[idx - 1]);
        }
        size++;
    }

    template<typename T_Arr>
    void shiftLeft(T_Arr* arr, std::size_t i, std::size_t& size)
    {
        if (i >= size || size == 0) return;
        for (std::size_t idx = i; idx < size - 1; ++idx) {
            arr[idx] = std::move(arr[idx + 1]);
        }
        arr[size - 1] = {};    
        size--;
    }
    
    template<typename T, std::size_t M>
    class BTree;
    
    template<typename T, std::size_t M> // M is tree degree
    class Node
    {
	friend class BTree<T, M>;
    public:
        std::unique_ptr<T>         data[M - 1];
        std::unique_ptr<Node<T,M>> children[M];
        std::size_t                keys_stored;
	std::size_t                min_keys;
	std::size_t                divisor;
        bool                       is_leaf;
    public:
        Node(bool leaf = true)
            : keys_stored{ 0 }
            , min_keys{ static_cast<std::size_t>(std::ceil(M / 2.0)) - 1 }
	    , divisor{ (M - 1) / 2 }
            , is_leaf{ leaf }
        {}

	T* getRightMostData() const
	{
	    const Node* cur = this;
    
	    while (!cur->is_leaf) {
		cur = cur->children[cur->keys_stored].get();
	    }
	    assert(cur->keys_stored > 0);
	    const auto& last_data = cur->data[cur->keys_stored - 1];
	    return last_data.get();
	}

	
	template<typename ItemViewer>
        void traverse(ItemViewer&& viewer)
        {
            std::size_t i = 0;
            for (i = 0; i < keys_stored; ++i) {
                if (!is_leaf) {
                    children[i]->traverse(viewer);
                }
                viewer(*(data[i].get()));
            }

            if (!is_leaf) {
                children[i]->traverse(viewer);
	    }
	}
	
	int erase(std::size_t k)
        {
            bool found = false;
            std::size_t i;
            for (i = 0; i < keys_stored; ++i) {
                if (k == data[i]->key) {
                    found = true;
                    break;
                }
                if (k < data[i]->key) {
                    break;
                }
            }

            if (found) {
                if (is_leaf) {
                    data[i] = {};
                    shiftLeft(data, i, keys_stored);
                    return keys_stored >= min_keys ? 0 : 1;
                }

                // Case 2: Key found in an internal node
                
                // 1. Try borrowing from left subtree
                if (children[i]->keys_stored > children[i]->min_keys) {
                    const auto pred = getPredecessor(i);
                    children[i]->erase(pred.key);
                    *data[i] = pred;
                    return keys_stored >= min_keys ? 0 : 1;
                }

                // 2. Try borrowing from right subtree
                if (children[i + 1]->keys_stored > children[i + 1]->min_keys) {
                    const auto succ = getSuccessor(i);
                    children[i + 1]->erase(succ.key);
                    *data[i] = succ;
                    return keys_stored >= min_keys ? 0 : 1;
                }

                // 3. Swap with in-order successor and recurse into right child
                const auto succ = getSuccessor(i);
                *data[i] = succ;
                int res = children[i + 1]->erase(succ.key);

                if (res == 1) {
                    if (children[i]->keys_stored > children[i]->min_keys) {
                        borrowLeft(i + 1);
                    } else {
                        mergeChildren(i);
                    }
                }

                return keys_stored >= min_keys ? 0 : 1;

            } else {
                if (is_leaf) {
                    return -1;
                }

                int res = children[i]->erase(k);
                if (res < 1) {
                    return res;
                }

                bool has_left = (i > 0);
                bool has_right = (i < keys_stored);

                if (has_left && children[i - 1]->keys_stored > children[i - 1]->min_keys) {
                    borrowLeft(i);
                    return 0;
                }

                if (has_right && children[i + 1]->keys_stored > children[i + 1]->min_keys) {
                    borrowRight(i);
                    return 0;
                }

                if (has_left) {
                    mergeChildren(i - 1);
                } else if (has_right) {
                    mergeChildren(i);
                }
                
                return keys_stored >= min_keys ? 0 : 1;
            }
        }
	
        void insertNonFull(std::unique_ptr<T> t)
        {
	    std::size_t i = 0;
	    if (is_leaf) {
		while (i < keys_stored && t->key > data[i]->key) i++;
		shiftRight(data, i, keys_stored);
		data[i] = std::move(t);
	    } else {
		while (i < keys_stored && t->key > data[i]->key) i++;
		if (children[i]->keys_stored == M - 1) {
		    splitChild(i);
		    if (t->key > data[i]->key) i++;
		}
		children[i]->insertNonFull(std::move(t));
	    }
        }
	
        T* search(std::size_t k)
        {   
	    std::size_t i = 0;
            for (; i < keys_stored && k >= data[i]->key; ++i) {
                // check if cursors offset falls into data[i] piece
                if (data[i]->key + data[i]->piece.length > k) {
		    // returns accumulated bytes count from beginnig of the file
		    // and piece structure itself
                    return data[i].get();
                }
            }
                
            return is_leaf ? nullptr : children[i]->search(k);
        }
    private:
	void shiftKeysAbove(std::size_t threshold, std::size_t delta, bool is_positive)
	{
	    std::size_t i;
	    for (i = 0; i < keys_stored; ++i) {
                if (data[i]->key > threshold) {
		    if (!is_leaf) {
			children[i]->shiftKeysAbove(threshold, delta, is_positive);
		    }

		    if (is_positive) {
			data[i]->key += delta;
		    } else {
			data[i]->key -= delta;
		    }
                }
		
            }
	    
            if (!is_leaf) {
                children[i]->shiftKeysAbove(threshold, delta, is_positive);
	    }
	}
	
	void splitChild(std::size_t i)
	{
	    auto y = std::move(children[i]);                   // steal child node from this node
	    auto z = std::make_unique<Node<T, M>>(y->is_leaf); // create new child node

	    z->keys_stored = (M - 1) - divisor - 1;

	    // move data from y to z using divisor
	    for (std::size_t j = 0; j < z->keys_stored; ++j) {
		z->data[j] = std::move(y->data[j + divisor + 1]);
	    }

	    // copy children from y to z if z is not leaf
	    if (!y->is_leaf) {
		for (std::size_t j = 0; j <= z->keys_stored; ++j) {
		    z->children[j] = std::move(y->children[j + divisor + 1]);
		}
	    }
	    y->keys_stored = divisor;

	    std::size_t child_count = keys_stored + 1;
	    shiftRight(children, i + 1, child_count);
	    
	    children[i + 1] = std::move(z);
	    
	    shiftRight(data, i, keys_stored);
	    data[i] = std::move(y->data[divisor]);
	    children[i] = std::move(y);
	}

	T getPredecessor(std::size_t i)
        {
            Node<T, M>* cur = children[i].get();
            while (!cur->is_leaf) {
                cur = cur->children[cur->keys_stored].get();
            }
            return *cur->data[cur->keys_stored - 1].get();
        }
    
        T getSuccessor(std::size_t i)
        {
            Node<T, M>* cur = children[i + 1].get();
            while (!cur->is_leaf) {
                cur = cur->children[0].get();
            }
            return *cur->data[0].get();
        }
    
        void mergeChildren(std::size_t i)
        {
            auto& y = children[i];
            auto& z = children[i + 1];

            std::size_t orig_y_keys = y->keys_stored;

            y->data[y->keys_stored] = std::move(data[i]);
            y->keys_stored++;

            for (std::size_t j = 0; j < z->keys_stored; ++j) {
                y->data[y->keys_stored] = std::move(z->data[j]);
                y->keys_stored++;
            }

            if (!y->is_leaf) {
                for (std::size_t j = 0; j <= z->keys_stored; ++j) {
                    y->children[orig_y_keys + 1 + j] = std::move(z->children[j]);
                }
            }

            shiftLeft(data, i, keys_stored);
            
            std::size_t child_count = keys_stored + 2;
            shiftLeft(children, i + 1, child_count);
        }
    
        void borrowLeft(std::size_t i)
        {
            auto& child = children[i];
            auto& sibling = children[i - 1];

            if (!child->is_leaf) {
                std::size_t num_children = child->keys_stored + 1;
                shiftRight(child->children, 0, num_children);
                child->children[0] = std::move(sibling->children[sibling->keys_stored]);
            }

            shiftRight(child->data, 0, child->keys_stored);
            child->data[0] = std::move(data[i - 1]);

            data[i - 1] = std::move(sibling->data[sibling->keys_stored - 1]);
            sibling->data[sibling->keys_stored - 1] = {};
            sibling->keys_stored--;
        }
    
        void borrowRight(std::size_t i)
        {
            auto& child = children[i];
            auto& sibling = children[i + 1];

            child->data[child->keys_stored] = std::move(data[i]);
            child->keys_stored++;

            if (!child->is_leaf) {
                child->children[child->keys_stored] = std::move(sibling->children[0]);
                std::size_t num_children = sibling->keys_stored + 1;
                shiftLeft(sibling->children, 0, num_children);
            }

            data[i] = std::move(sibling->data[0]);
            shiftLeft(sibling->data, 0, sibling->keys_stored);
        }
    };

    template<typename T, std::size_t M>
    class BTree
    {
    private:
        std::unique_ptr<Node<T, M>> root;
    public:
        BTree()
	    : root( std::make_unique<Node<T,M>>() )
	{}

	bool empty() const
	{
	    return root->keys_stored == 0;
	}

	T* getRightMostPiece() const
	{
	    return root->getRightMostData();
	}

	template<typename ItemViewer>
        void traverse(ItemViewer&& viewer)
        {
            if (root) {
                root->traverse(std::forward<ItemViewer>(viewer));
            }
        }

	bool erase(std::size_t k)
	{
	    if (!root) return false;

	    int res = root->erase(k);

	    if (res < 0) return false;
	    if (root->keys_stored == 0) {
		if (!root->is_leaf) {
		    root = std::move(root->children[0]);
		}
	    }
	    return true;
	}
	
        void insert(std::unique_ptr<T> elem)
	{
	    if (root->keys_stored == M - 1) {
		splitRoot();
		root->insertNonFull(std::move(elem));
	    } else {
		root->insertNonFull(std::move(elem));
	    }
	}
	
        void insert(std::vector<std::unique_ptr<T>> elems)
	{
	    for (auto& e : elems) {
		insert(std::move(e));
	    }
	}

	void incrementKeysAbove(std::size_t threshold, std::size_t delta)
	{
	    root->shiftKeysAbove(threshold, delta, true);
	}

	void decrementKeysAbove(std::size_t threshold, std::size_t delta)
	{
	    root->shiftKeysAbove(threshold, delta, false);
	}
	
	T* findDataAt(std::size_t k)
        {
            return root->search(k);
        }

	std::vector<const T*> findDataInRange(std::size_t left_bound, std::size_t right_bound)
	{
	    std::vector<const T*> overlapped_pieces{};
	    std::size_t start = left_bound;

	    // for big diff between left_bound and right_bound
	    // this operation can be pretty expensive
	    while (start < right_bound) {
		const auto* p = findDataAt(start);
		overlapped_pieces.push_back(p);
		start = std::min(start, p->key) + p->piece.length;
	    }
	    return overlapped_pieces;
	}

	const std::unique_ptr<Node<T,M>>& getRoot() const
	{
	    return root;
	}
	
    private:
	void splitRoot()
        {
            auto new_root = std::make_unique<Node<T,M>>(false);
            new_root->children[0] = std::move(root);
            root = std::move(new_root);
            root->splitChild(0);
        }
    };
}
