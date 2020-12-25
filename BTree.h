#ifndef __SKYLINEONRDF__BTREE__
#define __SKYLINEONRDF__BTREE__

namespace Skyline {
	template<class K, int M = 3>
	struct BTreeNode
	{
		K _keys[M];					// 关键字数组
		BTreeNode<K, M>* _subs[M + 1];	// 孩子数组
		size_t _size;				// 关键字的个数

		BTreeNode<K, M>* _parent;	// 父亲

		BTreeNode()
			:_size(0)
			, _parent(NULL)
		{
			for (size_t i = 0; i < M + 1; ++i)
			{
				_subs[i] = NULL;
			}
		}
	};

	template<class K, class V>
	struct Pair
	{
		K _first;
		V _second;

		Pair(const K& k = K(), const V& v = V())
			:_first(k)
			, _second(v)
		{}
	};

	template<class K, int M = 3>
	class BTree
	{
		typedef BTreeNode<K, M> Node;
	public:
		BTree()
			:_root(NULL)
		{}

		Pair<Node*, int> Find(const K& key)
		{
			Node* parent = NULL;
			Node* cur = _root;
			while (cur)
			{
				int i = 0;
				while (i < cur->_size && cur->_keys[i] < key)
				{
					++i;
				}

				if (cur->_keys[i] == key)
				{
					return Pair<Node*, int>(cur, i);
				}

				parent = cur;
				cur = cur->_subs[i];
			}

			return Pair<Node*, int>(parent, -1);
		}

		bool Insert(const K& key)
		{
			if (_root == NULL)
			{
				_root = new Node;
				_root->_keys[0] = key;
				++_root->_size;

				return true;
			}

			Pair<Node*, int> ret = Find(key);
			if (ret._second != -1)
			{
				return false;
			}

			K k = key;
			Node* cur = ret._first;
			Node* sub = NULL;

			// 在cur节点插入一个k
			while (1)
			{
				_InsertKey(cur, k, sub);
				if (cur->_size < M)
				{
					return true;
				}

				// 分裂
				int boundary = M / 2;

				Node* tmp = new Node;
				size_t index = 0;
				size_t size = cur->_size;

				// 拷贝key
				for (int i = boundary + 1; i < size; ++i)
				{
					tmp->_keys[index++] = cur->_keys[i];
					tmp->_size++;
					cur->_size--;
				}
				// 拷贝子节点
				index = 0;
				for (int i = boundary + 1; i <= size; ++i)
				{
					tmp->_subs[index] = cur->_subs[i];
					if (tmp->_subs[index])
						tmp->_subs[index]->_parent = tmp;

					++index;
				}

				k = cur->_keys[boundary];
				cur->_size--;

				// 没有父亲
				if (cur->_parent == NULL)
				{
					_root = new Node;
					_root->_keys[0] = k;
					_root->_subs[0] = cur;
					_root->_subs[1] = tmp;
					_root->_size = 1;

					tmp->_parent = _root;
					cur->_parent = _root;

					return true;
				}

				cur = cur->_parent;
				sub = tmp;
			}
		}

		void _InsertKey(Node* cur, const K& k, Node* sub)
		{
			int i = cur->_size - 1;
			while (i >= 0)
			{
				if (cur->_keys[i] > k)
				{
					cur->_keys[i + 1] = cur->_keys[i];
					cur->_subs[i + 2] = cur->_subs[i + 1];

					--i;
				}
				else
				{
					break;
				}
			}

			cur->_keys[i + 1] = k;
			cur->_subs[i + 2] = sub;
			if (sub)
			{
				sub->_parent = cur;
			}

			cur->_size++;
		}

		void InOrder()
		{
			_InOrder(_root);
			cout << endl;
		}

		void _InOrder(Node* root)
		{
			if (root == NULL)
			{
				return;
			}

			for (size_t i = 0; i < root->_size; ++i)
			{
				_InOrder(root->_subs[i]);
				cout << root->_keys[i] << " ";
			}

			_InOrder(root->_subs[root->_size]);
		}

	protected:
		Node* _root;
	}
}

#endif