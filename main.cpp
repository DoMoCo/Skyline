#include <iostream>
#include <sstream>
#include "Skyline.h"
#include <unordered_map>
#include <fstream>
#include <ctime>
#include <tuple>
using namespace std;

void read_node_from_file(
	const string path, 
	unordered_map<int, string> &id_to_name,
	unordered_map<string, int> &name_to_id,
	vector<SkylineOnRDF::RDFNode> &nodes)
{
	ifstream f(path.c_str());
	if (!f.is_open())
	{
		cerr << "file not open !" << endl;
		return;
	}
	string line;
	int id = 0;
	while (getline(f, line))
	{
		SkylineOnRDF::RDFNode node;
		string name, info;
		size_t pos = line.find(':');
		name = line.substr(0, pos);
		info = line.substr(pos + 2);
		stringstream ss(info);
		while (getline(ss, info, ',')) {
			node.info.push_back(info);
		}
		node.is_location = true;
		id_to_name.insert({ id, name });
		name_to_id.insert({ name, id });
		nodes.push_back(node);
		id++;
	}
	f.close();
}

void read_edge_from_file(const string path, SkylineOnRDF::Skyline &sky, unordered_map<string, int>& name_to_id)
{
	ifstream f(path.c_str());
	if (!f.is_open())
	{
		cerr << "file not open !" << endl;
		return;
	}
	string line;
	set<int> nexts;
	int lose_vertex = 0, lose_edge = 0;
	while (getline(f, line))
	{
		string name, next;
		size_t pos = line.find(':');
		name = line.substr(0, pos);
		next = line.substr(pos + 2);
		stringstream ss(next);
		nexts.clear();
		while (getline(ss, next, ',')) {
			auto iter = name_to_id.find(next);
			if (iter != name_to_id.end())
				nexts.insert(iter->second);
			else
				lose_edge++;
		}
		auto it = name_to_id.find(name);
		if (it != name_to_id.end())
		{
			sky.addNext(name_to_id.find(name)->second, nexts);
			sky.addPrior(name_to_id.find(name)->second, nexts);
		}
		else
			lose_vertex++;
	}
	cout << "lose vertex: " << lose_vertex << endl;
	cout << "lost edge: " << lose_edge << endl;
}
int main()
{
	/*int num;
	cin >> num;
	string name, info, next;*/
	SkylineOnRDF::Skyline sky;
	unordered_map<int, string> id_to_name;
	unordered_map<string, int> name_to_id;
	vector<SkylineOnRDF::RDFNode> nodes;
	clock_t t0 = clock();
	read_node_from_file("node_keywords.txt", id_to_name, name_to_id, nodes);
	/*int round = num;
	int id = 0;
	while(round-- > 0)
	{
		SkylineOnRDF::RDFNode node;
		cin >> name >> info;
		stringstream ss(info);
		while (getline(ss, info, ',')) {
			node.info.push_back(info);
		}
		if (name.find('P') != name.npos)
			node.is_location = true;
		else
			node.is_location = false;
		id_to_name.insert({ id, name });
		name_to_id.insert({ name, id });
		nodes.push_back(node);
		id++;
	}*/
	sky.init(nodes, id_to_name);
	/*round = num;
	while(round-- > 0)
	{
		cin >> name >> next;
		if (next == "@end")
			continue;
		stringstream ss(next);
		set<int> nexts;
		while (getline(ss, next, ',')) {
			nexts.insert(name_to_id.find(next)->second);
		}
		sky.addNext(name_to_id.find(name)->second, nexts);
		sky.addPrior(name_to_id.find(name)->second, nexts);
	}*/
	read_edge_from_file("edge.txt", sky, name_to_id);
	cout << "Init success." << endl;
	clock_t t1 = clock();
	cout << "Read data cost: " << (double)(t1 - t0) / CLOCKS_PER_SEC << endl;
	sky.buildKeywordMap(true);
	clock_t t2 = clock();
	cout << "build keywords Hash Table cost: " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl;
	while (true)
	{
		cout << "please input the keywords:";
		string keywords;
		cin >> keywords;
		string word;
		vector<string> words;
		stringstream ss(keywords);
		while (getline(ss, word, ',')) {
			words.push_back(word);
		}
		sky.setKeywords(words);
		/*cout << "please input the keywords num:";
		int num;
		cin >> num;
		sky.setkeywords(num);*/
		clock_t t1 = clock();
		sky.computeDistanceMatrix();
		clock_t t2 = clock();
		//sky.fastComputeDistanceMatrix();
		clock_t t3 = clock();
		int candidate_sp = sky.BNL();
		clock_t t4 = clock();
		if (candidate_sp > 0)
		{
			sky.buildTree();
		}
		cout << "©Ç©¥©¥©¥©¥©¥©¥©¥©¥©¥BDM Cost: " << (double)(t2 - t1) / CLOCKS_PER_SEC << endl <<endl;
		//cout << "©Ç©¥©¥©¥©¥©¥©¥©¥©¥©¥BDM Cost: " << (double)(t3 - t2) / CLOCKS_PER_SEC << endl << endl;
		cout << "©Ç©¥©¥©¥©¥©¥©¥©¥©¥©¥BNL Cost: " << (double)(t4 - t3) / CLOCKS_PER_SEC << endl <<endl;
		cout << "©Ç©¥©¥©¥©¥©¥©¥©¥©¥©¥All Cost: " << (double)(t4 - t1) / CLOCKS_PER_SEC << endl <<endl;
		cout << "©»";
		int counter = 33 + 7 + 3 * words.size() - 2 - 2; 
		while (counter-- > 0)
			cout << "©¥";
		cout << "©¿\n" << std::endl;
	}
	//system("pause");
	return 0;
}