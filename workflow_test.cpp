#include "workflow.h"

pNodeMap initialmap() {
    pNodeMap map=(pNodeMap)calloc(sizeof(NodeMap),1);
    return map;
}

int NODEMAP_addnode(pNodeMap map,char* node_text) {
    if(!map) return -1;
    
    /*
     * 如果没有 node_list ， 先初始化20个
     */
    if(!map->node_list) {
        map->node_list=(pFlowNode)calloc(sizeof(FlowNode),20);
        if(!map->node_list) return -1;
    }
    int node_mem_size=_msize(map->node_list)/sizeof(FlowNode);
    if(map->node_count>=node_mem_size) {
        /*
         * 扩容
         */
        map->node_list=(pFlowNode)realloc(map->node_list,(node_mem_size+20)*sizeof(FlowNode));
        if(!map->node_list) return -1;
    }
    
    if(map->node_count==0) map->id_sequence=0;
    
    map->node_list[map->node_count].id=(++map->id_sequence);
    strcpy(map->node_list[map->node_count].text,node_text);
    
    map->node_count++;
    return 0;    
}

int NODEMAP_removenode(pNodeMap map,int nodeid) { 
    return -1;
}

int NODEMAP_connect(pNodeMap map,int prenodeid,int nodeid) {
    if(!map) return -1;
    
    if(!map->conn_list) {
        map->conn_list=(pNodeConn)calloc(sizeof(NodeConn),20*20);
    }
    if(!map->conn_list) return -1;
        
    int conn_mem_size=_msize(map->conn_list)/sizeof(NodeConn);
    if(conn_mem_size<=map->conn_count) {
        map->conn_list=(pNodeConn)realloc(map->conn_list,sizeof(NodeConn)*conn_mem_size+20);
        if(!map->conn_list) return -1;
    }
    
    map->conn_list[map->conn_count].preid=prenodeid;
    map->conn_list[map->conn_count].nextid=nodeid;
    map->conn_list[map->conn_count].connect=1;
    
    map->conn_count++;
    
    return 0;
}

int NODEMAP_disconnect(pNodeMap map,int prenodeid,int nodeid) {
    return -1;
}

/*
 * 功能： 生成最新的map信息
 * 
 * pNodeMap 根据存储的node 和 connection 信息
 * 生成一张完整的 map
 * 注意map可能随时生成，一旦有新的节点或者连接加入/删除，map 可能就会发生改变
 * ----------------------------------------------------------------------------
 * 问题1. 如果连通的两个节点，位于相邻的两层，比较简单，但是如果连通的两个节点跨越了层，算法该如何计算行列？
 * 如下图， E到底应该放在什么位置？
 *  A                     A            
 *  +-------+             +-------+    
 *  |       |             |       |    
 *  B---+   |             B---+   E(2)    
 *  |   |   |             |   |   |    
 *  |   |   |             |   |   |    
 *  C   D   E(1)          C   D   |   
 *  |   |   |             |   |   |    
 *  |   |   |             |   |   |    
 *  +---+---+             +---+---+    
 *  |                     |            
 *  |                     |            
 *  F                     F            
 */
/*
 * 思路1：
 * 1.找出所有没有前驱的节点，定义为第一层
 * 2.从剩余节点中查找遍历每一个节点，如果前驱为第一层的节点，该节点的层级定义为 上层层级+1，
 *   在完成上一层所有节点的遍历后，以层级最大值定义为该节点的层级
 * 3.循环，直到所有节点都定义层级
 * 
 */
int qs_cmp_nodeid(const void* a,const void* b) { int id_a=*((int*)a); int id_b=*((int*)b); return id_a-id_b; }
int qs_cmp_nodelv(const void* a,const void* b) { int id_a=*((int*)a+1); int id_b=*((int*)b+1); return id_a-id_b; }
int bs_cmp_nodeid(const void* a,const void* b) { return (*(int*)a)-(*(int*)b); }
int NODEMAP_map(pNodeMap map) {
    if(!map||!map->node_list||!map->conn_list) return -1;
    
    //1.提取所有的节点ID
    int* id_array=(int*)calloc(sizeof(int)*map->node_count*2,1);
    if(!id_array) return -1;
        
    for(int idx=0;idx<map->node_count;idx++) id_array[idx*2]=map->node_list[idx].id;
    //排序一遍查找
    qsort(id_array,map->node_count,sizeof(int)*2,qs_cmp_nodeid);
    
    //2.确定每个节点的层级
    for(int idx=0;idx<map->node_count;idx++) {
        NODEMAP_map_calc_level(id_array[idx*2],id_array,map->node_count,map->conn_list,map->conn_count);
    }
    
    //3.确定每个层级的列序
    //重新对id_array排序，按照 level
    qsort(id_array,map->node_count,sizeof(int)*2,qs_cmp_nodelv);
    
    if(map->levels) {
        for(int idx=0;idx<map->rows;idx++) {
            pNMLevel pL=map->levels+idx;
            if(pL->pos) free(pL->pos);
        }
        
        //删除重新分配
        free(map->levels);
        map->levels=NULL;
    }
    //最大层级
    int maxlevel=id_array[(map->node_count-1)*2+1];
    /*
     * 注意：如果没有零层，map->rows=maxlevel;
     * 否则 map->rows=maxlevel+1;
     */
    //map->rows=maxlevel+1;//没连通的节点视为第0层
    if (id_array[1] > 0) {
        map->rows = maxlevel;
    }
    else map->rows=maxlevel+1;
    
    map->levels=(pNMLevel)calloc(sizeof(NMLevel)*(map->rows+1),1);
    if(!map->levels) return -1; //有点问题
    
    int i_cols=0;
    for(int idx=0;idx<=map->rows;idx++) {
        
        map->levels[idx].level_idx=idx;
        int i_begin=i_cols;
        for(;i_cols<map->node_count;i_cols++) {
            if(id_array[i_cols*2+1]!=map->levels[idx].level_idx) break;
        }
        
        map->levels[idx].count=i_cols-i_begin;
        map->levels[idx].pos=(int*)calloc(sizeof(int)*2,map->levels[idx].count);
        
        memcpy(map->levels[idx].pos,id_array+i_begin*2,map->levels[idx].count*2*sizeof(int));
        for(int col=0;col<map->levels[idx].count;col++) {
            map->levels[idx].pos[col*2+1]=col;
        }
        if(map->cols<map->levels[idx].count) map->cols=map->levels[idx].count;
    }
    
    if(id_array) free(id_array);
    return 0;
}

/*
 * 反向更新 node level
 */
void NODEMAP_map_reverse_level(int id, int level,int* id_array, int node_count, pNodeConn conn_list, int conn_count) {
    for(int idx=0;idx<conn_count;idx++) {
        if(conn_list[idx].connect==1&&id==conn_list[idx].nextid) {
            int* preid_addr=(int*)bsearch(&conn_list[idx].preid, id_array, node_count, sizeof(int) * 2, bs_cmp_nodeid);
            if(!preid_addr) return;
            int preid_level=*(preid_addr+1);
            
            if(preid_level<level-1) {
                *(preid_addr+1)=level-1;
                NODEMAP_map_reverse_level(*preid_addr, *(preid_addr + 1), id_array, node_count, conn_list, conn_count);
            }
        }
    }
}

void NODEMAP_map_calc_level(int id,int* id_array,int node_count,pNodeConn conn_list,int conn_count) {
    for(int idx=0;idx<conn_count;idx++) {
        if(conn_list[idx].connect==1&&id==conn_list[idx].preid) {
            int* preid_addr=(int*)bsearch(&conn_list[idx].preid,id_array,node_count,sizeof(int)*2,bs_cmp_nodeid);
            if(!preid_addr) return;
            if(*(preid_addr+1)==0) *(preid_addr+1)=1;//第一层   
            
            int* nextid_addr=(int*)bsearch(&conn_list[idx].nextid,id_array,node_count,sizeof(int)*2,bs_cmp_nodeid);
            if(!nextid_addr) return;
            //if(*(nextid_addr+1)==0) {
            //if(*(nextid_addr+1)<)
                int pre_level=*(preid_addr+1);
                int next_level=*(nextid_addr+1);
                
                if(pre_level+1>next_level) {
                    *(nextid_addr+1)=pre_level+1;
                    /*
                     * 需要倒回去更改其所有前驱
                     * 如果前驱的level<当前节点的level-1 ， 需要递归更新 前驱节点的 level = 当前节点的 level-1;
                     * 否则 不处理
                     */
                    NODEMAP_map_reverse_level(*nextid_addr,*(nextid_addr+1),id_array,node_count,conn_list,conn_count);
                }
            //}
            
            NODEMAP_map_calc_level(conn_list[idx].nextid,id_array,node_count,conn_list,conn_count);
        }
    }
}

void NODEMAP_release(pNodeMap map) {
    if(!map) return;
    
    if(map->levels) {
        for(int idx=0;idx<map->rows;idx++) {
            pNMLevel pL=map->levels+idx;
            if(pL->pos) free(pL->pos);
        }
        
        free(map->levels);
        map->levels=NULL;
    }
    
    if(map->node_list) {
        free(map->node_list);
        map->node_list=NULL;
    }
    
    if(map->conn_list) {
        free(map->conn_list);
        map->conn_list=NULL;
    }
    
    free(map);
}