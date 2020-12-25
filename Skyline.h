#ifndef __SKYLINEONRDF__SKYLINE__
#define __SKYLINEONRDF__SKYLINE__

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <iterator>
#include <queue>
#include <stack> 
#include <set>
//#include "BTree.h"


namespace SkylineOnRDF
{
	const int inf = (1 << 31) - 1;

	typedef struct Node
	{
		std::vector<std::string> info; //附加信息
		std::set<int> next;  //下一个结点 
		std::set<int> prior; //上一个结点
		//std::string name;
		bool is_location;
		int visited;
		Node() { visited = -1; }
	}RDFNode;

	typedef struct TNode
	{
		int id; //结点编号
		std::set<struct TNode*> child;
		int space_num; 
		TNode(){ space_num = 0; }
		
	}TreeNode,*Tree;

	/*typedef struct LNode
	{
		int id;
		struct LNode* next;
	}LinkNode, *LinkList;*/

	class Skyline
	{
	 public:
		Skyline() {};
		//插入结点
		void init(const std::vector<RDFNode> &nodes, const std::unordered_map<int, std::string> &id_to_name);
		//添加后继信息
		void addNext(const int index, const std::set<int> &nexts);
		//添加前驱信息
		void addPrior(const int index, const std::set<int> &nexts);
		//设置查询关键字
		void setKeywords(const std::vector<std::string> &keywords);
		//产生测试用例
		void setKeywords(const int num);
		//通过BFS建立关键字map，所有结点按层数排序
		void buildKeywordMap(bool acquire_depth = false);
		//计算各结点到待查询关键字的最短距离
		void fastComputeDistanceMatrix();
		void computeDistanceMatrix();
		//skyline查询
		int BNL();
		//void BFS();
		//构建树
		void buildTree();
		//回溯法：各选一个关键字链表构建树
		std::stack<std::pair<TreeNode*, bool>> backTrack(Tree &T, int which, int index);
		//显示所有树
		void displayTree(Tree T);
		~Skyline();

	 private:
		//反向BFS
		void reverseBFS(const std::vector<int> origins, const int which);
		void reverseBFS(int origin, int which);
		//删除树
		void deleteTree(Tree t);
		//合并路径到已有树
		bool meregeListToTree(const std::list<int>& L, Tree &T, std::stack<std::pair<TreeNode *, bool>> &track_pos);
		//设置树结点据屏幕最左端的距离（空格数）
		int setSpace(Tree T, int left_num);

		std::vector<RDFNode> rdf_;
		std::vector<std::vector<int>> distance_matrix_;
		std::vector<Tree> trees_;
		std::vector<int> sp_;
		std::vector<std::string> keywords_;
		//sp到关键字的一条路径（链表）
		std::vector<std::vector<std::list<int>>> sp_keyword_lists_;
		std::unordered_map<int, std::string> id_to_name_;
		std::unordered_map<std::string, std::vector<std::pair<int, int>>> keyword_HashMap_;
		std::unordered_map<std::string, std::vector<int>> mini_keyword_HashMap_;
		int name_max_size_;

	};
}

#endif
