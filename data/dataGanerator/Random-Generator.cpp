#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ll;
const ll n = 1e6;
const ll m = 5e6;
string outfile = "../random-5M.txt";
vector<vector<ll> > Node(n+1);

std::random_device rd;
std::mt19937_64 engine(rd());  // 使用随机设备作为种子
std::uniform_int_distribution<uint64_t> distribution;

ll rand64(){
    ll ret = distribution(engine);
    if(ret<0) ret = -ret;
    return distribution(engine);
}
int main(){
    std::ios_base::sync_with_stdio(false);
    freopen(outfile.c_str(), "w", stdout);   

    for(int i=0;i<m;i++){
        ll vid = (rand64() % n) + 1;
        ll eid = (rand64() % n) + 1;
        assert(vid > 0);
        assert(eid > 0);
        
        cout<<vid<<" "<<eid<<'\n';
    } 

}