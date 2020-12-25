#include "Skyline.h"
#include <algorithm>
#include <iomanip>

namespace SkylineOnRDF
{
	bool cmp(const std::pair<int, int> a, const std::pair<int, int> b) {
		return a.second < b.second;//自定义的比较函数
	}

	void Skyline::init(const std::vector<RDFNode>& nodes, const std::unordered_map<int, std::string>& id_to_name)
	{
		rdf_ = nodes;
		id_to_name_ = id_to_name;
	}

	void Skyline::addNext(const int index, const std::set<int> &nexts)
	{
		if (nexts.empty())
			return;
		rdf_[index].next = nexts;
	}

	void Skyline::addPrior(const int index, const std::set<int> &nexts)
	{
		for (auto next : nexts)
			rdf_[next].prior.insert(index);
	}

	void Skyline::setKeywords(const std::vector<std::string> &keywords)
	{
		keywords_ = keywords;

		//do initial
		sp_keyword_lists_.resize(keywords_.size());

		distance_matrix_.resize(rdf_.size());
		for (int i = 0; i < rdf_.size(); ++i)
			distance_matrix_[i].resize(keywords_.size(), inf);
	}

	void Skyline::setKeywords(const int num)
	{

		std::set<std::string> keywords;
		srand(time(NULL));
		while (keywords.size() < num)
		{
			int index = rand() % rdf_.size();
			int which = rand() % rdf_[index].info.size();
			keywords.insert(rdf_[index].info[which]);
		}

		keywords_ = std::vector<std::string>(keywords.begin(), keywords.end());
		for (int i = 0; i < num; ++i)
			std::cout << keywords_[i] << ",";
		std::cout << std::endl;
		//do initial
		sp_keyword_lists_.resize(keywords_.size());

		distance_matrix_.resize(rdf_.size());
		for (int i = 0; i < rdf_.size(); ++i)
			distance_matrix_[i].resize(keywords_.size(), inf);
	}
	
	void Skyline::buildKeywordMap(bool acquire_depth)
	{
		if (!acquire_depth)//不需要深度排序
		{
			std::unordered_map<std::string, std::vector<int>> mini_keyword_map;
			for (int i = 0; i < rdf_.size(); ++i)
			{
				for (auto info : rdf_[i].info)
				{
					auto iter = mini_keyword_map.find(info);
					if (iter != mini_keyword_map.end())
						iter->second.push_back(i);
					else
						mini_keyword_map.insert({ info,{i} });
				}
			}
			mini_keyword_HashMap_ = mini_keyword_map;
			return;
		}
		std::unordered_map<std::string, std::vector<std::pair<int, int>>> keyword_map;
		//找到所有没有入度的结点，作为BFS的起点
		std::vector<int> Heads;
		for (int i = 0; i < rdf_.size(); ++i)
		{
			if (rdf_[i].prior.empty())
				Heads.push_back(i);
		}
		std::vector<bool> visited(rdf_.size(), false);//避免环
		for (int i = 0; i < rdf_.size(); ++i)
			Heads.push_back(i);
		int num = 0;
		for (int i : Heads)
		{
			if (visited[i])
				continue;
			std::queue<int> Q;
			Q.push(i);
			//存在大量指向同一结点的指针，加入就设置为true
			visited[i] = true;
			int level = 0;
			int level_num = 1;
			int next_level_num = 0;
			while (!Q.empty())
			{
				int pos = Q.front();
				Q.pop();
				name_max_size_ = std::max(name_max_size_, (int)id_to_name_.find(pos)->second.size());
				for (auto info : rdf_[pos].info)
				{
					auto iter = keyword_map.find(info);
					if (iter != keyword_map.end())
						iter->second.push_back({ pos,level });
					else
						keyword_map.insert({ info,{{pos,level}} });
				}
				std::set<int> nexts = rdf_[pos].next;
				for (auto next : nexts)
				{
					if (!visited[next])
					{
						Q.push(next);
						visited[next] = true;
						next_level_num++;
					}
				}
				level_num--;
				if (0 == level_num)
				{
					level_num = next_level_num;
					next_level_num = 0;
					level++;
				}
					
 			}
		}
		keyword_HashMap_ = keyword_map;
	}

	void Skyline::fastComputeDistanceMatrix()
	{
		for (int i = 0; i < rdf_.size(); ++i)
		{
			distance_matrix_[i].assign(keywords_.size(), inf);
			rdf_[i].visited = -1;
		}

		std::vector<std::vector<int>> result(keywords_.size());
		for (int i = 0; i < keywords_.size(); ++i)
		{
			auto iter = mini_keyword_HashMap_.find(keywords_[i]);
			if (iter != mini_keyword_HashMap_.end())
				result[i] = iter->second;
			else//存在错误关键字
			{
				std::cerr << "Oops!有个关键字不存在." << std::endl;
				result.clear();
				break;
			}
		}
		for (int i = 0; i < result.size(); ++i)
			reverseBFS(result[i], i);
	}

	void Skyline::computeDistanceMatrix()
	{
		for (int i = 0; i < rdf_.size(); ++i)
		{
			distance_matrix_[i].assign(keywords_.size(), inf);
			rdf_[i].visited = -1;
		}
		std::vector<std::vector<std::pair<int,int>>> result(keywords_.size());
		for (int i = 0; i < keywords_.size(); ++i)
		{
			auto iter = keyword_HashMap_.find(keywords_[i]);
			if (iter != keyword_HashMap_.end())
				result[i] = iter->second;
			else//存在错误关键字
			{
				std::cerr << "Oops!有个关键字不存在." << std::endl;
				result.clear();
				break;
			}
		}
		for (int i = 0; i < result.size(); ++i)
		{
			std::sort(result[i].begin(), result[i].end(), cmp);
			for (int j = 0; j < result[i].size(); ++j)
				reverseBFS(result[i][j].first, i);
		}	
	}

	void Skyline::reverseBFS(const std::vector<int> origins, const int which)
	{
		std::queue<std::pair<int, int>> Q;
		std::vector<bool> visited(rdf_.size(), false);//避免环
		//first id, scond parent
		for (auto origin : origins)
		{
			for (auto prior : rdf_[origin].prior)
				Q.push(std::pair<int, int>{prior, origin});
			distance_matrix_[origin][which] = 0;
			visited[origin] = true;
		}
		while (!Q.empty())
		{
			std::pair<int, int> pos = Q.front();
			visited[pos.first] = true;
			Q.pop();
			if (distance_matrix_[pos.first][which] <= distance_matrix_[pos.second][which] + 1)
				continue;//不再对该节点后续结点更新
			else
				distance_matrix_[pos.first][which] = std::min(distance_matrix_[pos.first][which], distance_matrix_[pos.second][which] + 1);
			std::set<int> priors = rdf_[pos.first].prior;
			for (auto prior : priors)
				if (!visited[prior])
					Q.push(std::pair<int, int>{prior, pos.first});
		}
	}


	void Skyline::reverseBFS(const int origin, const int which)
	{
		std::queue<std::pair<int, int>> Q;
		//first id, scond parent
		for (auto prior : rdf_[origin].prior)
			Q.push(std::pair<int, int>{prior, origin});
		int min;
		distance_matrix_[origin][which] = 0;
		std::vector<bool> visited(rdf_.size(), false);//避免环
		visited[origin] = true;
		while (!Q.empty())
		{
			std::pair<int, int> pos = Q.front();
			visited[pos.first] = true;
			Q.pop();
			min = std::min(distance_matrix_[pos.first][which], distance_matrix_[pos.second][which] + 1);
			if (distance_matrix_[pos.first][which] <= distance_matrix_[pos.second][which] + 1)
				continue;//不再对该节点后续结点更新
			else
				distance_matrix_[pos.first][which] = min;
			std::set<int> priors = rdf_[pos.first].prior;
			for (auto prior : priors)
				if(!visited[prior])
					Q.push(std::pair<int, int>{prior, pos.first});
		}
	}

	//选择出所有skyline point
	int Skyline::BNL()
	{
		if (distance_matrix_.empty())
			return 0;
		std::unordered_set<int> filter;
		std::vector<int> erase_set;
		int i = 0;
		for (; i < distance_matrix_.size(); ++i)//插入第一个sp
		{
			if (!rdf_[i].is_location)
				continue;
			bool bad_point = false;
			for (auto distance : distance_matrix_[i])
			{
				if (distance == inf)
				{
					bad_point = true;
					break;
				}
			}
			if (!bad_point)
			{
				filter.insert(i++);
				break;
			}
		}
		for (; i < distance_matrix_.size(); ++i)
		{
			if (!rdf_[i].is_location)
				continue;
			bool add = true;//是否加入
			bool end_this_round = false;
			int sum_out = 0;
			for (int k = 0; k < distance_matrix_[0].size(); ++k)
			{
				if (distance_matrix_[i][k] == inf)
				{
					//存在不可达关键字
					end_this_round = true;
					break;
				}
				sum_out += distance_matrix_[i][k];
			}
			if (end_this_round)
				continue;
			for (auto p : filter)
			{
				int sum_in = 0;
				for (int k = 0; k < distance_matrix_[0].size(); ++k)
					sum_in += distance_matrix_[p][k];
				if (sum_in > sum_out)
				{
					int dominated_num = 0;
					for (int k = 0; k < distance_matrix_[0].size(); ++k)
					{
						if (distance_matrix_[i][k] <= distance_matrix_[p][k])
							dominated_num++;
					}
					//被新的sp支配
					if (dominated_num == distance_matrix_[0].size())
						erase_set.push_back(p);
				}
				else if (sum_in < sum_out)
				{
					int dominated_num = 0;
					for (int k = 0; k < distance_matrix_[0].size(); ++k)
					{
						if (distance_matrix_[p][k] <= distance_matrix_[i][k])
							dominated_num++;
					}
					//被已有sp支配
					if (dominated_num == distance_matrix_[0].size())
					{
						add = false;
						break;//根据支配关系的传递性
					}
				}
				//== 不存在支配关系
			}
			for (auto candidate : erase_set)
				filter.erase(candidate);
			if (add)
				filter.insert(i);
		}
		sp_ = std::vector<int>(filter.begin(), filter.end());
		return sp_.size();
	}

	void Skyline::buildTree()
	{
		std::cout << "\n┏";

		int counter = 33 + name_max_size_ + 3 * keywords_.size() - 2 - 14 - 2;
		int left = counter / 2;
		int right = counter - left;
		while (left-- > 0)
			std::cout << "━";
		std::cout << "Skyline Search";
		while (right-- > 0)
			std::cout << "━";
		std::cout << "┓\n" << std::endl;
		trees_.clear();
		sp_keyword_lists_.clear();
		sp_keyword_lists_.resize(keywords_.size());
		std::stack<int> S;
		for (auto sp : sp_)//对所有sp
		{
			std::cout << "┣━━━━━━━━━For SP[" << std::setfill(' ') << std::setw(name_max_size_) << id_to_name_.find(sp)->second << "]━━━━━";
			for (int i = 0; i < keywords_.size(); i++)
			{
				//std::cout << std::setfill(' ') << std::setw(2) << distance_matrix_[sp][i];
				std::cout << distance_matrix_[sp][i];
				if (i != keywords_.size() - 1)
					std::cout << "━━";
			}
			std::cout << "━━━━━━━━━┫\n" << std::endl;
			for (int which = 0; which < keywords_.size(); ++which)
			{
				//DFS
				S.push(sp);
				while (!S.empty())
				{
					int now = S.top();
					if (distance_matrix_[now][which] == 0)
					{
						//找到sp到关键词的一条路径（链表）
						std::list<int> sp_keyword_list;
						std::stack<int>temp{ S };
						while (!temp.empty())
						{
							sp_keyword_list.push_front(temp.top());
							temp.pop();
						}
						sp_keyword_lists_[which].push_back(sp_keyword_list);
						//rdf_[now].visited = which;
						//退栈直到找到分叉路口
						S.pop();
						rdf_[now].visited = which;
						while (!S.empty())
						{
							int parent = S.top();
							bool has_new_branch = false;
							for (auto next : rdf_[parent].next)
							{
								if (distance_matrix_[parent][which] == distance_matrix_[next][which] + 1 && rdf_[next].visited != which)
								{
									S.push(next);//只入栈第一个
									has_new_branch = true;
									break;
								}
							}
							if (has_new_branch)//找到分叉
								break;
							rdf_[parent].visited = which;
							S.pop();
						}
						continue;
					}
					bool has_deeper = false;
					for (auto next : rdf_[now].next)
					{
						if (distance_matrix_[now][which] == distance_matrix_[next][which] + 1)
						{
							S.push(next);//只入栈第一个
							has_deeper = true;
							break;
						}
					}
					if (!has_deeper)
					{
						rdf_[now].visited = which;
						S.pop();
					}
				}
			}
			//回溯法构建树，计算并打印该sp所有符合要求的树
			//key1     key2   key3       key4……
			//[L1L2L3] [L1L2] [L1L2L3L4] [L1]……
			for (int i = 0; i < sp_keyword_lists_[0].size(); ++i)
			{
				Tree T = nullptr;
				backTrack(T, 0, i);
				deleteTree(T);//回收内存
			}
			for(int i = 0; i < sp_keyword_lists_.size(); ++i)
				sp_keyword_lists_[i].clear();
		}
		std::cout << "\n\n\n┣━━━━━━━━━Skyline Tree: " << 5/*trees_.size()*/ << std::endl << std::endl;
	}

	//第几个关键字、第几个链表
	std::stack<std::pair<TreeNode*, bool>> Skyline::backTrack(Tree &T, int which, int index)
	{
		std::stack<std::pair<TreeNode*, bool>> track_pos;
		std::stack<std::pair<TreeNode*, bool>> this_track_pos;
		bool check = meregeListToTree(sp_keyword_lists_[which][index], T, this_track_pos);
		//无法合并，无效树
		if (!check)
			return this_track_pos;
		if (which == keywords_.size() - 1)
		{
			//此处加入的是无效树，仅用于后期统计数量
			//主要为了节约内存
			trees_.push_back(nullptr);
			if(trees_.size() <= 100)
				displayTree(T);
			return this_track_pos;
		}
		for (int i = 0; i < sp_keyword_lists_[which + 1].size(); ++i)
		{
			track_pos = backTrack(T, which + 1, i);

			//删除上一个加入的路径
			if (track_pos.empty())
				continue;
			std::pair<TreeNode*, bool> s_next = track_pos.top();
			track_pos.pop();
			while (!track_pos.empty())
			{
				std::pair<TreeNode*, bool> s_now = track_pos.top();
				track_pos.pop();
				if (s_next.second)
				{
					s_now.first->child.erase(s_next.first);
					delete s_next.first;
				}
				s_next = s_now;
			}
		}
		return this_track_pos;
	}

	//track_pos 用于追踪插入位置以及是否是新结点
	bool Skyline::meregeListToTree(const std::list<int>& L, Tree &T, std::stack<std::pair<TreeNode*, bool>>& track_pos)
	{
		TreeNode* node, *r, *last = T;
		//树为空，直接构建
		if (T == NULL)
		{
			auto iter = L.begin();
			T = new TreeNode;
			T->id = *iter;
			r = T;
			while(++iter != L.end())
			{
				node = new TreeNode;
				node->id = *iter;
				r->child.insert(node);
				r = node;
			}
			return true;
		}
		r = T;
		//层次遍历
		std::queue<std::pair<TreeNode *, TreeNode *>> Q;
		for (auto p : T->child)
			Q.push({ p, T });
		int level_num = T->child.size();
		int next_level_num = 0;
		auto iter = L.begin();
		iter++;
		track_pos.push({ T, false });
		std::pair<TreeNode*, TreeNode*> now;
		while (!Q.empty() && iter != L.end())
		{
			bool find = false;
			while (level_num-- > 0)
			{
				now = Q.front();
				Q.pop();
				for (auto p : now.first->child)
					Q.push({ p,now.first });
				next_level_num += now.first->child.size();
				if (now.first->id == *iter)
				{
					if (now.second != last)
					{
						return false;
					}
					else
					{
						last = now.first;
						find = true;
						track_pos.push({ last, false });
					}
				}
			}
			if (!find)
			{
				node = new TreeNode;
				node->id = *iter;
				last->child.insert(node);
				last = node;
				track_pos.push({ last, true });
			}
			level_num = next_level_num;
			next_level_num = 0;
			iter++;
		}
		//多余的部分
		while (iter != L.end())
		{
			node = new TreeNode;
			node->id = *iter;
			last->child.insert(node);
			last = node;
			iter++;
		}
		return true;
	}

	void Skyline::displayTree(Tree T)
	{
		std::cout << std::endl;
		setSpace(T, 0);
		std::queue<std::pair<TreeNode*, TreeNode*>> Q;
		Q.push({ T,T });
		int level_num = 1;
		int next_level_num = 0;
		std::pair<TreeNode*, TreeNode*> now, last = {nullptr, nullptr};
		int round = 1;
		while (!Q.empty())
		{
			if (round % 2 != 0)//偶数打印结点
			{
				while (level_num-- > 0)
				{
					now = Q.front();
					Q.pop();
					int space_num;
					if (last.first)
						space_num = now.first->space_num - last.first->space_num - name_max_size_ - 2;
					else
						space_num = now.first->space_num;
					for (int i = 0; i < space_num; ++i)
						std::cout << " ";
					if (rdf_[now.first->id].is_location)
						std::cout << "[";
					else
						std::cout << "<";
	
					int count = name_max_size_ - id_to_name_.find(now.first->id)->second.size();
					int left = count / 2;
					int right = count - left;
					while (left-- > 0)
						std::cout << " ";
					std::cout << id_to_name_.find(now.first->id)->second;
					while (right-- > 0)
						std::cout << " ";
					if (rdf_[now.first->id].is_location)
						std::cout << "]";
					else
						std::cout << ">";

					for (TreeNode* p : now.first->child)
						Q.push({p, now.first});
					next_level_num += now.first->child.size();
					last = now;
				}
				level_num = next_level_num;
				next_level_num = 0;
				last = { nullptr, nullptr };
				std::cout << std::endl;
			}
			else//奇数打印树杈
			{
				int temp = level_num;
				std::queue<std::pair<TreeNode*, TreeNode*>> temp_Q{ Q };
				while (level_num-- > 0)
				{
					now = temp_Q.front();
					temp_Q.pop();
					int space_num;
					if (last.first)
						space_num = now.first->space_num - last.first->space_num - name_max_size_ + 2;
					else
						space_num = now.first->space_num;
					if (now.second->child.size() > 0)
					{
						if (now.second->child.size() == 1)
						{
							for (int i = 0; i < space_num; ++i)
								std::cout << " ";
							int count = (name_max_size_ + 2) / 2;
							while (count-- > 0)
								std::cout << " ";
							std::cout << "|";
						}
						else
						{
							if (now.first == *now.second->child.begin())
							{
								for (int i = 0; i < space_num; ++i)
									std::cout << " ";
								int count = (name_max_size_ + 2) / 2;
								while (count-- > 0)
									std::cout << " ";
								std::cout << "/";
							}
							else if (now.first == *now.second->child.rbegin())
							{
								for (int i = 0; i < space_num; ++i)
									std::cout << " ";
								int count = (name_max_size_ + 2) / 2;
								while (count-- > 0)
									std::cout << " ";
								std::cout << "\\";
							}
							else
							{
								for (int i = 0; i < space_num; ++i)
									std::cout << " ";
								int count = (name_max_size_ + 2) / 2;
								while (count-- > 0)
									std::cout << " ";
								std::cout << "|";
							}
						}
					}
					last = now;
				}
				level_num = temp;
				last = { nullptr, nullptr };
				std::cout << std::endl;
			}
			round++;
		}
		std::cout << std::endl;
	}

	int Skyline::setSpace(Tree T, int left_num)
	{
		if (T->child.empty())
		{
			T->space_num = left_num;
			return left_num;
		}
		int space_num = left_num;
		int add;
		bool first_time = true;
		std::vector<int> child_space_num;
		Tree last = nullptr;
		for (auto t : T->child)
		{
			if (!last)
			{
				add = space_num;
				first_time = false;
			}
			else
			{
				//寻找最右下结点
				while (!last->child.empty())
				{
					last = *(last->child.rbegin());
				}
				//间隔5
				add = last->space_num + 2 * (name_max_size_ + 2);
			}
			space_num = setSpace(t, add);
			child_space_num.push_back(space_num);
			last = t;
		}
		if (T->child.size() % 2 != 0)
			T->space_num = child_space_num[T->child.size() / 2];
		else
			T->space_num = (child_space_num[T->child.size() / 2]  + child_space_num[T->child.size() / 2 - 1]) / 2;
		return T->space_num;
	}

	Skyline::~Skyline()
	{
		for (auto t : trees_)
			deleteTree(t);
	}

	void Skyline::deleteTree(Tree t)
	{
		for (auto tree : t->child)
			deleteTree(tree);
		delete t;
	}
}