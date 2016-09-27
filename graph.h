#ifndef GRAPH_H_
#define GRAPH_H_

#include <fstream>
#include <exception>
#include <iostream>
#include <iterator>
#include <set>
#include <map>
#include "StringTokenizer.h"
#include "random.h"
#include <string.h>

using namespace std;
using namespace boost;
typedef pair<int,int> EDGE;

struct prop_edge
{
	EDGE edge;
	bool validTC;
	int TC;//triangle count

	bool validDC;
	int DC;
};

struct prop_vertex
{
	int vertex;
	int adj_size;
	vector<int> adj;      //no order is maintained
};

struct cmpE{
	bool operator()(const EDGE& e1,const EDGE& e2)const{
		if(e1.first<e2.first){
			return true;
		}else if(e1.first>e2.first){
			return false;
		}else{
			if(e1.second<e2.second){
				return true;
			}
			return false;
		}
	}
};
class graph_{
public:
	typedef pair<int,int> EDGE;
private:
	int x;

	vector<prop_vertex> graphAdj;
	int vertexCount;

	map<int,int> vertexToIndex;
	bool vertexToIndexValid;

	vector<prop_edge> edge_list;
	int edgeCount;
	bool triCountDone;
	bool degreeCountDone;

	map<EDGE,int,cmpE> edgeToIndex;
	bool edgeToIndexValid;

	vector<int> edgeCW;
	bool edgeCWValid;

	//mcmc degree based
	int walkL;
	int randomizeFactor;
	int rFT;
	EDGE current;
	bool MCMCDInitiated;

	//mcmc triangle based
	bool MCMCTInitiated;

	//node sampling
	vector<int> nodeShuffle;

public:
	~graph_(){
		for(int i=0;i<graphAdj.size();i++){
			graphAdj[i].adj.clear();
		}
		graphAdj.clear();
		vertexToIndex.clear();
		edge_list.clear();
		edgeToIndex.clear();
		edgeCW.clear();
		//cout<<"graph destructor\n";
	}

	graph_(graph_ *bg, const char* EorV, double reductionFactor){
		x=0;
		triCountDone=false;
		degreeCountDone=false;
    	edgeCWValid=false;
    	edgeToIndexValid=false;
    	MCMCDInitiated=false;
    	MCMCTInitiated=false;
    	walkL=-1;
    	randomizeFactor=-1;
    	current=make_edge(0,0);
    	vertexToIndexValid=false;

    	if(strcmp(EorV,"edge")==0)
    	{
    		bg->load_reduced_edge_graph(reductionFactor,edge_list);
    		prepare_variable();
    	}else if(strcmp(EorV,"vertex")==0)
    	{
    		bg->load_reduced_vertex_graph(reductionFactor,edge_list);
    		prepare_variable();
    	}
    	else{
    		cout<<"error!!!!";
    		exit(1);
    	}
    	vertexToIndexValid=true;
	}

    graph_(const char* filename) {
    	x=0;
    	triCountDone=false;
    	degreeCountDone=false;
    	edgeCWValid=false;
    	edgeToIndexValid=false;
    	MCMCDInitiated=false;
    	MCMCTInitiated=false;
    	walkL=-1;
    	randomizeFactor=-1;
    	current=make_edge(0,0);
    	vertexToIndexValid=false;

    	ifstream infile(filename, ios::in);
    	read_graph(infile);
    	vertexToIndexValid=true;
    }

    void load_reduced_vertex_graph(double reductionFactor,vector<prop_edge> &redge_list)
    {
    	double rand;
    	set<int> vertices;
    	set<int>::iterator it;
    	for(int i=0;i < graphAdj.size();i++){
    		if(graphAdj[i].adj_size==0) continue;
    		rand = random_uni01();
    		if(rand <= reductionFactor){
    			vertices.insert(graphAdj[i].vertex);
    		}
    	}

    	for(int i=0;i<edge_list.size();i++){
    		it=vertices.find(edge_list[i].edge.first);
    		if(it == vertices.end()) continue;
    		it=vertices.find(edge_list[i].edge.second);
    		if(it == vertices.end()) continue;

    		redge_list.push_back(edge_list[i]);
    	}


    }

    void load_reduced_edge_graph(double reductionFactor,vector<prop_edge> &redge_list){
    	double rand;
    	for(int i=0;i<edge_list.size();i++){
    		rand=random_uni01();
    		if(rand<=reductionFactor){
    			redge_list.push_back(edge_list[i]);
    			//redge_list[redge_list.size()-1].validTC=false;
    			//redge_list[redge_list.size()-1].validDC=false;
    		}
    	}
    }

    void prepare_variable()
    {
    	int largest_v=0;
    	for(int i=0;i<edge_list.size();i++){
    		edge_list[i].validDC=false;
    		edge_list[i].validTC=false;
    		if(edge_list[i].edge.second>largest_v) largest_v=edge_list[i].edge.second;
    	}
    	vertexCount = largest_v+1;
    	edgeCount=edge_list.size();
    	//make adjacency list
    	int vIndex=0;
    	map<int,int>::iterator it;
    	for(int i=0;i<edge_list.size();i++)
       	{
    		it=vertexToIndex.find(edge_list[i].edge.first);
    		if(it == vertexToIndex.end()){
    			prop_vertex a;
    			graphAdj.push_back(a);
    			graphAdj[vIndex].vertex=edge_list[i].edge.first;
    			vertexToIndex.insert(pair<int,int>(edge_list[i].edge.first,vIndex));
    			vIndex++;
    		}
    		int ti=vertexToIndex.find(edge_list[i].edge.first)->second;
   			if(edge_list[i].edge.first!=graphAdj[ti].vertex) {
   				cout<<"error read_graph!!!\n";
   				exit(1);
   			}
   			graphAdj[ti].adj.push_back(edge_list[i].edge.second);

   			it=vertexToIndex.find(edge_list[i].edge.second);
   			if(it == vertexToIndex.end()){
   				prop_vertex a;
   				graphAdj.push_back(a);
   				graphAdj[vIndex].vertex=edge_list[i].edge.second;
   		    	vertexToIndex.insert(pair<int,int>(edge_list[i].edge.second,vIndex));
   		    	vIndex++;
   			}
   			ti=vertexToIndex.find(edge_list[i].edge.second)->second;
   			if(edge_list[i].edge.second!=graphAdj[ti].vertex) {
   				cout<<"error read_graph!!!\n";
   				exit(1);
   			}
   			graphAdj[ti].adj.push_back(edge_list[i].edge.first);
       	}
    	for(int i=0;i<graphAdj.size();i++)
    	{
       		sort(graphAdj[i].adj.begin(),graphAdj[i].adj.end()); //pre-sort the adjacency list...needed for intersection operation
       		graphAdj[i].adj_size=graphAdj[i].adj.size();//pre-compute the adjacency list size
    	}
    }

    void read_graph(ifstream& datafile)
    {

    	/*
    	 * v1\tv2\n
    	 * Graph should be undirected. No duplicated edge must be present.
    	 * For best result, vertex labels should start from 0 and end to n.
    	 * All numbers between 0 to n should represent a valid node of graph.
    	 * File should end with a newline '\n'
    	 */
    	std::string oneline;
    	int largest_v=0;

    	while(1)								//read all the edges in a vector
    	{
    		std::getline(datafile,oneline);
    		if (oneline.length() < 1) break;
    		StringTokenizer strtok = StringTokenizer(oneline,"\t");
    		int v1 = strtok.nextIntToken();
    		int v2 = strtok.nextIntToken();
    		if (v1>largest_v) largest_v=v1;
    		if (v2>largest_v) largest_v=v2;
    		prop_edge e;
    		e.edge= make_edge(v1,v2);
    		e.validTC=false;
    		e.validDC=false;
    		edge_list.push_back(e);
    	}
    	vertexCount = largest_v+1;
    	edgeCount = edge_list.size();
    	//make adjacency list
    	int vIndex=0;
    	map<int,int>::iterator it;
    	//graphAdj.resize(vertexCount);
    	for(int i=0;i<edge_list.size();i++)
    	{
    		it=vertexToIndex.find(edge_list[i].edge.first);
    		if(it == vertexToIndex.end()){
    			prop_vertex a;
    			graphAdj.push_back(a);
    			graphAdj[vIndex].vertex=edge_list[i].edge.first;
    			vertexToIndex.insert(pair<int,int>(edge_list[i].edge.first,vIndex));
    			vIndex++;
    		}
    		int ti=vertexToIndex.find(edge_list[i].edge.first)->second;
    		if(edge_list[i].edge.first!=graphAdj[ti].vertex) {
    			cout<<"error read_graph!!!\n";
    			exit(1);
    		}
    		graphAdj[ti].adj.push_back(edge_list[i].edge.second);

    		it=vertexToIndex.find(edge_list[i].edge.second);
    		if(it == vertexToIndex.end()){
       			prop_vertex a;
       			graphAdj.push_back(a);
       			graphAdj[vIndex].vertex=edge_list[i].edge.second;
    		    vertexToIndex.insert(pair<int,int>(edge_list[i].edge.second,vIndex));
    		    vIndex++;
    		}
    		ti=vertexToIndex.find(edge_list[i].edge.second)->second;
    		if(edge_list[i].edge.second!=graphAdj[ti].vertex) {
    			cout<<"error read_graph!!!\n";
       			exit(1);
    		}
    		graphAdj[ti].adj.push_back(edge_list[i].edge.first);


    		//graphAdj[edge_list[i].edge.second].adj.push_back(edge_list[i].edge.first);
    	}
    	for(int i=0;i<graphAdj.size();i++)
    	{
    		sort(graphAdj[i].adj.begin(),graphAdj[i].adj.end()); //pre-sort the adjacency list...needed for intersection operation
    		graphAdj[i].adj_size=graphAdj[i].adj.size();//pre-compute the adjacency list size
    		//graphAdj[i].vertex=i;
    	}


    }

    int triangle_count_edge_index(int i){
    	return triangle_count(edge_list[i].edge);
    }

    int triangle_count_vertex_index(int i){
    	map<int,int>::iterator it=vertexToIndex.begin();
    	for(int t=0;t<i;t++){
    		it++;
    	}
    	return triangle_count_vertex(it);
    }
    int triangle_count(EDGE e)
    {
    	map<int,int>::const_iterator f,s;
    	f=vertexToIndex.find(e.first);
       	s=vertexToIndex.find(e.second);
    	if(f == vertexToIndex.end() or s == vertexToIndex.end())
    	{
    		cout<<"the vertex does not exist\n";
    		//return -1;//the vertex does not exist
    		exit(1);
    	}
    	if(!binary_search(graphAdj[f->second].adj.begin(), graphAdj[f->second].adj.end(), e.second)
    			or !binary_search(graphAdj[s->second].adj.begin(), graphAdj[s->second].adj.end(), e.first)){
    		print_edge(e);
    		cout<<"the edge does not exist\n";
    		//return 0; //the edge does not exist
    		exit(1);
    	}
    	vector<int> v;
    	v.resize(graphAdj[f->second].adj.size()+graphAdj[s->second].adj.size());
    	vector<int>::iterator it;
    	it=set_intersection(graphAdj[f->second].adj.begin(),graphAdj[f->second].adj.end(),graphAdj[s->second].adj.begin(),graphAdj[s->second].adj.end(),v.begin());
    	return it-v.begin();
    }

    int triangle_print(EDGE e)
    {
    	map<int,int>::const_iterator f,s;
    	f=vertexToIndex.find(e.first);
       	s=vertexToIndex.find(e.second);
    	if(f == vertexToIndex.end() or s == vertexToIndex.end())
    	{
    		cout<<"the vertex does not exist\n";
    		//return -1;//the vertex does not exist
    		exit(1);
    	}
    	if(!binary_search(graphAdj[f->second].adj.begin(), graphAdj[f->second].adj.end(), e.second)
    			or !binary_search(graphAdj[s->second].adj.begin(), graphAdj[s->second].adj.end(), e.first)){
    		print_edge(e);
    		cout<<"the edge does not exist\n";
    		//return 0; //the edge does not exist
    		exit(1);
    	}
    	vector<int> v;
    	v.resize(graphAdj[f->second].adj.size()+graphAdj[s->second].adj.size());
    	vector<int>::iterator it;
    	it=set_intersection(graphAdj[f->second].adj.begin(),graphAdj[f->second].adj.end(),graphAdj[s->second].adj.begin(),graphAdj[s->second].adj.end(),v.begin());

    	for (vector<int>::iterator i=v.begin();i!=it;i++){
    		if(((f->first) < (*i)) && ((s->first) < (*i))){
    			x++;
    			cout<<x<<": "<<"("<<f->first<<","<<s->first<<","<<*i<<")\n";
    			}
    	}
    	return it-v.begin();
    }


    int triangle_count_vertex(map<int,int>::iterator it){
    	if(it == vertexToIndex.end())
    	{
    		cout<<"the vertex does not exist\n";
    		exit(1);
    	}
    	int t=0;
    	for(vector<int>::iterator i = graphAdj[it->second].adj.begin(); i!=graphAdj[it->second].adj.end();i++){
    		map<int,int>::const_iterator v1=vertexToIndex.find(*(i));
    		for(vector<int>::iterator j = i+1; j!=graphAdj[it->second].adj.end();j++){
    			if(binary_search(graphAdj[v1->second].adj.begin(), graphAdj[v1->second].adj.end(), *(j))){
    				t++;
    			}

    		}
    	}
    	return t;

    }

    int triangle_count_vertex_no_map(int index){
    	int t=0;
    	for(vector<int>::iterator i = graphAdj[nodeShuffle[index]].adj.begin(); i!=graphAdj[nodeShuffle[index]].adj.end();i++){
    		map<int,int>::const_iterator v1=vertexToIndex.find(*(i));
    		for(vector<int>::iterator j = i+1; j!=graphAdj[nodeShuffle[index]].adj.end();j++){
    			if(binary_search(graphAdj[v1->second].adj.begin(), graphAdj[v1->second].adj.end(), *(j))){
    				t++;
    			}

    		}
    	}
    	return t;

    }

    void precompute_all_TC()
    {
    	if(triCountDone==true) return;

    	for(int i=0;i<edge_list.size();i++)
		{
    		if(edge_list[i].validTC==false){
    			edge_list[i].TC=triangle_count(edge_list[i].edge);
    			edge_list[i].validTC=true;
    		}
		}
    	triCountDone=true;
    	/*for(int i=0;i<edge_list.size();i++)
    	{
    		if(edge_list[i].validTC==false)
    		{
    			cout<<"error!!! TC not counted\n";
    		}
    		cout<<"edge: ("<<edge_list[i].edge.first<<","<<edge_list[i].edge.second<<") TC: "<<edge_list[i].TC<<'\n';
    	}*/
    }

    int uniformEdgeSampleTC()//returns triangle count of a uniformly sampled edge
    {
    	int rand = boost_get_a_random_number(0,edge_list.size());
    	//print_edge(edge_list[rand].edge);
    	//cout<< triangle_count(edge_list[rand].edge)<<endl;
    	return triangle_count(edge_list[rand].edge);
    }

    EDGE uniformEdgeSample()
    {
    	int rand = boost_get_a_random_number(0,edge_list.size());
    	return edge_list[rand].edge;
    }


    void initiateWeightedSampleTC()
    {
    	if(edgeCWValid==true) return;

    	precompute_all_TC();
       	edgeCW.resize(edge_list.size());
       	edgeCW[0]=edge_list[0].TC;
       	for(int i=1;i<edge_list.size();i++)
       	{
       		edgeCW[i]=edgeCW[i-1]+edge_list[i].TC;
       	}

       	edgeCWValid=true;
    }

    void initiateEdgeToIndexMap()
    {
    	if(edgeToIndexValid==true) return;
    	for(int i=0;i<edge_list.size();i++){
    		edgeToIndex[edge_list[i].edge]=i;
    	}
    	edgeToIndexValid=true;

    	/*for(int i=0;i<edge_list.size();i++)
    	{
    		print_edge(edge_list[edgeToIndex[edge_list[i].edge]].edge);
    		print_edge(edge_list[i].edge);
    		cout<<" "<<i<<"\n";
    	}*/
    }



    bool isTriPresent(const vector<int> &v)
    {
    	map<EDGE,int,cmpE>::const_iterator cit;
       	initiateEdgeToIndexMap();
       	cit=edgeToIndex.find(make_edge(v[0],v[1]));
       	if(cit==edgeToIndex.end()) return false;
       	cit=edgeToIndex.find(make_edge(v[0],v[2]));
       	if(cit==edgeToIndex.end()) return false;
       	cit=edgeToIndex.find(make_edge(v[1],v[2]));
       	if(cit==edgeToIndex.end()) return false;
       	return true;
    }

    void print_adjacency_list()
    {
      	cout<<"-------graph begin-------\n";
      	for(int i=0;i<graphAdj.size();i++)
      	{
       		cout<<"vertex: "<<graphAdj[i].vertex<<" degree: "<<graphAdj[i].adj_size<<"---- ";
       		for(int j=0;j<graphAdj[i].adj.size();j++)
       		{
       			cout<<graphAdj[i].adj[j]<<", ";
       		}
       		cout<<"\n";
      	}
       	cout<<"-------graph end-------\n";
    }

    EDGE make_edge(int a, int b)
    {
    	if (a<b) return EDGE(a,b);
     	return EDGE(b,a);
    }

    void print_edge(EDGE e)
    {
    	cout<<"("<<e.first<<","<<e.second<<") ";
    }

    long triMul3()
    {
    	//initiateWeightedSampleTC();
    	//return edgeCW[edgeCW.size()-1];
    	long TCMul3=0;
    	for(int i=0;i<edge_list.size();i++)
   		{
    		////edge_list[i].validTC==false
    		if(true){
   				edge_list[i].TC=triangle_count(edge_list[i].edge);
    			//edge_list[i].TC=triangle_print(edge_list[i].edge);
   				edge_list[i].validTC=true;
    		}
    		TCMul3+=edge_list[i].TC;
   		}
    	return TCMul3;
    }

    long triMul3App(double p){
    	double rand;
    	long TCMul3=0;
    	for(int i=0;i<edge_list.size();i++)
   		{
    		rand = random_uni01();
    		if(rand <= p)
    		{
    			////edge_list[i].validTC==false
    			if(true){
    				edge_list[i].TC=triangle_count(edge_list[i].edge);
    				edge_list[i].validTC=true;
   	    		}
    			TCMul3+=edge_list[i].TC;
    		}
   		}
    	return TCMul3/p;
    }

    long triMul3AppNI(double p){//node iterator
    	double rand;
    	long TCMul3=0;
    	map<int,int>::iterator it;
    	for(it=vertexToIndex.begin();it!=vertexToIndex.end();it++)
   		{
    		rand = random_uni01();
    		if(rand <= p)
    		{
    			TCMul3+=triangle_count_vertex(it);
    		}
   		}
    	return TCMul3/p;
    }

    int getEdgeCount(){return edgeCount;}

    int getVertexCount(){return vertexToIndex.size();}

    int sampleEdge(float sf){
    	int edgesToSample = sf* getEdgeCount();
    	unsigned x;
    	prop_edge te;
    	for(int i=0;i<edgesToSample;i++){
    		x=boost_get_a_random_number(i,getEdgeCount());
    		te=edge_list[i];
    		edge_list[i]=edge_list[x];
    		edge_list[x]=te;
    	}
    	return edgesToSample;
    }

    int sampleNode(float sf){
    	int nodesToSample = sf* getVertexCount();
    	unsigned x;
    	nodeShuffle.resize(getVertexCount());
    	for(int i=0;i<getVertexCount();i++){
    		nodeShuffle[i]=i;
    	}
    	int t;
    	for(int i=0;i<nodesToSample;i++){
    		x=boost_get_a_random_number(i,getVertexCount());
    		t=nodeShuffle[i];
    		nodeShuffle[i]=nodeShuffle[x];
    		nodeShuffle[x]=t;
    	}
    	return nodesToSample;
    }



};
#endif
