/*
 * 编写一个流程控件
 * 流程节点，流程处理，前驱后继
 * 节点进度、节点详情
 *
 * -----------------------------------------------
 * 目前的情况是
 * 已知有n个页面
 * n个页面之间的切换状态
 * 大致如下
 * 配置 n 个 节点， 任意两个节点间 可以配置通路或者不通
 * 从n个节点中 可以选出 m 个 节点， 组成一组循环 , 该 过程 需要 支持能够配置
 * 形成通路的相邻两个节点a 和 b，存在一组操作 OP_ab，使流程 从 a 过渡到 b，在不同的循环或者通路中
 * 操作 OP_ab 可能是固定的，也可能是不固定的，但必须可以配置。
 * 
 */
/*************************************************************************/
#ifndef WORKFLOW_HEADER
#define WORKFLOW_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * OP_ab的函数指针
 */
typedef int (*nextflow)(int preid,int nextid,void* param);

/*
 * 节点结构
 */
typedef struct flow_node {
    int id;
    char name[256];
    char text[256];
    
    nextflow flowing;
    void* param; 
}FlowNode,*pFlowNode;

/*
 * 连通图
 */
typedef struct node_connection {
    int preid;
    int nextid;
    
    int connect;
}NodeConn,*pNodeConn;

/*
 * 有了节点， 有了连通性
 * 可以绘制出完整的流程图
 * node map 应该是什么样子？
 * 为了方便画图，图可以分层，从上到下，每层有几个节点
 * 图还可以添加一些附加信息，比如节点的位置.
 * -----------------------------------------------
 * 
 * 目前的设想：
 * 通过 连通图信息，生成 流程图
 * 流程图的层数，每层节点信息
 */
typedef struct nodemap_level {
    /*
     * 第几层
     */
    int level_idx;
    
    /*
     * 当前层有多少 node
     */
    int count; 
    
    /*
     * 当前层每个node的附加信息，node id , node 所处的列索引
     * --------------------------------------------------------------------
     * 设定为 一个[][2]的数组，
     * 简单处理为是一个 2*count的一维数组， plt+2*i id / plt+(2*i+1) col_idx;
     */
    int* pos;
                 
    /*
     * 当前层每个node的前驱节点id信息
     * --------------------------------------------------------------------
     * preid_list的内容介绍
     * prenode id (如果该节点有多个前驱？所以前驱是个多维数组) ,
     * preid_list+idx 指向一个地址，该地址分配了其前驱节点id 数组的空间
     * 其分配过程大致如下
     * pFlowNode** list=(pFlowNode**)calloc(sizeof(pFlowNode*)*count);
     * for(int idx=0;idx<count;idx++) {
     *     //根据第idx个node的id获取其前驱节点数量
     *     int prenode_count=GetPreNodeCount( pos[idx*2] );
     *     list+idx=(pFlowNode*)calloc(sizeof(pFlowNode),prenode_count);
     *     //逐一赋值
     * 
     * }
     * 
     * 下列代码展示了获取当前层第idx个node的前驱节点信息的记录
     * ////////////////////////////////////////////////////////////////////
     * pFlowNode* preid_item = preid_list+idx;
     * int preid_item_count=_msize(preid_item)/sizeof(pFlowNode);
     * for(int i=0;i<preid_item_count;i++) {
     *     pFlowNode prenode_i=preid_item[i];
     *     // prenode_i 就是 当前层level 下， 第 idx 个节点的 第 i 个 前驱节点
     * }
     *
     */
    pFlowNode* preid_list;
    
}NMLevel,*pNMLevel;

typedef struct node_map {
    int rows;
    int cols;
    
    pNMLevel levels;
    
    int direction;//绘图方向  VERT-0 纵向图 /HORZ-1 横向图
    
    //附加信息
    pFlowNode node_list;
    int node_count;
    
    pNodeConn conn_list;
    int conn_count;
    
    int id_sequence;
    
    void* param;
}NodeMap,*pNodeMap;

pNodeMap initialmap();
#define NODEMAP_initial initialmap
int NODEMAP_addnode(pNodeMap map,char* node_text);
int NODEMAP_removenode(pNodeMap map,int nodeid);
int NODEMAP_connect(pNodeMap map,int prenodeid,int nodeid);
int NODEMAP_disconnect(pNodeMap map,int prenodeid,int nodeid);
int NODEMAP_map(pNodeMap map);
void NODEMAP_map_calc_level(int id,int* id_array,int node_count,pNodeConn conn_list,int conn_count);
void NODEMAP_release(pNodeMap map);

#endif
