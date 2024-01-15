

#pragma GCC optimize(2)
#include <cmath>
#include <stdlib.h>
#include "data.hpp"
#include <iostream>
#include <unistd.h>
#include <limits.h>
#include <string>
#include <sys/stat.h>
#include <sys/mman.h>     
#include <fcntl.h>  
// #include <omp.h>

typedef long long Int64;
int n,m;
HyperNode *Node;
HyperEdge *Edge;
std::vector<int> nn;
std::vector<int> mm;
std::vector<std::string> filename;
typedef std::pair<int,int> P;
int Emaxi_degree;
int total_edge;
std::unordered_map<int,int> Esum;
std::unordered_map<int,double> Mlog;
class Score_List{
public:
    int maxi_degree;
    int num_node;
    int cur_maxi;
    std::vector<std::unordered_map<int,int> > degree_mp;
    std::unordered_map<int,double> Eval;
    std::unordered_set<int> wait_assign;
    Score_List(int Maxi_degree,int Num_node){
        cur_maxi = 0;
        this->maxi_degree = Maxi_degree;
        this->num_node = Num_node;
        for(int i=0; i<=Maxi_degree;i++){
            degree_mp.push_back(std::unordered_map<int,int>());
        }
        for(int i=0;i<num_node;i++) wait_assign.insert(i);
    }
    bool assigned(int id){
        return wait_assign.find(id) == wait_assign.end();
    }
    void add(int id,double val = 1){     
        assert(val>0);
        if(assigned(id)) return;
        int pre_v = Eval[id];
        Eval[id] += val;        
        while(Eval[id]>=degree_mp.size()) degree_mp.push_back(std::unordered_map<int,int>() );
        assert(Eval[id]<degree_mp.size());
        int aft_v = Eval[id];
        if(pre_v == aft_v) return;
   
        degree_mp[pre_v].erase(id);
        degree_mp[aft_v][id] = 1;

        if(aft_v>cur_maxi) cur_maxi = aft_v;        

    }
    
    int top(){
        if(cur_maxi == 0 &&degree_mp[cur_maxi].size() == 0){
            for(auto &item:wait_assign) return item;
        }
        
        for(auto &item : degree_mp[cur_maxi])  {
            int n_id = item.first;

            // cerr<<"top:"<<n_id<<" cur_maxi:"<<cur_maxi<<std::endl;
        
            assert(assigned(n_id) == false);
            return n_id; 
        }
        assert(false);
        return -1;
    }
    void erase(int id){
        assert(assigned(id) == false);
        wait_assign.erase(id);
        degree_mp[int(Eval[id])].erase(id);
        if(int(Eval[id]) == cur_maxi) {
            while(cur_maxi != 0 && degree_mp[cur_maxi].size() == 0) cur_maxi --;
        }
    }
    void clear(){
        Eval = std::unordered_map<int,double>();
        cur_maxi = 0;
        for(int i=0; i< degree_mp.size();i++) degree_mp[i].clear();
    }
};
void load_data(std::string path,HyperNode * Node,HyperEdge * Edge){
    // swap(n,m);
    int fd;
    struct stat sb;
    if ((fd = open(path.c_str(), O_RDONLY)) < 0) {  
            perror("open file error!");  
    }  

    if ((fstat(fd, &sb)) == -1) {  
        perror("file status get failed!");  
    }  

    Int64 fileSize = sb.st_size;
    Int64 offset = 0;
    Int64 e[2];
    e[0] = e[1] = 0;
    Int64 blockSize = 16 * 1024 * 100;
    int cur = 0;

    while(offset < fileSize){
        Int64 length = std::min(fileSize - offset, blockSize);
            char* mapped = (char *)mmap(NULL, length, PROT_READ , MAP_PRIVATE, fd, offset);
        for(int i=0;i<length ;i++){
            if(mapped[i] >= '0' && mapped[i] <='9') e[cur] = e[cur]*10 + mapped[i] - '0';
            else if (mapped[i] == ' ') cur = 1;
            else if (mapped[i] == '\n'){
                cur = 0;
                // if(e[0] >= Node.size()) Node.push_back(std::std::vector<int>());
                // if(e[1] >= Edge.size()) Edge.push_back(std::std::vector<int>());
                Node[e[0]].edges.push_back(e[1]);
                Edge[e[1]].nodes.push_back(e[0]);
                e[0] = e[1] = 0;
            }
        }
        offset += length;
    }
}

void solve(int n,int m,std::string path, int p,double sheild = 0,std::string method = "entropy",std::string output = "None"){
    // n: number of HyperNode
    // m: number of HyperEdge
    // Node: array of HyperNode
    // Edge: array of HyperEdge
    // p: number of partition 
    // sheild: sheild update edge degree(log)
    // eval function: log 1 or 1/x
    //output: output partition infomation
    clock_t beg_time = clock(); 

    Node = new HyperNode[n];
    Edge = new HyperEdge[m];
    load_data(path,Node,Edge);
    std::vector<int> edge_degree;
    Emaxi_degree = 0;
    for(int i=0;i<m;i++) {
        Edge[i].id = i;
        Edge[i].degree = Edge[i].nodes.size();
        edge_degree.push_back(Edge[i].nodes.size());
        total_edge+=Edge[i].degree;
        Esum[Edge[i].degree] += 1;
        if (Mlog[Edge[i].degree] == 0 && Mlog[Edge[i].degree]!=1)
            Mlog[Edge[i].degree] = -log2(Edge[i].degree);
            Emaxi_degree = std::max(Emaxi_degree,Edge[i].degree);
    }

    int maxi_cap = n/p + 1;
    std::vector<std::unordered_map<int,int> > part_node;
    std::vector<std::unordered_map<int,int> > part_edge;
    int maxi_degree = 0;

    Score_List score_list(maxi_degree,n);
    if(method == "anti-basic"){
        int max_val = -1;
        int cnt = 0;
        for(int i=0;i<n;i++) max_val = std::max(max_val,int(Node[i].edges.size())+1);
        for(int i=0;i<n;i++){
            score_list.add(i,max_val-Node[i].edges.size());
        }
    }

    int cnt = 0;
    int cur_p = 0;
    for(int i = 0; i < p; i++) part_node.push_back(std::unordered_map<int,int>());
    for(int i = 0; i < p; i++) part_edge.push_back(std::unordered_map<int,int>());  

    if(method == "random"){
        for(int i = 0 ; i < n; i++){
            int send_p = rand()%p;
            part_node[send_p][i] = 1;
            for(auto &e_id:Node[i].edges){
                part_edge[send_p][e_id] = 1;
            }
        }
    }
    else{
        double c = total_edge * sheild;
        int pos = Emaxi_degree;
        while(c>0) {
            c -= Esum[pos]*pos;
            pos--;
        }
        sheild = pos + 1;
        // int pos = 0;
        // while(c>0) c-= edge_degree[pos++];
        // sheild = ed
        while(cnt < n){   
            cnt ++; 
            int add_node = score_list.top();
            score_list.erase(add_node);
            part_node[cur_p][add_node] = 1;

            for(auto &e_id:Node[add_node].edges){
                part_edge[cur_p][e_id] += 1;
                if(Edge[e_id].degree>sheild) continue;
                if(part_edge[cur_p][e_id] == 1){
                    double val ;
                    if(method == "entropy") val = Mlog[Edge[e_id].degree] - Mlog[Emaxi_degree] + 0.000001;
                    else if (method == "basic") val = 1; 
                    else if (method == "anti-basic") val = 1; 
                    else assert(false);

// #pragma omp parallel for
                    for(int i=0;i<Edge[e_id].degree;i++){
                        int n_id = Edge[e_id].nodes[i];
                        score_list.add(n_id,val);
                        // if(score_list.Eval[n_id] == W[n_id]){
                        //     cnttt ++;
                        //     part_node[cur_p][n_id] = 1;
                        //     score_list.erase(n_id);
                        //     cnt++;    
                        // }
                        // if(part_node[cur_p].size() >= maxi_cap) break;
                    }
                }
            }
            if(part_node[cur_p].size() >= maxi_cap){   
                score_list.clear();
                cur_p += 1;
            }
        }
    }
    clock_t end_time = clock();
    int k_1 = 0;
    for(int i=0;i<p;i++) {
        k_1 += part_edge[i].size();
    }
    
    clock_t runtime = (end_time-beg_time)*1000/CLOCKS_PER_SEC;
    std::cout<<"runtime:"<<runtime <<"ms"<<std::endl;
    std::cout<<"k-1:"<<k_1-m<<std::endl;
    if(output != "None"){
        FILE *result;
        result = fopen(output.c_str(),"w");
        for(int i=0;i<p;i++){
            for(auto &item:part_node[i]){
                int n_id = item.first;
                fprintf(result,"%d %d\n",n_id,i);
            }
        }
        fclose(result);
    }
    delete []Node;
    delete []Edge;

}

void parsingCmd(int argc,char *argv[]){
    std::string input="../data/out.github";
    n = 56530;
    m = 120869;
    int p = 16;
    std::string method = "entropy";
    double sheild_heavy_node = 0.2;
    std::string save = "None";

    for (int i = 1; i < argc; ++i) {
        // 检查是否有 -o 选项
        if (std::string(argv[i]) == "-i" && i + 1 < argc) {
            input = argv[i + 1];
        }else if(std::string(argv[i]) == "-n" && i + 1 < argc){
            n = std::stoi(argv[i + 1]);
        }else if(std::string(argv[i]) == "-m" && i + 1 < argc){
            m = std::stoi(argv[i + 1]);
        }else if(std::string(argv[i]) == "-p" && i + 1 < argc){
            p = std::stoi(argv[i + 1]);
        }else if(std::string(argv[i]) == "-method" && i + 1 < argc){
            method = std::string(argv[i + 1]);
        }else if(std::string(argv[i]) == "-sheild" && i + 1 < argc){
            sheild_heavy_node = strtod(argv[i + 1], NULL);
        }else if(std::string(argv[i]) == "-save" && i + 1 < argc){
            save = argv[i + 1];
        }
    }

    std::cout<<"parameters:"<<std::endl;
    std::cout<<"dataset:"<<input<<"\tn:"<<n<<"\tm:"<<m<<std::endl;
    std::cout<<"method:"<<method<<"\tp:"<<p<<"\tsheild:"<<sheild_heavy_node<<std::endl;
    std::cout<<"save partitionFile:"<<save<<std::endl;
    solve(n,m,input,p,sheild_heavy_node,method,save);
}
int main(int argc,char *argv[]){
    parsingCmd(argc,argv);
    return 0;
}
