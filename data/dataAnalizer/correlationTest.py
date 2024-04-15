import matplotlib.pyplot as plt
import numpy as np
import random
import sys
import subprocess
import numpy as np
from matplotlib import rcParams
rc_fonts = {
    "font.family": "serif",
    'font.serif': 'Linux Libertine O',
    "font.size": 20,
    'figure.figsize': (5, 3),
}
rcParams.update(rc_fonts)


# filepath= "../out.github"
# filepath= "../BA-1e4.txt"
# filepath= "../../../hypergraph/data/enwiki.txt"
# filepath= "../../../hypergraph/data/out.trackers-swap.txt"
# filepath= "../../../hypergraph/data/trec-swap.txt"
Edge = {}
Node = {}
fig, axes = plt.subplots(1, 2, figsize=(9, 4))

def draw(data, data2):
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
    axes[0].plot(vec, value)
    axes[0].set_xlabel("Jaccard Similarity")
    axes[0].set_ylabel("Percentage Vertices %")
    axes[0].yaxis.grid(True, linestyle='--', linewidth=0.5, alpha=0.7)
    axes[0].tick_params(axis='both', right = True, top = True,direction='in')
    # print(data)


    mp = {}
    vec = []
    for i in data2:
        if i not in mp:
            mp[i] = 0
            vec.append(i)
        mp[i] += 1
    value = [mp[i]/len(data2) * 100 for i in vec]
    for i in range(1, len(value)):
        value[i] += value[i - 1]
    axes[1].plot(vec, value)
    axes[1].set_xlabel("P(AB)/P(A)P(B)")
    axes[1].set_ylabel("Percentage Vertices %")
    axes[1].yaxis.grid(True, linestyle='--', linewidth=0.5, alpha=0.7)
    axes[1].tick_params(axis='both', right = True, top = True,direction='in')

    axes[1].set_xscale("log")
    # axes[0].set_yscale("log")
    # axes[1].set_yscale("log")

    plt.tight_layout()
    # print('./pic/'+ filepath.split('/')[-1] +'-corre.pdf')
    # plt.show()


# def drawScale():
    

def progress_bar(iteration, total, prefix='', suffix='', length=50, fill='█'):
    percent = ("{0:.1f}").format(100 * (iteration / float(total)))
    filled_length = int(length * iteration // total)
    bar = fill * filled_length + '-' * (length - filled_length)
    sys.stdout.write('\r%s |%s| %s%% %s' % (prefix, bar, percent, suffix))
    sys.stdout.flush()

filepaths = ["../BA-1e4.txt", "../BA-5e4.txt", "../BA-1e5.txt", "../BA-5e4.txt", "../BA-1e6.txt"]
for filepath in filepaths:
# filepath= "../BA-1e4.txt"
    n = 0
    m = 0
    result = subprocess.run("wc -l " + filepath, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    totalLine = int(result.stdout.decode('utf-8').strip().split()[0])
    with open(filepath,"r") as file:
        # load data 
        cnt = 0
        for line in file:
            cnt += 1
            if cnt % 10000 == 0:
                progress_bar(cnt, totalLine, prefix='Progress:', suffix='Complete', length=50)
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
    print(" load data ok")
    print("test:",len(Node))
    correlation = []
    sampleNum = 1000000
    samplerList = [i for i in Node.keys()]
    for i in range(sampleNum):
        a,b = random.sample(samplerList,2)
        intersection = list(set(Node[a]) & set(Node[b]))
        union = list((set(Node[a]) | set(Node[b])))
        if len(union) < 2: continue
        correlation.append(len(intersection)/len(union))



    correlation2 = []
    samplerList = [i for i in Edge.keys()]
    for i in range(sampleNum):
        a,b = random.sample(samplerList,2)
        intersection = list(set(Edge[a]) & set(Edge[b]))
        # union = list((set(Node[a]) | set(Node[b])))
        if len(Edge[a])*len(Edge[b]) == 0: continue
        correlation2.append(len(intersection) * len(Node)/len(Edge[a])/len(Edge[b]))

    correlation = sorted(correlation)
    correlation2 = sorted(correlation2)
    draw(correlation,correlation2)

plt.savefig('./pic/'+ filepath.split('/')[-1] +'-corre-overall.pdf')