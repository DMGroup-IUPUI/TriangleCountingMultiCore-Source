//sampling before threading only for job 4 6
#include "graph.h"
#include "time.h"
#include "time_tracker.h"
#include <vector>
#include <pthread.h>
#include <string.h>
#define ThreadPoleCount 10
using namespace std;
int contribute(int index);
int contribute_nodeIterator(int index);
void *print_message_function( void *ptr );
void *print_message_function_app( void *ptr );
void *print_message_function_app_nodeIterator( void *ptr );
void setThreadStep(int TotalPoleCount);

//code to check if mutex is valid
#define checkResults(string, val) {\
	if (val) {                      \
		printf("Failed with %d at %s", val, string);\
		exit(1);                                     \
	}                                          \
}

//not explained global variables
vector<pthread_t> threads;//vector of thread pointers
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

int edgeCount;//no of edges in the given network
int sampleEdgeCount;
int sampleNodeCount;
int nodeCount;
int TT; //# of total threads
char* datafile;
int ThreadStep;

vector<long> PartialCount;//count of triangles returned by each thread
long exactCount;//total no of triangles
int currEdgeIndex;
int currNodeIndex;
graph_* g;
float samplingFactor;
int job;



void print_usage(char *prog) {
  cerr<<"Usage: "<<prog<<" -d data-file -tc thread count -sf samplingFactor -j job "<<endl;
  cerr<<"job: \n0 Doulion \n1 Seq Exact\n2 Thread Exact\n3 Seq App\n4 Thread App\n5 Seq App NI\n6 Thread App NI\n";
  exit(0);
}

void parse_args(int argc, char* argv[]) {
  if(argc<5) {
    print_usage(argv[0]);
  }
  for (int i=1; i < argc; ++i){
    if (strcmp(argv[i], "-d") == 0){
    	datafile=argv[++i];
    	cout<<"datafile: "<<argv[i]<<"\n";
    }else if(strcmp(argv[i], "-tc") == 0){
    	TT=atoi(argv[++i]);
    	cout<<"threads: "<<argv[i]<<"\n";
	}else if(strcmp(argv[i], "-sf") == 0){
    	samplingFactor=atof(argv[++i]);
    	cout<<"samplingFactor: "<<argv[i]<<"\n";
	}else if(strcmp(argv[i], "-j") == 0){
		job=atoi(argv[++i]);
		cout<<"job: "<<argv[i]<<"\n";
	}
  }
}//end parse_args()


int main(int argc, char *argv[]) {
	parse_args(argc, argv);
	g=new graph_(datafile);

	vector<int> iret;//have no use
	iret.resize(TT);//have no use
	cout<<"edge: "<<g->getEdgeCount()<<"\n";
	cout<<"vertex: "<<g->getVertexCount()<<"\n";

	//doulion
	if (job==0){
        cout<<"*****Doulion*****\n";
		time_tracker tt;
		tt.start();
		graph_* sg=new graph_(g,"edge",samplingFactor);
		cout<<"edges in original network: "<<g->getEdgeCount()<<"\n";
		cout<<"edges  in sampled network: "<<sg->getEdgeCount()<<"\n";
		cout<<"vertices  in original network: "<<g->getVertexCount()<<"\n";
		cout<<"vertices  in sampled network: "<<sg->getVertexCount()<<"\n";
		cout<< "App triangle count (Doulion Seq): "<< (sg->triMul3()/3)/(samplingFactor*samplingFactor*samplingFactor) <<"\n";
		tt.stop();
		cout << "execution time: "<<tt.print()<<endl;
	}
	//sequential exact...............................................................................
	if (job==1){
        cout<<"*****Sequential Exact (Edge Iterator)*****\n";
		time_tracker tt;
		tt.start();
		cout<< "exact triangle count (Seq): "<< g->triMul3()/3 <<"\n";
		tt.stop();
		cout << "execution time: "<<tt.print()<<endl;
	}
	//parallel exact...............................................................................
	if(job==2){
        cout<<"*****Parallel Exact (Edge Iterator)*****\n";
		time_tracker tt2;
		tt2.start();

		threads.resize(TT);
		PartialCount.resize(TT);
		sampleEdgeCount=g->getEdgeCount();
        edgeCount=sampleEdgeCount;
		//cout<<"edges: "<<sampleEdgeCount<<"\n";
		setThreadStep(edgeCount);
		exactCount=0;
		currEdgeIndex=0;
		currNodeIndex=0;

		for(int i=0;i<TT;i++){
			iret[i]=pthread_create( &threads[i], NULL, print_message_function, (void*) i);
			//cout<<iret[i]<<endl;
		}

		for(int i=0;i<TT;i++){ //wait for all the threads to finish
			pthread_join(threads[i],NULL);
		}

		cout<< "exact triangle count (Th): "<< exactCount/3<<"\n";
		tt2.stop();
		cout << "execution time: "<<tt2.print()<<endl;
	}
	//sequential app...............................................................................
	if(job==3){
        cout<<"*****Sequential Approximate (Edge Iterator)*****\n";
        time_tracker tt3;
		tt3.start();
		cout<< "App triangle count (Seq): "<< g->triMul3App(samplingFactor)/3 <<"\n";
		tt3.stop();
		cout << "execution time: "<<tt3.print()<<endl;
	}
	//parallel app...............................................................................
	if(job==4){
        cout<<"*****Parallel Approximate (Edge Iterator)*****\n";
		time_tracker tt4;
		tt4.start();

		threads.resize(TT);
		PartialCount.resize(TT);
		edgeCount=g->getEdgeCount();
		cout<<"edges in original network: "<<edgeCount<<"\n";
		sampleEdgeCount=g->sampleEdge(samplingFactor);
		cout<<"edges sampled for edge iteration: "<<sampleEdgeCount<<"\n";
		setThreadStep(sampleEdgeCount);
		exactCount=0;
		currEdgeIndex=0;

		for(int i=0;i<TT;i++){
			iret[i]=pthread_create( &threads[i], NULL, print_message_function_app, (void*) i);
			//cout<<iret[i]<<endl;
		}

		for(int i=0;i<TT;i++){ //wait for all the threads to finish
			pthread_join(threads[i],NULL);
		}

		cout<< "App triangle count (Th): "<< (exactCount/3)/samplingFactor<<"\n";
		tt4.stop();
		cout << "execution time: "<<tt4.print()<<endl;
	}
	//sequential app node iterator...............................................................................
	if(job==5){
        cout<<"*****Sequential Approximate (Node Iterator)*****\n";
		time_tracker tt5;
		tt5.start();
		cout<< "App triangle count (Seq-NODE iterator): "<< g->triMul3AppNI(samplingFactor)/3 <<"\n";
		tt5.stop();
		cout << "execution time: "<<tt5.print()<<endl;
	}
	//parallel app node iterator..............................................................................
	if(job==6){
        cout<<"*****Parallel Approximate (Node Iterator)*****\n";
		time_tracker tt6;
		tt6.start();

		threads.resize(TT);
		PartialCount.resize(TT);
		nodeCount=g->getVertexCount();
		cout<<"nodes in original network : "<<nodeCount<<"\n";
		sampleNodeCount=g->sampleNode(samplingFactor);
		cout<<"nodes sampled for node iteration: "<<sampleNodeCount<<"\n";
		setThreadStep(sampleNodeCount);
		exactCount=0;
		currNodeIndex=0;

		for(int i=0;i<TT;i++){
			iret[i]=pthread_create( &threads[i], NULL, print_message_function_app_nodeIterator, (void*) i);
			//cout<<iret[i]<<endl;
		}

		for(int i=0;i<TT;i++){ //wait for all the threads to finish
			pthread_join(threads[i],NULL);
		}

		cout<< "App triangle count (Th-NODE iterator): "<< (exactCount/3)/samplingFactor<<"\n";
		tt6.stop();
		cout << "execution time: "<<tt6.print()<<endl;
	}
	return 0;
}

void *print_message_function_app_nodeIterator( void *ptr )
{
	int rc;
	int index; //thread index in vector "threads"
	int nodeIndex=-1;
	index = (int &)ptr;
	PartialCount[index]=0;

	while(true){
		rc=pthread_mutex_lock(&mutex1); //get mutex
		checkResults("pthread_mutex_lock()\n", rc);//check mutex

		nodeIndex=contribute_nodeIterator(index);//add the partial count to the total "exactCount"

		rc = pthread_mutex_unlock(&mutex1);//release mutex
		checkResults("pthread_mutex_unlock()\n", rc);//check mutex
		PartialCount[index]=0; //partial count reinitialized after contribution is done
		if (nodeIndex==-1) break; //computation done
		//approximation part
		double rand;
		//approximation part
		for(int i=nodeIndex;i<nodeIndex+ThreadStep and i < sampleNodeCount ;i++){
			//rand = random_uni01();
			//if(rand <= samplingFactor)
			//{
			PartialCount[index]+=g->triangle_count_vertex_no_map(i);
			//}
		}
	}
	//cout<<"thread "<<index <<"done\n";
}

void *print_message_function_app( void *ptr )
{
	int rc;
	int index; //thread index in vector "threads"
	int edgeIndex=-1;
	index = (int &)ptr;
	PartialCount[index]=0;
	//time_tracker ptt;
	//ptt.start();

	while(true){
		rc=pthread_mutex_lock(&mutex1); //get mutex
		checkResults("pthread_mutex_lock()\n", rc);//check mutex

		edgeIndex=contribute(index);//add the partial count to the total "exactCount"

		rc = pthread_mutex_unlock(&mutex1);//release mutex
		checkResults("pthread_mutex_unlock()\n", rc);//check mutex
		PartialCount[index]=0; //partial count reinitialized after contribution is done
		if (edgeIndex==-1) break; //computation done
		//approximation part
		double rand;
		//approximation part
		for(int i=edgeIndex;i<edgeIndex+ThreadStep and i < sampleEdgeCount ;i++){
			//rand = random_uni01();
			//if(rand <= samplingFactor)
			//{
			PartialCount[index]+=g->triangle_count_edge_index(i);
			//}
		}
	}
	//ptt.stop();
	//cout << "Thread: "<<index<<" Ttime: "<<ptt.print()<<endl;
	//cout<<"thread "<<index <<"done\n";
}


void *print_message_function( void *ptr )
{
	int rc;
	int index; //thread index in vector "threads"
	int edgeIndex=-1;
	index = (int &)ptr;
	PartialCount[index]=0;

	while(true){
		rc=pthread_mutex_lock(&mutex1); //get mutex
		checkResults("pthread_mutex_lock()\n", rc);//check mutex

		edgeIndex=contribute(index);//add the partial count to the total "exactCount"

		rc = pthread_mutex_unlock(&mutex1);//release mutex
		checkResults("pthread_mutex_unlock()\n", rc);//check mutex
		PartialCount[index]=0; //partial count reinitialized after contribution is done
        if (edgeIndex==-1) break; //computation done
		for(int i=edgeIndex;i<edgeIndex+ThreadStep and i < sampleEdgeCount ;i++){
			PartialCount[index]+=g->triangle_count_edge_index(i);
        }
	}
	//cout<<"thread "<<index <<"done\n";
}

int contribute(int index){
	exactCount+=PartialCount[index];
	if (currEdgeIndex>=sampleEdgeCount){
		return -1; //computation done
	}
	int t=currEdgeIndex;
	currEdgeIndex+=ThreadStep;
	return t;
}

int contribute_nodeIterator(int index){
	exactCount+=PartialCount[index];
	if (currNodeIndex>=nodeCount){
		return -1; //computation done
	}
	int t=currNodeIndex;
	currNodeIndex+=ThreadStep;
	return t;
}

void setThreadStep(int TotalPoleCount){
	ThreadStep=(TotalPoleCount/TT)/ThreadPoleCount;
	if(ThreadStep < 10){
		ThreadStep=10;
	}
	cout<<"TotalPoleCount (Edges/Nodes to iterate through): "<<TotalPoleCount<<"\nThreadStepSize (Edges/Nodes assigned at a time): "<<ThreadStep<<"\n";
    cout<<"(Max) Number of times a thread accesses the queue: "<<ThreadPoleCount<<endl;
}
