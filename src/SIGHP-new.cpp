#pragma GCC optimize(3)
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
#include <chrono>
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
    int cur_pos;
    std::vector<std::unordered_map<int,int> > degree_mp;
    std::unordered_map<int,double> Eval;
    std::vector<int> wait_assign;
    Score_List(int Maxi_degree,int Num_node){
        this->cur_maxi = 0;
        this->maxi_degree = Maxi_degree;
        this->num_node = Num_node;
        for(int i=0; i<=Maxi_degree;i++){
            this->degree_mp.push_back(std::unordered_map<int,int>());
        }
        // for(int i=0;i<num_node;i++) wait_assign.push_back(i);
        this->cur_pos = 0;
        this->wait_assign = std::vector<int>(num_node+1, 0);
    }
    bool assigned(int id){
        return wait_assign[id] == 1;
    }
    void add(int id,double val = 1){     
        if(assigned(id)) return;
        int pre_v = Eval[id];
        Eval[id] += val;        
        while(Eval[id]>=degree_mp.size()) degree_mp.push_back(std::unordered_map<int,int>() );
        assert(Eval[id]<degree_mp.size());
        int aft_v = Eval[id];
        if(pre_v == aft_v) return;
   
        degree_mp[pre_v].erase(id);
        degree_mp[aft_v][id] = 1;

        if(aft_v > cur_maxi) cur_maxi = aft_v;        

    }
    
    int top(){
        if(cur_maxi == 0 &&degree_mp[cur_maxi].size() == 0){
            while(wait_assign[cur_pos] == 1) cur_pos++;
            return cur_pos;
        }
        
        for(auto &item : degree_mp[cur_maxi])  {
            int n_id = item.first;
            assert(assigned(n_id) == false);
            return n_id; 
        }
        assert(false);
        return -1;
    }
    void erase(int id){
        assert(assigned(id) == 0);
        wait_assign[id] = 1;
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

class MmapAllocator {
private:
    void* node_ptr;
    void* edge_ptr;
    size_t node_size;
    size_t edge_size;
    
public:
    MmapAllocator(size_t n, size_t m) {
        node_size = n * sizeof(HyperNode);
        edge_size = m * sizeof(HyperEdge);
        
        // 使用mmap分配大内存块，对大内存更高效
        node_ptr = mmap(nullptr, node_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        edge_ptr = mmap(nullptr, edge_size, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        
        if (node_ptr == MAP_FAILED || edge_ptr == MAP_FAILED) {
            throw std::bad_alloc();
        }
        
        // 建议操作系统使用大页面
        madvise(node_ptr, node_size, MADV_HUGEPAGE);
        madvise(edge_ptr, edge_size, MADV_HUGEPAGE);
    }
    
    HyperNode* get_nodes() { return (HyperNode*)node_ptr; }
    HyperEdge* get_edges() { return (HyperEdge*)edge_ptr; }
    
    ~MmapAllocator() {
        munmap(node_ptr, node_size);
        munmap(edge_ptr, edge_size);
    }
};


class OpenMPProcessor {
    static constexpr size_t CHUNK_SIZE = 1024 * 4 * 128;
    
public:
    void process(const char* filename, HyperNode * Node, HyperEdge * Edge) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        auto chunks = getChunks(filename);
        auto chunk_time = std::chrono::high_resolution_clock::now();
        double chunk_duration = std::chrono::duration_cast<std::chrono::milliseconds>(chunk_time - start_time).count() / 1000.0;
        std::cout << "Chunk division time: " << chunk_duration << " seconds" << std::endl;
        
        std::vector<std::vector<std::pair<int,int>>> thread_edges(chunks.size());
        #pragma omp parallel for schedule(static)
        for (size_t i = 0; i < chunks.size(); i++) {
            thread_edges[i] = processChunk(filename, chunks[i]);
        }
        
        auto process_time = std::chrono::high_resolution_clock::now();
        double process_duration = std::chrono::duration_cast<std::chrono::milliseconds>(process_time - chunk_time).count() / 1000.0;
        std::cout << "Chunk processing time: " << process_duration << " seconds" << std::endl;
        
        buildGraph(mergeEdges(thread_edges), Node, Edge);
        auto end_time = std::chrono::high_resolution_clock::now();
        double build_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - process_time).count() / 1000.0;
        double total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() / 1000.0;
        
        std::cout << "Graph building time: " << build_duration << " seconds" << std::endl;
        std::cout << "Total processing time: " << total_duration << " seconds" << std::endl;
    }
    
private:
    std::vector<std::pair<size_t, size_t>> getChunks(const char* filename) {
        struct stat st;
        stat(filename, &st);
        size_t file_size = st.st_size;
        
        int fd = open(filename, O_RDONLY);
        char* data = (char*)mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
        
        std::vector<std::pair<size_t, size_t>> chunks;
        for (size_t pos = 0; pos < file_size; ) {
            size_t start = pos;
            pos = std::min(pos + CHUNK_SIZE, file_size);
            // 调整到行边界
            while (pos < file_size && data[pos] != '\n') pos++;
            if (pos < file_size) pos++;
            chunks.emplace_back(start, pos - start);
        }
        
        munmap(data, file_size);
        close(fd);
        return chunks;
    }
    
    std::vector<std::pair<int,int>> processChunk(const char* filename, const std::pair<size_t, size_t>& chunk) {
        auto [offset, length] = chunk;
        // std::cout<<"offset:"<<offset<<" length:"<<length<<std::endl;
        size_t aligned_offset = (offset / 4096) * 4096;
        size_t delta_offset = offset - aligned_offset;
        int fd = open(filename, O_RDONLY);
        char* data = (char*)mmap(nullptr, length + delta_offset, PROT_READ, MAP_PRIVATE, fd, aligned_offset);
        
        // 预分配边的数组（假设每行平均20字符）
        // std::cout<<"length:"<<length<<std::endl;
        size_t max_edges = length + 1;
        std::pair<int,int>* edge_buffer = new std::pair<int,int>[max_edges];
        size_t edge_count = 0;
        
        int nums[2] = {0, 0}, idx = 0;
        for (size_t i = delta_offset; i < length + delta_offset; i++) {
            char c = data[i];
            if (c >= '0' && c <= '9') {
                nums[idx] = nums[idx] * 10 + (c - '0');
            } else if (c == ' ') {
                idx = 1;
            } else if (c == '\n') {
                edge_buffer[edge_count++] = {nums[0], nums[1]};
                nums[0] = nums[1] = idx = 0;
                // #pragma omp atomic
                // total_edge++;
            }
        }
        // std::cout<<"edge_count:"<<edge_count<<std::endl;
        // 创建最终的vector并转移数据
        std::vector<std::pair<int,int>> edges;
        edges.reserve(edge_count);  // 预分配确切的大小
        for(size_t i = 0; i < edge_count; i++) {
            edges.push_back(edge_buffer[i]);
        }

        
        delete[] edge_buffer;
        munmap(data, length);
        close(fd);
        return edges;
    }
    
    std::vector<std::pair<int,int>> mergeEdges(const std::vector<std::vector<std::pair<int,int>>>& thread_edges) {
        std::vector<std::pair<int,int>> all_edges;
        for (const auto& edges : thread_edges) {
            all_edges.insert(all_edges.end(), edges.begin(), edges.end());
        }
        return all_edges;
    }
    
    void buildGraph(const std::vector<std::pair<int,int>>& edges, HyperNode * Node, HyperEdge * Edge) { 
        // 构建图结构
        // #pragma omp parallel for schedule(static)
        // int cnt = 0;
        std::vector<int> cnt_edge(n + 1,0);
        std::vector<int> cnt_node(m + 1,0);
        #pragma omp parallel for schedule(static)
        for (const auto& [from, to] : edges) {
            #pragma omp atomic
            cnt_edge[from]++;
            #pragma omp atomic
            cnt_node[to]++;
        }

        #pragma omp parallel for schedule(static)
        for(int i=0;i<n+1;i++){
            // Node[i].edges.reserve(cnt_edge[i]);
            Node[i].edges.resize(cnt_edge[i]);
        }
        #pragma omp parallel for schedule(static)
        for(int i=0;i<m+1;i++){
            // Edge[i].nodes.reserve(cnt_node[i]);
            Edge[i].nodes.resize(cnt_node[i]);
            Edge[i].degree = cnt_node[i];
        }


        #pragma omp parallel for schedule(static)
        for (const auto& [from, to] : edges) {
            int edge_idx, node_idx;
            
            // 原子地递减并捕获新值
            #pragma omp atomic capture
            edge_idx = (--cnt_edge[from]);
            
            #pragma omp atomic capture  
            node_idx = (--cnt_node[to]);
            
            // 现在可以安全地使用捕获的索引
            Node[from].edges[edge_idx] = to;
            Edge[to].nodes[node_idx] = from;
        }
    }
};

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

    std::cout<<"load data"<<std::endl;
    MmapAllocator allocator(n + 5, m + 5);
    HyperNode* Node = allocator.get_nodes();
    HyperEdge* Edge = allocator.get_edges();

    OpenMPProcessor processor;  // 创建OpenMPProcessor实例
    processor.process(path.c_str(), Node, Edge);  // 使用实例调用process方法
    std::cout<<"load data done"<<std::endl;

    Emaxi_degree = 0;
    for(int i=0;i<m+1;i++) {
        total_edge+=Edge[i].degree;
        Esum[Edge[i].degree] += 1;
        Emaxi_degree = std::max(Emaxi_degree, Edge[i].degree);
    }

    Mlog[0] = 0;
    for(int i=1;i<=Emaxi_degree;i++){
        Mlog[i] = -log2(1.0*i/Emaxi_degree) + 0.000001;
    }

    int maxi_cap = (n/p)*(1+e) + 1;
    std::vector<std::unordered_map<int,int> > part_node;
    std::vector<std::unordered_map<int,int> > part_edge;

    Score_List score_list(Emaxi_degree,n);

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
    // while(c > 0) c-= edge_degree[pos++];
    // sheild = ed
    auto end_time_2 = std::chrono::high_resolution_clock::now();
    double runtime_2 = std::chrono::duration_cast<std::chrono::milliseconds>(end_time_2 - beg_time).count() / 1000.0;
    std::cout<<"Initial time:"<<runtime_2 <<" ms"<<std::endl;
    std::cout << "Partition Begin:" << std::endl;
    while(cnt < n){
        
        cnt ++; 
        int add_node = score_list.top();
        score_list.erase(add_node);
        part_node[cur_p][add_node] = 1;
        for(auto &e_id:Node[add_node].edges){
            const int e_degree = Edge[e_id].degree;
            const auto& e_nodes = Edge[e_id].nodes;
            // std::cout<<"e_id:"<<e_id<<" e_degree:"<<e_degree<<" e_nodes[0]:"<<e_nodes[0]<<std::endl;

            part_edge[cur_p][e_id] += 1;
            if(e_degree>sheild) continue;

            if(part_edge[cur_p][e_id] == 1){            
                // std::cout<<"check1 "<<e_degree<<std::endl;
                // #pragma omp parallel for schedule(static)
                for(int i=0;i<e_degree;i++){
                    // std::cout<<"i "<<i<<std::endl;
                    // std::cout<<"e_nodes[i] "<<e_nodes[i]<<std::endl;
                    // std::cout<<"Mlog[e_degree] "<<Mlog[e_degree]<<std::endl;
                    int n_id = e_nodes[i];
                    score_list.add(n_id, Mlog[e_degree]);
                }
                // std::cout<<"check2"<<std::endl; 
            }        
        }

        if(part_node[cur_p].size() >= maxi_cap){   
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
    // std::cout<<"k_1:"<<k_1<<std::endl;
    
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
    // delete []Node;
    // delete []Edge;

}

void parsingCmd(int argc,char *argv[]){
    // std::string input="../data/out.github";
    // n = 56530;
    // m = 120869;

    // std::string input="../data/test.txt";
    // n = 21;
    // m = 21;

    std::string input="../data/BA-1B.txt";
    n = 5e8 + 2;
    m = 5e8 + 2;

    // std::string input="../data/BA-1K.txt";
    // n = 5e2 + 2;
    // m = 5e2 + 2;

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
