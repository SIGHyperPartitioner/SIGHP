import numpy as np
from sklearn.decomposition import PCA
import matplotlib.pyplot as plt
import numpy as np
from sklearn.manifold import TSNE
import matplotlib.pyplot as plt
import random
from matplotlib import rcParams
from sklearn.metrics import pairwise_distances
from sklearn.metrics import silhouette_score,silhouette_samples

# rcParams['font.family'] = 'serif'
rcParams['font.serif'] = 'Times New Roman'
# rcParams['font.size'] = 18
# rcParams['text.usetex'] = True

# rc_fonts = {
#     # "font.family": "serif",
#     "font.family": "Times New Roman",
#     "font.size": 20,
#     'figure.figsize': (5, 3),
# }

methods = ["HYPE","SIGHP","MinMax","BiPart","Random","KaHyPar","Random"]
methods = ["HYPE","SIGHP"]
p = 4
colorSet = ["r","b","g","y"]

def loadData(vertexScheme,EdgeInfo):
    v2p = {}
    with open(vertexScheme, "r") as file:
        for line in file:
            vid,pid = line.split()
            vid = int(vid)
            pid = int(pid)
            v2p[vid] = pid

    p2e = {}
    for i in range(p):
        p2e[i] = set()
    v2e = {}
    e2v = {}
    with open(EdgeInfo, "r") as file:
        for line in file:
            if line[0] >'9' or line[0] < '0':
                continue
            vid,eid = line.split()
            vid = int(vid)
            eid = int(eid)
            if vid not in v2e: 
                v2e[vid] = []
            if eid not in e2v:
                e2v[eid] = []
            v2e[vid].append(eid)
            p2e[v2p[vid]].add(eid)
            e2v[eid].append(vid)
    return v2p,p2e,v2e,e2v

def calRelaventV(v2p, v2e, p2e, lst):
    data = []
    for vid in lst:
        data.append([])
        cnt = 0
        for pid in range(p):
            data[-1].append(0)
            if vid not in v2e :
                continue
            for eid in v2e[vid]:
                if eid in p2e[pid]:
                    data[-1][-1] += 1
                    cnt += 1
        if cnt == 0:
            continue
        for pid in range(p):
            data[-1][pid] = data[-1][pid]/len(v2e[vid])
    colors = [colorSet[v2p[i]] for i in lst]
    return data, colors

def calRelaventE(v2p, v2e, e2v, lst):
    nodes = lst
    features = []
    colors = []
    for vid in nodes:
        features.append([0 for i in range(len(e2v))])
        for eid in v2e[vid]:
            features[-1][eid] = 1
        colors.append(colorSet[v2p[vid]])

    cnt={"r":0,"b":0,"g":0,"y":0}
    for i in colors:
        cnt[i] += 1
    print("color:",cnt)

    return nodes, features, colors

def sample(v2e,e2v,num):
    nodes = set()
    nodes.add(random.choices(list(v2e.keys()))[0])
    cnt = 0
    while num != len(nodes):
        vid = random.choices(list(nodes))[0]
        # print(vid)
        eid = random.choices(v2e[vid])[0]
        # print(e2v[eid])
        addv = random.choices(e2v[eid])[0]
        if addv in nodes:
            if cnt < 10:
                cnt += 1
                continue
            else :
                nodes.add(random.choices(list(v2e.keys()))[0])
                cnt = 0
        else :
            cnt = 0
            nodes.add(addv)
    return nodes


v2p,p2e,v2e,e2v = loadData("./testdata/github2/SIGHP","./testdata/github2/edge_info")
# Elst = list(set(v2e.keys())) 
# Elst = sample(v2e,e2v,2000) # sampler
newV = []
for vid,edges in v2e.items():
    if(len(edges)>5): 
        continue
    newV.append(vid)
print("newV:",len(newV))
Vlst = random.sample(newV, k = 1000)
Elst = list(Vlst)
# cnt = 0
# for vid in Vlst:
#     cnt += 1
#     eid = random.choices(v2e[vid])[0]
#     addv = random.choices(e2v[eid])[0]
#     if addv not in Elst:
#         Elst.append(addv)
# print(cnt)
# print("node num:",len(Elst))

for Method in methods:
    plt.clf()
    vertexScheme = "./testdata/github2/"+Method
    EdgeInfo = "./testdata/github2/edge_info"

    v2p,p2e,_,_ = loadData(vertexScheme,EdgeInfo)
    print("p2e", [len(i) for i in p2e.values()])
    lst,data,colors = calRelaventE(v2p, v2e, e2v, Elst)

    # lst = list(set(v2p.keys())) 
    # lst = random.choices(lst, k=5000) # sampler
    # data,colors = calRelaventV(v2p, v2e, p2e, lst)

    # print(len(v2p.keys()))
    tag = ["Partition 1", "Partition 2", "Partition 3", "Partition 4"]

    np.random.seed(42)
    print(len(data))
    data = np.array(data)
    print(data)

    # for vid in lst:
    tsne = TSNE(n_components=2, random_state=42, n_iter=500,init='pca',perplexity=50,learning_rate=200)
    # ,perplexity=50,learning_rate=200
    data_2d = tsne.fit_transform(data)

    # fig, axes = plt.subplots(1, 1, figsize=(10, 4))
    plt.scatter(data_2d[:, 0], data_2d[:, 1] ,c = colors, marker='o', alpha=0.5, s=35, edgecolors='none')
    for i in range(4):
        plt.scatter([], [] ,c = colorSet[i], marker='o', alpha=0.5, s=75, edgecolors='none', label = tag[i])
    plt.title(Method,fontsize = 18)

    plt.tick_params(axis='both', right = True, top = True,direction='in')
    deltax = (max(data_2d[:,0]) - min(data_2d[:,0]))/7
    deltay = (max(data_2d[:,1]) - min(data_2d[:,1]))/7
    x = [min(data_2d[:, 0]) + i * deltax for i in range(8)]
    y = [min(data_2d[:, 1]) + i * deltay for i in range(8)]
    plt.xticks(ticks=x,labels = [])
    plt.yticks(ticks=y,labels = [])

    silhouette_avg = silhouette_score(data_2d, [v2p[i] for i in lst])
    print(silhouette_avg)
    plt.legend(loc='upper left')


    plt.grid(True)

    silhouette_avg = silhouette_score(data_2d, [v2p[i] for i in lst])
    print(silhouette_avg)

    plt.text(1, -0.1, 'Silhouette Coefficient:'+str(round(silhouette_avg,3)), fontsize=15, ha='right', va='bottom', transform=plt.gca().transAxes)

    plt.savefig("./pic/"+ Method +".pdf")
    plt.show()
