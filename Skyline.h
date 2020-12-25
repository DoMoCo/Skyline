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
		std::vector<std::string> info; //������Ϣ
		std::set<int> next;  //��һ����� 
		std::set<int> prior; //��һ�����
		//std::string name;
		bool is_location;
		int visited;
		Node() { visited = -1; }
	}RDFNode;

	typedef struct TNode
	{
		int id; //�����
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
		//������
		void init(const std::vector<RDFNode> &nodes, const std::unordered_map<int, std::string> &id_to_name);
		//��Ӻ����Ϣ
		void addNext(const int index, const std::set<int> &nexts);
		//���ǰ����Ϣ
		void addPrior(const int index, const std::set<int> &nexts);
		//���ò�ѯ�ؼ���
		void setKeywords(const std::vector<std::string> &keywords);
		//������������
		void setKeywords(const int num);
		//ͨ��BFS�����ؼ���map�����н�㰴��������
		void buildKeywordMap(bool acquire_depth = false);
		//�������㵽����ѯ�ؼ��ֵ���̾���
		void fastComputeDistanceMatrix();
		void computeDistanceMatrix();
		//skyline��ѯ
		int BNL();
		//void BFS();
		//������
		void buildTree();
		//���ݷ�����ѡһ���ؼ�����������
		std::stack<std::pair<TreeNode*, bool>> backTrack(Tree &T, int which, int index);
		//��ʾ������
		void displayTree(Tree T);
		~Skyline();

	 private:
		//����BFS
		void reverseBFS(const std::vector<int> origins, const int which);
		void reverseBFS(int origin, int which);
		//ɾ����
		void deleteTree(Tree t);
		//�ϲ�·����������
		bool meregeListToTree(const std::list<int>& L, Tree &T, std::stack<std::pair<TreeNode *, bool>> &track_pos);
		//������������Ļ����˵ľ��루�ո�����
		int setSpace(Tree T, int left_num);

		std::vector<RDFNode> rdf_;
		std::vector<std::vector<int>> distance_matrix_;
		std::vector<Tree> trees_;
		std::vector<int> sp_;
		std::vector<std::string> keywords_;
		//sp���ؼ��ֵ�һ��·��������
		std::vector<std::vector<std::list<int>>> sp_keyword_lists_;
		std::unordered_map<int, std::string> id_to_name_;
		std::unordered_map<std::string, std::vector<std::pair<int, int>>> keyword_HashMap_;
		std::unordered_map<std::string, std::vector<int>> mini_keyword_HashMap_;
		int name_max_size_;

	};
}

#endif
