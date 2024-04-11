import matplotlib.pyplot as plt
import numpy as np
filepath= "../out.github"
Edge = {}
Node = {}

def draw(data):
    plt.figure(figsize=(9, 6))  
    kwargs = {
        "bins": 1000,
        "histtype": "stepfilled",
        "alpha": 1,
        "density": None
    }
    mp = {}
    vec = []
    for i in data:
        if i not in mp:
            mp[i] = 0
            vec.append(i)
        mp[i] += 1
    value = [mp[i]/len(data) * 100 for i in vec]
    for i in range(1, len(value)):
        value[i] += value[i - 1]
    plt.plot(vec, value)

    print(data)
    # hist_values, bin_edges, _  = plt.hist(data, **kwargs)
    plt.xlim((0, max(data)))
    
    # plt.twinx().set_yscale("log")
    plt.yscale("log")
    plt.tick_params(axis='both', right = True, top = True,direction='in')
    # plt.xscale("log")
    plt.tight_layout()
    plt.xlabel("Jaccard Similarity")
    plt.savefig('./pic/corre.pdf')
    # plt.show()

n = 0
m = 0
with open(filepath,"r") as file:
    # load data 
    for line in file:
        vid,eid = line[:-1].split()
        vid = int(vid)
        eid = int(eid)
        while vid >= n:
            Node[n] = []
            n += 1
        while eid >= m:
            Edge[m] = []
            m += 1

        Node[vid].append(eid)
        Edge[eid].append(vid)

correlation = []
for i in range(10):
    if len(Edge[i]) == 0: continue
    for j in range(i+1, len(Edge)):
        if i == j: continue
        if(len(Edge[j]) <= 5) : continue
        intersection = list(set(Edge[i]) & set(Edge[j]))
        union = list((set(Edge[i]) | set(Edge[j])))
        # correlation.append(len(intersection) * len(Node) / len(Edge[i]) / len(Edge[j]))
        correlation.append(len(intersection)/len(union))

# print(correlation)

correlation = sorted(correlation)

draw(correlation)