# import matplotlib as plt
# plt.get_cachedir()

import matplotlib.pyplot as plt
import numpy as np
import random
import sys
import subprocess
import numpy as np
from matplotlib import rcParams
from matplotlib.ticker import ScalarFormatter
from matplotlib.pyplot import MultipleLocator

def save_subfig(fig,ax,save_path,fig_name):
    bbox = ax.get_tightbbox(fig.canvas.get_renderer()).expanded(1.02, 1.02)
    extent = bbox.transformed(fig.dpi_scale_trans.inverted())
    fig.savefig(save_path+fig_name, bbox_inches=extent)

rc_fonts = {
    # "font.family": "serif",
    "font.family": "Times New Roman",
    "font.size": 20,
    'figure.figsize': (5, 3),
}
rcParams.update(rc_fonts)

# plt.rc('text', usetex=True)
# plt.rc('font', family='serif')
# plt.rcParams['text.latex.preamble'] = r'''
# \usepackage{amsmath}    % 为数学表达式提供额外的功能
# \usepackage{amssymb}     % 提供\mathcal等命令
# '''

# rcParams['font.serif'] = 'Times New Roman'


# filepath= "../out.github"
# filepath= "../BA-1e4.txt"
# filepath= "../../../hypergraph/data/enwiki.txt"
# filepath= "../../../hypergraph/data/out.trackers-swap.txt"
# filepath= "../../../hypergraph/data/trec-swap.txt"
Edge = {}
Node = {}
fig, axes = plt.subplots(1, 2, figsize=(12, 5))
checkPoint = [0.01, 0.05, 0.10, 0.15, 0.20]
checkPoint2 = [2, 10]

def progress_bar(iteration, total, prefix='', suffix='', length=50, fill='█'):
    percent = ("{0:.1f}").format(100 * (iteration / float(total)))
    filled_length = int(length * iteration // total)
    bar = fill * filled_length + '-' * (length - filled_length)
    sys.stdout.write('\r%s |%s| %s%% %s' % (prefix, bar, percent, suffix))
    sys.stdout.flush()

def draw2():
    fig, axes = plt.subplots(1, 4, figsize=(20, 5))
    axes[0].set_title("GH", fontsize = 36)
    axes[1].set_title("GH", fontsize = 36)
    axes[2].set_title("EW", fontsize = 36)
    axes[3].set_title("EW", fontsize = 36)

    filepath1 = "../out.github"
    # filepath2 = "../../../hypergraph/data/enwiki.txt"

    # filepath1 = "../BA-1e3.txt"
    filepath2 = "../BA-1e3.txt"

    n = 0
    m = 0
    result = subprocess.run("wc -l " + filepath1, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    totalLine = int(result.stdout.decode('utf-8').strip().split()[0])
    with open(filepath1,"r") as file:
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
    # print("test:",len(Node))
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

    data = sorted(correlation)
    data2 = sorted(correlation2)

    n = 0
    m = 0
    # Edge = {}
    # Node = {}
    result = subprocess.run("wc -l " + filepath2, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    totalLine = int(result.stdout.decode('utf-8').strip().split()[0])
    with open(filepath2,"r") as file:
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
    # print("test:",len(Node))
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

    data3 = sorted(correlation)
    data4 = sorted(correlation2)


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
    axes[0].plot(vec, value, label = "GH")
    axes[0].set_xlabel("Jaccard Similarity", fontsize = 30)
    axes[0].set_ylabel("Cumulative Prob.%", fontsize = 30)
    axes[0].yaxis.grid(True, linestyle='--', linewidth=0.5, alpha=0.7)
    axes[0].tick_params(axis='both', right = True, top = True,direction='in')
    # axes[0].yaxis.set_major_locator(MultipleLocator(0.02))
    axes[0].yaxis.get_major_formatter().set_scientific(False)
    axes[0].ticklabel_format(useOffset=False, style='plain')


    # y_formatter = ScalarFormatter(useMathText=False)

    pos = 0
    print("JS:")
    for i in checkPoint:
        # print("test:",i)
        for j in range(pos,len(data)):
            pos += 1
            if data[j] > i :
                print(i,":",pos/len(data) * 100)
                break
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
    axes[1].plot(vec, value,label = "GH")
    axes[1].set_xlabel("P(AB) / (P(A)P(B))",fontsize = 30)
    # axes[1].set_ylabel("Cumulative Prob %")
    axes[1].yaxis.grid(True, linestyle='--', linewidth=0.5, alpha=0.7)
    axes[1].tick_params(axis='both', right = True, top = True,direction='in')
    # axes[1].yaxis.set_major_locator(MultipleLocator(0.5))


    pos = 0
    print("PAB:")
    for i in checkPoint2:
        # print("test:",i)
        for j in range(pos,len(data2)):
            pos += 1
            if data2[j] > i :
                print(i,":",pos/len(data2) * 100)
                break

    axes[1].set_xscale("log")


    mp = {}
    vec = []
    for i in data3:
        if i not in mp:
            mp[i] = 0
            vec.append(i)
        mp[i] += 1
    value = [mp[i]/len(data3) * 100 for i in vec]
    for i in range(1, len(value)):
        value[i] += value[i - 1]
        print("len a: {} len b:{}".format(len(vec),len(value)))
    axes[2].plot(vec, value, label = "EW")
    axes[2].set_xlabel("Jaccard Similarity", fontsize = 30)
    # axes[2].set_ylabel("Cumulative Prob %")
    axes[2].yaxis.grid(True, linestyle='--', linewidth=0.5, alpha=0.7)
    axes[2].tick_params(axis='both', right = True, top = True,direction='in')
    axes[2].yaxis.set_major_locator(MultipleLocator(0.02))
    axes[2].yaxis.get_major_formatter().set_scientific(False)
    axes[2].ticklabel_format(useOffset=False, style='plain')


    # y_formatter = ScalarFormatter(useMathText=False)

    pos = 0
    print("JS:")
    for i in checkPoint:
        # print("test:",i)
        for j in range(pos,len(data3)):
            pos += 1
            if data3[j] > i :
                print(i,":",pos/len(data3) * 100)
                break
    # print(data)


    mp = {}
    vec = []
    for i in data4:
        if i not in mp:
            mp[i] = 0
            vec.append(i)
        mp[i] += 1
    value = [mp[i]/len(data4) * 100 for i in vec]
    for i in range(1, len(value)):
        value[i] += value[i - 1]
    axes[3].plot(vec, value,label = "EW")
    axes[3].set_xlabel("P(AB) / (P(A)P(B))",fontsize = 30)
    # axes[1].set_ylabel("Cumulative Prob %")
    axes[3].yaxis.grid(True, linestyle='--', linewidth=0.5, alpha=0.7)
    axes[3].tick_params(axis='both', right = True, top = True,direction='in')
    axes[3].yaxis.set_major_locator(MultipleLocator(0.5))


    pos = 0
    print("PAB:")
    for i in checkPoint2:
        # print("test:",i)
        for j in range(pos,len(data4)):
            pos += 1
            if data4[j] > i :
                print(i,":",pos/len(data4) * 100)
                break

    axes[3].set_xscale("log")

    # axes[0].set_yscale("log")
    # axes[1].set_yscale("log")

    plt.tight_layout()
    # print('./pic/'+ filepath.split('/')[-1] +'-corre.pdf')
    # plt.show()
    plt.savefig('./pic/test-corre-overall.pdf')

    save_subfig(fig, axes[0], "./pic/","test-corre-overall1.pdf")
    save_subfig(fig, axes[1], "./pic/","test-corre-overall2.pdf")
    save_subfig(fig, axes[2], "./pic/","test-corre-overall3.pdf")
    save_subfig(fig, axes[3], "./pic/","test-corre-overall4.pdf")

draw2()

def draw(data, data2, label = ""):
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
    axes[0].plot(vec, value, label = label)
    axes[0].set_xlabel("Jaccard Similarity")
    axes[0].set_ylabel("Cumulative Prob.%")
    axes[0].yaxis.grid(True, linestyle='--', linewidth=0.5, alpha=0.7)
    axes[0].tick_params(axis='both', right = True, top = True,direction='in')
    # axes[0].yaxis.set_major_locator(MultipleLocator(0.02))
    axes[0].yaxis.get_major_formatter().set_scientific(False)
    axes[0].ticklabel_format(useOffset=False, style='plain')


    # y_formatter = ScalarFormatter(useMathText=False)

    pos = 0
    print("JS:")
    for i in checkPoint:
        # print("test:",i)
        for j in range(pos,len(data)):
            pos += 1
            if data[j] > i :
                print(i,":",pos/len(data) * 100)
                break
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
    axes[1].plot(vec, value,label = label)
    axes[1].set_xlabel("P(AB) / (P(A)P(B))", fontsize = 24)
    axes[1].set_ylabel("Cumulative Prob.%")
    axes[1].yaxis.grid(True, linestyle='--', linewidth=0.5, alpha=0.7)
    axes[1].tick_params(axis='both', right = True, top = True,direction='in')
    # axes[1].yaxis.set_major_locator(MultipleLocator(0.5))


    pos = 0
    print("PAB:")
    for i in checkPoint2:
        # print("test:",i)
        for j in range(pos,len(data2)):
            pos += 1
            if data2[j] > i :
                print(i,":",pos/len(data2) * 100)
                break

    axes[1].set_xscale("log")
    # axes[0].set_yscale("log")
    # axes[1].set_yscale("log")

    plt.tight_layout()
    # print('./pic/'+ filepath.split('/')[-1] +'-corre.pdf')
    # plt.show()


def drawScale():
    filepaths = [
        # "../BA-1e2.txt",\
        # "../BA-1e3.txt", \
        "../BA-1e4.txt",\
        "../BA-5e4.txt", \
        "../BA-1e5.txt", \
        "../BA-5e5.txt", \
        "../BA-1e6.txt",\
        # "../out.github",\
        # "../../../hypergraph/data/enwiki.txt"
        ]
    # filepaths = ["../"]
    labels = ["|$\mathcal{V}$| = $10^4$","|$\mathcal{V}$| = $5x10^4$", "|$\mathcal{V}$| = $10^5$", "|$\mathcal{V}$| = $5x10^5$", "|$\mathcal{V}$| = $10^6$"]
    cur = -1
    for filepath in filepaths:
        cur += 1
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
        # print("test:",len(Node))
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
        # draw(correlation,correlation2,filepath[3:-4])
        draw(correlation,correlation2,labels[cur])

    lines, labels = fig.axes[0].get_legend_handles_labels()
    print(lines, labels)
    fig.legend( lines, labels, ncol=3, framealpha=1, loc='upper center',  bbox_to_anchor=(0.5, 1.03), fontsize=18, frameon=False, handlelength=2.0, handletextpad=1)
    plt.subplots_adjust(top=0.82)
    print("filename:{}".format('./pic/'+ filepath.split('/')[-1] +'-corre-overall.pdf'))
    plt.savefig('./pic/'+ filepath.split('/')[-1] +'-corre-overall.pdf')

# drawScale()