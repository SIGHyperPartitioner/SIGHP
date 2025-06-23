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
#include <omp.h>


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
        #pragma omp parallel for schedule(static)
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
        #pragma omp parallel for schedule(static)
        for(int i=0; i< degree_mp.size();i++) degree_mp[i].clear();
    }
};
void load_data(std::string path, HyperNode * Node, HyperEdge * Edge){
    // Open file
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
    // Calculate optimal buffer size based on system page size
    const size_t BUFFER_SIZE = sysconf(_SC_PAGESIZE) * 256;
    int cur = 0;

    while(offset < fileSize){
        // Map file chunk into memory
        Int64 length = std::min(fileSize - offset, (Int64)BUFFER_SIZE);
        char* mapped = (char *)mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, offset);
        if (mapped == MAP_FAILED) {
            perror("mmap failed");
            close(fd);
            return;
        }

        // Process mapped data
        for(int i = 0; i < length; i++){
            if(mapped[i] >= '0' && mapped[i] <= '9') {
                e[cur] = e[cur] * 10 + (mapped[i] - '0');
            }
            else if (mapped[i] == ' ') {
                cur = 1;
            }
            else if (mapped[i] == '\n'){
                cur = 0;
                Node[e[0]].edges.push_back(e[1]);
                Edge[e[1]].nodes.push_back(e[0]);
                e[0] = e[1] = 0;
            }
        }

        // Unmap current chunk
        munmap(mapped, length);
        offset += length;
    }

    // Close file descriptor
    close(fd);
}

void solve(int n,int m,std::string path, int p,double sheild = 0,std::string output = "None", double e = 0){
    // n: number of HyperNode
    // m: number of HyperEdge
    // Node: array of HyperNode
    // Edge: array of HyperEdge
    // p: number of partition 
    // sheild: sheild update edge degree(log)
    // eval function: log 1 or 1/x
    //output: output partition infomation
    auto beg_time = std::chrono::high_resolution_clock::now(); 

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

    int maxi_cap = (n/p)*(1+e) + 1;
    std::vector<std::unordered_map<int,int> > part_node;
    std::vector<std::unordered_map<int,int> > part_edge;
    int maxi_degree = 0;

    Score_List score_list(maxi_degree,n);

    int cnt = 0;
    int cur_p = 0;
    for(int i = 0; i < p; i++) part_node.push_back(std::unordered_map<int,int>());
    for(int i = 0; i < p; i++) part_edge.push_back(std::unordered_map<int,int>());  

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
    auto end_time_2 = std::chrono::high_resolution_clock::now();
    double runtime_2 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_2 - beg_time).count();
    std::cout<<"Initial time:"<<runtime_2 <<" ms"<<std::endl;
    int cntt = 0;
    while(cnt < n){   
        cnt ++; 
        int add_node = score_list.top();
        score_list.erase(add_node);
        part_node[cur_p][add_node] = 1;
        // std::cout<<"add_node:"<<add_node<<std::endl;
        for(auto &e_id:Node[add_node].edges){
            const int e_degree = Edge[e_id].nodes.size();
            const auto& e_nodes = Edge[e_id].nodes;

            part_edge[cur_p][e_id] += 1;
            if(e_degree>sheild) continue;
            if(part_edge[cur_p][e_id] == 1){
                cntt ++;
                double val ;
                val = Mlog[e_degree] - Mlog[Emaxi_degree] + 0.000001;
                for(int i=0;i<e_degree;i++){
                    int n_id = e_nodes[i];
                    score_list.add(n_id,val);
                }
            }
        }
        // std::cout<<"cur_cap:"<<part_node[cur_p].size()<<std::endl;
        if(part_node[cur_p].size() >= maxi_cap){   
            // std::cout<<"new part"<<std::endl;
            score_list.clear();
            cur_p += 1;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    int k_1 = 0;
    for(int i=0;i<p;i++) {
        k_1 += part_edge[i].size();
        // std::cout<<"part_edge["<<i<<"]:"<<part_edge[i].size()<<std::endl;
    }
    // std::cout<<"cnt:"<<cnt<<" cntt:"<<cntt<<" maxi_cap:"<<maxi_cap<<std::endl;
    
    for(int i=0;i<m;i++){
        if(Edge[i].degree!=0){
            k_1--; 
        }
    } 
    double runtime = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - beg_time).count();
    std::cout<<"runtime:"<<runtime <<"ms"<<std::endl;
    std::cout<<"k-1:"<<k_1<<std::endl;
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

    // std::string input="../data/BA-1B.txt";
    // n = 5e8 + 2;
    // m = 5e8 + 2;

    int p = 2;
    double sheild_heavy_node = 0.2;
    std::string save = "None";
    double e = 0;

    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-i" && i + 1 < argc) {
            input = argv[i + 1];
        }else if(std::string(argv[i]) == "-n" && i + 1 < argc){
            n = std::stoi(argv[i + 1]);
        }else if(std::string(argv[i]) == "-m" && i + 1 < argc){
            m = std::stoi(argv[i + 1]);
        }else if(std::string(argv[i]) == "-p" && i + 1 < argc){
            p = std::stoi(argv[i + 1]);
        }else if(std::string(argv[i]) == "-sheild" && i + 1 < argc){
            sheild_heavy_node = strtod(argv[i + 1], NULL);
        }else if(std::string(argv[i]) == "-save" && i + 1 < argc){
            save = argv[i + 1];
        }else if(std::string(argv[i]) == "-e" && i + 1 < argc){
            e = strtod(argv[i + 1], NULL);
        }
    }

    std::cout<<"parameters:"<<std::endl;
    std::cout<<"dataset:"<<input<<"\tn:"<<n<<"\tm:"<<m<<std::endl;
    std::cout<<"p:"<<p<<"\tsheild:"<<sheild_heavy_node<<std::endl;
    std::cout<<"save partitionFile:"<<save<<std::endl;
    solve(n,m,input,p,sheild_heavy_node,save,e);
}
int main(int argc,char *argv[]){
    parsingCmd(argc,argv);
    return 0;
}
