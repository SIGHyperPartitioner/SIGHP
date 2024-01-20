import networkx as nx
import matplotlib.pyplot as plt

def log(content):
    import time
    import pytz
    import datetime

    timestamp = datetime.datetime.now()
    desired_timezone = pytz.timezone('Asia/Shanghai')
    localized_time = desired_timezone.localize(timestamp)
    formatted_time = localized_time.strftime("%Y-%m-%d %H:%M:%S\t")

    print(formatted_time + ": " + content)


# 生成 BA 图
n = int(1e9)  # 节点数
m = int(1e9)    # 总边数
outFile = open("../random-1B.txt","w") 

log("generate graph")
random_graph = nx.gnm_random_graph(n, m)
log("finished")
# degree_sequence = sorted([d for n, d in ba_graph.degree()], reverse=True)
# degree_count = nx.degree_histogram(ba_graph)

for node in random_graph.nodes():
    for nid in list(random_graph.neighbors(node)):
        outFile.write(str(node)+" "+str(nid)+"\n")
    
outFile.close()

# 可视化 BA 图
# pos = nx.spring_layout(ba_graph)
# nx.draw(ba_graph, pos, with_labels=True, node_size=10, node_color='skyblue', edge_color='gray')
# plt.title("Barabási-Albert (BA) Graph")
# plt.show()
