#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll n = 1e2;
const ll m = 2;
string outfile = "../BA-1e2.txt";
vector<vector<ll> > Node(n+5);
vector<ll> degree;

std::random_device rd;
std::mt19937_64 engine(rd());  
std::uniform_int_distribution<uint64_t> distribution;

ll rand64(){
    return distribution(engine);
}
int main(){
    std::ios_base::sync_with_stdio(false);
    freopen(outfile.c_str(), "w", stdout);
    degree.push_back(1);
    clock_t timestamp = clock();
    for(ll i=2;i<=n;i++) {
        if(i*100 % n == 0) {
            cerr<<i*100/n<<"% finished"<<endl;
        }
        for(int j = 0; j < m;j++) {
            ll sel = degree[rand64() % degree.size()];
            degree.push_back(sel);
            degree.push_back(i);
            Node[i].push_back(sel);
            Node[sel].push_back(i);
        }
    }
    for(int i = 1; i <= n; i++){
        if(i*100 % n == 0) {
            cerr<<i*100/n<<"% writed"<<endl;
        }
        for(int j = 0; j < Node[i].size(); j++){
            cout<<i<<" "<<Node[i][j]<<'\n';
        }
    }

}
