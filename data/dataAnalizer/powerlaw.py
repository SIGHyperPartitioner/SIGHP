import matplotlib.pyplot as plt
filepath= "../BA-1M.txt"
Edge = {}
Node = {}

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

fig, axes = plt.subplots(1, 2, figsize=(8, 2.5))

color = (50/255,205/255,50/255)
maxFrequence = 0
vertFreq = []
for i in range(n):
    maxFrequence = max(maxFrequence,len(Node[i]))
    while maxFrequence >= len(vertFreq):
        vertFreq.append(0)
    vertFreq[len(Node[i])] += 1

axes[0].scatter([i for i in range(len(vertFreq))],vertFreq,s = 5, color=color, label = 'Vertex degree')
axes[0].set_xscale("log")
axes[0].set_yscale("log")
axes[0].grid(True)
axes[0].set_xlim(1)
axes[0].set_ylabel("frequency")
axes[0].set_xlabel("Vertices Degree |vi|")
axes[0].tick_params(direction='in',which="both")
axes[0].legend()

maxFrequence = 0
edgeFreq = []
for i in range(n):
    maxFrequence = max(maxFrequence,len(Edge[i]))
    while maxFrequence >= len(edgeFreq):
        edgeFreq.append(0)
    edgeFreq[len(Edge[i])] += 1

print(edgeFreq)
axes[1].scatter([i for i in range(len(edgeFreq))],edgeFreq,s = 5, color=color, label = 'Vertex degree')
axes[1].set_xscale("log")
axes[1].set_yscale("log")
axes[1].grid(True)
axes[1].set_xlim(1)
axes[1].set_ylabel("frequency")
axes[1].set_xlabel("Edges Degree |vi|")
axes[1].tick_params(direction='in',which="both")
axes[1].legend()
    
plt.savefig('./pic/distribution.pdf')
plt.show()